// CGS HW Project A "Line Land".
// Author: L.Norri CD CGS, FullSail University

// Introduction:
// Welcome to the hardware project of the Computer Graphics Systems class.
// This assignment is fully guided but still requires significant effort on your part. 
// Future classes will demand the habits & foundation you develop right now!  
// It is CRITICAL that you follow each and every step. ESPECIALLY THE READING!!!

// TO BEGIN: Open the word document that acompanies this project and start from the top.

//************************************************************
//************ INCLUDES & DEFINES ****************************
//************************************************************

#include <iostream>
#include <ctime>
#include "XTime.h"

using namespace std;

// BEGIN PART 1
// TODO: PART 1 STEP 1a
#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")
// TODO: PART 1 STEP 1b
#include <DirectXMath.h>
using namespace DirectX;
// TODO: PART 2 STEP 6
#include "Trivial_PS.csh"
#include "Trivial_VS.csh"

#include "SampleVertexShader.csh"
#include "SamplePixelShader.csh"

#define BACKBUFFER_WIDTH	500
#define BACKBUFFER_HEIGHT	500

//************************************************************
//************ SIMPLE WINDOWS APP CLASS **********************
//************************************************************

class DEMO_APP
{	
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;

	XMFLOAT2 speed;
	float turn;
	
	// Matrices
	XMFLOAT4X4 m_view;
	XMMATRIX m_Projection;
	XMFLOAT4X4 m_CubeWorld;

	// Buffers
	ID3D11Buffer *vb_Cube;
	ID3D11Buffer *ib_Cube;
	ID3D11Buffer *vb_Grid;
	ID3D11Buffer *cBuff_perspective;
	
	// Layouts
	ID3D11InputLayout *lay_perspective;

	// Shaders
	ID3D11VertexShader *VertSha_perspective;
	ID3D11PixelShader *PixSha_perspective;
	// Pending...


	// TODO: PART 1 STEP 2
	ID3D11Device *iDevice;
	ID3D11DeviceContext *iDeviceContext;
	ID3D11RenderTargetView *iRenderTarget;
	D3D11_VIEWPORT viewPort;
	IDXGISwapChain *swapChain;
	// TODO: PART 2 STEP 2
	ID3D11Buffer *vBuffer;
	ID3D11InputLayout *iLayout;
	UINT32 vCount_Crcl = 360;
	
	// BEGIN PART 5
	// TODO: PART 5 STEP 1
	ID3D11Buffer *checkers;
	UINT32 vCount_Checkers=1200;

	// TODO: PART 2 STEP 4
	ID3D11VertexShader *iVertShader;
	ID3D11PixelShader *iPixShader;
	
	// BEGIN PART 3
	// TODO: PART 3 STEP 1
	ID3D11Buffer *constBuffer;
	XTime timeX;
	// TODO: PART 3 STEP 2b
	struct SEND_TO_VRAM
	{
		XMFLOAT4 constantColor;
		XMFLOAT2 constantOffset;
		XMFLOAT2 padding;
	};

	struct cbMirror_3D{
		XMMATRIX model;
		XMMATRIX view;
		XMMATRIX projection;
	};

	// TODO: PART 3 STEP 4a
	SEND_TO_VRAM toShader;
	SEND_TO_VRAM toShader_2;
	cbMirror_3D toShader_perspective;

public:
	// BEGIN PART 2
	// TODO: PART 2 STEP 1
	struct SIMPLE_VERTEX{
		XMFLOAT2 POSITION;
	};

	struct VERTEX_3D{
		XMFLOAT3 pos;
		XMFLOAT4 col;
	};
	
	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
};

//************************************************************
//************ CREATION OF OBJECTS & RESOURCES ***************
//************************************************************

