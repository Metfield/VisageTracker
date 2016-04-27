#include <_VisageTracker.h>
#include <GLES2/gl2.h>

#include <NativeTrackerRenderer.h>

#include <Logging.h>
#include <string>

// Extern JNI bullcrap
extern "C"
{
	// onSurfaceCreated
	JNIEXPORT void JNICALL Java_com_visage_visagetracker_TrackerRenderer_nativeOnSurfaceCreated(JNIEnv *env, jclass cls)
	{
		NativeTrackerRenderer::getInstance().onSurfaceCreated();
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
}

// `vertexArrayObject' holds the data for each vertex. Data for each vertex
// consists of positions (from positionBuffer) and color (from colorBuffer)
// in this example.
GLuint vertexArrayObject;

// The shaderProgram combines a vertex shader (vertexShader) and a
// fragment shader (fragmentShader) into a single GLSL program that can
// be activated (glUseProgram()).
GLuint shaderProgram;

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

void NativeTrackerRenderer::onSurfaceCreated()
{
	glClearColor(0.0f, 1.0f, 1.0f, 1.0f);

	// Define the positions for each of the three vertices of the triangle
	const float positions[] = {
		//	 X      Y     Z
		0.0f,   0.5f, 1.0f,		// v0
		-0.5f,  -0.5f, 1.0f,	// v1
		0.5f,  -0.5f, 1.0f		// v2
	};

	// Define the colors for each of the three vertices of the triangle
	const float colors[] = {
		//  R     G		B
		1.0f, 1.0f, 1.0f,		// White
		1.0f, 1.0f, 1.0f,		// White
		1.0f, 1.0f, 1.0f		// White
	};

	// Create a handle for the position vertex buffer object
	// See OpenGL Spec §2.9 Buffer Objects
	// - http://www.cse.chalmers.se/edu/course/TDA361/glspec30.20080923.pdf#page=54
	GLuint positionBuffer;
	glGenBuffers( 1, &positionBuffer );
	// Set the newly created buffer as the current one
	glBindBuffer( GL_ARRAY_BUFFER, positionBuffer );
	// Send the vertex position data to the current buffer
	glBufferData( GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW );

	// Create a handle for the vertex color buffer
	GLuint colorBuffer;
	glGenBuffers( 1, &colorBuffer );
	// Set the newly created buffer as the current one
	glBindBuffer( GL_ARRAY_BUFFER, colorBuffer );
	// Send the vertex color data to the current buffer
	glBufferData( GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW );

	//******* Connect triangle data with the vertex array object *******
	//
	// Connect the vertex buffer objects to the vertex array object
	// See OpenGL Spec §2.10
	// - http://www.cse.chalmers.se/edu/course/TDA361/glspec30.20080923.pdf#page=64
	glGenBuffers(1, &vertexArrayObject);
	//glGenVertexArrays(1, &vertexArrayObject);   BLAH!!!!!!!!!!!!!!!

	// Bind the vertex array object
	// The following calls will affect this vertex array object.
	glBindBuffer(1, vertexArrayObject);
	//glBindVertexArray(vertexArrayObject); BLAH!!!!!!!!!!!!!!!!!!!!!!!
	// Makes positionBuffer the current array buffer for subsequent calls.
	glBindBuffer( GL_ARRAY_BUFFER, positionBuffer );
	// Attaches positionBuffer to vertexArrayObject, in the 0th attribute location
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0 );

	// Makes colorBuffer the current array buffer for subsequent calls.
	glBindBuffer( GL_ARRAY_BUFFER, colorBuffer );
	// Attaches colorBuffer to vertexArrayObject, in the 1st attribute location
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0 );

	glEnableVertexAttribArray(0); // Enable the vertex position attribute
	glEnableVertexAttribArray(1); // Enable the vertex color attribute

	// Create shaders
	///////////////////////////////////////////////////////////////////////////

	// See OpenGL spec §2.20 http://www.cse.chalmers.se/edu/course/TDA361/glspec30.20080923.pdf#page=104&zoom=75
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Invoke helper functions (in glutil.h/cpp) to load text files for vertex and fragment shaders.
	const char *vs = 	"attribute   vec3 position;					\
						attribute   vec3 color;						\
						varying vec3 outColor;					\
															\
						void main() 						\
						{									\
							gl_Position = vec4(position,1);	\
							outColor = color;				\
						}";

	const char *fs = 	"precision highp float; 			\
						varying vec3 outColor;				\
															\
						void main() 						\
						{									\
							gl_FragColor = vec4(outColor, 1.0);	\
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
	glBindAttribLocation(shaderProgram, 1, "color");

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

void NativeTrackerRenderer::onSurfaceChanged(int w, int h)
{

}

void NativeTrackerRenderer::onDrawFrame()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_CULL_FACE);

	// Shader Program
	glUseProgram( shaderProgram );			// Set the shader program to use for this draw call
	// Bind the vertex array object that contains all the vertex data.

	//glBindVertexArray(vertexArrayObject);

	   // Load the vertex data
	glBindBuffer( GL_ARRAY_BUFFER, vertexArrayObject );
	glEnableVertexAttribArray(0);

	// Submit triangles from currently bound vertex array object.
	glDrawArrays( GL_TRIANGLES, 0, 3 );				// Render 1 triangle


	glUseProgram( 0 );						// "unsets" the current shader program. Not really necessary.

	//glutSwapBuffers();
}



