#include <windows.h>

#include <FrameworkHeader.h>


class BlueScreen : public NFramework::TFrameworkMain {

	GLuint gVertexShaderObject;
	GLuint gFragmentShaderObject;
	GLuint gShaderProgramObject;

	void initialize() {
		//Code for VERTEX SHADER start here 
		// step 1. Create Shader
		// step 2. Provide Shader source
		// step 3. Compile Shader
		gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

		const GLchar *vertexShaderSourceCode =
			"void main(void) "\
			"{               "\
			"}               ";
		glShaderSource(gVertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);

		glCompileShader(gVertexShaderObject);

		//code for FRAGMENT SHADER start here
		// step 1. Create Shader
		// step 2. Provide Shader source
		// step 3. Compile Shader
		gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

		const GLchar *fragmentShaderSourceCode =
			"void main(void) "\
			"{				 "\
			"}               ";
		glShaderSource(gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);

		glCompileShader(gFragmentShaderObject);

		// Create RunTime Program Object that will use vertex shader and fragment shader 
		// step 1. Create Program
		// step 2. Attach Vertex Shader and Fragment shader to program object
		// step 3. Link program object 
		gShaderProgramObject = glCreateProgram();

		glAttachShader(gShaderProgramObject, gVertexShaderObject);

		glAttachShader(gShaderProgramObject, gFragmentShaderObject);

		glLinkProgram(gShaderProgramObject);

		glShadeModel(GL_SMOOTH);
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glEnable(GL_CULL_FACE);

		glClearColor(0.0f, 1.0f, 1.0f, 0.0f);

		resize(WIN_WIDTH, WIN_HEIGHT);
	}

	void display() {
		//code
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(gShaderProgramObject);

		// OpenGL Drawing

		glUseProgram(0);
	}

	void resize(int width, int height) {
		//code
		if (height == 0)
			height = 1;
		glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	}

	void uninitialize() {

		glDetachShader(gShaderProgramObject, gVertexShaderObject);
		glDetachShader(gShaderProgramObject, gFragmentShaderObject);

		glDeleteShader(gVertexShaderObject);
		gVertexShaderObject = 0;
		glDeleteShader(gFragmentShaderObject);
		gFragmentShaderObject = 0;

		glDeleteProgram(gShaderProgramObject);
		gShaderProgramObject = 0;

		glUseProgram(0);

	}
	
};


DECLARE_MAIN(BlueScreen)
