#include <windows.h>

LRESULT CALLBACK WndCallackProc(HWND, UINT, WPARAM, LPARAM);
HWND glWndHwnd;
bool glToggleFullScreen;
WINDOWPLACEMENT glWndPrevPlacement;
DWORD glDwStyle;

int WINAPI WinMain(HINSTANCE currentInstance, HINSTANCE prevInstance, LPSTR lpszCmdLine, int cmdShow)
{
	WNDCLASSEX wndClass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("MyWindow");
	HBRUSH hbWndBackground = CreateSolidBrush(RGB(0, 0, 0));
	glToggleFullScreen = false;

	// Initialize Window Object properies
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.lpfnWndProc = WndCallackProc;
	wndClass.hInstance = currentInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = hbWndBackground;
	wndClass.lpszClassName = szAppName;
	wndClass.lpszMenuName = NULL;

	// register window object
	RegisterClassEx(&wndClass);

	// create window
	hwnd = CreateWindow(szAppName,
		TEXT("Full Screen Demo"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		currentInstance,
		NULL);

	// render window
	ShowWindow(hwnd, cmdShow);
	UpdateWindow(hwnd);
	glWndHwnd = hwnd;

	// message loop
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return ((int)msg.wParam);
}

LRESULT CALLBACK WndCallackProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	void toggleFullscreen();

	switch (iMsg)
	{
	case WM_CREATE:
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'F':
			glToggleFullScreen = !glToggleFullScreen;
			toggleFullscreen();
			break;
		
		case VK_ESCAPE:
			DestroyWindow(glWndHwnd);
			break;
		
		default:
			break;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	

	return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}


void toggleFullscreen()
{
	MONITORINFO monInfo;
	HMONITOR hMonitor;
	monInfo = { sizeof(MONITORINFO) };
	hMonitor = MonitorFromWindow(glWndHwnd, MONITORINFOF_PRIMARY);

	if (glToggleFullScreen == true) {
		glDwStyle = GetWindowLong(glWndHwnd, GWL_STYLE);
		if(glDwStyle & WS_OVERLAPPEDWINDOW)
		{
			glWndPrevPlacement = { sizeof(WINDOWPLACEMENT) };
			if (GetWindowPlacement(glWndHwnd, &glWndPrevPlacement) && GetMonitorInfo(hMonitor, &monInfo))
			{
				SetWindowLong(glWndHwnd, GWL_STYLE, glDwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(glWndHwnd, HWND_TOP, monInfo.rcMonitor.left, monInfo.rcMonitor.top, (monInfo.rcMonitor.right- monInfo.rcMonitor.left), (monInfo.rcMonitor.bottom - monInfo.rcMonitor.top), SWP_NOZORDER | SWP_FRAMECHANGED);
			}

			ShowCursor(FALSE);
		}
	}
	else 
	{
		SetWindowLong(glWndHwnd, GWL_STYLE, glDwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(glWndHwnd, &glWndPrevPlacement);
		SetWindowPos(glWndHwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		
		ShowCursor(TRUE);
	}

}