#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <gl/glew.h> // must come before GL.h header
#include <gl/GL.h>

#include "vmath.h"
#include "Sphere.h"

#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "opengl32.lib")

#pragma comment (lib, "Sphere.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

using namespace vmath;

enum
{
	VDG_ATTRIBUTE_VERTEX,
	VDG_ATTRIBUTE_COLOR,
	VDG_ATTRIBUTE_NORMAL,
	VDG_ATTRIBUTE_TEXTURE0,
};

LRESULT CALLBACK WndCallbackProc(HWND, UINT, WPARAM, LPARAM);
void uninitialize(void);
void resize(int, int);
void display(void);
void update(void);

FILE *gpFile = NULL;

HWND gblHwnd = NULL;
HDC gblHdc = NULL;
HGLRC gblHrc = NULL;

DWORD gblDwStyle;
WINDOWPLACEMENT gblWindowPlcaement = { sizeof(WINDOWPLACEMENT) };

bool gblFullScreen = false;
bool gblIsEscPressed = false;
bool gblActiveWindow = false;

GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint gNumElements;
GLuint gNumVertices;
float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];

GLuint gVaoSphere;
GLuint gVboSpherePosition;
GLuint gVboShereNormal;
GLuint gVboSphereElements;

GLuint gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;
GLuint gLaUniform, gLdUniform, gLsUniform, gLightPositionUniform;
GLuint gKaUniform, gKdUniform, gKsUniform, gMaterialShinessUniform;
GLuint gLKeyPressedUniform;

mat4 gPerspectiveProjectionMatrix;

GLfloat gLightAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat gLightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat gLightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat gLightPosition[] = { 100.0f, 100.0f, 100.0f, 1.0f };

GLfloat gMaterialAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat gMaterialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat gMaterialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat gMaterialShininess = 50.0f;

bool gbLight;

int WINAPI WinMain(HINSTANCE currentHInstance, HINSTANCE prevHInstance, LPSTR lpszCmdLune, int iCmdShow)
{
	// function prototype
	void initialize(void);
	void uninitialize(void);
	void display(void);
	void update(void);

	// variables declartion
	WNDCLASSEX wndClass;
	HWND hwnd;
	MSG msg;
	int iScreenWidth, iScreenHeight;
	TCHAR szClassName[] = TEXT("RTR OpenGL");
	bool bDone = false;

	if (fopen_s(&gpFile, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Log file cant be created \n existing"), TEXT("ERROR"), MB_OK | MB_TOPMOST | MB_ICONSTOP);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Log file created succcessfully \n");
	}

	//initialize window object
	wndClass.cbSize = sizeof(WNDCLASSEX);
	/*
	CS_OWNDC : Is required to make sure memory allocated is neither movable or discardable
	in OpenGL.
	*/
	wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = currentHInstance;
	wndClass.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wndClass.hIcon = wndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.lpfnWndProc = WndCallbackProc;
	wndClass.lpszClassName = szClassName;
	wndClass.lpszMenuName = NULL;

	// register class
	RegisterClassEx(&wndClass);

	iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	iScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	// create window
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szClassName,
		TEXT("OpenGL Programmable Pipeline : Light : Per Vertex Light"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		(iScreenWidth / 2) - (WIN_WIDTH / 2),
		(iScreenHeight / 2) - (WIN_HEIGHT / 2),
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		currentHInstance,
		NULL);
	gblHwnd = hwnd;

	// initialize
	initialize();

	// render window
	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// game loop
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
			if (gblActiveWindow == true)
			{
				display();
				update();

				if (gblIsEscPressed == true)
					bDone = true;
			}
		}
	}

	uninitialize();

	return ((int)msg.wParam);
}

