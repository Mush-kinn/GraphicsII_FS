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
#include <vector>

using namespace std;

#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")
#include <DirectXMath.h>
using namespace DirectX;
#include "Assets\Test_UV_Map.h"


#include "Trivial_PS.csh"
#include "Trivial_VS.csh"

#include "SampleVertexShader.csh"
#include "SamplePixelShader.csh"

#include "RTT_PixelShader.csh"

#define BACKBUFFER_WIDTH	500
#define BACKBUFFER_HEIGHT	500

#define HOVERCAM_WITDH (BACKBUFFER_WIDTH * 0.25)
#define HOVERCAM_HEIGHT (BACKBUFFER_HEIGHT * 0.25)

// Warning: Testure #defines, keep collapsed. 
#pragma region WARNING_TexttureArraysDefines

#define TEST_UV_MAP_PIXELS Test_UV_Map_pixels

#pragma endregion

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
	XTime timeX;
	unsigned int ObjIndxCount;

	// Matrices
	XMFLOAT4X4 m_view;
	XMMATRIX m_Projection;
	XMFLOAT4X4 m_CubeWorld;
	XMFLOAT4X4 m_hoverCam;
	XMFLOAT4X4 m_RTTView;
	XMFLOAT4X4 m_RTTProjection;

	// Buffers
	ID3D11Buffer *vb_Cube;
	ID3D11Buffer *ib_Cube;
	ID3D11Buffer *cBuff_perspective;
	ID3D11Buffer *vb_Platform;
	ID3D11Buffer *ib_Platform;

	// Layouts
	ID3D11InputLayout *lay_perspective;
	ID3D11InputLayout *lay_OBJModel;

	// Shaders
	ID3D11VertexShader *VertSha_perspective;
	ID3D11PixelShader *PixSha_perspective;
	ID3D11PixelShader *PixSha_RTT;

	// textures
	ID3D11Texture2D *tx_UVMap;
	ID3D11Texture2D *tx_RTT;

	// Views
	ID3D11RenderTargetView *iRenderTarget;
	ID3D11ShaderResourceView *ShaderView;
	ID3D11RenderTargetView *RTT_RenderTarget;
	D3D11_VIEWPORT hovCam_view;
	ID3D11ShaderResourceView *RTT_ShaderView;

	// Pending...
	ID3D11SamplerState *SampleState;

	ID3D11Device *iDevice;
	ID3D11DeviceContext *iDeviceContext;
	D3D11_VIEWPORT viewPort;
	IDXGISwapChain *swapChain;
	struct SEND_TO_VRAM
	{
		XMFLOAT4 constantColor;
		XMFLOAT2 constantOffset;
		XMFLOAT2 padding;
	};

	struct cbMirror_Perspective{
		XMMATRIX model;
		XMMATRIX view;
		XMMATRIX projection;
	};
	
	SEND_TO_VRAM toShader;
	cbMirror_Perspective toShader_perspective;
	cbMirror_Perspective toshader_hoverCam;
	cbMirror_Perspective toShader_RTT;

public:

	struct SIMPLE_VERTEX{
		XMFLOAT2 POSITION;
	};

	struct VERTEX_3D{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};

	struct VERTEX_OBJMODEL{
		XMFLOAT3 pos;
		XMFLOAT3 uv;
		XMFLOAT3 norm;
	};
	
	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
	bool LoadObjFile(const char *_filename, std::vector<VERTEX_OBJMODEL> &_forVB);
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

#if _DEBUG
	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, 
		&FeatureLevelsRequested, numLevelsRequested, D3D11_SDK_VERSION, &chainDesc, &swapChain, 
		&iDevice, &FeatureLevelsSupported, &iDeviceContext);
#else
	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, 
		&FeatureLevelsRequested, numLevelsRequested, D3D11_SDK_VERSION, &chainDesc, &swapChain, 
		&iDevice, &FeatureLevelsSupported, &iDeviceContext);