DEMO_APP::DEMO_APP(HINSTANCE hinst, WNDPROC proc)
{
	// ****************** BEGIN WARNING ***********************// 
	// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY! 
	application = hinst; 
	appWndProc = proc; 

	WNDCLASSEX  wndClass;
    ZeroMemory( &wndClass, sizeof( wndClass ) );
    wndClass.cbSize         = sizeof( WNDCLASSEX );             
    wndClass.lpfnWndProc    = appWndProc;						
    wndClass.lpszClassName  = L"DirectXApplication";            
	wndClass.hInstance      = application;		               
    wndClass.hCursor        = LoadCursor( NULL, IDC_ARROW );    
    wndClass.hbrBackground  = ( HBRUSH )( COLOR_WINDOWFRAME ); 
	//wndClass.hIcon			= LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FSICON));
    RegisterClassEx( &wndClass );

	RECT window_size = { 0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(	L"DirectXApplication", L"Project And Portfolio 4",	WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME|WS_MAXIMIZEBOX), 
							CW_USEDEFAULT, CW_USEDEFAULT, window_size.right-window_size.left, window_size.bottom-window_size.top,					
							NULL, NULL,	application, this );												

    ShowWindow( window, SW_SHOW );
	//********************* END WARNING ************************//

	// TODO: PART 1 STEP 3a
	DXGI_SWAP_CHAIN_DESC chainDesc;
	ZeroMemory(&chainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	chainDesc.BufferCount = 1;
	chainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	chainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	chainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	chainDesc.BufferDesc.Height = BACKBUFFER_HEIGHT;
	chainDesc.BufferDesc.Width = BACKBUFFER_WIDTH;
	chainDesc.OutputWindow = window;
	chainDesc.SampleDesc.Count = 1;
	chainDesc.SampleDesc.Quality = 0;
	chainDesc.Windowed = TRUE;

	D3D_FEATURE_LEVEL  FeatureLevelsRequested = D3D_FEATURE_LEVEL_10_0;
	UINT               numLevelsRequested = 1;
	D3D_FEATURE_LEVEL  FeatureLevelsSupported;

	// TODO: PART 1 STEP 3b
#if _DEBUG
	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, 
		&FeatureLevelsRequested, numLevelsRequested, D3D11_SDK_VERSION, &chainDesc, &swapChain, 
		&iDevice, &FeatureLevelsSupported, &iDeviceContext);
#else
	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, 
		&FeatureLevelsRequested, numLevelsRequested, D3D11_SDK_VERSION, &chainDesc, &swapChain, 
		&iDevice, &FeatureLevelsSupported, &iDeviceContext);
#endif
	// TODO: PART 1 STEP 4
	ID3D11Resource *iResource;
	ZeroMemory(&iResource, sizeof(ID3D10Resource));
	swapChain->GetBuffer(0, __uuidof(iResource), reinterpret_cast<void**>(&iResource));
	iDevice->CreateRenderTargetView(iResource, NULL, &iRenderTarget);
	iResource->Release();
	
	// TODO: PART 1 STEP 5
	swapChain->GetDesc(&chainDesc);
	ZeroMemory(&viewPort, sizeof(D3D11_VIEWPORT));
	viewPort.Height = static_cast<FLOAT>(chainDesc.BufferDesc.Height);
	viewPort.Width = static_cast<FLOAT>(chainDesc.BufferDesc.Width);
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;

// <Mah 3D>
	unsigned int indx = 0;

	VERTEX_3D aTri[4] = { 
			{ XMFLOAT3(0, 0.8f, 0), XMFLOAT4(1,1,1,1)},
			{ XMFLOAT3(0.5f, 0, -0.4f),XMFLOAT4(0,1,0,1)},
			{ XMFLOAT3(-0.5f, 0, -0.4f),XMFLOAT4(0,0,1,1)}, 
			{ XMFLOAT3(0, 0, 0.6f), XMFLOAT4(1, 0, 0, 1)} 
	};

	D3D11_BUFFER_DESC desc_cube;
	ZeroMemory(&desc_cube, sizeof(D3D11_BUFFER_DESC));
	desc_cube.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
	desc_cube.ByteWidth = sizeof(VERTEX_3D) * 4;
	desc_cube.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA res_cube;
	ZeroMemory(&res_cube, sizeof(D3D11_SUBRESOURCE_DATA));
	res_cube.pSysMem = aTri;
	iDevice->CreateBuffer(&desc_cube, &res_cube, &vb_Cube);

	unsigned int indices[12];
#if 1
	bool flip = true;
	for (unsigned int i = 0; i < 4; i++){
		int e = i;
		for (unsigned int laps = 0; laps < 3; laps++){
			indices[indx++] = e;
			if (flip){
				if (++e > 3)
					e = 0;
			}
			else{
				if (--e < 0)
					e = 3;
			}
		}
		flip = !flip;
	}
