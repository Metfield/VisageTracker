#include <_VisageTracker.h>
#include <GLES2/gl2.h>

#include <NativeTrackerRenderer.h>

#include <Logging.h>
#include <string>

// glm includes
// translate, rotate, scale, perspective
#include <gtc/matrix_transform.hpp>

// value_ptr
#include <gtc/type_ptr.hpp>

// PNGLoader
#include <PNGLoader.h>

// Pre-calculated value of PI / 180.
#define kPI180     0.017453

// Pre-calculated value of 180 / PI.
#define k180PI    57.295780

// Converts degrees to radians.
#define degreesToRadians(x) (x * kPI180)

// Converts radians to degrees.
#define radiansToDegrees(x) (x * k180PI)

#define MESH_EYES 1
#define MESH_SHIRT 5

#define JEANS_TEXTURE 6
#define SHOES_TEXTURE 7

// Extern JNI bullcrap
extern "C"
{
	// onSurfaceCreated
	JNIEXPORT void JNICALL Java_com_visage_visagetracker_TrackerRenderer_nativeOnSurfaceCreated(JNIEnv *env, jclass cls, jint width, jint height)
	{
		NativeTrackerRenderer::getInstance().onSurfaceCreated((int) width, (int) height);
	}

	// onSurfaceChanged
	JNIEXPORT void JNICALL Java_com_visage_visagetracker_TrackerRenderer_nativeOnSurfaceChanged(JNIEnv *env, jclass cls, jint width, jint height)
	{
		NativeTrackerRenderer::getInstance().onSurfaceChanged(width, height);
	}

	// onDrawFrame
	JNIEXPORT void JNICALL Java_com_visage_visagetracker_TrackerRenderer_nativeOnDrawFrame(JNIEnv *env, jclass cls)
	{
		NativeTrackerRenderer::getInstance().onDrawFrame();
	}

	JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_nativeTouches(JNIEnv *env, jclass cls, jfloat value, jboolean released)
	{
		NativeTrackerRenderer::getInstance().auxValue = value;
		NativeTrackerRenderer::getInstance().touchReleased = (bool)released;
	}
}

// Different buffers used to hold rendering info
GLuint 	positionBuffer,
		indexBuffer,
		textureBuffer,
		normalBuffer;

// The shaderProgram combines a vertex shader (vertexShader) and a
// fragment shader (fragmentShader) into a single GLSL program that can
// be activated (glUseProgram()).
GLuint shaderProgram;

std::string GetShaderInfoLog(GLuint obj);
void fatal_error(std::string err );
void createShaders();
inline void setUniformColor(glm::vec3 color);

 GLuint elementbuffer;
using namespace glm;

void NativeTrackerRenderer::onSurfaceCreated(int w, int h)
{
	this->width = w;
	this->height = h;

	glClearColor(0.1f, 0.4f, 0.6f, 1.0f);
	glEnable(GL_DEPTH_TEST);	// enable Z-buffering

	// Generate VBO
	glGenBuffers(1, &positionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);

	// Set Vertex position attribute and enable it
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0 );
	glEnableVertexAttribArray(0);

	// Generate normals buffer
	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);

	// Set normals attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(1);

	// Generate texture coordinates buffer
	glGenBuffers(1, &textureBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);

	// Set texture coordinates attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(2);

	// Load textures
	loadTextures();

	// Generate IBO
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	// Load, compile and generate shaders
	createShaders();
}

void NativeTrackerRenderer::onSurfaceChanged(int w, int h)
{
	this->width = w;
	this->height = h;
}

