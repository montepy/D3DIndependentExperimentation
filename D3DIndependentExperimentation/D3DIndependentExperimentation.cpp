// D3DIndependentExperimentation.cpp : Defines the entry point for the application.
//

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"DXGI.lib")
#pragma comment(lib,"dwrite.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")


#include "stdafx.h"
#include "Cube.h"
#include "D3DIndependentExperimentation.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <dinput.h>
#include <vector>

using namespace DirectX;
#define MAX_LOADSTRING [100]

LPCWSTR windowName = L"D3D Window";
HWND hWnd;
const int SCREENHEIGHT = 600;
const int SCREENWIDTH = 800;

IDXGISwapChain* SwapChain; //allows for swapping between front and back buffers, which in turn allows smooth rendering
ID3D11Device* d3d11Device; //handles loading of objects into memory
ID3D11DeviceContext* d3d11DevCon; //handles actual rendering
ID3D11RenderTargetView* renderTargetView; //essentially the back buffer. Data gets written to this object, and subsequently rendered
ID3D11Buffer* VertexBuffer;
ID3D11Buffer* IndexBuffer;
ID3D11VertexShader* VS;
ID3D11PixelShader* PS;
ID3D10Blob* PS_Buffer;
ID3D10Blob* VS_Buffer;
ID3D11InputLayout* vertLayout;
ID3D11DepthStencilView* depthStencilView;
ID3D11Texture2D* depthStencilBuffer;
ID3D11Buffer* cbPerObjectBuffer;
ID3D11RasterizerState* FULL;
ID3D11RasterizerState* CWcullMode;
ID3D11RasterizerState* CCWcullMode;

//texture sampler state
ID3D11SamplerState*CubesTexSamplerState;
//lighting buffers
ID3D11Buffer* cbPerFrameBuffer;

double countsPerSecond;
__int64 CounterStart = 0;

int frameCount = 0;
int fps = 0;
__int64 frameTimeOld = 0;
double frameTime;

XMMATRIX WVP;
XMMATRIX World;
XMMATRIX camView;
XMMATRIX camProjection;

XMVECTOR camPosition;
XMVECTOR camTarget;
XMVECTOR camUp;

XMMATRIX Rotation;
XMMATRIX Translation;

struct cbPerObject {
	XMMATRIX WVP;
	XMMATRIX World;
};

cbPerObject constbuffer;

XMMATRIX cube1World;

