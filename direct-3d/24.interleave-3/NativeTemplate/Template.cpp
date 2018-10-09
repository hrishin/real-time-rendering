#include <windows.h>
#include <stdio.h>
#include "logging.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include "WICTextureLoader.h"

#pragma warning(disable: 4838)
#include "XNAMath\xnamath.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "D3dcompiler.lib")
#pragma comment (lib, "DirectXTK.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

LRESULT CALLBACK WndCallbackProc(HWND, UINT, WPARAM, LPARAM);

FILE *gpFile = NULL;
const char *logFileName = "log.txt";

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

ID3D11VertexShader *gpID3D11VertexShader = NULL;
ID3D11PixelShader *gpID3D11PixelShader = NULL;
ID3D11Buffer *gpID3D11BufferCube = NULL;
ID3D11InputLayout *gpID3D11InputLayout = NULL;
ID3D11Buffer *gpID3D11BufferConstantBuffer = NULL;

ID3D11RasterizerState *gpID3D11RasterizerState = NULL;
ID3D11DepthStencilView *gpID3D11DepthStencilView = NULL;

ID3D11ShaderResourceView *gpID3DShaderResourceView;
ID3D11SamplerState *gpID3D11SamplerStateTexture;

float angle = 0.0f;
bool gbLight = false;

struct VCNT 
{
	float vertex[3];
	float color[3];
	float normals[3];
	float tex[2];
};

// if not working make first letter of member capital
struct CBUFFER
{
	XMMATRIX worldViewMatrix;
	XMMATRIX projectionMatrix;
	XMVECTOR lD;
	XMVECTOR kD;
	XMVECTOR lightPosition;
	UINT keyPressed;
};

XMMATRIX gPerspectiveProjectionMatrix;