#endif

	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(D3D11_SAMPLER_DESC));
	sampler_desc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = Test_UV_Map_numlevels;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_GREATER_EQUAL;
	
	iDevice->CreateSamplerState(&sampler_desc, &SampleState);
	
	D3D11_TEXTURE2D_DESC tx_UV_Desc;
	ZeroMemory(&tx_UV_Desc, sizeof(D3D11_TEXTURE2D_DESC));
	tx_UV_Desc.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
	tx_UV_Desc.Format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	tx_UV_Desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	tx_UV_Desc.ArraySize = 1;
	tx_UV_Desc.MipLevels = Test_UV_Map_numlevels;
	tx_UV_Desc.Height = Test_UV_Map_height;
	tx_UV_Desc.Width = Test_UV_Map_width;
	tx_UV_Desc.SampleDesc.Count = 1;

	D3D11_SUBRESOURCE_DATA tx_UV_Data[Test_UV_Map_numlevels];
	for (unsigned int i = 0; i < Test_UV_Map_numlevels; ++i){
		ZeroMemory(&tx_UV_Data[i], sizeof(tx_UV_Data[i]));
		tx_UV_Data[i].pSysMem = &TEST_UV_MAP_PIXELS[Test_UV_Map_leveloffsets[i]];
		tx_UV_Data[i].SysMemPitch = (Test_UV_Map_width >> i) * sizeof(unsigned int);
	}

	iDevice->CreateTexture2D(&tx_UV_Desc, tx_UV_Data, &tx_UVMap);

	D3D11_SHADER_RESOURCE_VIEW_DESC Shaderview_desc;
	ZeroMemory(&Shaderview_desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	Shaderview_desc.Format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	Shaderview_desc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	Shaderview_desc.Texture2DArray.ArraySize = 1;
	Shaderview_desc.Texture2DArray.FirstArraySlice = 0;
	Shaderview_desc.Texture2DArray.MipLevels = Test_UV_Map_numlevels;
	Shaderview_desc.Texture2DArray.MostDetailedMip = 0;

	iDevice->CreateShaderResourceView(tx_UVMap, &Shaderview_desc, &ShaderView);

	ID3D11Resource *iResource;
	ZeroMemory(&iResource, sizeof(ID3D10Resource));
	swapChain->GetBuffer(0, __uuidof(iResource), reinterpret_cast<void**>(&iResource));
	iDevice->CreateRenderTargetView(iResource, NULL, &iRenderTarget);
	iResource->Release(); 

	swapChain->GetDesc(&chainDesc);
	ZeroMemory(&viewPort, sizeof(D3D11_VIEWPORT));
	viewPort.Height = static_cast<FLOAT>(chainDesc.BufferDesc.Height);
	viewPort.Width = static_cast<FLOAT>(chainDesc.BufferDesc.Width);
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;


// <Mah hovercam>
	ZeroMemory(&hovCam_view, sizeof(hovCam_view));
	hovCam_view.Height = HOVERCAM_HEIGHT;
	hovCam_view.Width = HOVERCAM_WITDH;
	hovCam_view.TopLeftX = BACKBUFFER_WIDTH * 0.65;
	hovCam_view.TopLeftY = BACKBUFFER_HEIGHT * 0.75;
	hovCam_view.MinDepth = 0.0f;
	hovCam_view.MaxDepth = 1.0f;

// <mah hovercam/>

// <Mah 3D>
	unsigned int indx = 0;

	VERTEX_3D aTri[4] = { 
			{ XMFLOAT3(0, 0.8f, 0), XMFLOAT2(0.5f, 0)},
			{ XMFLOAT3(0.7f, 0, -0.4f),XMFLOAT2(1,1)},
			{ XMFLOAT3(-0.7f, 0, -0.4f),XMFLOAT2(0,1)}, 
			{ XMFLOAT3(0, 0.4f, 0), XMFLOAT2( 0.5f,0.5 )} 
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
	model = XMMatrixTranslation(0.7f, 0, 0);
	toShader_perspective.model = XMMatrixTranspose(model);
	XMStoreFloat4x4(&m_CubeWorld, model);

// <mah hovercam>
	aspect = HOVERCAM_WITDH / HOVERCAM_HEIGHT;
	toshader_hoverCam.projection = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XMConvertToRadians(90.0f), aspect, 0.1f, 1000.0f));

	view = XMMatrixIdentity();
	view = XMMatrixTranslation(0.0f, 1.5f, -1.7f);
	XMFLOAT3 focus, up;
	focus = XMFLOAT3(0, 0, 0);
	up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	rotation_trix = XMMatrixLookAtLH(view.r[3], XMLoadFloat3(&focus), XMLoadFloat3(&up));
	view = rotation_trix;
	toshader_hoverCam.view = XMMatrixTranspose(view);
	XMStoreFloat4x4(&m_hoverCam, view);
	