LRESULT CALLBACK WndCallbackProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	void display(void);
	void toggleFullScreen(void);
	void uninitialize(void);

	// varibles
	static bool isLPressed = false;

	switch (iMsg)
	{
	case WM_ACTIVATE:
		gblActiveWindow = (HIWORD(wParam) == 0);
		break;

	case WM_PAINT:
		/*
		it's single buffered rendering/painting
		single threaded
		can't save the state on stack
		so that's why tearing and flicekring happens
		*/
		break;

		//TO-DO: No need due double buffer
	case WM_ERASEBKGND:
		/*
		telling windows, dont paint window background, this program
		has ability to paint window background by itself.*/
		return(0);

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
		case 'q':
		case 'Q':
			gblIsEscPressed = true;
			break;

		case 0x46: // 'f' or 'F'
			gblFullScreen = !gblFullScreen;
			toggleFullScreen();
			break;

		case 0x4c: // 'L' or 'l'
			isLPressed = !isLPressed;
			gbLight = isLPressed;
			break;

		default:
			break;
		}
		break;

	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
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

	return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void toggleFullScreen(void)
{
	MONITORINFO monInfo;
	HMONITOR hMonitor;
	monInfo = { sizeof(MONITORINFO) };
	hMonitor = MonitorFromWindow(gblHwnd, MONITORINFOF_PRIMARY);

	if (gblFullScreen == true) {
		gblDwStyle = GetWindowLong(gblHwnd, GWL_STYLE);
		if (gblDwStyle & WS_OVERLAPPEDWINDOW)
		{
			gblWindowPlcaement = { sizeof(WINDOWPLACEMENT) };
			if (GetWindowPlacement(gblHwnd, &gblWindowPlcaement) && GetMonitorInfo(hMonitor, &monInfo))
			{
				SetWindowLong(gblHwnd, GWL_STYLE, gblDwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(gblHwnd, HWND_TOP, monInfo.rcMonitor.left, monInfo.rcMonitor.top, (monInfo.rcMonitor.right - monInfo.rcMonitor.left), (monInfo.rcMonitor.bottom - monInfo.rcMonitor.top), SWP_NOZORDER | SWP_FRAMECHANGED);
			}

			ShowCursor(FALSE);
		}
	}
	else
	{
		SetWindowLong(gblHwnd, GWL_STYLE, gblDwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(gblHwnd, &gblWindowPlcaement);
		SetWindowPos(gblHwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
	}
}

void initialize(void)
{
	void releaseDeviceContext(void);
	void uninitialize(void);

	/*It has 26 members*/
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;

	// zero out 
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	// added double buffer now
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;

	gblHdc = GetDC(gblHwnd);

	iPixelFormatIndex = ChoosePixelFormat(gblHdc, &pfd);
	if (iPixelFormatIndex == 0)
	{
		releaseDeviceContext();
	}

	if (SetPixelFormat(gblHdc, iPixelFormatIndex, &pfd) == FALSE)
	{
		releaseDeviceContext();
	}

	/*
	1) every window has individual rendering context (In maya/phosohop every window has own context)
	if we have multiple child window and want to render same things accross all then
	main window knows to with it has shared the context but child/other window doesnt know about it
	2) usually 1 viewport has 1 rendering context. we can split 1 window in multiple viewports and can 1 rendering
	context into multiple viewports

	Transition from window to OpenGL rendering context
	Copies Windows context property to OpenGL context properties
	Windows: WGL (Windows GL)

	This is called Bridging
	*/
	gblHrc = wglCreateContext(gblHdc);
	if (gblHrc == NULL)
	{
		releaseDeviceContext();
	}

	if (wglMakeCurrent(gblHdc, gblHrc) == FALSE)
	{
		wglDeleteContext(gblHrc);
		gblHrc = NULL;
		releaseDeviceContext();
	}

	//GLEW initialization
	GLenum glew_error = glewInit();
	if (glew_error != GLEW_OK)
	{
		wglDeleteContext(gblHrc);
		gblHrc = NULL;
		releaseDeviceContext();
	}

	// VERTEX SHADER
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	// source
	const GLchar *vertexShaderSourceCode =
		"#version 130" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec3 vNormal;" \
		"uniform mat4 u_model_matrix;" \
		"uniform mat4 u_view_matrix;" \
		"uniform mat4 u_projection_matrix;" \
		"uniform int u_lighting_enabled;" \
		"uniform vec4 u_light_position;" \
		"out vec3 trasnformed_normals;" \
		"out vec3 light_direction ;" \
		"out vec3 viewer_vector;" \
		"void main(void)" \
		"{" \
		"if (u_lighting_enabled == 1)" \
		"{" \
		"vec4 eye_coordinates = u_view_matrix * u_model_matrix * vPosition;"\
		"trasnformed_normals = mat3(u_view_matrix * u_model_matrix) * vNormal;" \
		"light_direction = vec3(u_light_position) - eye_coordinates.xyz;" \
		"viewer_vector = -eye_coordinates.xyz;" \
		"}" \
		"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" \
		"}";

	glShaderSource(gVertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);
	glCompileShader(gVertexShaderObject);

	// Error checking
	GLint iInfoLogLength = 0;
	GLint iShaderCompiledStatus = 0;
	char *szInfoLog = NULL;
	glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				//fclose(gpFile);  //if any error occurs write immidiately 
				exit(0);
			}
		}
	}

	// FRAGMENT SHADER
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar *fragmentShaderSourceCode =
		"#version 130" \
		"\n" \
		"in vec3 trasnformed_normals;" \
		"in vec3 light_direction;" \
		"vec3 viewer_vector;" \
		"out vec4 FragColor;" \
		"uniform int u_lighting_enabled;" \
		"uniform vec3 u_La;" \
		"uniform vec3 u_Ld;" \
		"uniform vec3 u_Ls;" \
		"uniform vec3 u_Ka;" \
		"uniform vec3 u_Kd;" \
		"uniform vec3 u_Ks;" \
		"uniform float u_material_shininess;" \
		"void main(void)" \
		"{" \
		"vec3 phong_ads_color;" \
		"if (u_lighting_enabled == 1)" \
		"{" \
		"vec3 normalized_trasnformed_normals = normalize(trasnformed_normals);" \
		"vec3 normalized_light_direction = normalize(light_direction);" \
		"float tn_dot_ld = max(dot(normalized_trasnformed_normals, normalized_light_direction), 0.0);" \
		"vec3 reflection_vector = reflect(-normalized_light_direction, normalized_trasnformed_normals);" \
		"vec3 normalized_viewer_vector = normalize(viewer_vector);" \
		"vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, normalized_viewer_vector) , 0.0) , u_material_shininess);" \
		"vec3 ambient = u_La * u_Ka;" \
		"vec3 diffuse = u_Ld * u_Kd * tn_dot_ld;" \
		"phong_ads_color = ambient + diffuse + specular;" \
		"}" \
		"else" \
		"{" \
		"phong_ads_color = vec3(1.0, 1.0, 1.0);" \
		"}" \
		"FragColor = vec4(phong_ads_color, 1.0);" \
		"}";

	glShaderSource(gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);
	glCompileShader(gFragmentShaderObject);

	// Error checking
	glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				//fclose(gpFile);  //if any error occurs write immidiately 
				exit(0);
			}
		}
	}

	// SHADER PROGRAM
	gShaderProgramObject = glCreateProgram();

	// attach vertex shader to shader program
	glAttachShader(gShaderProgramObject, gVertexShaderObject);

	// attach vertex shader to shader program
	glAttachShader(gShaderProgramObject, gFragmentShaderObject);

	// pre-link binding of shader program object with vertex shader position attribute
	glBindAttribLocation(gShaderProgramObject, VDG_ATTRIBUTE_VERTEX, "vPosition");

	// pre-link binding of shader program object with fragment shader normal attribute
	glBindAttribLocation(gShaderProgramObject, VDG_ATTRIBUTE_NORMAL, "vNormal");

	// link shader
	glLinkProgram(gShaderProgramObject);
	GLint iShaderProgramLinkStatus = 0;
	glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);
	if (iShaderProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "Shader Program Link Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				//fclose(gpFile);  //if any error occurs write immidiately
				exit(0);
			}
		}
	}

	// get MVP uniform location
	gModelMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
	gViewMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
	gProjectionMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");

	gLKeyPressedUniform = glGetUniformLocation(gShaderProgramObject, "u_lighting_enabled");

	// ambient color of light
	gLaUniform = glGetUniformLocation(gShaderProgramObject, "u_La");
	// diffuse color of light
	gLdUniform = glGetUniformLocation(gShaderProgramObject, "u_Ld");
	// specular color of light
	gLsUniform = glGetUniformLocation(gShaderProgramObject, "u_Ls");
	// light position
	gLightPositionUniform = glGetUniformLocation(gShaderProgramObject, "u_light_position");

	// ambient color of material
	gKaUniform = glGetUniformLocation(gShaderProgramObject, "u_Ka");
	// diffuse color of material
	gKdUniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");
	// specular color of light
	gKsUniform = glGetUniformLocation(gShaderProgramObject, "u_Ks");
	// shininess of material
	gMaterialShinessUniform = glGetUniformLocation(gShaderProgramObject, "u_material_shininess");

	// get sphere vertices, normals, textures, elements
	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
	gNumVertices = getNumberOfSphereVertices();
	gNumElements = getNumberOfSphereElements();

	// Cube VAO
	glGenVertexArrays(1, &gVaoSphere);
	glBindVertexArray(gVaoSphere);

	glGenBuffers(1, &gVboSpherePosition);
	glBindBuffer(GL_ARRAY_BUFFER, gVboSpherePosition);
	// move data from main memory to graphics memory
	// GL_STATIC_DRAW or GL_DYNAMIC_DRAW : How you want load data run or preloading. example game shows loadingbar and "loading" messages
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Normals
	glGenBuffers(1, &gVboShereNormal);
	glBindBuffer(GL_ARRAY_BUFFER, gVboShereNormal);
	// move data from main memory to graphics memory
	// GL_STATIC_DRAW or GL_DYNAMIC_DRAW : How you want load data run or preloading. example game shows loadingbar and "loading" messages
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);
	glVertexAttribPointer(VDG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(VDG_ATTRIBUTE_NORMAL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Elements
	glGenBuffers(1, &gVboSphereElements);
	glBindBuffer(GL_ARRAY_BUFFER, gVboSphereElements);
	// move data from main memory to graphics memory
	// GL_STATIC_DRAW or GL_DYNAMIC_DRAW : How you want load data run or preloading. example game shows loadingbar and "loading" messages
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	glShadeModel(GL_SMOOTH);

	glClearDepth(1.0f); // set depth buffer
	glEnable(GL_DEPTH_TEST); // enable depth testing
	glDepthFunc(GL_LEQUAL); // type of depth testing (may be Direxct3D, Vulkcan doesnt requires this test)

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// culling for better performance
	glEnable(GL_CULL_FACE);

	/*state function*/
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // BLACK

	// Perspective Matrix to identity matrix
	gPerspectiveProjectionMatrix = mat4::identity();

	gbLight = false;

	// warmup
	resize(WIN_WIDTH, WIN_HEIGHT);
}

void display(void)
{
	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// start using OpenGL program object
	glUseProgram(gShaderProgramObject);

	if (gbLight)
	{
		// setting light enabled uniform
		glUniform1i(gLKeyPressedUniform, 1);

		// setting light properties uniform
		glUniform3fv(gLaUniform, 1, gLightAmbient);
		glUniform3fv(gLdUniform, 1, gLightDiffuse);
		glUniform3fv(gLsUniform, 1, gLightSpecular);
		glUniform4fv(gLightPositionUniform, 1, gLightPosition);

		glUniform3fv(gKaUniform, 1, gMaterialAmbient);
		glUniform3fv(gKdUniform, 1, gMaterialDiffuse);
		glUniform3fv(gKsUniform, 1, gMaterialSpecular);
		glUniform1f(gMaterialShinessUniform, gMaterialShininess);
	}
	else 
	{
		glUniform1i(gLKeyPressedUniform, 0);
	}

	// set all matrices to identity
	mat4 modelMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();

	// translate z axis
	modelMatrix = translate(0.0f, 0.0f, -2.0f);

	// pass the modelMatrix to vertex shader variable
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);

	// pass the viewMatrix to vertex shader variable
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);

	// pass the projectionlViewMatrix to vertex shader variable
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);

	// bind VAO
	glBindVertexArray(gVaoSphere);

	// draw sphere
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVboSphereElements);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	// unbind VAO
	glBindVertexArray(0);

	// stop using OpenGL program object
	glUseProgram(0);

	SwapBuffers(gblHdc);
}

