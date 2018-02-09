
#include"myHeader.h"


bool				g_bIsWinActive = false;
bool				g_bIsEscapeKeyPressed = false;
bool				g_bIsFullScreen = false;
bool				g_bIsLight = false;
int					g_Earth = 0;
HWND				g_hWnd = NULL;
WINDOWPLACEMENT		g_hPrevWndPlace;
DWORD				g_dwStyle = NULL;
cLog				&fout = cLog::getLogHandle("Log.txt");
float				g_angle = 0.0f;
float				g_lx = 0.0f;
float				g_lz = -1.0f;
float				g_x = 0.0f;
float				g_z = 5.0f;
float				eyex = 0.0f;
float				eyey = 0.0f;
float				eyez = 0.0f;
float				centerx = 0.0f;
float				centery = 0.0f;
float				centerz = 0.0f;
float				upmtrx = 0.0f;
float				upmtry = 0.0f;
float				upmtrz = 1.0f;

int WINAPI WinMain(HINSTANCE hPrevInstance, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	fout << "Begin WinMain";

	void		doInitialize(void);
	void		doUnInitialize(void);
	void		doRendering(void);

	WNDCLASSEX	hWndClassEx;
	HWND		hWnd;
	MSG			hMsg;
	TCHAR		szClassName[] = TEXT("LIGHT");
	bool		bDone = false;

	hWndClassEx.cbClsExtra = 0;
	hWndClassEx.cbSize = sizeof(WNDCLASSEX);
	hWndClassEx.cbWndExtra = 0;
	hWndClassEx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	hWndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	hWndClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	hWndClassEx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	hWndClassEx.hInstance = hInstance;
	hWndClassEx.lpfnWndProc = WndProc;
	hWndClassEx.lpszClassName = szClassName;
	hWndClassEx.lpszMenuName = NULL;
	hWndClassEx.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

	RegisterClassEx(&hWndClassEx);

	hWnd = CreateWindowEx(
		WS_EX_APPWINDOW,
		szClassName,
		TEXT("3D Rotation of Pyramid and Cube under Light"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		0,
		0,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	g_hWnd = hWnd;
	doInitialize();
	ShowWindow(hWnd, nCmdShow);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	//Game Loop
	while (!bDone)
	{
		if (PeekMessage(&hMsg, NULL, NULL, NULL, PM_REMOVE)) {
			if (hMsg.message == WM_QUIT)
				bDone = true;
			else
			{
				TranslateMessage(&hMsg);
				DispatchMessage(&hMsg);
			}
		}
		else {
			if (g_bIsWinActive == true) {
				if (g_bIsEscapeKeyPressed == true)
					bDone = true;
				else
				{
					display();
					update();
				}

			}
		}
	}
	doUnInitialize();
	fout << "End WinMain";
	cLog::distroyLog();
	return ((int)hMsg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	void doToggleFullScreen(void);
	char sz[2014];
	switch (iMsg)
	{
	case WM_CREATE:
		fout << "********************************************************************************************************";
		fout << "Assignment on 13Nov2017: Basic assignment of Light, explaining how to define Light and Material parameters. ";
		fout << "********************************************************************************************************";
		break;
	case WM_ACTIVATE:
		fout << "WM_ACTIVATE";
		if (HIWORD(wParam) == 0)
			g_bIsWinActive = true;
		else
			g_bIsWinActive = false;
		break;
	case WM_SIZE:
		fout << "WM_SIZE";
		doResizing(LOWORD(lParam), HIWORD(lParam));
		break;
	/* WM_CHAR:
		sprintf(sz, "\n WM_CHAR: %d", g_Earth);
		fout << sz;
		switch (wParam)
		{
		case 'E':
			g_Earth = (g_Earth + 3) % 360;   //Evolving 
			break;
		case 'e':
			g_Earth = (g_Earth - 3) % 360;
			break;
		default:
			break;
		}
		
		break*/

	case WM_KEYDOWN:
		fout << "\tWM_KEYDOWN";
		switch (wParam)
		{
		case VK_ESCAPE:
			fout << "\tVK_ESCAPE";
			g_bIsEscapeKeyPressed = true;
			break;
		case VK_UP:
			g_lx = g_lx+ 0.1f;
			MessageBox(NULL, TEXT("VK_UP"), TEXT("TEST"), MB_OK);
			break;
		case VK_DOWN:
			g_lz = g_lz + (-0.1f);
			break;
		case VK_RIGHT:
			g_x = g_x + 0.1f;
			break;
		case VK_LEFT:
			g_z = g_z + 0.1f;
			break;
		case VK_END:
			g_angle = g_angle + 0.1f;
			break;
		case 0x46:
			fout << "\tF/f";

			doToggleFullScreen();
			g_bIsFullScreen = g_bIsFullScreen ? false : true;
			break;
		case 0x4C:
			fout << "\tL/l";

			if (g_bIsLight)
				glDisable(GL_LIGHTING);
			else
				glEnable(GL_LIGHTING);
			g_bIsLight = g_bIsLight ? false : true;
			break;

		case 0x51: //q
			eyex = eyex + 0.01;
			break;
		case 0x57: //w
			eyey = eyey + 0.01;
			break;

		case 0x45: //e
			eyez = eyez + 0.01;
			break;

		case 0x52: //r 
			eyex = eyex - 0.01;
			break;

		case 0x54: //t
			eyey = eyey - 0.01;
			break;

		case 0x59: //y
			eyez = eyez - 0.01;
			break;

		case 0x41: //a
			centerx = centerx + 1;
			break;
		
		case 0x53: //s
			centery = centery + 1;
			break;

		case 0x44: //d
			centerz = centerz + 1;
			break;

		case 0x47: //g
			centerx = centerx - 1;
			break;

		case 0x48: //h
			centery = centery - 1;
			break;

		case 0x4A: //j
			centerz = centerz - 1;
			break;

		case 0x5A: //z
			upmtrx = upmtrx + 1;
			break;

		case 0x58: //x
			upmtry = upmtry + 1;
			break;

		case 0x43: //c
			upmtrz = upmtrz + 1;
			break;

		case 0x56: //v
			upmtrx = upmtrx - 1;
			break;

		case 0x42: //b
			upmtry = upmtry - 1;
			break;

		case 0x4E: //n
			upmtrz = upmtrz - 1;
			break;

		default:
			break;
		}
		break;

	case WM_LBUTTONDOWN:
		fout << "WM_LBUTTONDOWN";
		break;
	case WM_CLOSE:
		fout << "WM_CLOSE";
		PostQuitMessage(0);
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

void doToggleFullScreen(void)
{
	MONITORINFO hMi;
	BOOL bIsMonitorInfo = FALSE;
	BOOL bIsWindowPlacement = FALSE;
	HMONITOR hMonitor;


	fout << "Begin doToggleFullScreen";
	if (!g_bIsFullScreen)
	{
		g_dwStyle = GetWindowLong(g_hWnd, GWL_STYLE);
		if (g_dwStyle & WS_OVERLAPPEDWINDOW)
		{
			fout << "\tWS_OVERLAPPEDWINDOW style is present";
			hMi = { sizeof(MONITORINFO) };
			bIsWindowPlacement = GetWindowPlacement(g_hWnd, &g_hPrevWndPlace);
			hMonitor = MonitorFromWindow(g_hWnd, MONITOR_DEFAULTTOPRIMARY);
			bIsMonitorInfo = GetMonitorInfo(hMonitor, &hMi);
			if (bIsWindowPlacement && bIsMonitorInfo)
			{
				fout << "\tHandle of WINDOWPLACEMENT and HMONITOR are obtained successfully";
				SetWindowLong(g_hWnd, GWL_STYLE, g_dwStyle &~WS_OVERLAPPEDWINDOW);
				fout << "\tHandle of WINDOWPLACEMENT and HMONITOR are obtained successfully";
				SetWindowPos(g_hWnd,
					HWND_TOP,
					hMi.rcMonitor.left,
					hMi.rcMonitor.top,
					hMi.rcMonitor.right - hMi.rcMonitor.left,
					hMi.rcMonitor.bottom - hMi.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED
				);
				fout << "\tWindow position has changed successfully";
			}
			doResizing(hMi.rcMonitor.right - hMi.rcMonitor.left, hMi.rcMonitor.bottom - hMi.rcMonitor.top);
			fout << "\tViewport has changed to see the full screen";
			ShowCursor(FALSE);
			fout << "\tDisplay of cursor is prohibited";
		}
	}
	else
	{
		SetWindowLong(g_hWnd, GWL_STYLE, g_dwStyle & WS_OVERLAPPEDWINDOW);
		fout << "\tWS_OVERLAPPEDWINDOW style has set";
		SetWindowPlacement(g_hWnd, &g_hPrevWndPlace);
		SetWindowPos(g_hWnd,
			HWND_TOP,
			0,
			0,
			WIN_WIDTH,
			WIN_HEIGHT,
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED
		);
		fout << "\tWindow position has changed successfully";
		ShowCursor(TRUE);
		fout << "\tDisplay of cursor is permitted";
	}
	fout << "End doToggleFullScreen";
	return;
}

