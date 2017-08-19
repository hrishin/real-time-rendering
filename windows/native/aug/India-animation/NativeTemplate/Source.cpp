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

int glwidth, glheight;
GLUquadric *qudric = NULL;

GLfloat firstLetterPosition = -0.3f;
GLfloat secondLetterPosition = 2.0f;
GLfloat thirdLetterPositin =  2.0f;
GLfloat orangeColor[] = {0.0f, 0.0f, 0.0f};
GLfloat greenColor[] = {0.0f, 0.0f, 0.0f};
GLfloat fourthLetterPosition = -2.0f;
GLfloat flagX1Position = -1.5f;
GLfloat flagX2Position = -1.5f;

int animationTime = 2000;

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
		TEXT("India"),
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
	glEnable(GL_DEPTH_TEST); // enable depth testing
	glDepthFunc(GL_LEQUAL); // type of depth testing (may be Direxct3D, Vulkcan doesnt require this test)
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
	glVertex3f(-1.5, 0, 0);
	glVertex3f(1.5, 0, 0);

	for (int i = 0; i <= 5; i++) 
	{
		glVertex3f(x, -1, 0);
		glVertex3f(x, 1, 0);
		x = x + 0.6;
	}
	
	glVertex3f(-1.5, 0.8, 0);
	glVertex3f(1.5, 0.8, 0);

	glVertex3f(-1.5, -0.8, 0);
	glVertex3f(1.5, -0.8, 0);

	glEnd();
}

void drawFirstLetter()
{
	glBegin(GL_LINES);
		glColor3f(1.0, 0.6, 0.2);
		glVertex3f(-1.5f+0.3, 0.8, 0.0);
		glColor3f(0.07, 0.5, 0.2);
		glVertex3f(-1.5f+0.3, -0.8, 0.0);
	glEnd();

}

void drawSecondLetter()
{
	glBegin(GL_LINES);
		glColor3f(1.0, 0.6, 0.2);
		glVertex3f(-0.9, 0.8, 0.0);
		glColor3f(0.07, 0.5, 0.2);
		glVertex3f(-0.9, -0.8, 0.0);

		glColor3f(1.0, 0.6, 0.2);
		glVertex3f(-0.9 + 0.0, 0.8, 0.0);
		glColor3f(0.07, 0.5, 0.2);
		glVertex3f(-0.9 + 0.4, -0.8, 0.0);

		glColor3f(1.0, 0.6, 0.2);
		glVertex3f(-0.9 + 0.4, 0.8, 0.0);
		glColor3f(0.07, 0.5, 0.2);
		glVertex3f(-0.9 + 0.4, -0.8, 0.0);
	glEnd();
}

void drawThirdLetter()
{
	glBegin(GL_LINES);
		// top line
		glColor3f(orangeColor[0], orangeColor[1], orangeColor[2]);
		glVertex3f(-0.3, 0.8, 0.0);
		glVertex3f(0.3 - 0.05, 0.8, 0.0);

		// left line
		glColor3f(orangeColor[0], orangeColor[1], orangeColor[2]);
		glVertex3f(0.3 - 0.05, 0.8, 0.0);
		glColor3f(greenColor[0], greenColor[1], greenColor[2]);
		glVertex3f(0.3 - 0.05, -0.8, 0.0);

		//bottom line
		glColor3f(greenColor[0], greenColor[1], greenColor[2]);
		glVertex3f(-0.3, -0.8, 0.0);
		glVertex3f(0.3 - 0.05, -0.8, 0.0);

		// right line
		glColor3f(orangeColor[0], orangeColor[1], orangeColor[2]);
		glVertex3f(-0.3 + 0.05, 0.8, 0.0);
		glColor3f(greenColor[0], greenColor[1], greenColor[2]);
		glVertex3f(-0.3 + 0.05, -0.8, 0.0);
	glEnd();
}

void drawFourthLetter()
{
	glBegin(GL_LINES);
		glColor3f(1.0, 0.6, 0.2);
		glVertex3f(0.3 + 0.3, 0.8, 0.0);
		glColor3f(0.07, 0.5, 0.2);
		glVertex3f(0.3 + 0.3, -0.8, 0.0);
	glEnd();
}

void drawFifthLetter()
{
	glBegin(GL_LINES);
		glColor3f(1.0, 0.6, 0.2);
		glVertex3f(1.2, 0.8, 0.01);
		glColor3f(0.07, 0.5, 0.2);
		glVertex3f(0.9, -0.8, 0.01);

		glColor3f(1.0, 0.6, 0.2);
		glVertex3f(1.2, 0.8, 0.01);
		glColor3f(0.07, 0.5, 0.2);
		glVertex3f(1.5, -0.8, 0.01);
	glEnd();
}

void drawFlag()
{
	glBegin(GL_LINES);
		glColor3f(1.0, 0.6, 0.2);
		glVertex3f(flagX1Position, 0.015, 0.0);
		glVertex3f(flagX2Position, 0.015, 0.0);
		
		glColor3f(1.0, 1.0, 1.0);
		glVertex3f(flagX1Position, 0.0, 0.0);
		glVertex3f(flagX2Position, 0.0, 0.0);

		glColor3f(0.07, 0.5, 0.2);
		glVertex3f(flagX1Position, -0.015, 0.0);
		glVertex3f(flagX2Position, -0.015, 0.0);
	glEnd();
}

void update()
{
	if (firstLetterPosition <= 0.0f)
		firstLetterPosition = firstLetterPosition + 0.00015;

	if (firstLetterPosition >= 0.0f && secondLetterPosition >= 0.0f)
		secondLetterPosition = secondLetterPosition - 0.001;
	
	if (secondLetterPosition <= 0.0f && thirdLetterPositin >= 0.0f) {
		orangeColor[0] = orangeColor[0] + 0.0005f;
		orangeColor[1] = orangeColor[1] + 0.0003f;
		orangeColor[2] = orangeColor[2] + 0.0001f;
		greenColor[0] = greenColor[0] + 0.000035;
		greenColor[1] = greenColor[1] + 0.00025;
		greenColor[2] = greenColor[2] + 0.0001;
		thirdLetterPositin = thirdLetterPositin - 0.001;
	}

	if (thirdLetterPositin <= 0.0 && fourthLetterPosition <= 0.0f)
		fourthLetterPosition = fourthLetterPosition + 0.001;

	if (fourthLetterPosition >= 0.0f && fifthLetterPosition >= 0.0f)
		fifthLetterPosition = fifthLetterPosition - 0.00050;

	if (fifthLetterPosition <= 0.0f && flagX1Position <= 1.35f) {
		flagX1Position = flagX1Position + 0.0050;
	}

	if (flagX1Position >= 1.35f && flagX2Position <= 1.05f) {
		flagX2Position = flagX2Position + 0.0050;
	}
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(-0.14f, 0, -2);

	//drawGrid();

	glLineWidth(5.0f);

	glPushMatrix();
		glTranslatef(firstLetterPosition, 0.0f, 0.0f);
		drawFirstLetter();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.0f, secondLetterPosition, 0.0f);
		drawSecondLetter();
	glPopMatrix();

	glPushMatrix();
		drawThirdLetter();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.0f, fourthLetterPosition, 0.0f);
		drawFourthLetter();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(fifthLetterPosition, 0.0f, 0.0f);
		drawFifthLetter();
	glPopMatrix();

	glPushMatrix();
		drawFlag();
	glPopMatrix();

	update();

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

	gluPerspective(61.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
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