D3D11_INPUT_ELEMENT_DESC layout[] = { 
	{ "POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA,0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,20,D3D11_INPUT_PER_VERTEX_DATA,0 }
};

struct Light
{
	Light() {
		ZeroMemory(this, sizeof(Light));
	}
	XMFLOAT3 dir;
	float pad;
	XMFLOAT3 att;
	XMFLOAT3 pos;
	float pad2;
	float range;
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
};

Light light;

struct cbPerFrame {
	Light light;
};

cbPerFrame constBufferPerFrame;


IDirectInputDevice8* DIKeyboard;
IDirectInputDevice8* DIMouse;

DIMOUSESTATE mouseLastState;
LPDIRECTINPUT8 DirectInput;


bool InitializeDirect3dApp(HINSTANCE hInstance);
void ReleaseObjects();
bool InitScene(std::vector<Cube>);
void UpdateScene(double time);
void DrawScene();
bool InitializeWindow(HINSTANCE,
	int,
	int, int,
	bool);

void StartTimer();
double GetTime();
double GetFrameTime();

bool InitDirectInput(HINSTANCE hInstance);
void DetectInput(double time);

int messageloop();
UINT NUMELEMENTS = ARRAYSIZE(layout);
HINSTANCE hInst;      
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPWSTR    lpCmdLine,
                     int       nCmdShow)
{
    // Perform application initialization:
    if (!InitializeWindow(hInstance,nCmdShow,SCREENWIDTH,SCREENHEIGHT,true))
    {
		MessageBox(hWnd, L"Window Initialization Failed", L"Error", MB_OK);
    }
	if (!InitializeDirect3dApp(hInstance)) {
		MessageBox(0, L"Direct3D Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}
	/*if (!InitDirectInput(hInstance)) {
		MessageBox(0, L"DirectInput Initialization - Failed",
			L"Error", MB_OK);
	}*/
	std::vector<Cube> cubeList;
	if (!InitScene(cubeList)) {
		MessageBox(0, L"Scene Initialization - Failed",
			L"Error", MB_OK);
	}

	messageloop();

	return 0;
	ReleaseObjects();
}


bool InitializeWindow(HINSTANCE hInstance, int nCmdShow, int width, int height, bool windowed)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+2);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = windowName;
    wcex.hIconSm        = LoadIcon(NULL,IDI_APPLICATION);

	if (!RegisterClassExW(&wcex)) {
		MessageBox(NULL, L"Error registering class", L"Error", MB_OK|MB_ICONERROR);
		return 1;
	}

	hWnd = CreateWindowEx(NULL, windowName, L"Title", WS_OVERLAPPEDWINDOW, 50, 50, width, height, NULL, NULL, hInstance, NULL);
	if (!hWnd) {
		MessageBox(NULL, L"Error creating window", L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			if (MessageBox(0, L"Are you sure you want to exit?",
				L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
				DestroyWindow(hWnd);
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int messageloop() {
	MSG msg;

	ZeroMemory(&msg, sizeof(MSG));
	while (true) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {//takes one message from queue and inserts it into msg
			if (msg.message == WM_QUIT) {
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg); //sends message to WndProc for processing
		}
		else {
			frameCount++;
			if (GetTime() > 1.0f) {
				fps = frameCount;
				frameCount = 0;
				StartTimer();
			}
			UpdateScene(GetFrameTime());
			DrawScene();

		}

	}
	return msg.wParam;
}

void StartTimer() {
	LARGE_INTEGER frequencyCount;
	QueryPerformanceFrequency(&frequencyCount);
	countsPerSecond = double(frequencyCount.QuadPart);

	QueryPerformanceCounter(&frequencyCount);
	CounterStart = frequencyCount.QuadPart;
}

double GetTime() {
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	return double(currentTime.QuadPart - CounterStart) / countsPerSecond;
}

double GetFrameTime() {
	LARGE_INTEGER currentTime;
	__int64 TickCount;
	QueryPerformanceCounter(&currentTime);

	TickCount = currentTime.QuadPart - frameTimeOld;
	frameTimeOld = currentTime.QuadPart;

	if (TickCount < 0.0f) {
		TickCount = 0.0f;
	}

	return float(TickCount) / countsPerSecond;
}

bool InitializeDirect3dApp(HINSTANCE hInstance) {
	HRESULT hr;

	DXGI_MODE_DESC bufferDesc; //buffer description object. Allows specification of buffer characteristics

	ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

	bufferDesc.Width = SCREENWIDTH;
	bufferDesc.Height = SCREENHEIGHT;
	bufferDesc.RefreshRate.Numerator = 60;
	bufferDesc.RefreshRate.Denominator = 1; //back buffer refresh rate is represented by the numerator over denominator
	bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //sets pixel format. Currently set to eight bits for R,G,B, and alpha
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; //specifies when each scanline is rendered. Currently unspecified becaused scanline order doesn't matter if you don't see it.
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; //determines how image is scaled to a monitor. We are windowed so specifications are unneeded

	DXGI_SWAP_CHAIN_DESC swapChainDesc; //allows description of swapchain characteristics

	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferDesc = bufferDesc;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0; //SampleDesc subobject describes amount of anti-aliasing that occurs.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //sets buffer as the render output
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; //allows display driver to determine best use for used buffers

	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_UNKNOWN, NULL, D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT, NULL, NULL,
		D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &d3d11Device, NULL, &d3d11DevCon);

	ID3D11Texture2D* BackBuffer;
	hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);

	hr = d3d11Device->CreateRenderTargetView(BackBuffer, NULL, &renderTargetView); //back buffer creation
	BackBuffer->Release();

	//Depth Stencil Buffer description
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = SCREENWIDTH;
	depthStencilDesc.Height = SCREENHEIGHT;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; //this format allocates 24 bits for the depth and 8 for the stencil
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	//Creation of Depth Stencil Buffer
	d3d11Device->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
	d3d11Device->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);
	d3d11DevCon->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	return true;
}

void ReleaseObjects() {

	SwapChain->Release();
	d3d11Device->Release();
	d3d11DevCon->Release();
	VS->Release();
	PS->Release();
	VS_Buffer->Release();
	PS_Buffer->Release();
	vertLayout->Release();
	renderTargetView->Release();
	VertexBuffer->Release();
	IndexBuffer->Release();
	SwapChain->Release();
	depthStencilBuffer->Release();
	depthStencilView->Release();
	cbPerObjectBuffer->Release();
	cbPerFrameBuffer->Release();
}

bool InitScene(std::vector<Cube> cubelist) {
	HRESULT hr;
	//Compiling Shaders
	hr = D3DCompileFromFile(L"Effects.fx", 0, 0, "VS", "vs_5_0", 0, 0, &VS_Buffer, 0);
	hr = D3DCompileFromFile(L"Effects.fx", 0, 0, "PS", "ps_5_0", 0, 0, &PS_Buffer, 0);
	//Creating Shaders
	hr = d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);
	hr = d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);
	//Setting Shaders
	d3d11DevCon->VSSetShader(VS, NULL, NULL);
	d3d11DevCon->PSSetShader(PS, NULL, NULL);

	//Creating and populating Vertex Buffers
	//Buffer description
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT; //describes how buffer is used
	vertexBufferDesc.ByteWidth = (cubelist.size() + 1) * sizeof(*cubelist.at(1).getVertices); // specifies the size of buffer; dependent on amount of vertices passed and size of vertices
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;//Specifies that this is a vertex buffer
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	//Specifies what kind of data is placed in buffer
	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	std::vector<int> cubeVertices;
	for (int i = 0; i < cubelist.size(); i++) {
		Vertex*point = cubelist.at(i).getVertices();
		cubeVertices.insert(cubeVertices.end(), point, point + sizeof(*point));
	}

	vertexBufferData.pSysMem = &cubeVertices;
	hr = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &VertexBuffer);
	//set buffer data
	UINT stride = sizeof(Vertex);//size of each Vertex
	UINT offset = 0;// how far from the buffer beginning we start
	d3d11DevCon->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);

	//create indices to be put into index buffer
	
	//Buffer description is mostly the same as vertex buffer
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * sizeof(*cubelist.at(1).getIndices());
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	std::vector<short> cubeIndices;
	D3D11_SUBRESOURCE_DATA indexBufferData;
	ZeroMemory(&indexBufferData, sizeof(indexBufferData));

	for (int i = 0; i < cubelist.size(); i++) {
		short*point = cubelist.at(i).getIndices();
		cubeIndices.insert(cubeIndices.end(), point, point + sizeof(*point));
	}
	indexBufferData.pSysMem = &cubeIndices;
	d3d11Device->CreateBuffer(&indexBufferDesc, &indexBufferData, &IndexBuffer);
	d3d11DevCon->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//set input layout
	hr = d3d11Device->CreateInputLayout(layout, NUMELEMENTS, VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), &vertLayout);
	d3d11DevCon->IASetInputLayout(vertLayout);

	d3d11DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//Create and set viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREENWIDTH;
	viewport.Height = SCREENHEIGHT;
	viewport.MinDepth = 0.0;
	viewport.MaxDepth = 1.0;

	d3d11DevCon->RSSetViewports(1, &viewport);


	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));

	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDesc.ByteWidth = sizeof(cbPerObject);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.MiscFlags = 0;

	hr = d3d11Device->CreateBuffer(&constantBufferDesc, NULL, &cbPerObjectBuffer);

	ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));

	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDesc.ByteWidth = sizeof(cbPerFrame);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.MiscFlags = 0;

	hr = d3d11Device->CreateBuffer(&constantBufferDesc, NULL, &cbPerFrameBuffer);

	camPosition = XMVectorSet(0.0f, 5.0f, -10.0f, 0.0f);
	camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);

	camProjection = XMMatrixPerspectiveFovLH(0.4f*3.14f, (float)SCREENWIDTH / SCREENHEIGHT, 1.0f, 1000.0f);

	//Describe and create rasterizer state
	D3D11_RASTERIZER_DESC wfdesc;
	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	wfdesc.FillMode = D3D11_FILL_SOLID; 
	wfdesc.CullMode = D3D11_CULL_FRONT;
	hr = d3d11Device->CreateRasterizerState(&wfdesc, &FULL);


	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = d3d11Device->CreateSamplerState(&sampDesc, &CubesTexSamplerState);

	//describe and create blend state
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	D3D11_RENDER_TARGET_BLEND_DESC rtbd;
	ZeroMemory(&rtbd, sizeof(rtbd));

	rtbd.BlendEnable = true;
	rtbd.SrcBlend = D3D11_BLEND_SRC_COLOR;
	rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = rtbd;

	//d3d11Device->CreateBlendState(&blendDesc, &Transparency);

	//define rasterizer states for blending
	D3D11_RASTERIZER_DESC cmdesc;
	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));

	cmdesc.CullMode = D3D11_CULL_BACK;
	cmdesc.FillMode = D3D11_FILL_SOLID;

	cmdesc.FrontCounterClockwise = true;
	hr = d3d11Device->CreateRasterizerState(&cmdesc, &CCWcullMode);

	cmdesc.FrontCounterClockwise = false;
	hr = d3d11Device->CreateRasterizerState(&cmdesc, &CWcullMode);

	//light setting
	//light.dir = XMFLOAT3(1.0f, 0.0f, 0.0f);
	light.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	light.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//light.pos = XMFLOAT3(7.0f, 7.0f, 0.0f);
	light.range = 100.0f;
	light.att = XMFLOAT3(0.0f, 0.2f, 0.0f);

	return true;
}

