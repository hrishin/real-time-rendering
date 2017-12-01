#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <gl/GL.h>
#include <gl/GLU.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "GLU32.lib")

LRESULT CALLBACK WndCallbackProc(HWND, UINT, WPARAM, LPARAM);
void uninitialize(void);
void resize(int, int);
void display(void);

HWND gblHwnd = NULL;
HDC gblHdc = NULL;
HGLRC gblHrc = NULL;

DWORD gblDwStyle;
WINDOWPLACEMENT gblWindowPlcaement = { sizeof(WINDOWPLACEMENT) };

bool gblFullScreen = false;
bool gblIsEscPressed = false;
bool gblActiveWindow = false;
bool gblLPressed = false;
bool gblXPressed = false;
bool gblYPressed = false;
bool gblZPressed = false;

GLfloat angleXLight    = 0.0f;
GLfloat angleYLight	 = 0.0f;
GLfloat angleZLight	 = 0.0f;

GLfloat light1_ambience[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat light1_defiuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat light1_spcular[]  = { 1.0f, 0.0f, 0.0f, 1.0f };
GLfloat light1_x_position[] = { 1.0f, 1.0f, 1.0f, 0.0f };
GLfloat light1_y_position[] = { 1.0f, 1.0f, 1.0f, 0.0f };
GLfloat light1_z_position[] = { 1.0f, 1.0f, 1.0f, 0.0f };

GLfloat material_ambience[24][4] = {
	{ 0.0215, 0.1745, 0.0215, 1.0f},
	{ 0.135, 0.2225, 0.1575, 1.0f },
	{ 0.05375, 0.05, 0.06625, 1.0f },
	{ 0.25, 0.20725, 0.20725, 1.0f },
	{ 0.1745, 0.01175, 0.01175, 1.0f},
	{ 0.1, 0.18725, 0.1745, 1.0f},
	{ 0.329412, 0.223529, 0.027451, 1.0f },
	{ 0.2125, 0.1275,0.054, 1.0f},
	{ 0.25, 0.25, 0.25, 1.0f },
	{ 0.19125, 0.0735, 0.0225, 1.0f },
	{ 0.24725, 0.1995, 0.0745, 1.0f },
	{ 0.19225, 0.19225, 0.19225, 1.0f },
	{ 0.25, 0.20725, 0.20725, 1.0f },
	{ 0.0, 0.1, 0.06, 1.0f },
	{ 0.0, 0.0, 0.0, 1.0f },
	{ 0.0, 0.0, 0.0, 1.0f },
	{ 0.0, 0.0, 0.0, 1.0f },
	{0.0, 0.0, 0.0, 1.0f},
	{0.02, 0.02, 0.02, 1.0f},
	{ 0.0, 0.05, 0.05, 1.0f },
	{ 0.0, 0.05, 0.0, 1.0f },
	{0.05, 0.0, 0.0, 1.0f},
	{ 0.05, 0.05, 0.05, 1.0f },
	{0.05, 0.05, 0.0, 1.0f}

};
GLfloat material_diffuse[24][4] = { 
	{0.07568, 0.61424, 0.07568, 1.0f},
	{ 0.54, 0.89, 0.63, 1.0f },
	{ 0.18275, 0.17, 0.22525, 1.0f },
	{ 1.0, 0.829, 0.829, 1.0f }, 
	{ 0.61424, 0.04136, 0.04136, 1.0f },
	{ 0.396, 0.74151, 0.69102, 1.0f },
	{ 0.780392, 0.568627, 0.113725,1.0f},
	{ 0.714, 0.4284, 0.18144, 1.0f},
	{ 0.4, 0.4, 0.4, 1.0f },
	{ 0.7038, 0.27048, 0.0828, 1.0f },
	{ 1.0, 0.829, 0.829, 1.0f},
	{ 0.50754, 0.50754, 0.50754, 1.0f },
	{ 0.01, 0.01, 0.01, 1.0f },
	{ 0.0, 0.50980392, 0.50980392, 1.0f },
	{ 0.1, 0.35, 0.1, 1.0f },
	{ 0.5, 0.0, 0.0, 1.0f },
	{ 0.55, 0.55, 0.55, 1.0f },
	{ 0.5, 0.5, 0.0, 1.0f },
	{ 0.01, 0.2, 0.2, 1.0f },
	{ 0.4, 0.5, 0.5, 1.0f },
	{ 0.4, 0.5, 0.4, 1.0f },
	{ 0.5, 0.4, 0.4, 1.0f },
	{ 0.5, 0.5, 0.5, 1.0f },
	{ 0.5, 0.5, 0.4, 1.0f }
};
GLfloat material_specular[24][4] = { 
	{0.633, 0.727811, 0.633, 1.0f},
	{ 0.316228, 0.316228, 0.316228, 1.0f },
	{ 0.332741, 0.328634, 0.346435, 1.0f },
	{ 0.296648, 0.296648, 0.296648, 1.0f },
	{ 0.727811, 0.626959, 0.626959, 1.0f },
	{ 0.297254, 0.30829, 0.306678, 1.0f},
	{ 0.992157, 0.941176, 0.807843, 1.0f},
	{ 0.393548, 0.271906, 0.166721, 1.0f },
	{ 0.774597, 0.774597, 0.774597, 1.0f },
	{ 0.256777, 0.137622, 0.086014, 1.0f },
	{ 0.508273, 0.508273, 0.508273, 1.0f },
	{ 0.50196078, 0.50196078, 0.50196078, 1.0f },
	{ 0.296648, 0.296648, 0.296648, 1.0f},
	{ 0.45, 0.55, 0.45, 1.0f },
	{0.7, 0.6, 0.6, 1.0f},
	{ 0.70, 0.70, 0.70, 1.0f },
	{0.60, 0.60, 0.50, 1.0f},
	{0.4, 0.4, 0.4, 1.0f},
	{0.04, 0.7, 0.7, 1.0f},
	{0.04, 0.7, 0.04, 1.0f},
	{0.7, 0.04, 0.04, 1.0f},
	{0.7, 0.7, 0.7, 1.0f},
	{0.7, 0.7, 0.04, 1.0f}

};
GLfloat material_shiness[24][1] = { 
	{0.1 * 128},
	{0.3 * 128},
	{ 0.088 * 128},
	{ 0.6 * 128},
	{ 0.1 * 128},
	{ 0.21794872 * 128},
	{0.2 * 128},
	{ 0.6 * 128 },
	{0.1 * 128},
	{0.4 * 128},
	{0.4 * 128},
	{0.25 * 128},
	{0.25 * 128},
	{ 0.088 * 128 },
	{0.25 * 128},
	{0.25 * 128},
	{0.25 * 128},
	{0.25 * 128},
	{0.078125 * 128},
	{0.078125 * 128},
	{0.078125 * 128},
	{0.078125 * 128},
	{0.078125 * 128},
	{0.078125 * 128}
};

GLfloat light_model_ambient[] = { 0.2, 0.2, 0.2, 0.0 };
GLfloat light_model_local_viewer[] = { 0.0 };
GLUquadric *qudric = NULL;

int WINAPI WinMain(HINSTANCE currentHInstance, HINSTANCE prevHInstance, LPSTR lpszCmdLune, int iCmdShow)
{
	// function prototype
	void initialize(void);

	// variables declartion
	WNDCLASSEX wndClass;
	HWND hwnd;
	MSG msg;
	int iScreenWidth, iScreenHeight;
	TCHAR szClassName[] = TEXT("RTR OpenGL");
	bool bDone = false;

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
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
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
		TEXT("OpenGL Fixed Fucntion Pipeline : Material Demo"),
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

		//TD-DO: No need due double buffer
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

		case 'L':
		case 'l':
			if (gblLPressed)
			{
				gblLPressed = false;
				glDisable(GL_LIGHTING);
			}
			else
			{
				gblLPressed = true;
				glEnable(GL_LIGHTING);
			}
			break;

		case 'X':
		case 'x':
			gblXPressed = true;
			gblYPressed = false;
			gblZPressed = false;
			break;

		case 'Y':
		case 'y':
			gblYPressed = true;
			gblXPressed = false;
			gblZPressed = false;
			break;

		case 'Z':
		case 'z':
			gblZPressed = true;
			gblXPressed = false;
			gblYPressed = false;
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

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		break;
	}

	return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void initialize(void)
{
	void ReleaseDeviceContext(void);
	int parse(char *file_path);

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
		ReleaseDeviceContext();
	}

	if (SetPixelFormat(gblHdc, iPixelFormatIndex, &pfd) == FALSE)
	{
		ReleaseDeviceContext();
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
		ReleaseDeviceContext();
	}

	if (wglMakeCurrent(gblHdc, gblHrc) == FALSE)
	{
		wglDeleteContext(gblHrc);
		gblHrc = NULL;
		ReleaseDeviceContext();
	}

	qudric = gluNewQuadric();

	/*state function*/
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glClearDepth(1.0f); // set depth buffer
	glEnable(GL_DEPTH_TEST); // enable depth testing
	glDepthFunc(GL_LEQUAL); // type of depth testing (may be Direxct3D, Vulkcan doesnt requires this test)

	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_model_ambient);
	glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, light_model_local_viewer);

	glLightfv(GL_LIGHT0, GL_AMBIENT, light1_ambience);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light1_defiuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light1_spcular);
	glEnable(GL_LIGHT0);

	parse("./Materials-ReadMe.txt");

	// warmup
	resize(WIN_WIDTH, WIN_HEIGHT);
}

