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

#define checkImageHeight 64
#define checkImageWidth 64

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
ID3D11Buffer *gpID3D11BufferSquareVertices = NULL;
ID3D11Buffer *gpID3D11BufferSquarelTexture = NULL;
ID3D11InputLayout *gpID3D11InputLayout = NULL;
ID3D11Buffer *gpID3D11BufferConstantBuffer = NULL;

uint8_t checkImage[checkImageHeight * checkImageWidth * 4];
ID3D11ShaderResourceView *gpID3DShaderResourceViewCB;
ID3D11SamplerState *gpID3D11SamplerStateTextureSmiley;

struct CBUFFER
{
	XMMATRIX WorldViewProjectionMatrix;
};

XMMATRIX gPerspectiveProjectionMatrix;

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
		TEXT("Direct3D11 : Texture Checkerboard"),
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
	HRESULT Load2DTexture(ID3D11ShaderResourceView **);
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
		"float4x4 worldViewProjectionMatrix;" \
		"}"

		"struct VertexOutput" \
		"{" \
		"float4 position: SV_POSITION;" \
		"float2 texcoord: TEXCOORD;" \
		"}output;" \

		"VertexOutput main(float4 pos: POSITION, float2 texcoord: TEXCOORD)" \
		"{" \
		"VertexOutput output;" \
		"output.position = mul(worldViewProjectionMatrix, pos);" \
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

		"float4 main(float4 position: SV_POSITION, float2 texcoord : TEXCOORD): SV_TARGET" \
		"{" \
			"float4 color = myTexture2D.Sample(mySamplerState, texcoord);" \
			"return color;" \
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

	//Square


	//create vertex buffer position
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(float) * 18;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&vertexBufferDesc, NULL, &gpID3D11BufferSquareVertices);
	if (FAILED(hr))
	{
		LOG_ERROR(gpFile, logFileName, "ID3D11Device::CreateBuffer() failed for square vertex buffer \n");
		return hr;
	}
	else
	{
		LOG_ERROR(gpFile, logFileName, "ID3D11Device::CreateBuffer() succeeded for square vertex buffer \n");
	}

	//color
	float squareTexCords[] =
	{
		+0.0f, +0.0f,
		+0.0f, +1.0f,
		+1.0f, +0.0f,
		// triangle 2
		+1.0f, +0.0f,
		+0.0f, +1.0f,
		+1.0f, +1.0f,
	};

	// create vertex buffer for texture
	D3D11_MAPPED_SUBRESOURCE mappedSubSource;
	ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(float) * _ARRAYSIZE(squareTexCords);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device -> CreateBuffer(&vertexBufferDesc, NULL, &gpID3D11BufferSquarelTexture);
	if (FAILED(hr))
	{
		LOG_ERROR(gpFile, logFileName, "ID3D11Device::CreateBuffer() failed for vertex buffer \n");
		return hr;
	}
	else
	{
		LOG_ERROR(gpFile, logFileName, "ID3D11Device::CreateBuffer() succeeded for vertex buffer \n");
	}

	//copy data into buffer
	ZeroMemory(&mappedSubSource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext -> Map(gpID3D11BufferSquarelTexture, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubSource);
	memcpy(mappedSubSource.pData, squareTexCords, sizeof(squareTexCords));
	gpID3D11DeviceContext -> Unmap(gpID3D11BufferSquarelTexture, NULL);

	//create and set input layout
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[2];
	inputElementDesc[0].SemanticName = "POSITION";
	inputElementDesc[0].SemanticIndex = 0;
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[0].InputSlot = 0;
	inputElementDesc[0].AlignedByteOffset = 0;
	inputElementDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[0].InstanceDataStepRate = 0;

	inputElementDesc[1].SemanticName = "TEXCOORD";
	inputElementDesc[1].SemanticIndex = 0;
	inputElementDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDesc[1].InputSlot = 1;
	inputElementDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	inputElementDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[1].InstanceDataStepRate = 0;

	hr = gpID3D11Device -> CreateInputLayout(
		inputElementDesc, 
		_ARRAYSIZE(inputElementDesc),
		pID3DBlobVertexShaderCode -> GetBufferPointer(),
		pID3DBlobVertexShaderCode ->GetBufferSize(),
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

	// load texture
	hr = Load2DTexture(&gpID3DShaderResourceViewCB);
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
	hr = gpID3D11Device -> CreateSamplerState(&samplerDesc, &gpID3D11SamplerStateTextureSmiley);
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

HRESULT Load2DTexture(ID3D11ShaderResourceView **ppID3D11ShaderResourceView)
{
	void makeCheckImage(void);

	HRESULT hr;

	makeCheckImage();
	ID3D11Texture2D *texture = NULL;
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = checkImageWidth;
	textureDesc.Height = checkImageHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA subData;
	subData.pSysMem = checkImage;
	subData.SysMemPitch = checkImageWidth * 4;
	
	hr = gpID3D11Device -> CreateTexture2D(&textureDesc, &subData, &texture);
	if (FAILED(hr))
	{
		LOG_ERROR(gpFile, logFileName, "CreateTexture2D() failed to create texture \n")
		return(hr);
	}
	else
	{
		LOG_ERROR(gpFile, logFileName, "CreateTexture2D() succeeded to create texture \n")
		fclose(gpFile);
	}
	
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(SRVDesc));
	SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;
	
	hr = gpID3D11Device -> CreateShaderResourceView(texture, &SRVDesc, ppID3D11ShaderResourceView);
	if (FAILED(hr))
	{
		LOG_ERROR(gpFile, logFileName, "CreateShaderResourceView() failed to create texture \n")
		return(hr);
	}
	else
	{
		LOG_ERROR(gpFile, logFileName, "CreateShaderResourceView() succeeded to create texture \n")
		fclose(gpFile);
	}

	return(hr);
}

void makeCheckImage(void)
{
	int i, j, c, heightIndex, widthIndex, postion;

	for (i = 0; i < checkImageHeight; i++)
	{
		heightIndex = i * checkImageWidth * 4;

		for (j = 0; j < checkImageWidth; j++)
		{
			widthIndex = j * 4;
			postion = heightIndex + widthIndex;

			c = (((i & 0X8) == 0) ^ ((j & 0X8) == 0)) * 255;

			checkImage[postion + 0] = c;
			checkImage[postion + 1] = c;
			checkImage[postion + 2] = c;
			checkImage[postion + 3] = 255;
		}
	}
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
	void leftSquare();
	void rightSquare();
	gpID3D11DeviceContext -> ClearRenderTargetView(gpID3D11RenderTargetView, gClearColor);

	leftSquare();
	rightSquare();

	// switch between front and back buffer
	gpIDXGISwapChain -> Present(0, 0);
}

void leftSquare()
{
	// square
	// select buffer to display
	UINT stride = sizeof(float) * 3;
	UINT offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11BufferSquareVertices, &stride, &offset);

	stride = sizeof(float) * 2;
	offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11BufferSquarelTexture, &stride, &offset);

	//copy data into buffer
	//clockwise winding order
	float squareVertices[] =
	{
		-1.0f, +1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		+1.0f, -1.0f, -1.0f,
	};
	D3D11_MAPPED_SUBRESOURCE mappedSubSource;
	ZeroMemory(&mappedSubSource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11BufferSquareVertices, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubSource);
	memcpy(mappedSubSource.pData, squareVertices, sizeof(squareVertices));
	gpID3D11DeviceContext->Unmap(gpID3D11BufferSquareVertices, NULL);

	// bind texture and sampler as pixel shader resource
	gpID3D11DeviceContext->PSSetShaderResources(0, 1, &gpID3DShaderResourceViewCB);
	gpID3D11DeviceContext->PSSetSamplers(0, 1, &gpID3D11SamplerStateTextureSmiley);

	// select geometry premetive (topology)
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// translation
	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX viewMatrix = XMMatrixIdentity();

	worldMatrix = XMMatrixTranslation(-1.5f, 0.0f, 6.0f);

	// final worldviewProjection matrix
	XMMATRIX wvpMatrix = worldMatrix * viewMatrix * gPerspectiveProjectionMatrix;

	// load the data inot constant buffer
	CBUFFER constantBuffer;
	constantBuffer.WorldViewProjectionMatrix = wvpMatrix;
	gpID3D11DeviceContext->UpdateSubresource(gpID3D11BufferConstantBuffer, 0, NULL, &constantBuffer, 0, 0);

	// draw vertex buffer to render target
	gpID3D11DeviceContext->Draw(6, 0);
}