#endif

	D3D11_BUFFER_DESC indx_Cube_desc;
	ZeroMemory(&indx_Cube_desc, sizeof(D3D11_BUFFER_DESC));
	indx_Cube_desc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	indx_Cube_desc.ByteWidth = sizeof(unsigned int)*12;
	indx_Cube_desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indx_subRes_cube;
	ZeroMemory(&indx_subRes_cube, sizeof(D3D11_SUBRESOURCE_DATA));
	indx_subRes_cube.pSysMem = indices;

	iDevice->CreateBuffer(&indx_Cube_desc, &indx_subRes_cube, &ib_Cube);

	XMStoreFloat4x4(&m_CubeWorld,XMMatrixIdentity());

	m_view = XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, -1.5f, 1.0f);

	XMMATRIX rotation_trix;
	XMMATRIX view = XMLoadFloat4x4(&m_view);
	rotation_trix = XMMatrixRotationX(XMConvertToRadians(18));
	view = XMMatrixMultiply(view, rotation_trix);
	XMVECTOR determinant = XMMatrixDeterminant(view);
	view = XMMatrixInverse(nullptr, view);
	view = XMMatrixTranspose(view);
	XMStoreFloat4x4(&m_view, view);

	// zNear = 0.1;
	// zFar = 10
	// vFOV = 90
	float aspect = BACKBUFFER_WIDTH / BACKBUFFER_HEIGHT;
	m_Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(90.0f), aspect, 0.1f, 1000.0f);

	toShader_perspective.projection = XMMatrixTranspose(m_Projection);
	toShader_perspective.view = XMLoadFloat4x4(&m_view);	

	XMMATRIX model = XMLoadFloat4x4(&m_CubeWorld);
	model = XMMatrixTranslation(0, 0, 0);
	toShader_perspective.model = XMMatrixTranspose(model);
	XMStoreFloat4x4(&m_CubeWorld, model);


	iDevice->CreateVertexShader(&SampleVertexShader, sizeof(SampleVertexShader), NULL, &VertSha_perspective);
	iDevice->CreatePixelShader(&SamplePixelShader, sizeof(SamplePixelShader), NULL, &PixSha_perspective);

	D3D11_INPUT_ELEMENT_DESC layout3d[2];
	layout3d[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	layout3d[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };

	iDevice->CreateInputLayout(layout3d, 2, &SampleVertexShader, sizeof(SampleVertexShader), &lay_perspective);

	D3D11_BUFFER_DESC cb_3d;
	ZeroMemory(&cb_3d, sizeof(D3D11_BUFFER_DESC));
	cb_3d.Usage = D3D11_USAGE_DYNAMIC;
	cb_3d.ByteWidth = sizeof(cbMirror_3D);
	cb_3d.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb_3d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	iDevice->CreateBuffer(&cb_3d, NULL, &cBuff_perspective);

// <mah 3D />


	// TODO: PART 2 STEP 3a 
	SIMPLE_VERTEX mahCircle[360];
	for (unsigned int i = 0; i < vCount_Crcl; ++i){
		mahCircle[i].POSITION = XMFLOAT2(cos((3.14159265f * 0.005556f)*i), sin((3.14159265f * 0.005556f)*i));
	}

	// BEGIN PART 4
	// TODO: PART 4 STEP 1
	for (unsigned int i = 0; i < vCount_Crcl; ++i){
		mahCircle[i].POSITION.x *= 0.2f;
		mahCircle[i].POSITION.y *= 0.2f;
	}

	// TODO: PART 2 STEP 3b
	D3D11_BUFFER_DESC buffDesc;
	ZeroMemory(&buffDesc, sizeof(D3D11_BUFFER_DESC));
	buffDesc.Usage =  D3D11_USAGE_IMMUTABLE;
	buffDesc.ByteWidth = sizeof(SIMPLE_VERTEX)*360;
	buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	
    // TODO: PART 2 STEP 3c
	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	resourceData.pSysMem = mahCircle;
	
	// TODO: PART 2 STEP 3d
	iDevice->CreateBuffer(&buffDesc, &resourceData, &vBuffer);
	
	// TODO: PART 5 STEP 2a
	SIMPLE_VERTEX staggerdGrid[1200];

	// TODO: PART 5 STEP 2b
	XMFLOAT2 Zig[6];
	Zig[0] = XMFLOAT2(0.0f, 0.0f);
	Zig[1] = XMFLOAT2(0.1f, 0.0f);
	Zig[2] = XMFLOAT2(0.0f, -0.1f);
	Zig[3] = XMFLOAT2(0.1f, 0.0f);
	Zig[4] = XMFLOAT2(0.1f, -0.1f);
	Zig[5] = XMFLOAT2(0.0f, -0.1f);

	indx = 0;
	XMFLOAT2 OFF = XMFLOAT2(-0.9f,1.0f);
	for (; OFF.y > -0.99999f; OFF.y -= 0.1f){
		for (; OFF.x < 0.99999f; OFF.x += 0.2f){
			staggerdGrid[indx++].POSITION = XMFLOAT2(OFF.x + Zig[0].x, OFF.y + Zig[0].y);
			staggerdGrid[indx++].POSITION = XMFLOAT2(OFF.x + Zig[1].x, OFF.y + Zig[1].y);
			staggerdGrid[indx++].POSITION = XMFLOAT2(OFF.x + Zig[2].x, OFF.y + Zig[2].y);
			staggerdGrid[indx++].POSITION = XMFLOAT2(OFF.x + Zig[3].x, OFF.y + Zig[3].y);
			staggerdGrid[indx++].POSITION = XMFLOAT2(OFF.x + Zig[4].x, OFF.y + Zig[4].y);
			staggerdGrid[indx++].POSITION = XMFLOAT2(OFF.x + Zig[5].x, OFF.y + Zig[5].y);
		}
		OFF.x -= 2.1f;
		OFF.y -= 0.1f;
		for (; OFF.x < 0.99999f; OFF.x += 0.2f){
			staggerdGrid[indx++].POSITION = XMFLOAT2(OFF.x + Zig[0].x, OFF.y + Zig[0].y);
			staggerdGrid[indx++].POSITION = XMFLOAT2(OFF.x + Zig[1].x, OFF.y + Zig[1].y);
			staggerdGrid[indx++].POSITION = XMFLOAT2(OFF.x + Zig[2].x, OFF.y + Zig[2].y);
			staggerdGrid[indx++].POSITION = XMFLOAT2(OFF.x + Zig[3].x, OFF.y + Zig[3].y);
			staggerdGrid[indx++].POSITION = XMFLOAT2(OFF.x + Zig[4].x, OFF.y + Zig[4].y);
			staggerdGrid[indx++].POSITION = XMFLOAT2(OFF.x + Zig[5].x, OFF.y + Zig[5].y);
		}
		OFF.x -= 1.9f;
	}
	// TODO: PART 5 STEP 3
	D3D11_BUFFER_DESC checkDesc;
	ZeroMemory(&checkDesc, sizeof(D3D11_BUFFER_DESC));
	checkDesc.Usage = D3D11_USAGE_IMMUTABLE;
	checkDesc.ByteWidth = sizeof(SIMPLE_VERTEX)*vCount_Checkers;
	checkDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA checkerResource;
	ZeroMemory(&checkerResource, sizeof(D3D11_SUBRESOURCE_DATA));
	checkerResource.pSysMem = staggerdGrid;
	iDevice->CreateBuffer(&checkDesc, &checkerResource, &checkers);
	
		
	// TODO: PART 2 STEP 5
	// ADD SHADERS TO PROJECT, SET BUILD OPTIONS & COMPILE

	// TODO: PART 2 STEP 7
	iDevice->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &iVertShader);
	iDevice->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &iPixShader);

	// TODO: PART 2 STEP 8a
	D3D11_INPUT_ELEMENT_DESC layoutDesc;
	layoutDesc = { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };

	// TODO: PART 2 STEP 8b
	iDevice->CreateInputLayout(&layoutDesc, 1, Trivial_VS, sizeof(Trivial_VS), &iLayout);

	// TODO: PART 3 STEP 3
	D3D11_BUFFER_DESC cBuffDesc;
	ZeroMemory(&cBuffDesc, sizeof(D3D11_BUFFER_DESC));
	cBuffDesc.Usage =  D3D11_USAGE_DYNAMIC;
	cBuffDesc.ByteWidth = sizeof(SEND_TO_VRAM);
	cBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	iDevice->CreateBuffer(&cBuffDesc, NULL, &constBuffer);


	// TODO: PART 3 STEP 4b
	toShader.constantColor.x = 1.0f;
	toShader.constantColor.y = 1.0f;
	toShader.constantColor.z = 0.0f;
	toShader.constantColor.w = 1.0f;
	toShader.constantOffset.x = 0;
	toShader.constantOffset.y = 0;
	toShader.padding.x = 0;
	toShader.padding.y = 0;

	speed.x = 1;
	speed.y = 1;
	turn = 0.07f;
	timeX.Throttle(60);

}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	// TODO: PART 4 STEP 2	
	timeX.Signal();


	// TODO: PART 4 STEP 3
	//NOTE: speed is declared in the class declaration at the top.Assingment is done in the end of the constructor.
		speed.x = static_cast<float>(1.0f*timeX.Delta() * (speed.x < 0 ? -1 : 1));
		  speed.y = static_cast<float>(0.5f*timeX.Delta() * (speed.y < 0 ? -1 : 1));
		  toShader.constantOffset.x += speed.x;
		  toShader.constantOffset.y += speed.y;

		  //// TODO: PART 4 STEP 5

		  if (toShader.constantOffset.x > 1 || toShader.constantOffset.x < -1){
			  speed.x *= -1;
			  toShader.constantOffset.x += speed.x;
		  }
		  if (toShader.constantOffset.y >1 || toShader.constantOffset.y < -1){
			  speed.y *= -1;
			  toShader.constantOffset.y += speed.y;
		  }
	

	// END PART 4

	// TODO: PART 1 STEP 7a
	iDeviceContext->OMSetRenderTargets(1, &iRenderTarget, NULL);

	// TODO: PART 1 STEP 7b
	iDeviceContext->RSSetViewports(1, &viewPort);

	// TODO: PART 1 STEP 7c
	FLOAT DarkBlue[] = { 0.0f, 0.0f, 0.45f, 1.0f };
	iDeviceContext->ClearRenderTargetView(iRenderTarget, DarkBlue);