void update()
{

}

void releaseDeviceContext(void)
{
	ReleaseDC(gblHwnd, gblHdc);
	gblHdc = NULL;
}

void uninitialize(void)
{
	if (gblFullScreen == false)
	{
		gblDwStyle = GetWindowLong(gblHwnd, GWL_STYLE);
		SetWindowLong(gblHwnd, GWL_STYLE, gblDwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(gblHwnd, &gblWindowPlcaement);
		SetWindowPos(gblHwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
	}

	// destroy vao
	if (gVaoSphere)
	{
		glDeleteVertexArrays(1, &gVaoSphere);
		gVaoSphere = 0;
	}

	// destroy vbo
	if (gVboSpherePosition)
	{
		glDeleteBuffers(1, &gVboSpherePosition);
		gVboSpherePosition = 0;
	}
	if (gVboShereNormal)
	{
		glDeleteBuffers(1, &gVboShereNormal);
		gVboShereNormal = 0;
	}
	if (gVboSphereElements)
	{
		glDeleteBuffers(1, &gVboSphereElements);
		gVboSphereElements = 0;
	}

	// detach shader from shader program
	glDetachShader(gShaderProgramObject, gVertexShaderObject);
	glDetachShader(gShaderProgramObject, gFragmentShaderObject);

	// delete vertex shader object
	glDeleteShader(gVertexShaderObject);
	gVertexShaderObject = 0;

	// delete fragment shader object
	glDeleteShader(gFragmentShaderObject);
	gFragmentShaderObject = 0;

	// delete shader program object
	glDeleteProgram(gShaderProgramObject);
	gShaderProgramObject = 0;

	// unlink shader program
	glUseProgram(0);

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(gblHrc);
	gblHrc = NULL;

	releaseDeviceContext();

	DestroyWindow(gblHwnd);
	gblHwnd = NULL;

	if (gpFile)
	{
		fprintf(gpFile, "Log file is succefully closed\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}

/*
Very important for Dirext X not for OpenGL
becuase DirextX is not state machine and change in windows resize empose
re-rendering of Direct X (even for Vulcan) scenes.
*/
void resize(int width, int height)
{
	if (height == 0)
		height = 1;

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	gPerspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

}

