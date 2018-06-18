#include<windows.h>

#include<FrameworkHeader.h>

#include <vector>

using namespace vmath;


class BasicShapes : public NFramework::TFrameworkMain
{

	GLuint gVertexShaderObject;
	GLuint gFragmentShaderObject;
	GLuint gShaderProgramObject;

	enum {
		SAM_ATTRIBUTE_VERTEX = 0,
		SAM_ATTRIBUTE_COLOR,
		SAM_ATTRIBUTE_NORMAL,
		SAM_ATTRIBUTE_TEXTURE0,
	};

	//GLuint gNumVertices, gNumElements;

	GLuint gVao_Sphere;
	GLuint gVbo_Sphere_Position;
	GLuint gVbo_Sphere_Normal;
	GLuint gVbo_Sphere_element;

	GLuint gModelViewUniform, gProjectionMatrixUniform;
	GLuint ldUniform, kdUniform, lightPositionUniform;
	GLuint lightToggleUniform;

	GLfloat fovy = 30.0f;
	mat4 gPerspectiveProjectionMatrix;

	std::vector<GLfloat> sphere_vertices;
	std::vector<GLfloat> sphere_normals;
	std::vector<GLfloat> sphere_texcoords;
	std::vector<GLushort> sphere_elements;

	int gAnimationToggle, gLightToggle;