// <mah hovercam/>

// <RTT>
	aspect = BACKBUFFER_WIDTH / BACKBUFFER_HEIGHT;
	XMMATRIX projection = XMMatrixPerspectiveFovLH(XMConvertToDegrees(90.0f), aspect, 0.1f, 1000.0f);
	toShader_RTT.projection = XMMatrixTranspose(projection);
	XMStoreFloat4x4(&m_RTTProjection, projection);

	view = XMMatrixIdentity();
	view = XMMatrixTranslation(0.0f, 0.0f, -1.5f);
	rotation_trix = XMMatrixRotationX(XMConvertToRadians(18));
	view = rotation_trix * view;
	view = XMMatrixInverse(nullptr, view);
	XMStoreFloat4x4(&m_RTTView, view);
	toShader_RTT.view = XMMatrixTranspose(view);

	toShader_RTT.model = XMMatrixIdentity();

	D3D11_TEXTURE2D_DESC tempRTT_Desc;
	ZeroMemory(&tempRTT_Desc, sizeof(D3D11_TEXTURE2D_DESC));
	tempRTT_Desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	tempRTT_Desc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	tempRTT_Desc.MiscFlags = D3D11_RESOURCE_MISC_FLAG::D3D11_RESOURCE_MISC_GENERATE_MIPS;
	tempRTT_Desc.ArraySize = 1;
	tempRTT_Desc.Height = BACKBUFFER_HEIGHT;
	tempRTT_Desc.Width = BACKBUFFER_WIDTH;
	tempRTT_Desc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	tempRTT_Desc.SampleDesc.Count = 1;

	iDevice->CreateTexture2D(&tempRTT_Desc, NULL, &tx_RTT);

	D3D11_RENDER_TARGET_VIEW_DESC render_desc;
	ZeroMemory(&render_desc, sizeof(render_desc));
	render_desc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	render_desc.ViewDimension = D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2D;
	render_desc.Texture2D.MipSlice = 0;

	iDevice->CreateRenderTargetView(tx_RTT, &render_desc, &RTT_RenderTarget);

	D3D11_SHADER_RESOURCE_VIEW_DESC RTTShaderview_desc;
	ZeroMemory(&RTTShaderview_desc, sizeof(RTTShaderview_desc));
	RTTShaderview_desc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	RTTShaderview_desc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;
	RTTShaderview_desc.Texture2D.MipLevels = 1;
	RTTShaderview_desc.Texture2D.MostDetailedMip = 0;

	iDevice->CreateShaderResourceView(tx_RTT, &RTTShaderview_desc, &RTT_ShaderView);

	iDevice->CreatePixelShader(&RTT_PixelShader, sizeof(RTT_PixelShader), NULL, &PixSha_RTT);

	