void NativeTrackerRenderer::onDrawFrame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clears the color buffer and the z-buffer

	glUseProgram( shaderProgram );

	// Get translation/rotation info
	const VisageSDK::FaceData *faceData = this->mLoader->getFaceData();

	// Get tracker transformation info
	vec3 faceTranslation(faceData->faceTranslationCompensated[0], faceData->faceTranslationCompensated[1], -faceData->faceTranslationCompensated[2]+auxValue);
	vec3 faceRotation(faceData->faceRotation[0], faceData->faceRotation[1], faceData->faceRotation[2]);

	// Set Matrices
	vec3 Translate(this->meshOffset);   //    0.0f, -8.25f, -2.0f /*+ auxValue*/);
	vec3 Rotate(faceData->faceRotation[1] * 0.5, 0.0, 0.0);

	// Set and bind uniform attribute
	setUniformMVP(Translate, Rotate);

	// Update/Blend meshes
	this->mLoader->blendMeshes();

	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	Mesh *meshToRender;
	std::vector<float> positionsArray;

	for(int i = 0; i < this->blendedMeshes->size(); i++)
	{
		// Get mesh
		meshToRender = &this->blendedMeshes->at(i);

		if(i == MESH_SHIRT)
		{
			// Ugly hardcode, but it gave me no option..
			// Fixes weird neck/shirt overlapping
			Translate.z = -1.85 /*+ auxValue*/;

			// Set and bind uniform attribute
			setUniformMVP(Translate, Rotate);
		}

		//if(!(i == JEANS_TEXTURE || i == SHOES_TEXTURE))
		this->bindMeshAttributes(meshToRender, i);

		// Set provisional diffuse color
		setUniformColor(vec3(meshToRender->diffuseColor[0], meshToRender->diffuseColor[1], meshToRender->diffuseColor[2]));

		// Need to copy data from array of pointers
		positionsArray.resize(0);

		for(int i = 0; i < meshToRender->vertices.size(); i++)
		{
			positionsArray.push_back(*meshToRender->vertices.at(i));
		}

		// Set VBO data
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glBufferData(GL_ARRAY_BUFFER, positionsArray.size() * sizeof(GL_FLOAT), &positionsArray[0], GL_DYNAMIC_DRAW);

		// Set Normals data
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		glBufferData(GL_ARRAY_BUFFER, meshToRender->normals.size() * sizeof(GL_FLOAT), &meshToRender->normals[0], GL_STATIC_DRAW);

		// Set IBO data
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshToRender->v_indices.size() * sizeof(GL_UNSIGNED_SHORT), &meshToRender->v_indices[0], GL_DYNAMIC_DRAW);

		int texLerpFactor = glGetUniformLocation(shaderProgram, "textLerp");

		glUniform1f(texLerpFactor, 1.0);
		glDrawArrays(GL_TRIANGLES, 0, meshToRender->vertices.size() / 3);
	}
}

void createShaders()
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Invoke helper functions (in glutil.h/cpp) to load text files for vertex and fragment shaders.
	const char *vs =   "attribute   vec3 position;					\
																	\
						attribute   vec3 normal;					\
			            varying	    vec3 normalFrag;            \
						varying vec3 viewSpacePosition;											\
\
			            attribute   vec2 texCoords;					\
                        varying	    vec2 texCoordsOut;               \
																	\
						uniform vec3 uniformColor;					\
						varying vec3 outColor;						\
			            uniform mat4 modelViewProjectionMatrix; 	\
						uniform mat4 modelViewMatrix;				\
																	\
						uniform mat4 normalMatrix;					\
																	\
															        \
						void main() 						        \
						{					                        \
							gl_Position =  modelViewProjectionMatrix*vec4(position.xyz, 1); 	\
							outColor = uniformColor; 						\
										                                    \
			                normalFrag = (normalMatrix * vec4(normal, 1.0)).xyz;                                              \
							viewSpacePosition = (modelViewMatrix*vec4(position, 1.0)).xyz;							\
							texCoordsOut = texCoords;											\
						}";

	const char *fs = 	"precision highp float; 			\
						varying vec3 outColor;				\
						                         			\
						uniform sampler2D texture;			\
			            varying vec2 texCoordsOut;	                    \
			                                                             \
						varying vec3 normalFrag;			              \
						varying vec3 viewSpacePosition;				       \
			            uniform float textLerp;					            \
						void main() 						                    \
						{								                        	\
							vec3 N = normalize(normalFrag);							                  	\
		                    vec3 L = normalize(-viewSpacePosition);                                                   \
			                vec4 texColor = texture2D(texture, texCoordsOut.xy);       if(texColor.a < 0.5) discard;                                             \
							gl_FragColor = texColor * max(0.0, dot(N, L));	                                                    \
						}";

	glShaderSource(vertexShader, 1, &vs, NULL);
	glShaderSource(fragmentShader, 1, &fs, NULL);

	// Compile the shader, translates into internal representation and checks for errors.
	glCompileShader(vertexShader);
	int compileOK;
	// check for compiler errors in vertex shader.
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileOK);
	if(!compileOK)
	{
		std::string err = GetShaderInfoLog(vertexShader);
		fatal_error( err );
		return;
	}

	// Compile the shader, translates into internal representation and checks for errors.
	glCompileShader(fragmentShader);
	// check for compiler errors in fragment shader.
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileOK);
	if(!compileOK)
	{
		std::string err = GetShaderInfoLog(fragmentShader);
		fatal_error( err );
		return;
	}

	// Create a program object and attach the two shaders we have compiled, the program object contains
	// both vertex and fragment shaders as well as information about uniforms and attributes common to both.
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, fragmentShader);
	glAttachShader(shaderProgram, vertexShader);

	// Now that the fragment and vertex shader has been attached, we no longer need these two separate objects and should delete them.
	// The attachment to the shader program will keep them alive, as long as we keep the shaderProgram.
	glDeleteShader( vertexShader );
	glDeleteShader( fragmentShader );

	// We have previously (in the glVertexAttribPointer calls) decided that our
	// vertex position data will be the 0th attribute. Bind the attribute with
	// name "position" to the 0th stream
	glBindAttribLocation(shaderProgram, 0, "position");

	// And bind the attribute called "normal" in the shader to the 1st attribute
	// stream.
	glBindAttribLocation(shaderProgram, 1, "normal");

	// Binds the textureCoordinates attribute
	glBindAttribLocation(shaderProgram, 2, "textCoordS");

	// Link the different shaders that are bound to this program, this creates a final shader that
	// we can use to render geometry with.
	glLinkProgram(shaderProgram);

	// Check for linker errors, many errors, such as mismatched in and out variables between
	// vertex/fragment shaders,  do not appear before linking.
	{
		GLint linkOk = 0;
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkOk);
		if(!linkOk)
		{
			std::string err = GetShaderInfoLog(shaderProgram);
			fatal_error( err );
			return;
		}
	}
}