	void initialize() {

		gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

		const GLchar *vertexShaderSourceCode =
			"#version 330								"\
			"\n											"\
			"in vec4 vPosition;							"\
			"in vec3 vNormal;							"\
			"uniform mat4 u_model_view_matrix;			"\
			"uniform mat4 u_projection_matrix;			"\
			"uniform mediump int u_light_toggle_uniform;"\
			"uniform vec3 u_Ld;							"\
			"uniform vec3 u_Kd; 						"\
			"uniform vec4 u_light_position;				"\
			"out vec3 out_diffuse_light;				"\
			"uniform mat4 u_mvp_matrix;					"\
			"void main(void)							"\
			"{											"\
			"if(u_light_toggle_uniform==1) "\
			"{"\
			"vec4 eyeCoordinates = u_model_view_matrix * vPosition; "\
			"vec3 tnorm = normalize(mat3(u_model_view_matrix)* vNormal);"\
			"vec3 s = normalize(vec3(u_light_position - eyeCoordinates));"\
			"out_diffuse_light = u_Ld * u_Kd * max(dot(s,tnorm),0.0);"

			"}"\
			"gl_Position = u_projection_matrix * u_model_view_matrix * vPosition;  "\

			"}											";

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
			"in vec3 out_diffuse_light;				"\
			"uniform int u_light_toggle_uniform;"\
			"void main(void)						"\
			"{										"\
			"if(u_light_toggle_uniform == 1 ){"\
			"FragColor = vec4(out_diffuse_light,1.0);"\
			"} else {"
			"FragColor = vec4(1.0,1.0,1.0,1.0);"\
			"}"\
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
		glBindAttribLocation(gShaderProgramObject, SAM_ATTRIBUTE_NORMAL, "vNormal");



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

		gModelViewUniform = glGetUniformLocation(gShaderProgramObject, "u_model_view_matrix");
		gProjectionMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");

		lightToggleUniform = glGetUniformLocation(gShaderProgramObject, "u_light_toggle_uniform");
		ldUniform = glGetUniformLocation(gShaderProgramObject, "u_Ld");
		kdUniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");
		lightPositionUniform = glGetUniformLocation(gShaderProgramObject, "u_light_position");

		getSphereVertexData(1, 6, 12);

		std::vector<GLfloat>::iterator v = sphere_vertices.begin();
		std::vector<GLfloat>::iterator n = sphere_normals.begin();
		std::vector<GLfloat>::iterator t = sphere_texcoords.begin();
		std::vector<GLushort>::iterator i = sphere_elements.begin();

		fprintf(gpFile,"Sphere Vertices size: %I64u\n", sphere_vertices.size());
		int count = 0;
		for (std::vector<GLfloat>::const_iterator i = sphere_vertices.begin() ; i != sphere_vertices.end() ; i++)
		{
			fprintf(gpFile, "%f,", *i);
			if (++count >= 3) 
			{
				fprintf(gpFile, "\n");
				count = 0;
			}
			
		}
		fprintf(gpFile, "Sphere Normals size: %I64u\n", sphere_normals.size());
		fprintf(gpFile, "Sphere Texcoords size: %I64u\n", sphere_texcoords.size());
		fprintf(gpFile, "Sphere Indices size: %I64u\n", sphere_elements.size());

		glGenVertexArrays(1, &gVao_Sphere);
		glBindVertexArray(gVao_Sphere);

		glGenBuffers(1, &gVbo_Sphere_Position);
		glBindBuffer(GL_ARRAY_BUFFER, gVbo_Sphere_Position);
		glBufferData(GL_ARRAY_BUFFER, sphere_vertices.size(), &sphere_vertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer(SAM_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(SAM_ATTRIBUTE_VERTEX);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &gVbo_Sphere_Normal);
		glBindBuffer(GL_ARRAY_BUFFER, gVbo_Sphere_Normal);
		glBufferData(GL_ARRAY_BUFFER, sphere_normals.size(), &sphere_normals[0], GL_STATIC_DRAW);
		glVertexAttribPointer(SAM_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(SAM_ATTRIBUTE_NORMAL);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);

		glGenBuffers(1, &gVbo_Sphere_element);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_Sphere_element);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere_elements.size(), &sphere_elements[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glShadeModel(GL_SMOOTH);
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glEnable(GL_CULL_FACE);

		glClearColor(0.0f, 0.0f, 1.0f, 0.0f);

		gPerspectiveProjectionMatrix = mat4::identity();

		gAnimationToggle = 0;
		gLightToggle = 0;

		resize(WIN_WIDTH, WIN_HEIGHT);
	}

	void getSphereVertexData(float radius, unsigned int rings, unsigned int sectors)
	{
		float const R = 1. / (float)(rings - 1);
		float const S = 1. / (float)(sectors - 1);

		int r, s;

		sphere_vertices.resize(rings * sectors * 3);
		sphere_normals.resize(rings * sectors * 3);
		sphere_texcoords.resize(rings * sectors * 2);
		sphere_elements.resize(rings * sectors *4);

		std::vector<GLfloat>::iterator v = sphere_vertices.begin();
		std::vector<GLfloat>::iterator n = sphere_normals.begin();
		std::vector<GLfloat>::iterator t = sphere_texcoords.begin();
		std::vector<GLushort>::iterator i = sphere_elements.begin();

		for (r = 0; r < rings; r++)
		for (s = 0; s < sectors; s++) {
			float const y = sin(-M_PI_2 + M_PI * r * R);
			float const x = cos(2 * M_PI * s * S) * sin(M_PI * r * R);
			float const z = sin(2 * M_PI * s * S) * sin(M_PI * r * R);

			*t++ = s * S;
			*t++ = r * R;

			*v++ = x * radius;
			*v++ = y * radius;
			*v++ = z * radius;

			*n++ = x;
			*n++ = y;
			*n++ = z;
		}

		for (r = 0; r < rings; r++)
		for (s = 0; s < sectors; s++) {
			*i++ = r * sectors + s;
			*i++ = r * sectors + (s + 1);
			*i++ = (r + 1) * sectors + (s + 1);
			*i++ = (r + 1) * sectors + s;
		}
	}

	void display() {
		//code
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(gShaderProgramObject);

		if (gLightToggle == 1) {
			glUniform1i(lightToggleUniform, 1);
			glUniform3f(ldUniform, 1.0f, 1.0f, 1.0f); //diffuse intensity of light
			glUniform3f(kdUniform, 0.5f, 0.5f, 0.5f); //diffuse reflectivity of Material
			GLfloat lightPosition[] = { 0.0f,0.0f,2.0f,1.0f };
			glUniform4fv(lightPositionUniform, 1, lightPosition); //light position

		}
		else {
			glUniform1i(lightToggleUniform, 0);
		}

		// OpenGL Drawing
		mat4 modelViewMatrix = mat4::identity();
		mat4 modelViewProjectionMatrix = mat4::identity();
		//mat4 rotationMatrix = mat4::identity();

		modelViewMatrix = vmath::translate(0.0f, 0.0f, -6.0f);
		//use rotate T angle , T angle , T angle
		//rotationMatrix = vmath::rotate(gAngleCube, gAngleCube, gAngleCube);
		//modelViewMatrix = modelViewMatrix * rotationMatrix;
		modelViewProjectionMatrix = gPerspectiveProjectionMatrix * modelViewMatrix;

		glUniformMatrix4fv(gModelViewUniform, 1, GL_FALSE, modelViewMatrix);
		glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);


		glBindVertexArray(gVao_Sphere);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_Sphere_element);
		glDrawElements(GL_QUADS, (GLsizei)sphere_elements.size(), GL_UNSIGNED_SHORT, &sphere_elements[0]);

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

	void animationToggle(void)
	{
		gAnimationToggle++;
		if (gAnimationToggle > 1)
			gAnimationToggle = 0;
	}

	void lightToggle(void)
	{
		gLightToggle++;
		if (gLightToggle > 1)
			gLightToggle = 0;
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

		if (gVao_Sphere)
		{
			glDeleteVertexArrays(1, &gVao_Sphere);
			gVao_Sphere = 0;
		}


		if (gVbo_Sphere_Normal)
		{
			glDeleteBuffers(1, &gVbo_Sphere_Normal);
			gVbo_Sphere_Normal = 0;
		}

		if (gVbo_Sphere_Position)
		{
			glDeleteBuffers(1, &gVbo_Sphere_Position);
			gVbo_Sphere_Position = 0;
		}


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


DECLARE_MAIN(BasicShapes)