void UpdateScene(double time) { //implements any changes from previous frame
	//DetectInput(time);
	//TODO: object updates
}

void DrawScene() { // performs actual rendering
				   //clear backbuffer
	float bgColor[4] = { 0.0, 0.0, 0.0, 0.0f };

	d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);
	//clear depth stencil
	d3d11DevCon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0.0);
	//set default blend state(no blending)
	d3d11DevCon->OMSetBlendState(0, 0, 0xffffff);

	constBufferPerFrame.light = light;
	d3d11DevCon->UpdateSubresource(cbPerFrameBuffer, 0, NULL, &constBufferPerFrame, 0, 0);
	d3d11DevCon->PSSetConstantBuffers(0, 1, &cbPerFrameBuffer);

	//TODO: everything


	SwapChain->Present(0, 0);
}

/*bool InitDirectInput(HINSTANCE hInstance) {

	HRESULT hr;
	hr = DirectInput8Create(hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&DirectInput,
		NULL);

	hr = DirectInput->CreateDevice(GUID_SysKeyboard, &DIKeyboard, NULL);

	hr = DirectInput->CreateDevice(GUID_SysMouse, &DIMouse, NULL);

	hr = DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	hr = DIKeyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	hr = DIMouse->SetDataFormat(&c_dfDIMouse);
	hr = DIMouse->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE | DISCL_NOWINKEY);

	return true;
}

void DetectInput(double time) {
	DIMOUSESTATE mouseCurrState;

	BYTE keyboardState[256];
	DIKeyboard->Acquire();
	DIMouse->Acquire();

	DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);
	DIKeyboard->GetDeviceState(sizeof(keyboardState), &keyboardState);

	//TODO: inputs
	return;
}*/