int WINAPI WinMain(HINSTANCE currentHInstance, HINSTANCE prevHInstance, LPSTR lpszCmdLune, int iCmdShow)
{
	// function prototype
	HRESULT initialize(void);
	void display(void);
	void update();
	void uninitialize(void);

	// variables declartion
	WNDCLASSEX wndClass;
	HWND hwnd;
	MSG msg;
	int iScreenWidth, iScreenHeight;
	TCHAR szClassName[] = TEXT("Direct3D11");
	bool bDone = false;

	if (fopen_s(&gpFile, logFileName, "w") != 0)
	{
		MessageBox(NULL, TEXT("Log file can't be created \n existing"), TEXT("ERROR"), MB_OK | MB_TOPMOST | MB_ICONSTOP);
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
	CS_OWNDC : Is required to make sure memory allocated is neither movable or discardable.
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
		TEXT("Direct3D11 : Interleave 2"),
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
		LOG_ERROR(gpFile, logFileName, "Initiliazation failed. Exiting now \n");
		DestroyWindow(hwnd);
		hwnd = NULL;
	}
	else 
	{
		LOG_ERROR(gpFile, logFileName, "Initiliazation succeeded \n");
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
				update();
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

		case 0x4c: // 'L' or 'l'
			gbLight = !gbLight;
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
				LOG_ERROR(gpFile, logFileName, "resize() failed \n");
				return hr;
			}
			else
			{
				LOG_ERROR(gpFile, logFileName, "resize() succeeded \n");
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
	HRESULT LoadD3DTexture(const wchar_t *, ID3D11ShaderResourceView **);

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
		LOG_ERROR(gpFile, logFileName, "CreateSwapChain() failed \n");
		return hr;
	}
	else 
	{
		LOG_ERROR(gpFile, logFileName, "CreateSwapChain() succeeded \n");
		
		LOG_ERROR(gpFile, logFileName, "Choosen driver is of ");
		if (d3dDriverType == D3D_DRIVER_TYPE_HARDWARE)
		{
			LOG_ERROR(gpFile, logFileName, "Hardware type \n");
		}
		else if (d3dDriverType == D3D_DRIVER_TYPE_WARP)
		{
			LOG_ERROR(gpFile, logFileName, "Warp type \n");
		}
		else if (d3dDriverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			LOG_ERROR(gpFile, logFileName, "Reference type \n");
		}
		else
		{
			LOG_ERROR(gpFile, logFileName, "Unknown type \n");
		}

		LOG_ERROR(gpFile, logFileName, "Supported highest level feature is ");
		if (d3dFeatureLevelAcquired == D3D_FEATURE_LEVEL_11_0)
		{
			LOG_ERROR(gpFile, logFileName, "11.0 \n");
		}
		else if (d3dFeatureLevelAcquired == D3D_FEATURE_LEVEL_10_1)
		{
			LOG_ERROR(gpFile, logFileName, "10.1 \n");
		}
		else if (d3dFeatureLevelAcquired == D3D_FEATURE_LEVEL_10_0)
		{
			LOG_ERROR(gpFile, logFileName, "10.0 \n");
		}
		else
		{
			LOG_ERROR(gpFile, logFileName, "Unknown \n");
		}
	}

	// vertex shader
	const char *vertexShaderSourceCode =
		"cbuffer ConstantBuffer" \
		"{" \
			"float4x4 worldViewMatrix;" \
			"float4x4 projectionMatrix;" \
			"float4 lD;" \
			"float4 kD;" \
			"float4 lightPosition;" \
			"uint keyPressed;" \
		"}"

		"struct VertexOutput" \
		"{" \
			"float4 position: SV_POSITION;" \
			"float4 color: COLOR;" \
			"float4 diffuseLight: NORMAL1;" \
			"float2 texcoord: TEXCOORD;" \
		"} output;" \

		"VertexOutput main(float4 pos: POSITION, float4 color: COLOR, float4 normal: NORMAL, float2 texcoord: TEXCOORD)" \
		"{" \
			"VertexOutput output;" \
			"if(keyPressed == 1)" \
			"{" \
				"float4 eyeCoordinates = mul(worldViewMatrix, pos);" \
				"float3 tNorm = normalize(mul((float3x3)worldViewMatrix, (float3)normal));" \
				"float3 s = (float3) normalize(lightPosition - eyeCoordinates);" \
				"output.diffuseLight = mul(lD, mul(kD, max(dot(s, tNorm), 0.0)));" \
			"}" \
			"else" \
			"{" \
				"output.diffuseLight = float4(1.0, 1.0, 1.0, 1.0);" \
			"}" \
			"output.position = mul(projectionMatrix, mul(worldViewMatrix, pos));" \
			"output.color = color;" \
			"output.texcoord = texcoord;" \
			"return output;" \
		"}";

	ID3DBlob *pID3DBlobVertexShaderCode = NULL;
	ID3DBlob *pID3DBlobError= NULL;
		
	hr = D3DCompile(
		vertexShaderSourceCode,
		lstrlenA(vertexShaderSourceCode) + 1,
		"VS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		0,
		0,
		&pID3DBlobVertexShaderCode,
		&pID3DBlobError
	);
	if (FAILED(hr))
	{
		if (pID3DBlobError != NULL)
		{
			LOG_ERROR(gpFile, logFileName, "D3DCompile() vertex shader compilation failed : %s \n", (char*) pID3DBlobError -> GetBufferPointer());
			pID3DBlobError -> Release();
			pID3DBlobError = NULL;
		}
		else
		{
			LOG_ERROR(gpFile, logFileName, "D3DCompile() vertex shader compilation failed \n");
		}

		return hr;
	}
	else
	{
		LOG_ERROR(gpFile, logFileName, "D3DCompile() vertex shader compilation succeeded \n");
	}
	
	hr = gpID3D11Device -> CreateVertexShader(
		pID3DBlobVertexShaderCode -> GetBufferPointer(),
		pID3DBlobVertexShaderCode -> GetBufferSize(),
		NULL,
		&gpID3D11VertexShader
	);
	
	if (FAILED(hr))
	{
		LOG_ERROR(gpFile, logFileName, "ID3D11VertexShader:: CreateVertexShader() creation failed \n");
		return hr;
	}
	else
	{
		LOG_ERROR(gpFile, logFileName, "ID3D11VertexShader:: CreateVertexShader() creation succeeded \n");
	}
	gpID3D11DeviceContext -> VSSetShader(gpID3D11VertexShader, 0, 0);
	
	// pixel (fragment) shader
	const char *pixelShaderSourceCode =
		"Texture2D myTexture2D;" \
		"SamplerState mySamplerState;" \
		"float4 main(float4 position: SV_POSITION, float4 color: COLOR, float4 diffuseLight: NORMAL1, float2 texcoord : TEXCOORD): SV_TARGET" \
		"{" \
			"float4 light = color * diffuseLight;" \
			"return myTexture2D.Sample(mySamplerState, texcoord) * light;" \
		"}";

	ID3DBlob *pID3DBlobPixelShaderCode = NULL;
	pID3DBlobError = NULL;

	hr = D3DCompile(
		pixelShaderSourceCode,
		lstrlenA(vertexShaderSourceCode) + 1,
		"PS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		0,
		0,
		&pID3DBlobPixelShaderCode,
		&pID3DBlobError
	);
	if (FAILED(hr))
	{
		if (pID3DBlobError != NULL)
		{
			LOG_ERROR(gpFile, logFileName, "D3DCompile() pixel shader compilation failed: %s \n", (char*)pID3DBlobError->GetBufferPointer());
			pID3DBlobError -> Release();
			pID3DBlobError = NULL;
		}
		else
		{
			LOG_ERROR(gpFile, logFileName, "D3DCompile() pixel shader compilation failed \n");
		}

		return hr;
	}
	else
	{
		LOG_ERROR(gpFile, logFileName, "D3DCompile() pixel shader compilation succeeded \n");
	}

	hr = gpID3D11Device -> CreatePixelShader(
		pID3DBlobPixelShaderCode -> GetBufferPointer(),
		pID3DBlobPixelShaderCode -> GetBufferSize(),
		NULL,
		&gpID3D11PixelShader
	);

	if (FAILED(hr))
	{
		LOG_ERROR(gpFile, logFileName, "ID3D11PixelShader:: CreatePixelShader() creation failed \n");
		return hr;
	}
	else
	{
		LOG_ERROR(gpFile, logFileName, "ID3D11PixelShader:: CreatePixelShader() compilation succeeded \n");
	}
	gpID3D11DeviceContext -> PSSetShader(gpID3D11PixelShader, 0, 0);
	pID3DBlobPixelShaderCode-> Release();
	pID3DBlobPixelShaderCode = NULL;

	//Cube
	// verrtices, color, normals, tex
	VCNT data[36];
	data[0] = { -1.0f, +1.0f, +1.0f,	1.0f, 0.0f, 0.0f,	0.0f, +1.0f, 0.0f,	0.0f, 0.0f };
	data[1] = { +1.0f, +1.0f, +1.0f,	1.0f, 0.0f, 0.0f,	0.0f, +1.0f, 0.0f,	0.0f, 1.0f };
	data[2] = { -1.0f, +1.0f, -1.0f,	1.0f, 0.0f, 0.0f,	0.0f, +1.0f, 0.0f,	1.0f, 0.0f };
	data[3] = { -1.0f, +1.0f, -1.0f,	1.0f, 0.0f, 0.0f,	0.0f, +1.0f, 0.0f,	1.0f, 0.0f };
	data[4] = { +1.0f, +1.0f, +1.0f,	1.0f, 0.0f, 0.0f,	0.0f, +1.0f, 0.0f,	0.0f, 1.0f };
	data[5] = { +1.0f, +1.0f, -1.0f,	1.0f, 0.0f, 0.0f,	0.0f, +1.0f, 0.0f,	1.0f, 1.0f };
	data[6] = { +1.0f, -1.0f, -1.0f,	0.0f, 1.0f, 0.0f,	0.0f, -1.0f, 0.0f,	0.0f, 0.0f };
	data[7] = { +1.0f, -1.0f, +1.0f,	0.0f, 1.0f, 0.0f,	0.0f, -1.0f, 0.0f,	0.0f, 1.0f };
	data[8] = { -1.0f, -1.0f, -1.0f,	0.0f, 1.0f, 0.0f,	0.0f, -1.0f, 0.0f,	1.0f, 0.0f };
	data[9] = { -1.0f, -1.0f, -1.0f,	0.0f, 1.0f, 0.0f,	0.0f, -1.0f, 0.0f,	1.0f, 0.0f };
	data[10] = { +1.0f, -1.0f, +1.0f,	0.0f, 1.0f, 0.0f,	0.0f, -1.0f, 0.0f,	0.0f, 1.0f };
	data[11] = { -1.0f, -1.0f, +1.0f,	0.0f, 1.0f, 0.0f,	0.0f, -1.0f, 0.0f,	1.0f, 1.0f };
	data[12] = { -1.0f, +1.0f, -1.0f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f, -1.0f,	0.0f, 0.0f };
	data[13] = { 1.0f, 1.0f, -1.0f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f, -1.0f,	0.0f, 1.0f };
	data[14] = { -1.0f, -1.0f, -1.0f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f, -1.0f,	1.0f, 0.0f };
	data[15] = { -1.0f, -1.0f, -1.0f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f, -1.0f,	1.0f, 0.0f };
	data[16] = { 1.0f, 1.0f, -1.0f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f, -1.0f,	0.0f, 1.0f };
	data[17] = { +1.0f, -1.0f, -1.0f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f, -1.0f,	1.0f, 1.0f };
	data[18] = { +1.0f, -1.0f, +1.0f,	1.0f, 0.0f, 1.0f,	0.0f, 0.0f, +1.0f,	0.0f, 0.0f };
	data[19] = { +1.0f, +1.0f, +1.0f,	1.0f, 0.0f, 1.0f,	0.0f, 0.0f, +1.0f,	0.0f, 1.0f };
	data[20] = { -1.0f, -1.0f, +1.0f,	1.0f, 0.0f, 1.0f,	0.0f, 0.0f, +1.0f,	1.0f, 0.0f };
	data[21] = { -1.0f, -1.0f, +1.0f,	1.0f, 0.0f, 1.0f,	0.0f, 0.0f, +1.0f,	1.0f, 0.0f };
	data[22] = { +1.0f, +1.0f, +1.0f,	1.0f, 0.0f, 1.0f,	0.0f, 0.0f, +1.0f,	0.0f, 1.0f };
	data[23] = { -1.0f, +1.0f, +1.0f,	1.0f, 0.0f, 1.0f,	0.0f, 0.0f, +1.0f,	1.0f, 1.0f };
	data[24] = { -1.0f, +1.0f, +1.0f,	1.0f, 1.0f, 0.0f,	-1.0f, 0.0f, 0.0f,	0.0f, 0.0f };
	data[25] = { -1.0f, +1.0f, -1.0f,	1.0f, 1.0f, 0.0f,	-1.0f, 0.0f, 0.0f,	0.0f, 1.0f };
	data[26] = { -1.0f, -1.0f, +1.0f,	1.0f, 1.0f, 0.0f,	-1.0f, 0.0f, 0.0f,	1.0f, 0.0f };
	data[27] = { -1.0f, -1.0f, +1.0f,	1.0f, 1.0f, 0.0f,	-1.0f, 0.0f, 0.0f,	1.0f, 0.0f };
	data[28] = { -1.0f, +1.0f, -1.0f,	1.0f, 1.0f, 0.0f,	-1.0f, 0.0f, 0.0f,	0.0f, 1.0f };
	data[29] = { -1.0f, -1.0f, -1.0f,	1.0f, 1.0f, 0.0f,	-1.0f, 0.0f, 0.0f,	1.0f, 1.0f };
	data[30] = { +1.0f, -1.0f, -1.0f,	0.0f, 1.0f, 1.0f,	+1.0f, 0.0f, 0.0f,	0.0f, 0.0f };
	data[31] = { +1.0f, +1.0f, -1.0f,	0.0f, 1.0f, 1.0f,	+1.0f, 0.0f, 0.0f,	0.0f, 1.0f };
	data[32] = { +1.0f, -1.0f, +1.0f,	0.0f, 1.0f, 1.0f,	+1.0f, 0.0f, 0.0f,	1.0f, 0.0f };
	data[33] = { +1.0f, -1.0f, +1.0f,	0.0f, 1.0f, 1.0f,	+1.0f, 0.0f, 0.0f,	1.0f, 0.0f };
	data[34] = { +1.0f, +1.0f, -1.0f,	0.0f, 1.0f, 1.0f,	+1.0f, 0.0f, 0.0f,	0.0f, 1.0f };
	data[35] = { +1.0f, +1.0f, +1.0f,	0.0f, 1.0f, 1.0f,	+1.0f, 0.0f, 0.0f,	1.0f, 1.0f };
	
	//create vertex buffer
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(data);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&vertexBufferDesc, NULL, &gpID3D11BufferCube);
	if (FAILED(hr))
	{
		LOG_ERROR(gpFile, logFileName, "ID3D11Device::CreateBuffer() failed for square vertex buffer \n");
		return hr;
	}
	else
	{
		LOG_ERROR(gpFile, logFileName, "ID3D11Device::CreateBuffer() succeeded for square vertex buffer \n");
	}

	//copy data into buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubSource;
	ZeroMemory(&mappedSubSource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11BufferCube, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubSource);
	memcpy(mappedSubSource.pData, data, sizeof(data));
	gpID3D11DeviceContext->Unmap(gpID3D11BufferCube, NULL);

	//create and set input layout
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[4] = 
	{
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
		{
			"COLOR",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			1,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
		{
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			2,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			3,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		}
	};

	hr = gpID3D11Device -> CreateInputLayout(
		inputElementDesc, 
		_ARRAYSIZE(inputElementDesc),
		pID3DBlobVertexShaderCode -> GetBufferPointer(),
		pID3DBlobVertexShaderCode -> GetBufferSize(),
		&gpID3D11InputLayout);

	if (FAILED(hr))
	{
		LOG_ERROR(gpFile, logFileName, "ID3D11Device::CreateInputLayout() failed \n");
		return hr;
	}
	else
	{
		LOG_ERROR(gpFile, logFileName, "ID3D11Device::CreateInputLayout() succeeded \n");
	}
	gpID3D11DeviceContext -> IASetInputLayout(gpID3D11InputLayout);
	pID3DBlobVertexShaderCode -> Release();
	pID3DBlobVertexShaderCode = NULL;

	//define and set the constant buffer
	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDesc.ByteWidth = sizeof(CBUFFER);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = gpID3D11Device -> CreateBuffer(&constantBufferDesc, nullptr, &gpID3D11BufferConstantBuffer);
	if (FAILED(hr))
	{
		LOG_ERROR(gpFile, logFileName, "ID3D11Device::CreateBuffer() failed to create constant buffer \n")
	}
	else
	{
		LOG_ERROR(gpFile, logFileName, "ID3D11Device::CreateBuffer()  succeeded to create constant buffer \n")
	}
	gpID3D11DeviceContext -> VSSetConstantBuffers(0, 1, &gpID3D11BufferConstantBuffer);

	// disable culling
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory((void*)&rasterizerDesc, sizeof(rasterizerDesc));
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.CullMode = D3D11_CULL_NONE; //makes both side of geometry visible
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;

	hr = gpID3D11Device->CreateRasterizerState(&rasterizerDesc, &gpID3D11RasterizerState);
	if (FAILED(hr))
	{
		LOG_ERROR(gpFile, logFileName, "ID3D11Device::CreateRasterizerState() failed \n")
	}
	else
	{
		LOG_ERROR(gpFile, logFileName, "ID3D11Device::CreateRasterizerState() succeeded \n")
	}
	//gpID3D11DeviceContext->RSSetState(gpID3D11RasterizerState);

	// load textures
	// marble
	hr = LoadD3DTexture(L"marble.bmp", &gpID3DShaderResourceView);
	if (FAILED(hr))
	{
		LOG_ERROR(gpFile, logFileName, "LoadD3DTexture() failed to create texture \n")
			return hr;
	}
	else
	{
		LOG_ERROR(gpFile, logFileName, "LoadD3DTexture() succeeded to create texture \n")
	}

	//sample state
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = gpID3D11Device->CreateSamplerState(&samplerDesc, &gpID3D11SamplerStateTexture);
	if (FAILED(hr))
	{
		LOG_ERROR(gpFile, logFileName, "CreateSamplerState() failed to create texture \n")
	}
	else
	{
		LOG_ERROR(gpFile, logFileName, "CreateSamplerState() succeeded to create texture \n")
	}

	// d3d clear color (black)
	gClearColor[0] = 0.0f;
	gClearColor[1] = 0.0f;
	gClearColor[2] = 0.0f;
	gClearColor[3] = 1.0f;

	//set projection
	gPerspectiveProjectionMatrix = XMMatrixIdentity();

	hr = resize(WIN_WIDTH, WIN_HEIGHT);
	if (FAILED(hr))
	{
		LOG_ERROR(gpFile, logFileName, "resize() failed \n");
		return hr;
	}
	else
	{
		LOG_ERROR(gpFile, logFileName, "resize() succeeded \n");
	}

	return S_OK;
}

