#pragma once
#include<windows.h>

#include <FrameworkHeader.h>

#include<map>
#include<math.h>
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//FreeType
#include <ft2build.h>
#include <freetype\freetype.h>


using namespace vmath;


class FontRenderer : public NFramework::TFrameworkMain {


	enum
	{
		VDG_ATTRIBUTE_VERTEX = 0,
		VDG_ATTRIBUTE_COLOR,
		VDG_ATTRIBUTE_NORMAL,
		VDG_ATTRIBUTE_TEXTURE0,
	};

	GLuint gVao;
	GLuint gVbo;

	GLuint gVertexShaderObject;
	GLuint gFragmentShaderObject;
	GLuint gShaderProgramObject;

	GLuint textColor_uniform, projection_matrix_uniform;

	struct Character {
		GLuint TextureID;	// ID Handle of the glyph texture
		glm::ivec2 Size;	// Size of glyph
		glm::ivec2 Bearing;	// Offset from baseline to left/top of glyph
		GLuint Advance;		// Offset to advance to next glyph
	};

	std::map<GLchar, Character> Characters;

	mat4 gPerspectiveProjectionMatrix;
	GLuint gMVPUniform;

	void initialize() {
		// Enabling Fonts 
		FT_Library ft;
		if (FT_Init_FreeType(&ft))
		{
			//fopen_s(&gpFile, "Log.txt", "a+");
			fprintf(gpFile, "Error::FREETYPE: Could Not init freeTypeLibrary\n");
			//fclose(gpFile);

		}

		// Setting up font size
		//FT_Set_Pixel_Sizes(face,20,50);
		FT_Face face;
		if (FT_New_Face(ft, "fonts/Krutidev_080_condensed.TTF", 0, &face))
		{
			//fopen_s(&gpFile, "Log.txt", "a+");
			fprintf(gpFile, "Error::FREETYPE: Failed to Load Font\n");
			//fclose(gpFile);
		}
		// Setting up font size
		FT_Set_Pixel_Sizes(face, 20, 40);

		//FT_Done_Face(face);

		GLint num;
		glGetIntegerv(GL_NUM_EXTENSIONS, &num);
		fprintf(gpFile, "Number of OpenGL GLEW extensions : %d\n", num);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		for (GLubyte c = 0; c < 128; c++)
		{
			// Load Character glyph
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				//fopen_s(&gpFile, "Log.txt", "a+");
				fprintf(gpFile, "ERROR: Failed to load characters");
				//fclose(gpFile);
				continue;
			}

			// Generate Textures
			GLuint texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);

			// Set Texture option
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// Now Store Characters for later uses