int parse(char *file_path)
{
	FILE *file = NULL;
	int objects, i;
	char *line = NULL;
	size_t len = 0;
	size_t read;

	objects = 24;
	fopen_s(&file, file_path, "r");
	/*if (file == NULL)
	{
		fprintf(stderr, "\nError opening file\n");
		exit(EXIT_FAILURE);
	}*/

	return 0;
}

void ReleaseDeviceContext(void)
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

	wglMakeCurrent(NULL, NULL);

	wglDeleteContext(gblHrc);
	gblHrc = NULL;

	ReleaseDeviceContext();

	DestroyWindow(gblHwnd);
	gblHwnd = NULL;
}

void display(void)
{
	void update(void);
	/*
	state function: May be
	- clear the FrameBuffer for following properties
	- color Buffer, Depth Buffer, Stencil Buffer, Accumulate Buffer for fragement
	*/
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(0.0f, 0.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	
	if (gblXPressed == true)
	{
		glPushMatrix();
		glRotatef(angleXLight, 1.0f, 0.0f, 0.0f);
		light1_x_position[1] = angleXLight;
		glLightfv(GL_LIGHT0, GL_POSITION, light1_x_position);
		glPopMatrix();
	}
	else if (gblYPressed == true)
	{
		glPushMatrix();
		glRotatef(angleYLight, 0.0f, 1.0f, 0.0f);
		light1_y_position[0] = angleYLight;
		glLightfv(GL_LIGHT0, GL_POSITION, light1_y_position);
		glPopMatrix();
	} 
	else if(gblZPressed == true)
	{
		glPushMatrix();
		glRotatef(angleZLight, 0.0f, 0.0f, 1.0f);
		light1_z_position[0] = angleZLight;
		glLightfv(GL_LIGHT0, GL_POSITION, light1_z_position);
		glPopMatrix();
	}
	
	// row 1
	glPushMatrix();
	glTranslatef(-1.0f, 1.7f, -3.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[0]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[0]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[0]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[0]);
	gluSphere(qudric, 0.3f, 100, 100);

	glTranslatef(0.7f, 0.0f, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[1]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[1]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[1]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[1]);
	gluSphere(qudric, 0.3f, 100, 100);

	glTranslatef(0.7f, 0.0f, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[2]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[2]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[2]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[2]);
	gluSphere(qudric, 0.3f, 100, 100);

	glTranslatef(0.7f, 0.0f, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[3]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[3]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[3]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[3]);
	gluSphere(qudric, 0.3f, 100, 100);
	glPopMatrix();

	//row 2
	glPushMatrix();
	glTranslatef(-1.0f, 1.0f, -3.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[4]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[4]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[4]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[4]);
	gluSphere(qudric, 0.3f, 100, 100);

	glTranslatef(0.7f, 0.0f, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[5]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[5]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[5]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[5]);
	gluSphere(qudric, 0.3f, 100, 100);

	glTranslatef(0.7f, 0.0f, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[6]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[6]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[6]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[6]);
	gluSphere(qudric, 0.3f, 100, 100);

	glTranslatef(0.7f, 0.0f, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[7]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[7]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[7]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[7]);
	gluSphere(qudric, 0.3f, 100, 100);
	glPopMatrix();

	//row 3
	glPushMatrix();
	glTranslatef(-1.0f, 0.3f, -3.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[8]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[8]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[8]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[8]);
	gluSphere(qudric, 0.3f, 100, 100);

	glTranslatef(0.7f, 0.0f, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[9]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[9]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[9]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[9]);
	gluSphere(qudric, 0.3f, 100, 100);

	glTranslatef(0.7f, 0.0f, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[10]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[10]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[10]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[10]);
	gluSphere(qudric, 0.3f, 100, 100);

	glTranslatef(0.7f, 0.0f, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[11]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[11]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[11]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[11]);
	gluSphere(qudric, 0.3f, 100, 100);
	glPopMatrix();

	//row 4
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[12]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[12]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[12]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[12]);
	glTranslatef(-1.0f, -0.4f, -3.0f);
	gluSphere(qudric, 0.3f, 100, 100);

	glTranslatef(0.7f, 0.0f, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[13]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[13]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[13]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[13]);
	gluSphere(qudric, 0.3f, 100, 100);

	glTranslatef(0.7f, 0.0f, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[14]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[14]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[14]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[14]);
	gluSphere(qudric, 0.3f, 100, 100);

	glTranslatef(0.7f, 0.0f, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[15]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[15]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[15]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[15]);
	gluSphere(qudric, 0.3f, 100, 100);
	glPopMatrix();

	//row 5
	glPushMatrix();
	glTranslatef(-1.0f, -1.1f, -3.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[16]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[16]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[16]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[16]);
	gluSphere(qudric, 0.3f, 100, 100);

	glTranslatef(0.7f, 0.0f, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[17]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[17]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[17]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[17]);
	gluSphere(qudric, 0.3f, 100, 100);

	glTranslatef(0.7f, 0.0f, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[18]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[18]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[18]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[18]);
	gluSphere(qudric, 0.3f, 100, 100);

	glTranslatef(0.7f, 0.0f, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[19]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[19]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[19]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[19]);
	gluSphere(qudric, 0.3f, 100, 100);
	glPopMatrix();

	//row 6
	glPushMatrix();
	glTranslatef(-1.0f, -1.8f, -3.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[20]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[20]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[20]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[20]);
	gluSphere(qudric, 0.3f, 100, 100);

	glTranslatef(0.7f, 0.0f, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[21]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[21]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[21]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[21]);
	gluSphere(qudric, 0.3f, 100, 100);

	glTranslatef(0.7f, 0.0f, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[22]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[22]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[22]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[22]);
	gluSphere(qudric, 0.3f, 100, 100);

	glTranslatef(0.7f, 0.0f, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience[23]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse[23]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular[23]);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness[23]);
	gluSphere(qudric, 0.3f, 100, 100);
	glPopMatrix();
		
	/*Animation comes here*/
	update();
	/*
	push the everything to graphhics pipeline.
	it requires for single buffer only but for double buffer its not required.
	glFlush();
	*/

	/*
	Much more closer to WGL. Its Win32 API.
	*/
	SwapBuffers(gblHdc);
}

void update(void)
{
	angleXLight = angleXLight > 360 ? 0.0 : angleXLight + 0.3f;
	angleYLight = angleYLight > 360 ? 0.0 : angleYLight + 0.3f;
	angleZLight = angleZLight > 360 ? 0.0 : angleZLight + 0.3f;
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
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
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