// <Mah 3d>
	XMMATRIX cubeWorld= XMLoadFloat4x4(&m_CubeWorld);
	cubeWorld = XMMatrixRotationY(XMConvertToRadians(turn))*cubeWorld;
	toShader_perspective.model = XMMatrixTranspose(cubeWorld);
	XMStoreFloat4x4(&m_CubeWorld, cubeWorld);

	//ZeroMemory(&toShader_perspective, sizeof(cbMirror_3D));
	D3D11_MAPPED_SUBRESOURCE map_cube;
	ZeroMemory(&map_cube, sizeof(D3D11_MAPPED_SUBRESOURCE));
	iDeviceContext->Map(cBuff_perspective, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, NULL, &map_cube);
	memcpy(map_cube.pData, &toShader_perspective, sizeof(toShader_perspective));
	iDeviceContext->Unmap(cBuff_perspective, 0);
	iDeviceContext->VSSetConstantBuffers(0, 1, &cBuff_perspective);

	UINT _startSlot = 0;
	UINT _numBuffs = 1;
	UINT _strides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	UINT _offSets[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	ZeroMemory(_strides, sizeof(_strides));
	ZeroMemory(_offSets, sizeof(_offSets));
	_strides[0] = static_cast<UINT>(sizeof(VERTEX_3D));
	iDeviceContext->IASetVertexBuffers(0, 1, &vb_Cube, _strides, _offSets);

	iDeviceContext->IASetIndexBuffer(ib_Cube, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

	iDeviceContext->VSSetShader(VertSha_perspective, NULL, NULL);
	iDeviceContext->PSSetShader(PixSha_perspective, NULL, NULL);

	iDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	iDeviceContext->IASetInputLayout(lay_perspective);
	iDeviceContext->DrawIndexed(12, 0, 0);

// <Mah 3d/>

#if 0
	// TODO: PART 5 STEP 4
	ZeroMemory(&toShader_2, sizeof(SEND_TO_VRAM));
	// TODO: PART 5 STEP 5
	D3D11_MAPPED_SUBRESOURCE mapResource;
	ZeroMemory(&mapResource, sizeof(mapResource));
	iDeviceContext->Map(constBuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, NULL, &mapResource);
	memcpy(mapResource.pData, &toShader_2, sizeof(toShader_2));
	iDeviceContext->Unmap(constBuffer, 0);
	iDeviceContext->VSSetConstantBuffers(0, 1, &constBuffer);

	// TODO: PART 5 STEP 6
	_startSlot = 0;
	_numBuffs = 1;
	_strides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	_offSets[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	ZeroMemory(_strides, sizeof(_strides));
	ZeroMemory(_offSets, sizeof(_offSets));
	_strides[0] = static_cast<UINT>(sizeof(SIMPLE_VERTEX));
	iDeviceContext->IASetVertexBuffers(0, 1, &checkers, _strides, _offSets);

	iDeviceContext->VSSetShader(iVertShader, NULL, NULL);
	iDeviceContext->PSSetShader(iPixShader, NULL, NULL);

	iDeviceContext->IASetInputLayout(iLayout);
	
	iDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	iDeviceContext->Draw(vCount_Checkers, 0);
	// TODO: PART 5 STEP 7

	// END PART 5

	// TODO: PART 3 STEP 5
	ZeroMemory(&mapResource, sizeof(mapResource));
	iDeviceContext->Map(constBuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, NULL, &mapResource );
	memcpy(mapResource.pData, &toShader, sizeof(toShader));
	iDeviceContext->Unmap(constBuffer, 0);
	
	// TODO: PART 3 STEP 6
	iDeviceContext->VSSetConstantBuffers(0, 1, &constBuffer);
	
	// TODO: PART 2 STEP 9a
	UINT startSlot = 0;
	UINT numBuffs = 1;
	UINT strides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT]; 
	UINT offSets[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	ZeroMemory(strides, sizeof(strides));
	ZeroMemory(offSets, sizeof(offSets));
	strides[0] = static_cast<UINT>(sizeof(SIMPLE_VERTEX));
	iDeviceContext->IASetVertexBuffers( 0, 1, &vBuffer, strides, offSets );

	// TODO: PART 2 STEP 9b
	iDeviceContext->VSSetShader(iVertShader, NULL, NULL);
	iDeviceContext->PSSetShader(iPixShader, NULL, NULL);

	// TODO: PART 2 STEP 9c
	iDeviceContext->IASetInputLayout(iLayout);

	// TODO: PART 2 STEP 9d
	iDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	// TODO: PART 2 STEP 10
	iDeviceContext->Draw(vCount_Crcl, 0);
#endif

	// END PART 2

	// TODO: PART 1 STEP 8
	swapChain->Present(0, 0);
	// END OF PART 1
	return true; 
}

//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool DEMO_APP::ShutDown()
{
	// TODO: PART 1 STEP 6
	iDeviceContext->ClearState();
	iRenderTarget->Release();
	iDevice->Release();
	swapChain->Release();
	iDeviceContext->Release();

	iLayout->Release();
	iVertShader->Release();
	iPixShader->Release();
	vBuffer->Release();
	constBuffer->Release();
	checkers->Release();

	lay_perspective->Release();
	cBuff_perspective->Release();
	VertSha_perspective->Release();
	PixSha_perspective->Release();
	vb_Cube->Release();
	ib_Cube->Release();
	//vb_Grid->Release();



	UnregisterClass( L"DirectXApplication", application ); 
	return true;
}

//************************************************************
//************ WINDOWS RELATED *******************************
//************************************************************

// ****************** BEGIN WARNING ***********************// 
// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY!
	
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,	int nCmdShow );						   
LRESULT CALLBACK WndProc(HWND hWnd,	UINT message, WPARAM wparam, LPARAM lparam );		
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE, LPTSTR, int )
{
	srand(unsigned int(time(0)));
	DEMO_APP myApp(hInstance,(WNDPROC)WndProc);	
    MSG msg; ZeroMemory( &msg, sizeof( msg ) );
    while ( msg.message != WM_QUIT && myApp.Run() )
    {	
	    if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        { 
            TranslateMessage( &msg );
            DispatchMessage( &msg ); 
        }
    }
	myApp.ShutDown(); 
	return 0; 
}
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    if(GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
    switch ( message )
    {
        case ( WM_DESTROY ): { PostQuitMessage( 0 ); }
        break;
    }
    return DefWindowProc( hWnd, message, wParam, lParam );
}
//********************* END WARNING ************************//