std::string GetShaderInfoLog(GLuint obj)
{
	int logLength = 0;
	int charsWritten  = 0;
	char *tmpLog;
	std::string log;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength > 0) {
		tmpLog = new char[logLength];
		glGetShaderInfoLog(obj, logLength, &charsWritten, tmpLog);
		log = tmpLog;
		delete[] tmpLog;
	}

	return log;
}

void fatal_error(std::string err )
{
	LOGE("%s", err.c_str());
}


void NativeTrackerRenderer::loadTextures()
{
	Mesh *mesh;
	std::string filePath;
	AAsset* asset;
	off_t asset_size;
	GLuint texture;

	bool hasAlpha;

	for(int i = 0; i < this->mLoader->meshVector.size(); i++)
	{
		// Apparently devices with very low memory (< 512MB) can't hold every texture in memory,
		// since jeans and shoes can't be seen in the application let's just skip those.
//		if(i == JEANS_TEXTURE || i == SHOES_TEXTURE)
//			continue;

		texture = 0;
		mesh = &this->mLoader->meshVector.at(i);

		LOGI("Loading[%i] %s", i, mesh->materials.front().diffuse_texname.c_str());

		filePath = "models/Jones/Materials/" + mesh->materials.front().diffuse_texname;
		ImageData *textureData = FromAssetPNGFile(this->mLoader->getAssetManager(), filePath.c_str(), &hasAlpha);

		LOGI("width: %i height: %i HasAlpha: %i", textureData->img_width, textureData->img_height, hasAlpha);

		meshTextures.push_back(texture);

		glActiveTexture(GL_TEXTURE0);

		// Allocate texture and bind it
		glGenTextures(1, &meshTextures.back());
		glBindTexture(GL_TEXTURE_2D, meshTextures.back());

		// Create texture from data
		glTexImage2D(GL_TEXTURE_2D, 0, hasAlpha ? GL_RGBA : GL_RGB, textureData->img_width, textureData->img_height, 0,
						hasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) textureData->pixels);

		LOGI("GL Error: %i", glGetError());

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	LOGI("Finished loading textures!");
}

inline void NativeTrackerRenderer::bindMeshAttributes(Mesh const *mesh, int textureIndex)
{
	// Fill coordinates buffer
	glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, mesh->texcoords.size() * sizeof(GL_FLOAT) , &mesh->texcoords[0], GL_STATIC_DRAW);

	// Now set texture object
	// Bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, meshTextures.at(textureIndex));

	// Set texture uniform
	int textLoc = glGetUniformLocation(shaderProgram, "texture");
	glUniform1i(textLoc, 0);
}

inline void NativeTrackerRenderer::setUniformMVP(vec3 const &Translate, vec3 const &Rotate)
{
	mat4 Projection = perspective((float)this->fov, (float)this->width / (float)this->height, (float)this->nearPlane, (float)this->farPlane);
	mat4 ViewTranslate = translate(mat4(1.0f), Translate);
	mat4 ViewRotateX = rotate(ViewTranslate, Rotate.y, vec3(-1.0f, 0.0f, 0.0f));
	mat4 View = rotate(ViewRotateX, Rotate.x, vec3(0.0f, 1.0f, 0.0f));
	mat4 Model = scale(mat4(1.0f), vec3(0.5f));

	mat4 ModelViewProjection = Projection * View * Model;
	mat4 ModelViewMatrix = View * Model;
	mat4 NormalMatrix = inverse(transpose(ModelViewMatrix));

	int loc = glGetUniformLocation(shaderProgram, "modelViewProjectionMatrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(ModelViewProjection));

	loc = glGetUniformLocation(shaderProgram, "modelViewMatrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(ModelViewMatrix));

	// Now normalMatrix
	loc = glGetUniformLocation(shaderProgram, "normalMatrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
}

inline void setUniformColor(vec3 color)
{
	int uniCol = glGetUniformLocation(shaderProgram, "uniformColor");
	glUniform3f(uniCol, color.r, color.g, color.b);
}
