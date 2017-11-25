#include <windows.h>
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

GLfloat angleRedLight    = 0.0f;
GLfloat angleGreenLight	 = 0.0f;
GLfloat angleBlueLight	 = 0.0f;

int angleSphere = 0.0f;

GLfloat light1_ambience[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat light1_defiuse[]  = { 1.0f, 0.0f, 0.0f, 1.0f };
GLfloat light1_spcular[]  = { 1.0f, 0.0f, 0.0f, 1.0f };
GLfloat redlight_position[] = { 1.0f, 1.0f, 1.0f, 0.0f };

GLfloat light2_ambience[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat light2_defiuse[]  = { 0.0f, 1.0f, 0.0f, 1.0f };
GLfloat light2_spcular[]  = { 0.0f, 1.0f, 0.0f, 1.0f };
GLfloat greenlight_position[] = { 1.0f, 1.0f, 1.0f, 0.0f };

GLfloat light3_ambience[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat light3_defiuse[] = { 0.0f, 0.0f, 1.0f, 1.0f };
GLfloat light3_spcular[] = { 0.0f, 0.0f, 1.0f, 1.0f };
GLfloat bluelight_position[] = { 1.0f, 1.0f, 1.0f, 0.0f };

GLfloat material_ambience[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat material_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat material_defuse[]   = { 1.0, 1.0, 1.0, 1.0 };
GLfloat material_shiness[] = { 50.0 };

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
		TEXT("OpenGL Fixed Fucntion Pipeline : 3 color lighting"),
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

	glLightfv(GL_LIGHT0, GL_AMBIENT, light1_ambience);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light1_defiuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light1_spcular);
	//glLightfv(GL_LIGHT0, GL_POSITION, redlight_position);
	
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT1, GL_AMBIENT, light2_ambience);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light2_defiuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light2_spcular);
	//glLightfv(GL_LIGHT1, GL_POSITION, greenlight_position);

	glEnable(GL_LIGHT1);

	glLightfv(GL_LIGHT2, GL_AMBIENT, light3_ambience);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, light3_defiuse);
	glLightfv(GL_LIGHT2, GL_SPECULAR, light3_spcular);
	//glLightfv(GL_LIGHT2, GL_POSITION, bluelight_position);

	glEnable(GL_LIGHT2);

	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambience);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_defuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shiness);

	// warmup
	resize(WIN_WIDTH, WIN_HEIGHT);
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

	glPushMatrix();
	gluLookAt(0.0f, 0.0f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	glPushMatrix();
	glRotatef(angleRedLight, 1.0f, 0.0f, 0.0f);
	redlight_position[1] = angleRedLight;
	glLightfv(GL_LIGHT0, GL_POSITION, redlight_position);
	glPopMatrix();

	glPushMatrix();
	glRotatef(angleGreenLight, 0.0f, 1.0f, 0.0f);
	greenlight_position[0] = angleGreenLight;
	glLightfv(GL_LIGHT1, GL_POSITION, greenlight_position);
	glPopMatrix();

	glPushMatrix();
	glRotatef(angleBlueLight, 0.0f, 0.0f, 1.0f);
	bluelight_position[0] = angleBlueLight;
	glLightfv(GL_LIGHT2, GL_POSITION, bluelight_position);
	glPopMatrix();

	glTranslatef(0.0f, 0.0f, -3.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	glRotatef((GLfloat)angleSphere, 0.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	gluSphere(qudric, 0.75f, 30, 30);

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
	angleRedLight = angleRedLight > 360 ? 0.0 : angleRedLight + 1.0f;
	angleGreenLight = angleGreenLight > 360 ? 0.0 : angleGreenLight + 1.0f;
	angleBlueLight = angleBlueLight > 360 ? 0.0 : angleBlueLight + 1.0f;
	angleSphere = (angleSphere + 1) % 3600;
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