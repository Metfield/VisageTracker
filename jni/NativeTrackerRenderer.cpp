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

// Pre-calculated value of PI / 180.
#define kPI180     0.017453

// Pre-calculated value of 180 / PI.
#define k180PI    57.295780

// Converts degrees to radians.
#define degreesToRadians(x) (x * kPI180)

// Converts radians to degrees.
#define radiansToDegrees(x) (x * k180PI)

#define USE_TEST_GEOMETRY 0

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

	JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_nativeTouches(JNIEnv *env, jclass cls, jfloat value)
	{
		NativeTrackerRenderer::getInstance().auxValue = value;
	}
}

// `vertexArrayObject' holds the data for each vertex. Data for each vertex
// consists of positions (from positionBuffer) and color (from colorBuffer)
// in this example.
GLuint vertexArrayObject;

// The shaderProgram combines a vertex shader (vertexShader) and a
// fragment shader (fragmentShader) into a single GLSL program that can
// be activated (glUseProgram()).
GLuint shaderProgram;

std::string GetShaderInfoLog(GLuint obj);
void fatal_error(std::string err );
void createShaders();

 GLuint elementbuffer;
 int nIndices;
 int nVerts;

using namespace glm;

void NativeTrackerRenderer::onSurfaceCreated(int w, int h)
{
	this->width = w;
	this->height = h;

	glClearColor(0.0f, 1.0f, 1.0f, 1.0f);


	nVerts = meshes->at(0).vertices.size();

	glGenBuffers(1, &vertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexArrayObject);
	glBufferData(GL_ARRAY_BUFFER, meshes->at(0).vertices.size() * sizeof(GL_FLOAT), &meshes->at(0).vertices[0], GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0 );
	glEnableVertexAttribArray(0);



	createShaders();
}

void NativeTrackerRenderer::onSurfaceChanged(int w, int h)
{

}

void NativeTrackerRenderer::onDrawFrame()
{
	// Update/blend meshes
	this->mLoader->blendMeshes();

	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_CULL_FACE);

	glUseProgram( shaderProgram );

	vec3 Translate(0.0f, -8.25f, -2.0f);
	vec3 Rotate(0.0f, 0.0f, 0.0f);

	setUniformMVP(Translate, Rotate);

	glBindBuffer(GL_ARRAY_BUFFER, vertexArrayObject);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_POINTS, 0, meshes->at(0).vertices.size()/3);
}

void createShaders()
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Invoke helper functions (in glutil.h/cpp) to load text files for vertex and fragment shaders.
	const char *vs = 	"attribute   vec3 position;					\
						attribute   vec3 color;						\
						varying vec3 outColor;					\
			            uniform mat4 modelViewProjectionMatrix; 			\
															\
						void main() 						\
						{					\
							gl_Position =  modelViewProjectionMatrix*vec4(position.xyz, 1); 	\
							outColor = color;	gl_PointSize = 2.0f;			\
						}";

	const char *fs = 	"precision highp float; 			\
						varying vec3 outColor;				\
															\
						void main() 						\
						{									\
							gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);	\
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
	// And bind the attribute called "color" in the shader to the 1st attribute
	// stream.
//	glBindAttribLocation(shaderProgram, 1, "color");

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

inline void NativeTrackerRenderer::setUniformMVP(vec3 const &Translate, vec3 const &Rotate)
{
	mat4 Projection = perspective(45.0f, (float)this->width / (float)this->height, 0.01f, 500.f);
	mat4 ViewTranslate = translate(mat4(1.0f), Translate);
	mat4 ViewRotateX = rotate(ViewTranslate, Rotate.y, vec3(-1.0f, 0.0f, 0.0f));
	mat4 View = rotate(ViewRotateX, Rotate.x, vec3(0.0f, 1.0f, 0.0f));
	mat4 Model = scale(mat4(1.0f), vec3(0.5f));

	mat4 ModelViewProjection = Projection * View * Model;

	glEnableVertexAttribArray(0);

	int loc = glGetUniformLocation(shaderProgram, "modelViewProjectionMatrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(ModelViewProjection));
}