void rightSquare()
{
	// square
	// select buffer to display
	UINT stride = sizeof(float) * 3;
	UINT offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11BufferSquareVertices, &stride, &offset);

	stride = sizeof(float) * 2;
	offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11BufferSquarelTexture, &stride, &offset);

	//copy data into buffer
	//clockwise winding order
	float squareVertices[] =
	{
		1.0f, 1.0f, 0.0f,
		2.41421f, 1.0f, 1.41421f,
		1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		2.41421f, 1.0f, 1.41421f, 
		2.41421f, -1.0f, 1.41421f,
	};
	D3D11_MAPPED_SUBRESOURCE mappedSubSource;
	ZeroMemory(&mappedSubSource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11BufferSquareVertices, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubSource);
	memcpy(mappedSubSource.pData, squareVertices, sizeof(squareVertices));
	gpID3D11DeviceContext->Unmap(gpID3D11BufferSquareVertices, NULL);

	// bind texture and sampler as pixel shader resource
	gpID3D11DeviceContext->PSSetShaderResources(0, 1, &gpID3DShaderResourceViewCB);
	gpID3D11DeviceContext->PSSetSamplers(0, 1, &gpID3D11SamplerStateTextureSmiley);

	// select geometry premetive (topology)
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// translation
	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX viewMatrix = XMMatrixIdentity();

	worldMatrix = XMMatrixTranslation(1.5f, 0.0f, 6.0f);

	// final worldviewProjection matrix
	XMMATRIX wvpMatrix = worldMatrix * viewMatrix * gPerspectiveProjectionMatrix;

	// load the data inot constant buffer
	CBUFFER constantBuffer;
	constantBuffer.WorldViewProjectionMatrix = wvpMatrix;
	gpID3D11DeviceContext->UpdateSubresource(gpID3D11BufferConstantBuffer, 0, NULL, &constantBuffer, 0, 0);

	// draw vertex buffer to render target
	gpID3D11DeviceContext->Draw(6, 0);
}

void uninitialize(void)
{
	// tear down 
	if (gpID3D11SamplerStateTextureSmiley)
	{
		gpID3D11SamplerStateTextureSmiley -> Release();
		gpID3D11SamplerStateTextureSmiley = NULL;
	}

	if (gpID3DShaderResourceViewCB)
	{
		gpID3DShaderResourceViewCB->Release();
		gpID3DShaderResourceViewCB = NULL;
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

	if (gpID3D11BufferSquarelTexture)
	{
		gpID3D11BufferSquarelTexture->Release();
		gpID3D11BufferSquarelTexture = NULL;
	}

	if (gpID3D11BufferSquareVertices)
	{
		gpID3D11BufferSquareVertices -> Release();
		gpID3D11BufferSquareVertices = NULL;
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