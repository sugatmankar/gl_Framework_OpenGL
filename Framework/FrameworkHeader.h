#pragma once

#include<windows.h>
#include<stdio.h>

#include<gl/glew.h>
#include<gl/GL.h>

#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")


#define WIN_WIDTH 800
#define WIN_HEIGHT 600


namespace NFramework {

	class TFrameworkMain {

	private:
		bool gbActiveWindow = false;
		bool gbEscapeKeyIsPressed = false;
		bool gbFullScreen = false;

		HWND ghwnd = NULL;
		HDC ghdc = NULL;
		HGLRC ghrc = NULL;
		DWORD dwStyle;
		WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

		FILE *gpFile = NULL;

		GLuint gVertexShaderObject;
		GLuint gFragmentShaderObject;
		GLuint gShaderProgramObject;

		int i = 0;
	protected:
		static  NFramework::TFrameworkMain * app;
		MSG msg;

	public:
		TFrameworkMain() {}

		virtual ~TFrameworkMain() {}

		int run(NFramework::TFrameworkMain* the_app, HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
			bool running = true;
			app = the_app;

			WNDCLASSEX wndclass;
			HWND hwnd;

			TCHAR szClassName[] = TEXT("OpenGL PP");


			if (fopen_s(&gpFile, "Log.txt", "w") != 0) {
				MessageBox(NULL, TEXT("Log File Can Not Be Created\nExitting ..."), TEXT("Error"), MB_OK | MB_TOPMOST | MB_ICONSTOP);
				exit(0);
			}
			else {
				fprintf(gpFile, "Log File Is Successfully Opened.\n");
			}

			wndclass.cbSize = sizeof(WNDCLASSEX);
			wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wndclass.cbClsExtra = 0;
			wndclass.cbWndExtra = 0;
			wndclass.hInstance = hInstance;
			wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
			wndclass.hIcon = LoadIcon(hInstance, TEXT("MYICO"));
			wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
			wndclass.hIconSm = LoadIcon(hInstance, TEXT("MYICO"));
			wndclass.lpfnWndProc = app->WndProc;
			wndclass.lpszClassName = szClassName;
			wndclass.lpszMenuName = NULL;

			RegisterClassEx(&wndclass);

			hwnd = CreateWindowEx(WS_EX_APPWINDOW,
				szClassName,
				TEXT("OpenGL Programmable Pipeline : Window"),
				WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
				100,
				100,
				WIN_WIDTH,
				WIN_HEIGHT,
				NULL,
				NULL,
				hInstance,
				NULL);

			ghwnd = hwnd;
			//SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
			ShowWindow(hwnd, nCmdShow);
			SetForegroundWindow(hwnd);
			SetFocus(hwnd);

			initialize();

			bool bDone = false;

			while (bDone == false)
			{
				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_QUIT)
						bDone = true;
					else
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
				else
				{
					display();

					if (gbActiveWindow == true)
					{
						if (gbEscapeKeyIsPressed == true) {
							bDone = true;
							//MessageBox(NULL, TEXT("Ok "), TEXT("EVENT"), MB_OK);
						}

					}
				}
			}
			uninitialize();

			return((int)msg.wParam);
		}
		virtual void display(void)
		{
			//code
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(gShaderProgramObject);

			// OpenGL Drawing

			glUseProgram(0);

			SwapBuffers(ghdc);
		}



		virtual void initialize(void)
		{
			PIXELFORMATDESCRIPTOR pfd;
			int iPixelFormatIndex;

			ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

			pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = 32;
			pfd.cRedBits = 8;
			pfd.cGreenBits = 8;
			pfd.cBlueBits = 8;
			pfd.cAlphaBits = 8;
			pfd.cDepthBits = 32;

			ghdc = GetDC(ghwnd);

			iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
			if (iPixelFormatIndex == 0)
			{
				ReleaseDC(ghwnd, ghdc);
				ghdc = NULL;
			}

			if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == false)
			{
				ReleaseDC(ghwnd, ghdc);
				ghdc = NULL;
			}

			ghrc = wglCreateContext(ghdc);
			if (ghrc == NULL)
			{
				ReleaseDC(ghwnd, ghdc);
				ghdc = NULL;
			}

