#include <Windows.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HBRUSH wndBgHBrush;
BOOL gbFullscreen;
DWORD dwStyle;
WINDOWPLACEMENT wndPlacement = { sizeof(WINDOWPLACEMENT) };

int WINAPI WinMain(HINSTANCE currentInstance, HINSTANCE prevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	WNDCLASSEX wndClass;
	HWND hwnd;
	MSG msg;
	TCHAR appName[] = TEXT("WM Paint");
	TCHAR windowTitle[] = TEXT("WM Paint");
	wndBgHBrush = CreateSolidBrush(RGB(0, 0, 0));
	gbFullscreen = false;

	wndClass.cbSize = sizeof(wndClass);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	//why not address of WndProc
	wndClass.lpfnWndProc = &WndProc;
	wndClass.hInstance = currentInstance;
	wndClass.hIcon = wndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = wndBgHBrush;
	wndClass.lpszClassName = appName;
	wndClass.lpszMenuName = NULL;

	RegisterClassEx(&wndClass);

	hwnd = CreateWindow(appName, windowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, currentInstance, NULL);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}


	return ((int) msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rc;
	TCHAR keyPressed = (TCHAR) wParam;
	void ToggleFullScreen(HWND hwND, BOOL toggleState);

	switch (iMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0L;

	case WM_KEYDOWN:

		switch (keyPressed)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		case 'F':
			gbFullscreen = !gbFullscreen; // toggle curren state
			ToggleFullScreen(hwnd, gbFullscreen);
			break;

		case 'R':
			wndBgHBrush = CreateSolidBrush(RGB(221, 69, 53));
			break;

		case 'G':
			wndBgHBrush = CreateSolidBrush(RGB(49, 226, 58));
			break;

		case 'B':
			wndBgHBrush = CreateSolidBrush(RGB(49, 126, 226));
			break;

		case 'C':
			wndBgHBrush = CreateSolidBrush(RGB(0, 255, 255));
			break;

		case 'M':
			wndBgHBrush = CreateSolidBrush(RGB(255, 0, 255));
			break;

		case 'Y':
			wndBgHBrush = CreateSolidBrush(RGB(226, 220, 36));
			break;

		case 'W':
			wndBgHBrush = CreateSolidBrush(RGB(255, 255, 255));
			break;

		default:
			wndBgHBrush = CreateSolidBrush(RGB(0, 0, 0));
			break;
		}
		InvalidateRect(hwnd, NULL, TRUE);
		return 0L;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		GetClientRect(hwnd, &rc);
		FillRect(hdc, &rc, wndBgHBrush);
		EndPaint(hwnd, &ps);
		return 0L;
	}

	return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullScreen(HWND hwnd, BOOL toggleFullScreen)
{
	MONITORINFO mi;

	if (toggleFullScreen == true)
	{
		dwStyle = GetWindowLong(hwnd, GWL_STYLE);
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO) };
			BOOL bWindowPlacement = GetWindowPlacement(hwnd, &wndPlacement);
			BOOL bMonitorInfo = GetMonitorInfo(MonitorFromWindow(hwnd, MONITORINFOF_PRIMARY), &mi);

			if (bWindowPlacement && bMonitorInfo) 
			{
				SetWindowLong(hwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(hwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, (mi.rcMonitor.right - mi.rcMonitor.left), (mi.rcMonitor.bottom - mi.rcMonitor.top), SWP_NOZORDER | SWP_FRAMECHANGED);
				
				ShowCursor(FALSE);
			}
		}		
	}
	else
	{
		SetWindowLong(hwnd, GWL_STYLE, dwStyle & WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(hwnd, &wndPlacement);
		SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
	}
}