HRESULT LoadD3DTexture(const wchar_t *textureFileName, ID3D11ShaderResourceView **ppID3D11ShaderResourceView)
{
	HRESULT hr;

	// create texture
	hr = DirectX::CreateWICTextureFromFile(gpID3D11Device, gpID3D11DeviceContext, textureFileName, NULL, ppID3D11ShaderResourceView);
	if (FAILED(hr))
	{
		LOG_ERROR(gpFile, logFileName, "CreateWICTextureFromFile() failed to create texture \n")
			return(hr);
	}
	else
	{
		LOG_ERROR(gpFile, logFileName, "CreateWICTextureFromFile() succeeded to create texture \n")
			fclose(gpFile);
	}

	return(hr);
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
		LOG_ERROR(gpFile, logFileName, "ID3D11Device::CreateRenderTargetView()::CreateRenderTargetView failed \n" );
	}
	else
	{
		LOG_ERROR(gpFile, logFileName, "ID3D11Device::CreateRenderTargetView()::CreateRenderTargetView succeeded \n");
	}

	pID3D11Texture2DBackBuffer -> Release();
	pID3D11Texture2DBackBuffer = NULL;

	// create depth stencil buffer ( or zbuffer )
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = (UINT)width;
	textureDesc.Height = (UINT)height;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.SampleDesc.Count = 1; // in real world, this can be up to 4
	textureDesc.SampleDesc.Quality = 0; // if above is 4, then it is 1
	textureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	ID3D11Texture2D *pID3D11Texture2D_DepthBuffer;
	gpID3D11Device->CreateTexture2D(&textureDesc, NULL, &pID3D11Texture2D_DepthBuffer);

	// create depth stencil view from above depth stencil buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	hr = gpID3D11Device->CreateDepthStencilView(pID3D11Texture2D_DepthBuffer, &depthStencilViewDesc, &gpID3D11DepthStencilView);
	if (FAILED(hr))
	{
		LOG_ERROR(gpFile, logFileName, "ID3D11Device::CreateDepthStencilView() Failed.\n");
		return(hr);
	}
	else
	{
		LOG_ERROR(gpFile, logFileName, "ID3D11Device::CreateDepthStencilView() Succeeded.\n");
	}
	pID3D11Texture2D_DepthBuffer->Release();
	pID3D11Texture2D_DepthBuffer = NULL;

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

	gPerspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(45.0f, (float)width / (float)height, 0.1f, 100.0f);
	
	return hr;
}