			if (wglMakeCurrent(ghdc, ghrc) == false)
			{
				wglDeleteContext(ghrc);
				ghrc = NULL;
				ReleaseDC(ghwnd, ghdc);
				ghdc = NULL;
			}

			GLenum glew_error = glewInit();
			if (glew_error != GLEW_OK)
			{
				wglDeleteContext(ghrc);
				ghrc = NULL;
				ReleaseDC(ghwnd, ghdc);
				ghdc = NULL;
			}

			fprintf(gpFile, "%s %s %s", "GL Version : ", (const char*)glGetString(GL_VERSION), "\n");
			fprintf(gpFile, "%s %s %s", "GLSL Version: ", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION), "\n");

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

			glClearColor(0.0f, 0.0f, 1.0f, 0.0f);

			resize(WIN_WIDTH, WIN_HEIGHT);
		}


	public:
	 static LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
		{
			//NFramework::TFrameworkMain* obj = reinterpret_cast<NFramework::TFrameworkMain*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			if (app)
				return app->mainWndProc(hwnd, iMsg, wParam, lParam);
			return DefWindowProc(hwnd, iMsg, wParam, lParam);
		}

	private:
		LRESULT CALLBACK mainWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
			switch (iMsg)
			{
			case WM_ACTIVATE:
				if (HIWORD(wParam) == 0)
					gbActiveWindow = true;
				else
					gbActiveWindow = false;
				break;
			case WM_SIZE:
				resize(LOWORD(lParam), HIWORD(lParam));
				break;
			case WM_KEYDOWN:
				switch (wParam)
				{
				case VK_ESCAPE:
					gbEscapeKeyIsPressed = true;
					//MessageBox(NULL, TEXT("Ok "), TEXT("EVENT"),MB_OK);
					//fprintf(gpFile,"Escape pressed %d \n",i++);
					break;
				case 0x46:
					if (gbFullScreen == false)
					{
						ToggleFullscreen();
						gbFullScreen = true;
					}
					else
					{
						ToggleFullscreen();
						gbFullScreen = false;
					}
					break;
				default:
					break;
				}
				break;
			case WM_LBUTTONDOWN:
				break;
			case WM_CLOSE:
				uninitialize();
				break;
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
			default:
				break;
			}

			return(DefWindowProc(hwnd, iMsg, wParam, lParam));
		}
		void ToggleFullscreen(void)
		{
			MONITORINFO mi;

			if (gbFullScreen == false)
			{
				dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
				if (dwStyle & WS_OVERLAPPEDWINDOW)
				{
					mi = { sizeof(MONITORINFO) };
					if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
					{
						SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
						SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
					}
				}
				ShowCursor(FALSE);
			}

			else
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
				SetWindowPlacement(ghwnd, &wpPrev);
				SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

				ShowCursor(TRUE);
			}
		}

		virtual void resize(int width, int height)
		{
			//code
			if (height == 0)
				height = 1;
			glViewport(0, 0, (GLsizei)width, (GLsizei)height);
		}

		virtual void uninitialize(void)
		{
			//code
			if (gbFullScreen == true)
			{
				dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
				SetWindowPlacement(ghwnd, &wpPrev);
				SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

				ShowCursor(TRUE);

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

			wglMakeCurrent(NULL, NULL);

			wglDeleteContext(ghrc);
			ghrc = NULL;

			ReleaseDC(ghwnd, ghdc);
			ghdc = NULL;

			if (gpFile)
			{
				fprintf(gpFile, "Log File Is Successfully Closed.\n");
				fclose(gpFile);
				gpFile = NULL;
			}
		}

	};
}


#define DECLARE_MAIN(a)                             \
NFramework::TFrameworkMain *app = 0;                \
int WINAPI WinMain(HINSTANCE hInstance,           \
                     HINSTANCE hPrevInstance,       \
                     LPSTR lpCmdLine,               \
                     int nCmdShow)                  \
{                                                   \
    a *app = new a;									\
    int i = app->run(app,hInstance,hPrevInstance,lpCmdLine,nCmdShow);  \
    return i;                                       \
}														