			Character character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left,face->glyph->bitmap_top),
				face->glyph->advance.x
			};

			Characters.insert(std::pair<GLchar, Character>(c, character));
		}

		// *** Vertex Shader ***
		// Create Shader

		//fopen_s(&gpFile, "Log.txt", "a+");
		fprintf(gpFile, "Creating vertex shader object\n\n");
		//fclose(gpFile);

		gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

		// provide a source code to shader
		const GLchar *vertexShaderSourceCode =
			"#version 330" \
			"\n" \
			"in vec4 vertex;" \
			"uniform mat4 u_projection_matrix;" \
			"out vec2 TexCoords;" \
			"void main(void)" \
			"{" \
			"gl_Position = u_projection_matrix * vec4(vertex.xy,0.0,1.0);" \
			"TexCoords = vertex.zw;" \
			"}";

		glShaderSource(gVertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);

		// Compile Shader
		glCompileShader(gVertexShaderObject);

		GLint iShaderCompileStatus = 0;
		GLint iInfoLogLength = 0;
		char *szInfoLog = NULL;

		glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
		if (iShaderCompileStatus == GL_FALSE)
		{
			glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
			if (iInfoLogLength > 0)
			{
				szInfoLog = (char*)malloc(sizeof(iInfoLogLength));
				if (szInfoLog != NULL)
				{
					GLsizei written;
					glGetShaderInfoLog(gVertexShaderObject, iInfoLogLength, &written, szInfoLog);
					//fopen_s(&gpFile, "Log.txt", "a+");
					fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", szInfoLog);
					//fclose(gpFile);
					free(szInfoLog);
					uninitialize();
					exit(0);
				}
			}
		}

		//fopen_s(&gpFile, "Log.txt", "a+");
		fprintf(gpFile, "Done Creating vertex shader object\n\n");
		//fclose(gpFile);

		// *** Fragment Shader ***
		// Create Shader
		//fopen_s(&gpFile, "Log.txt", "a+");
		fprintf(gpFile, "Creating fragment shader\n\n");
		//fclose(gpFile);

		gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

		// Provide a source code to shader
		const GLchar *fragmentShaderSourceCode =
			"#version 330" \
			"\n" \
			"in vec2 TexCoords;" \
			"out vec4 color;" \
			"uniform sampler2D text;" \
			"uniform vec3 textColor;" \
			"void main(void)" \
			"{" \
			"vec4 sampled = vec4(1.0,1.0,1.0,texture(text,TexCoords).r);" \
			"color = vec4(textColor,1.0) * sampled;" \
			"}";

		glShaderSource(gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);

		// compile shader
		//fopen_s(&gpFile, "Log.txt", "a+");
		fprintf(gpFile, "Compiling Fragment Shader...\n\n");
		//fclose(gpFile);
		glCompileShader(gFragmentShaderObject);
		glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);

		if (iShaderCompileStatus == GL_FALSE)
		{
			glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
			if (iInfoLogLength > 0)
			{
				szInfoLog = (char*)malloc(sizeof(iInfoLogLength));
				if (szInfoLog != NULL)
				{
					GLsizei written;
					glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLength, &written, szInfoLog);
					//fopen_s(&gpFile, "Log.txt", "a+");
					fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", szInfoLog);
					//fclose(gpFile);
					free(szInfoLog);
					uninitialize();
					exit(0);
				}
			}
		}

		//fopen_s(&gpFile, "Log.txt", "a+");
		fprintf(gpFile, "Done Creating Fragment shader \n\n");
		//fclose(gpFile);

		// *** SHADER PROGRAM ***
		// create
		//fopen_s(&gpFile, "Log.txt", "a+");
		fprintf(gpFile, "Creating shader program object\n\n");
		//fclose(gpFile);
		gShaderProgramObject = glCreateProgram();

		// attach vertex shader object to shader program
		glAttachShader(gShaderProgramObject, gVertexShaderObject);
		glAttachShader(gShaderProgramObject, gFragmentShaderObject);

		// Link Shader
		glLinkProgram(gShaderProgramObject);

		GLint iShaderProgramLinkStatus = 0;
		glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iShaderCompileStatus);

		if (iShaderCompileStatus == GL_FALSE)
		{
			glGetProgramiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
			if (iInfoLogLength > 0)
			{
				szInfoLog = (char *)malloc(sizeof(iInfoLogLength));
				if (iInfoLogLength > 0)
				{
					GLsizei written;
					glGetProgramInfoLog(gShaderProgramObject, iInfoLogLength, &written, szInfoLog);
					//fopen_s(&gpFile, "Log.txt", "a+");
					fprintf(gpFile, "Shader Program Link Log : %s\n", szInfoLog);
					//fclose(gpFile);
					free(szInfoLog);
					uninitialize();
					exit(0);
				}
			}
		}

		//fopen_s(&gpFile, "Log.txt", "a+");
		fprintf(gpFile, "Creating shader program object Done\n\n");
		//fclose(gpFile);

		// Get MVP Uniform location
		textColor_uniform = glGetUniformLocation(gShaderProgramObject, "textColor");
		projection_matrix_uniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");



		// Vao
		//fopen_s(&gpFile, "Log.txt", "a+");
		fprintf(gpFile, "Creating VAO\n\n");
		//fclose(gpFile);
		glGenVertexArrays(1, &gVao);
		glBindVertexArray(gVao);

		//fopen_s(&gpFile, "Log.txt", "a+");
		fprintf(gpFile, "Creating VBO\n\n");
		//fclose(gpFile);
		// Starting VBO
		glGenBuffers(1, &gVbo);
		glBindBuffer(GL_ARRAY_BUFFER, gVbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

		// unbind VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		//fopen_s(&gpFile, "Log.txt", "a+");
		fprintf(gpFile, "VBO Creation Done\n\n");
		//fclose(gpFile);

		//unbind VAO
		glBindVertexArray(0);
		//fopen_s(&gpFile, "Log.txt", "a+");
		fprintf(gpFile, "VAO Creation Done\n\n");
		//fclose(gpFile);

		glShadeModel(GL_SMOOTH);
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthFunc(GL_LEQUAL);

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		glEnable(GL_CULL_FACE);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // blue

		gPerspectiveProjectionMatrix = mat4::identity();

		resize(WIN_WIDTH, WIN_HEIGHT);

		//fopen_s(&gpFile, "Log.txt", "a+");
		fprintf(gpFile, "Leaving initialize\n\n");
		//fclose(gpFile);
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
	
	void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
	{

		///void LoadFont(void);

		//LoadFont();

		//fopen_s(&gpFile, "Log.txt", "a+");
		fprintf(gpFile, "Entering RenderText()\n\n");
		//fclose(gpFile);
		// Activate corresponding render state	
		mat4 modelViewMatrix = mat4::identity();
		mat4 modelViewProjectionMatrix = mat4::identity();

		modelViewMatrix = translate(-50.0f, 10.0f, -100.0f);

		// Multiply model view and orthographic matrix to get model view projection matrix 
		modelViewProjectionMatrix = gPerspectiveProjectionMatrix * modelViewMatrix; // ORDER IS IMPORTANT

																					// Pass above Mode view projection matrix to the above shader in "u_mvp_matrix" shader variable 
																					// whose position value we already calculated in initWithFrame() by using glGetUniformLocation

																					//glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);

		glUniform3f(textColor_uniform, color.x, color.y, color.z);
		glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, modelViewProjectionMatrix);


		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(gVao);

		// Iterate through all characters
		std::string::const_iterator c;
		for (c = text.begin(); c != text.end(); c++)
		{
			Character ch = Characters[*c];

			GLfloat xpos = x + ch.Bearing.x * scale;
			GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

			GLfloat w = ch.Size.x * scale;
			GLfloat h = ch.Size.y * scale;
			// Update VBO for each character
			GLfloat vertices[6][4] = {
				{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos,     ypos,       0.0, 1.0 },
			{ xpos + w, ypos,       1.0, 1.0 },

			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos + w, ypos,       1.0, 1.0 },
			{ xpos + w, ypos + h,   1.0, 0.0 }
			};
			// Render glyph texture over quad
			glBindTexture(GL_TEXTURE_2D, ch.TextureID);
			// Update content of VBO memory
			glBindBuffer(GL_ARRAY_BUFFER, gVbo);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			// Render quad
			glDrawArrays(GL_TRIANGLES, 0, 6);
			// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
		}
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		//fopen_s(&gpFile, "Log.txt", "a+");
		fprintf(gpFile, "Leaving RenderText()...\n\n");
		//fclose(gpFile);
	}
	void resize(int width, int height) {
		GLfloat aspectRatio;

		if (height == 0)
			height = 1;

		glViewport(0, 0, (GLsizei)width, (GLsizei)height);

		aspectRatio = (GLfloat)width / (GLfloat)height;

		//pers(fovy,aspect,near, far);
		gPerspectiveProjectionMatrix = vmath::perspective(45.0f, aspectRatio, 0.1f, 1000.0f);

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


DECLARE_MAIN(FontRenderer)