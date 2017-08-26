#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <math.h>

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

int shoulder = 0;
int elbow = 0;

GLfloat angel = 45.0f;
int glwidth, glheight;
GLUquadric *qudric = NULL;

GLfloat xLeft = -1.5f, xRight = 1.5f;
GLfloat yTop = 1.0f, yBottom = -1.0f;

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
		TEXT("OpenGl Primitive Shapes"),
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

		case 'F':
		case 'f':
			gblFullScreen = !gblFullScreen;
			toggleFullScreen();
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

	/*state function*/
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glClearDepth(1.0f); // set depth buffer
	glEnable(GL_DEPTH_TEST | GL_BLEND); // enable depth testing
	glDepthFunc(GL_LEQUAL); // type of depth testing (may be Direxct3D, Vulkcan doesnt require this test)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

void drawGrid()
{
	int i;
	GLfloat x = -1.5;
	
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(xLeft, 0, 0);
	glVertex3f(xRight, 0, 0);

	for (int i = 0; i <= 4; i++) 
	{
		glVertex3f(x, yTop, 0);
		glVertex3f(x, yBottom, 0);
		x = x + 1.0f;
	}
	
	glVertex3f(xLeft, yTop, 0);
	glVertex3f(xRight, yTop, 0);

	glVertex3f(xLeft, yBottom, 0);
	glVertex3f(xRight, yBottom, 0);

	glEnd();
}

void drawDots(GLfloat xl, GLfloat xr, GLfloat yt, GLfloat yb, int devisions)
{
	int x, y;
	GLfloat xCord, yCord, nextX, nextY;

	yCord = yt;
	nextX = (fabs(xr) - fabs(xl)) / devisions;
	nextY = (fabs(yt) - fabs(yb)) / devisions;

	glLineWidth(2.0f);
	glBegin(GL_POINTS);
		for (y = 0; y <= devisions; y++)
		{
			xCord = xl;
			for (x = 0; x <= devisions; x++)
			{
				glVertex3f(xCord, yCord, 0.0f);
				xCord = xCord + nextX;
			}
			yCord = yCord - nextY;
		}
	glEnd();
}

void drawShape2(GLfloat xl, GLfloat xr, GLfloat yt, GLfloat yb, int devisions)
{
	GLfloat diff = fabs((fabs(yt) - fabs(yb)) / devisions);

	glBegin(GL_LINE_STRIP);
		glVertex3f(xl, yt, 0.0f);
		glVertex3f(xl, yb, 0.0f);
		glVertex3f(xr, yt, 0.0f);
		glVertex3f(xl, yt, 0.0f);
	
		glVertex3f(xl, yt - (diff * 1), 0.0f);
		glVertex3f(xr, yt - (diff * 1), 0.0f);
		glVertex3f(xl + (diff * 1), yb, 0.0f);
		glVertex3f(xl + (diff * 1), yt, 0.0f);
		glVertex3f(xl, yt - (diff * 1), 0.0f);

		glVertex3f(xl, yt - (diff * 2), 0.0f);
		glVertex3f(xr, yt - (diff * 2), 0.0f);
		glVertex3f(xl + (diff * 2), yb, 0.0f);
		glVertex3f(xl + (diff * 2), yt, 0.0f);
		glVertex3f(xl, yt - (diff * 2), 0.0f);

	glEnd();
}

void drawShape3(GLfloat xl, GLfloat xr, GLfloat yt, GLfloat yb, int devisions)
{
	GLfloat diff = (fabs(yt) - fabs(yb)) / devisions;
	glBegin(GL_LINE_LOOP);
		glVertex3f(xl, yt, 0.0f);
		glVertex3f(xl, yb, 0.0f);
		glVertex3f(xr, yb, 0.0f);
		glVertex3f(xr, yt, 0.0f);
		glVertex3f(xl, yt, 0.0f);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(xl, yt - (diff * 1), 0.0f);
		glVertex3f(xr, yt - (diff * 1), 0.0f);
		glVertex3f(xl, yt - (diff * 2), 0.0f);
		glVertex3f(xr, yt - (diff * 2), 0.0f);

		glVertex3f(xl + (diff * 1), yt, 0.0f);
		glVertex3f(xl + (diff * 1), yb, 0.0f);
		glVertex3f(xl + (diff * 2), yt, 0.0f);
		glVertex3f(xl + (diff * 2), yb, 0.0f);
	glEnd();
}

void drawShape4(GLfloat xl, GLfloat xr, GLfloat yt, GLfloat yb, int devisions)
{
	drawShape2(xl, xr, yt, yb, devisions);
	glBegin(GL_LINE_LOOP);
		glVertex3f(xl, yb, 0.0f);
		glVertex3f(xr, yb, 0.0f);
		glVertex3f(xr, yt, 0.0f);
	glEnd();
}

void drawShape5(GLfloat xl, GLfloat xr, GLfloat yt, GLfloat yb, int devisions)
{
	GLfloat diff = fabs((fabs(yt) - fabs(yb)) / devisions);

	glBegin(GL_LINE_LOOP);
		glVertex3f(xl, yt, 0.0f);
		glVertex3f(xr, yt, 0.0f);
		glVertex3f(xr, yb, 0.0f);
		glVertex3f(xl, yb, 0.0f);
		glVertex3f(xl, yt, 0.0f);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(xl, yt, 0.0f);
		glVertex3f(xl + (diff * 1), yb, 0.0f);
		glVertex3f(xl, yt, 0.0f);
		glVertex3f(xl + (diff *2), yb, 0.0f);
		glVertex3f(xl, yt, 0.0f);
		glVertex3f(xr, yb, 0.0f);
	glEnd();
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -3.0f);

	//drawGrid();

	drawDots(xLeft, (xLeft - 1.0f), yTop, (yTop - 1.0f), 3);

	glTranslatef(0.2f, 0, 0);
	drawShape2((xLeft + 1.0f), (xRight - 1.0f), yTop, (yTop - 1.0f), 3);

	glTranslatef(0.2f, 0, 0);
	drawShape3(xRight-1, xRight, yTop, (yTop - 1.0f), 3);

	glTranslatef(-0.4f, -0.2f, 0);
	drawShape4(xLeft, (xLeft + 1.0f), (yTop - 1.0f), yBottom, 3);

	glTranslatef(0.2f, 0, 0);
	drawShape5((xLeft + 1.0f), (xRight - 1.0f), (yTop - 1.0f), yBottom, 3);

	SwapBuffers(gblHdc);
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
	
	glwidth = width;
	glheight = height;

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(angel, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
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