void display(void)
{
	gpID3D11DeviceContext -> ClearRenderTargetView(gpID3D11RenderTargetView, gClearColor);

	CBUFFER constantBuffer;

	// square
	// select buffer to display
	UINT stride = sizeof(float) * 11;
	UINT offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11BufferCube, &stride, &offset);

	stride = sizeof(float) * 11;
	offset = sizeof(float) * 3;
	gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11BufferCube, &stride, &offset);

	stride = sizeof(float) * 11;
	offset = sizeof(float) * 6;
	gpID3D11DeviceContext->IASetVertexBuffers(2, 1, &gpID3D11BufferCube, &stride, &offset);

	stride = sizeof(float) * 11;
	offset = sizeof(float) * 9;
	gpID3D11DeviceContext->IASetVertexBuffers(3, 1, &gpID3D11BufferCube, &stride, &offset);

	// bind texture and sampler as pixel shader resource
	gpID3D11DeviceContext->PSSetShaderResources(0, 1, &gpID3DShaderResourceView);
	gpID3D11DeviceContext->PSSetSamplers(0, 1, &gpID3D11SamplerStateTexture);

	// select geometry premetive (topology)
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if (gbLight)
	{
		constantBuffer.lD = XMVectorSet(0.25f, 0.25f, 0.25f, 1.0f);
		constantBuffer.kD = XMVectorSet(0.5f, 0.5f, 0.5f, 0.5f);
		constantBuffer.lightPosition = XMVectorSet(0.0f, 0.0f, -2.0f, 1.0f);
		constantBuffer.keyPressed = 1;
	}
	else
	{
		constantBuffer.keyPressed = 0;
	}
	
	// translation
	XMMATRIX worldViewMatrix = XMMatrixIdentity();
	XMMATRIX modelMatrix = XMMatrixIdentity();
	XMMATRIX rotationMatrix = XMMatrixIdentity();

	modelMatrix = XMMatrixTranslation(0.0f, 0.0f, 6.0f);
	rotationMatrix = XMMatrixRotationX(angle) *  XMMatrixRotationY(angle) * XMMatrixRotationZ(angle);
	worldViewMatrix = rotationMatrix * modelMatrix;

	constantBuffer.worldViewMatrix = worldViewMatrix;
	constantBuffer.projectionMatrix = gPerspectiveProjectionMatrix;
	gpID3D11DeviceContext->UpdateSubresource(gpID3D11BufferConstantBuffer, 0, NULL, &constantBuffer, 0, 0);

	// draw vertex buffer to render target
	gpID3D11DeviceContext -> Draw(6, 0);
	gpID3D11DeviceContext -> Draw(6, 6);
	gpID3D11DeviceContext -> Draw(6, 12);
	gpID3D11DeviceContext -> Draw(6, 18);
	gpID3D11DeviceContext -> Draw(6, 24);
	gpID3D11DeviceContext -> Draw(6, 30);

	// switch between front and back buffer
	gpIDXGISwapChain -> Present(0, 0);
}

