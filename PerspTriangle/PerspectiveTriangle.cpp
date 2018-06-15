#include<windows.h>

#include <FrameworkHeader.h>

using namespace vmath;

class PerspectiveTriangle : public NFramework::TFrameworkMain {

	GLuint gVertexShaderObject;
	GLuint gFragmentShaderObject;
	GLuint gShaderProgramObject;

	enum {
		SAM_ATTRIBUTE_VERTEX = 0,
		SAM_ATTRIBUTE_COLOR,
		SAM_ATTRIBUTE_NORMAL,
		SAM_ATTRIBUTE_TEXTURE0,
	};

	GLuint gVao;
	GLuint gVbo;
	GLuint gMVPUniform;
	GLfloat fovy = 30.0f;
	mat4 gPerspectiveProjectionMatrix;

	void initialize() {

		gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

		const GLchar *vertexShaderSourceCode =
			"#version 330							"\
			"\n										"\
			"in vec4 vPosition;						"\
			"uniform mat4 u_mvp_matrix;				"\
			"void main(void)						"\
			"{										"\
			"gl_Position = u_mvp_matrix * vPosition;	"\
			"}										";

		glShaderSource(gVertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);

		glCompileShader(gVertexShaderObject);

		GLint iInfoLogLenth = 0;
		GLint iShaderCompiledStatus = 0;
		char *szInfoLog = NULL;
		glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);

		if (iShaderCompiledStatus == GL_FALSE) {
			glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLenth);
			if (iInfoLogLenth > 0) {
				szInfoLog = (char *)malloc(iInfoLogLenth);
				if (szInfoLog != NULL) {
					GLsizei written;
					glGetShaderInfoLog(gVertexShaderObject, iInfoLogLenth, &written, szInfoLog);
					fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", szInfoLog);
					free(szInfoLog);
					uninitialize();
					exit(0);
				}
			}
		}
		//code for FRAGMENT SHADER start here
		// step 1. Create Shader
		// step 2. Provide Shader source
		// step 3. Compile Shader
		gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

		const GLchar *fragmentShaderSourceCode =
			"#version 330							"\
			"\n										"\
			"out vec4 FragColor;					"\
			"void main(void)						"\
			"{										"\
			"FragColor = vec4(1.0,1.0,1.0,1.0);		"\
			"}										";


		glShaderSource(gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);

		glCompileShader(gFragmentShaderObject);

		iInfoLogLenth = 0;
		iShaderCompiledStatus = 0;
		szInfoLog = NULL;
		glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);

		if (iShaderCompiledStatus == GL_FALSE) {
			glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLenth);
			if (iInfoLogLenth > 0) {
				szInfoLog = (char *)malloc(iInfoLogLenth);
				if (szInfoLog != NULL) {
					GLsizei written;
					glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLenth, &written, szInfoLog);
					fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", szInfoLog);
					free(szInfoLog);
					uninitialize();
					exit(0);
				}
			}
		}
		// Create RunTime Program Object that will use vertex shader and fragment shader 
		// step 1. Create Program
		// step 2. Attach Vertex Shader and Fragment shader to program object
		// step 3. Link program object 
		gShaderProgramObject = glCreateProgram();

		glAttachShader(gShaderProgramObject, gVertexShaderObject);

		glAttachShader(gShaderProgramObject, gFragmentShaderObject);

		glBindAttribLocation(gShaderProgramObject, SAM_ATTRIBUTE_VERTEX, "vPosition");

		glLinkProgram(gShaderProgramObject);

		GLint iShaderProgramLinkStatus = 0;
		//reinitialize 
		iInfoLogLenth = 0;
		szInfoLog = NULL;
		glGetShaderiv(gShaderProgramObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);

		if (iShaderProgramLinkStatus == GL_FALSE) {
			glGetShaderiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLenth);
			if (iInfoLogLenth > 0) {
				szInfoLog = (char *)malloc(iInfoLogLenth);
				if (szInfoLog != NULL) {
					GLsizei written;
					glGetShaderInfoLog(gShaderProgramObject, iInfoLogLenth, &written, szInfoLog);
					fprintf(gpFile, "Shader Object Link Log : %s\n", szInfoLog);
					free(szInfoLog);
					uninitialize();
					exit(0);
				}
			}
		}

		gMVPUniform = glGetUniformLocation(gShaderProgramObject, "u_mvp_matrix");

		const GLfloat triangleVertices[] = {
			0.0f,1.0f,0.0f, //apex
			-1.0f,-1.0f,0.0f, // lb
			1.0f,-1.0f,0.0f //rb
		};

		glGenVertexArrays(1, &gVao);
		glBindVertexArray(gVao);

		glGenBuffers(1, &gVbo);
		glBindBuffer(GL_ARRAY_BUFFER, gVbo);

		glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

		glVertexAttribPointer(SAM_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(SAM_ATTRIBUTE_VERTEX);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glShadeModel(GL_SMOOTH);
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glEnable(GL_CULL_FACE);

		glClearColor(0.0f, 0.0f, 1.0f, 0.0f);

		gPerspectiveProjectionMatrix = mat4::identity();

		resize(WIN_WIDTH, WIN_HEIGHT);
	}

	void display() {
		//code
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(gShaderProgramObject);

		// OpenGL Drawing
		mat4 modelViewMatrix = mat4::identity();
		mat4 modelViewProjectionMatrix = mat4::identity();

		modelViewMatrix = vmath::translate(0.0f, 0.0f, -3.0f);

		modelViewProjectionMatrix = gPerspectiveProjectionMatrix * modelViewMatrix;

		glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);

		glBindVertexArray(gVao);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindVertexArray(0);

		glUseProgram(0);
	}

	void zoomInOut(short szDelta) {
		
		if (szDelta > 0)
			fovy += 3.0f;
		else 
			fovy -= 3.0f;

		resize(WIN_WIDTH, WIN_HEIGHT);

	}

	void resize(int width, int height) {
		GLfloat aspectRatio;

		if (height == 0)
			height = 1;

		glViewport(0, 0, (GLsizei)width, (GLsizei)height);

		aspectRatio = (GLfloat)width / (GLfloat)height;

		//pers(fovy,aspect,near, far);
		gPerspectiveProjectionMatrix = vmath::perspective(fovy, aspectRatio, 0.1f, 1000.0f);

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


DECLARE_MAIN(PerspectiveTriangle)
