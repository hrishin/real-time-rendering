#include <windows.h>
#include <stdio.h>
#include <d3d11.h>

#pragma comment (lib, "d3d11.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

LRESULT CALLBACK WndCallbackProc(HWND, UINT, WPARAM, LPARAM);
void LogMessage(const char *);

FILE *gpFile = NULL;

HWND gblHwnd = NULL;

DWORD gblDwStyle;
WINDOWPLACEMENT gblWindowPlcaement = { sizeof(WINDOWPLACEMENT) };

bool gblFullScreen = false;
bool gblIsEscPressed = false;
bool gblActiveWindow = false;

float gClearColor[4];
IDXGISwapChain *gpIDXGISwapChain = NULL;
ID3D11Device *gpID3D11Device = NULL;
ID3D11DeviceContext *gpID3D11DeviceContext = NULL;
ID3D11RenderTargetView *gpID3D11RenderTargetView = NULL;

int WINAPI WinMain(HINSTANCE currentHInstance, HINSTANCE prevHInstance, LPSTR lpszCmdLune, int iCmdShow)
{
	// function prototype
	HRESULT initialize(void);
	void display(void);
	void uninitialize(void);

	// variables declartion
	WNDCLASSEX wndClass;
	HWND hwnd;
	MSG msg;
	int iScreenWidth, iScreenHeight;
	TCHAR szClassName[] = TEXT("Direct3D11");
	bool bDone = false;

	if (fopen_s(&gpFile, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Log file cant be created \n existing"), TEXT("ERROR"), MB_OK | MB_TOPMOST | MB_ICONSTOP);
		exit(0);
	}
	else 
	{
		fprintf(gpFile, "Log file created succcessfully \n");
		fclose(gpFile);
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
		TEXT("Direct3D11 Windows"),
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

	// render window
	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// initialize
	HRESULT hr;
	hr = initialize();
	if (FAILED(hr)) {
		LogMessage("Initiliazation failed. Exiting now \n");
		DestroyWindow(hwnd);
		hwnd = NULL;
	}
	else 
	{
		LogMessage("Initiliazation succeeded \n");
	}

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

	return (int)msg.wParam;
}

LRESULT CALLBACK WndCallbackProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HRESULT resize(int, int);
	void uninitialize(void);
	void toggleFullScreen(void);

	HRESULT hr;

	switch (iMsg)
	{
	case WM_ACTIVATE:
		gblActiveWindow = (HIWORD(wParam) == 0);
		break;

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

		default:
			break;
		}
		break;

	case WM_SIZE:
		if (gpID3D11DeviceContext)
		{
			hr = resize(LOWORD(lParam), HIWORD(lParam));
			if (FAILED(hr)) {
				LogMessage("resize() failed \n");
				return hr;
			}
			else
			{
				LogMessage("resize() succeeded \n");
			}
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

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
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

HRESULT initialize(void)
{
	HRESULT resize(int, int);
	
	// variable declarations
	HRESULT hr;
	D3D_DRIVER_TYPE d3dDriverType;
	D3D_DRIVER_TYPE d3dDriverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE, 
		D3D_DRIVER_TYPE_WARP, 
		D3D_DRIVER_TYPE_REFERENCE, 
	};
	D3D_FEATURE_LEVEL d3dFeatureLevelRequired = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL d3dFeatureLevelAcquired = D3D_FEATURE_LEVEL_10_0; // default, lowest
	UINT createDeviceFlags = 0;
	UINT numDriverTypes = 0;
	UINT numFeatureLevels = 1; // based upon d3dFeatureLevelRequired

	numDriverTypes = sizeof(d3dDriverTypes) / sizeof(d3dDriverTypes[0]);

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	ZeroMemory((void *)&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	dxgiSwapChainDesc.BufferCount = 1;
	dxgiSwapChainDesc.BufferDesc.Width = WIN_WIDTH;
	dxgiSwapChainDesc.BufferDesc.Height = WIN_HEIGHT;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.OutputWindow = gblHwnd;
	dxgiSwapChainDesc.SampleDesc.Count = 1;
	dxgiSwapChainDesc.SampleDesc.Quality = 0;
	dxgiSwapChainDesc.Windowed = TRUE;

	for (UINT index = 0; index < numDriverTypes; index++)
	{
		d3dDriverType = d3dDriverTypes[index];
		hr = D3D11CreateDeviceAndSwapChain(
			NULL,                          
			d3dDriverType,                 
			NULL,                          
			createDeviceFlags,             
			&d3dFeatureLevelRequired,     
			numFeatureLevels,              
			D3D11_SDK_VERSION,             
			&dxgiSwapChainDesc,            
			&gpIDXGISwapChain,             
			&gpID3D11Device,               
			&d3dFeatureLevelAcquired,     
			&gpID3D11DeviceContext);       
		
		if (SUCCEEDED(hr))
		{
			break;
		}
	}

	if (FAILED(hr))
	{
		LogMessage("CreateSwapChain() failed \n");
		return hr;
	}
	else 
	{
		LogMessage("CreateSwapChain() succeeded \n");
		
		LogMessage("Choosen driver is of ");
		if (d3dDriverType == D3D_DRIVER_TYPE_HARDWARE)
		{
			LogMessage("Hardware type \n");
		}
		else if (d3dDriverType == D3D_DRIVER_TYPE_WARP)
		{
			LogMessage("Warp type \n");
		}
		else if (d3dDriverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			LogMessage("Reference type \n");
		}
		else
		{
			LogMessage("Unknown type \n");
		}

		LogMessage("Supported highest level feature is ");
		if (d3dFeatureLevelAcquired == D3D_FEATURE_LEVEL_11_0)
		{
			LogMessage("11.0 \n");
		}
		else if (d3dFeatureLevelAcquired == D3D_FEATURE_LEVEL_10_1)
		{
			LogMessage("10.1 \n");
		}
		else if (d3dFeatureLevelAcquired == D3D_FEATURE_LEVEL_10_0)
		{
			LogMessage("10.0 \n");
		}
		else
		{
			LogMessage("Unknown \n");
		}
	}

	// d3d clear color (blue)
	gClearColor[0] = 0.0f;
	gClearColor[1] = 0.0f;
	gClearColor[2] = 1.0f;
	gClearColor[3] = 1.0f;

	hr = resize(WIN_WIDTH, WIN_HEIGHT);
	if (FAILED(hr))
	{
		LogMessage("resize() failed \n");
		return hr;
	}
	else
	{
		LogMessage("resize() succeeded \n");
	}

	return S_OK;
}

/*
Very important for Dirext X not for OpenGL
becuase DirextX is not state machine and change in windows resize empose
re-rendering of Direct X (even for Vulcan) scenes.
*/
HRESULT resize(int width, int height)
{
	HRESULT hr = S_OK;
	ID3D11Texture2D *pID3D11Texture2DBackBuffer;

	if (gpID3D11RenderTargetView)
	{
		gpID3D11RenderTargetView->Release();
		gpID3D11RenderTargetView = NULL;
	}

	gpIDXGISwapChain -> ResizeBuffers(1, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);

	// use texture buffer as back buffer from swap chain
	gpIDXGISwapChain -> GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pID3D11Texture2DBackBuffer);

	hr = gpID3D11Device -> CreateRenderTargetView(pID3D11Texture2DBackBuffer, NULL, &gpID3D11RenderTargetView);

	if (FAILED(hr))
	{
		LogMessage("ID3D11Device::CreateRenderTargetView()::CreateRenderTargetView failed \n" );
	}
	else
	{
		LogMessage("ID3D11Device::CreateRenderTargetView()::CreateRenderTargetView succeeded \n");
	}

	pID3D11Texture2DBackBuffer -> Release();
	pID3D11Texture2DBackBuffer = NULL;

	// set render target view as target
	gpID3D11DeviceContext -> OMSetRenderTargets(1, &gpID3D11RenderTargetView, NULL);

	// set viewport
	D3D11_VIEWPORT d3dViewPort;
	d3dViewPort.TopLeftX = 0;
	d3dViewPort.TopLeftY = 0;
	d3dViewPort.Width = (float) width;
	d3dViewPort.Height = (float) height;
	d3dViewPort.MinDepth = 0.0f;
	d3dViewPort.MaxDepth = 1.0f;
	gpID3D11DeviceContext -> RSSetViewports(1, &d3dViewPort);

	return hr;
}

void display(void)
{
	gpID3D11DeviceContext -> ClearRenderTargetView(gpID3D11RenderTargetView, gClearColor);
	gpIDXGISwapChain -> Present(0, 0);
}

void uninitialize(void)
{
	// tear down 

	if (gpID3D11RenderTargetView)
	{
		gpID3D11RenderTargetView -> Release();
		gpID3D11RenderTargetView = NULL;
	}

	if (gpIDXGISwapChain)
	{
		gpIDXGISwapChain -> Release();
		gpIDXGISwapChain = NULL;
	}

	if (gpID3D11DeviceContext)
	{
		gpID3D11DeviceContext -> Release();
		gpID3D11DeviceContext = NULL;
	}

	if (gpID3D11Device)
	{
		gpID3D11Device -> Release();
		gpID3D11Device = NULL;
	}

	if (gpFile)
	{
		LogMessage("uninitialize() succeeded \n");
		LogMessage("Log file is closed \n");
	}
}

void LogMessage(const char*msg) {
	fopen_s(&gpFile, "Log.txt", "a+");
	fprintf_s(gpFile, msg);
	fclose(gpFile);
}