// <RTT/>

	iDevice->CreateVertexShader(&SampleVertexShader, sizeof(SampleVertexShader), NULL, &VertSha_perspective);
	iDevice->CreatePixelShader(&SamplePixelShader, sizeof(SamplePixelShader), NULL, &PixSha_perspective);

	D3D11_INPUT_ELEMENT_DESC layout3d[2];
	layout3d[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	layout3d[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };

	iDevice->CreateInputLayout(layout3d, 2, &SampleVertexShader, sizeof(SampleVertexShader), &lay_perspective);

	D3D11_BUFFER_DESC cb_3d;
	ZeroMemory(&cb_3d, sizeof(D3D11_BUFFER_DESC));
	cb_3d.Usage = D3D11_USAGE_DYNAMIC;
	cb_3d.ByteWidth = sizeof(cbMirror_Perspective);
	cb_3d.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb_3d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	iDevice->CreateBuffer(&cb_3d, NULL, &cBuff_perspective);

// <mah 3D />


// <Prototype Loader>
	std::vector<VERTEX_OBJMODEL> vObjModel;
	LoadObjFile("Assets\\BasicPlatform\\wall.obj", vObjModel);

	D3D11_BUFFER_DESC vModel_desc;
	ZeroMemory(&vModel_desc, sizeof(D3D11_BUFFER_DESC));
	vModel_desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	vModel_desc.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
	vModel_desc.ByteWidth = sizeof(VERTEX_OBJMODEL)*vObjModel.size();

	D3D11_SUBRESOURCE_DATA vSub_Model;
	ZeroMemory(&vSub_Model, sizeof(D3D11_SUBRESOURCE_DATA));
	vSub_Model.pSysMem = vObjModel.data();

	iDevice->CreateBuffer(&vModel_desc, &vSub_Model, &vb_Platform);


	std::vector<unsigned int> modelIndices;
	unsigned int Fst, Mid, Trd;
	for (unsigned int i = 0; i < vObjModel.size(); i+=4){
		Fst = i;
		Mid = i + 1;
		Trd = i + 2;
		
		modelIndices.push_back(Fst);
		modelIndices.push_back(Mid);
		modelIndices.push_back(Trd);

		Fst = Trd;
		Mid = i + 3;
		Trd = i;

		modelIndices.push_back(Fst);
		modelIndices.push_back(Mid);
		modelIndices.push_back(Trd);
	}
	ObjIndxCount = modelIndices.size();

	D3D11_BUFFER_DESC iModel_desc;
	ZeroMemory(&iModel_desc, sizeof(D3D11_BUFFER_DESC));
	iModel_desc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	iModel_desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
	iModel_desc.ByteWidth = sizeof(unsigned int) * ObjIndxCount;

	ZeroMemory(&vSub_Model, sizeof(D3D11_SUBRESOURCE_DATA));
	vSub_Model.pSysMem = modelIndices.data();

	iDevice->CreateBuffer(&iModel_desc, &vSub_Model, &ib_Platform);

	D3D11_INPUT_ELEMENT_DESC layout_desc[3];

	layout_desc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	layout_desc[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	layout_desc[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };

	iDevice->CreateInputLayout(layout_desc, 3, &SampleVertexShader, sizeof(SampleVertexShader), &lay_OBJModel);

// <Prototype Loader/>

	turn = 12.0f;
	timeX.Throttle(60);

}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	timeX.Signal();

	iDeviceContext->OMSetRenderTargets(1, &iRenderTarget, NULL);

	FLOAT DarkBlue[] = { 0.0f, 0.0f, 0.45f, 1.0f };
	FLOAT Black[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	iDeviceContext->RSSetViewports(1, &viewPort);
	iDeviceContext->ClearRenderTargetView(iRenderTarget, DarkBlue);

// <Mah 3d>
	XMMATRIX cubeWorld= XMLoadFloat4x4(&m_CubeWorld);
	cubeWorld = XMMatrixRotationX(-XMConvertToRadians(turn*timeX.Delta()))*cubeWorld;
	toShader_perspective.model = XMMatrixTranspose(cubeWorld);
	XMStoreFloat4x4(&m_CubeWorld, cubeWorld);

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

	iDeviceContext->PSSetSamplers(0, 1, &SampleState);
	iDeviceContext->PSSetShaderResources(0, 1, &ShaderView);
	 
	iDeviceContext->VSSetShader(VertSha_perspective, NULL, NULL);
	iDeviceContext->PSSetShader(PixSha_perspective, NULL, NULL);

	iDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	iDeviceContext->IASetInputLayout(lay_perspective);
	iDeviceContext->DrawIndexed(12, 0, 0);


// <Mah 3d/>

// <RTT>
	iDeviceContext->ClearRenderTargetView(RTT_RenderTarget, Black);
	iDeviceContext->OMSetRenderTargets(1, &RTT_RenderTarget, NULL);
	cubeWorld = XMLoadFloat4x4(&m_CubeWorld);
	toShader_RTT.model = XMMatrixTranspose(cubeWorld);
	
	ZeroMemory(&map_cube, sizeof(D3D11_MAPPED_SUBRESOURCE));
	iDeviceContext->Map(cBuff_perspective, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, NULL, &map_cube);
	memcpy(map_cube.pData, &toShader_RTT, sizeof(toShader_RTT));
	iDeviceContext->Unmap(cBuff_perspective, 0);
	iDeviceContext->VSSetConstantBuffers(0, 1, &cBuff_perspective);

	iDeviceContext->DrawIndexed(12, 0, 0);

	iDeviceContext->OMSetRenderTargets(1, &iRenderTarget, NULL);
	iDeviceContext->PSSetShaderResources(0, 1, &RTT_ShaderView);

// <RTT/>

// <Model Loader>
	cubeWorld = XMLoadFloat4x4(&m_CubeWorld);
	cubeWorld = XMMatrixScaling(0.25f, 0.25f, 0.25f) * cubeWorld;
	cubeWorld = cubeWorld * XMMatrixTranslation(-1.0, 0, 0);
	toShader_perspective.model = XMMatrixTranspose(cubeWorld);

	ZeroMemory(&map_cube, sizeof(D3D11_MAPPED_SUBRESOURCE));
	iDeviceContext->Map(cBuff_perspective, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, NULL, &map_cube);
	memcpy(map_cube.pData, &toShader_perspective, sizeof(toShader_perspective));
	iDeviceContext->Unmap(cBuff_perspective, 0);
	iDeviceContext->VSSetConstantBuffers(0, 1, &cBuff_perspective);

	UINT strides = sizeof(VERTEX_OBJMODEL);
	UINT offsets = 0;
	iDeviceContext->IASetVertexBuffers(0, 1, &vb_Platform, &strides, &offsets);

	iDeviceContext->IASetIndexBuffer(ib_Platform, DXGI_FORMAT_R32_UINT, 0);

	iDeviceContext->IASetInputLayout(lay_OBJModel);

// <RTT>
	iDeviceContext->PSSetShader(PixSha_RTT, 0, 0);

	iDeviceContext->DrawIndexed(ObjIndxCount, 0, 0);
// <Model loader/>

	iDeviceContext->PSSetShader(PixSha_perspective,0,0);
// <RTT/>


// <MultiViewport>
	iDeviceContext->PSSetShaderResources(0, 1, &ShaderView);
	XMMATRIX Hover = XMLoadFloat4x4(&m_hoverCam);
	Hover = XMMatrixRotationY(XMConvertToRadians(turn*timeX.Delta() * -3.2f)) * Hover;
	XMStoreFloat4x4(&m_hoverCam, Hover);
	cubeWorld = XMLoadFloat4x4(&m_CubeWorld);
	toshader_hoverCam.model = XMMatrixTranspose(cubeWorld);
	toshader_hoverCam.view = XMMatrixTranspose(Hover);

	ZeroMemory(&map_cube, sizeof(D3D11_MAPPED_SUBRESOURCE));
	iDeviceContext->Map(cBuff_perspective, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, NULL, &map_cube);
	memcpy(map_cube.pData, &toshader_hoverCam, sizeof(toshader_hoverCam));
	iDeviceContext->Unmap(cBuff_perspective, 0);
	iDeviceContext->VSSetConstantBuffers(0, 1, &cBuff_perspective);

	iDeviceContext->IASetVertexBuffers(0, 1, &vb_Cube, _strides, _offSets);
	iDeviceContext->IASetIndexBuffer(ib_Cube, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	iDeviceContext->IASetInputLayout(lay_perspective);
	iDeviceContext->RSSetViewports(1, &hovCam_view);
	iDeviceContext->DrawIndexed(12, 0, 0);
// <MultiViewport/>

	swapChain->Present(0, 0);

	return true; 
}

//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool DEMO_APP::ShutDown()
{
	iDeviceContext->ClearState();
	iRenderTarget->Release();
	iDevice->Release();
	swapChain->Release();
	iDeviceContext->Release();

	lay_perspective->Release();
	cBuff_perspective->Release();
	VertSha_perspective->Release();
	PixSha_perspective->Release();
	vb_Cube->Release();
	ib_Cube->Release();
	
	ShaderView->Release();
	tx_UVMap->Release();
	SampleState->Release();

	ib_Platform->Release();
	vb_Platform->Release();
	lay_OBJModel->Release();

	RTT_RenderTarget->Release();
	RTT_ShaderView->Release();
	tx_RTT->Release();
	PixSha_RTT->Release();

	UnregisterClass( L"DirectXApplication", application ); 
	return true;
}
//#include "Assets\BasicPlatform\Wall.obj
bool DEMO_APP::LoadObjFile(const char *_filename, std::vector<VERTEX_OBJMODEL> &_forVB){
	std::vector<XMFLOAT3> vertexHold;
	std::vector<XMFLOAT3> uvHold;
	std::vector<XMFLOAT3> normalHold;
	std::vector<unsigned int> indx_Vector, indx_UVs, indx_Normal;

	FILE *file = fopen( _filename, "r");
	if (file == NULL){
		printf("Connaot read this thing\n");
		return false;
	}
	for (;;){
		char mahLine[70];
		int result = fscanf(file, "%s", mahLine);
		if (result == EOF){
			break;
		}
		if (strcmp(mahLine, "v") == 0 ){
			XMFLOAT3 temp;
			fscanf(file, "%f %f %f\n", &temp.x, &temp.y, &temp.z);
			vertexHold.push_back(temp);
		}
		else if (strcmp(mahLine, "vt") == 0){
			XMFLOAT3 temp;
			fscanf(file, "%f %f %f\n", &temp.x, &temp.y, &temp.z);
			uvHold.push_back(temp);
		}
		else if (strcmp(mahLine, "vn") == 0){
			XMFLOAT3 temp;
			fscanf(file, "%f %f %f\n", &temp.x, &temp.y, &temp.z);
			normalHold.push_back(temp);
		}
		else if (strcmp(mahLine, "f") == 0){
			VERTEX_OBJMODEL temp;
			unsigned int vert[4];
			unsigned int UVs[4];
			unsigned int normals[4];
			int count = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n", &vert[0], &UVs[0], &normals[0], &vert[1], &UVs[1], &normals[1],
				&vert[2], &UVs[2], &normals[2], &vert[3], &UVs[3], &normals[3]);
			if (count != 12){
				printf("Oops mistake were made");
				return false;
			}
			for (unsigned int i = 0; i < 4; ++i){
				temp.pos = vertexHold[vert[i] - 1];
				temp.uv = uvHold[UVs[i] - 1];
				temp.norm = normalHold[normals[i] - 1];
				_forVB.push_back(temp);
			}
		}
	}
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