void update(void)
{
	angle = angle + 0.0009f;
}

void uninitialize(void)
{
	if (gpID3D11SamplerStateTexture)
	{
		gpID3D11SamplerStateTexture->Release();
		gpID3D11SamplerStateTexture = NULL;
	}

	if (gpID3DShaderResourceView)
	{
		gpID3DShaderResourceView->Release();
		gpID3DShaderResourceView = NULL;
	}

	if (gpID3D11RasterizerState)
	{
		gpID3D11RasterizerState->Release();
		gpID3D11RasterizerState = NULL;
	}

	if (gpID3D11BufferConstantBuffer)
	{
		gpID3D11BufferConstantBuffer -> Release();
		gpID3D11BufferConstantBuffer = NULL;
	}

	if (gpID3D11InputLayout)
	{
		gpID3D11InputLayout -> Release();
		gpID3D11InputLayout = NULL;
	}

	if (gpID3D11BufferCube)
	{
		gpID3D11BufferCube -> Release();
		gpID3D11BufferCube = NULL;
	}

	if (gpID3D11PixelShader)
	{
		gpID3D11PixelShader -> Release();
		gpID3D11PixelShader = NULL;
	}

	if (gpID3D11VertexShader)
	{
		gpID3D11VertexShader -> Release();
		gpID3D11VertexShader = NULL;
	}

	if (gpID3D11DepthStencilView)
	{
		gpID3D11DepthStencilView->Release();
		gpID3D11DepthStencilView = NULL;
	}

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
		LOG_ERROR(gpFile, logFileName, "uninitialize() succeeded \n");
		LOG_ERROR(gpFile, logFileName, "Log file is closed \n");
	}
}