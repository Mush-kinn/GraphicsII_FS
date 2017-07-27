//************************************************************
//************ INCLUDES & DEFINES ****************************
//************************************************************

#include <iostream>
#include <ctime>
#include "XTime.h"
#include <vector>
#include <thread>
//#include "DDSTextureLoader.h"

using namespace std;

#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")
#include <DirectXMath.h>
using namespace DirectX;
#include "Assets\Test_UV_Map.h"
#include "Assets\Portal\portal_bk.h"
#include "Assets\Portal\portal_dn.h"
#include "Assets\Portal\portal_ft.h"
#include "Assets\Portal\portal_lf.h"
#include "Assets\Portal\portal_rt.h"
#include "Assets\Portal\portal_up.h"

#include "AlphaDefines.h"

#include "Trivial_PS.csh"
#include "Trivial_VS.csh"
#include "SampleVertexShader.csh"
#include "SamplePixelShader.csh"
#include "RTT_PixelShader.csh"

#include "DDSTextureLoader.h"

#define BACKBUFFER_WIDTH	500
#define BACKBUFFER_HEIGHT	500

#define HOVERCAM_WITDH (BACKBUFFER_WIDTH * 0.25)
#define HOVERCAM_HEIGHT (BACKBUFFER_HEIGHT * 0.25)

// Warning: Testure #defines, keep collapsed. 
#pragma region WARNING_TexttureArraysDefines

#define TEST_UV_MAP_PIXELS Test_UV_Map_pixels

#define PORTAL_BK_PIXELS portal_bk_pixels

#define PORTAL_DN_PIXELS portal_dn_pixels

#define PORTAL_FT_PIXELS portal_ft_pixels

#define PORTAL_LF_PIXELS portal_lf_pixels

#define PORTAL_RT_PIXELS portal_rt_pixels

#define PORTAL_UP_PIXELS portal_up_pixels

#pragma endregion

enum MouseBehave{ MIA, IDLE, MOVING };
enum MouseStatus{ LOCKED, FREE };

enum ShadersSettings{ DEFAULT, CUSTOM, PREVIOUS, TEMP};
enum MahShaderType{ Pixel, Vertex, Geo };

//************************************************************
//************ SIMPLE WINDOWS APP CLASS **********************
//************************************************************

class DEMO_APP
{	
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;

	float speed = 4;
	float turn;
	XTime timeX;
	unsigned int ObjIndxCount;
	static bool mahKeys[256];
	static std::vector<UINT> KeyStateON;
	static std::vector<UINT> KeyStateOFF;
	MouseStatus MStatus = MouseStatus::FREE;
	
	// Matrices
	XMFLOAT4X4 m_view;
	XMMATRIX m_Projection;
	XMFLOAT4X4 m_CubeWorld;
	XMFLOAT4X4 m_hoverCam;
	XMFLOAT4X4 m_RTTView;
	XMFLOAT4X4 m_RTTProjection;
	XMFLOAT4X4 m_BoxWorld;

	XMFLOAT4X4 Spinny;

	// Vectors
	XMFLOAT3 newCamOffset;
	XMFLOAT3 Tracker_Up;
	XMFLOAT3 Tracker_Pos;
	XMFLOAT3 Tracker_Tgt;

	// Buffers
	ID3D11Buffer *vb_Cube;
	ID3D11Buffer *ib_Cube;
	ID3D11Buffer *cBuff_perspective;
	ID3D11Buffer *vb_Platform;
	ID3D11Buffer *ib_Platform;
	ID3D11Buffer *vb_Box;
	ID3D11Buffer *ib_Box;

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
	ID3D11Texture2D *tx_DepthStencil;
	ID3D11Texture2D *tx_Skybox;

	// Views
	ID3D11RenderTargetView *iRenderTarget;
	ID3D11ShaderResourceView *ShaderView;
	ID3D11RenderTargetView *RTT_RenderTarget;
	D3D11_VIEWPORT hovCam_view;
	ID3D11ShaderResourceView *RTT_ShaderView;
	ID3D11DepthStencilView *iDepthStencilView;
	ID3D11ShaderResourceView *SkyboxView;

	// Pending...
	ID3D11SamplerState *SampleState;
	ID3D11DeviceContext *DeffContext_Default;
	ID3D11DeviceContext *DeffCOntext_RTT;
	ID3D11DeviceContext *DeffContext_Model;
	ID3D11DeviceContext *DeffContext_Zero;

	ID3D11CommandList *CmdList_Default;
	ID3D11CommandList *CmdList_RTT;
	ID3D11CommandList *CmdList_Model;
	ID3D11CommandList *CmdList_Zero;

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
	
	ID3D11Debug *Debuger;
	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	static void UpdateKeyboardInput(UINT _key, bool _state, bool _toggle= false);
	void UpdateInput();
	bool Run();
	bool ShutDown();
	bool LoadObjFile(const char *_filename, std::vector<VERTEX_OBJMODEL> &_forVB);
	bool LoadDefault();
	bool LoadRTT();

	void RenderDefault( ID3D11DeviceContext *iDeviceContext , XMFLOAT3 _Offset);
	bool RenderRTT();
	bool RenderOBJ();

};

std::vector<UINT> DEMO_APP::KeyStateON;
std::vector<UINT> DEMO_APP::KeyStateOFF;
bool DEMO_APP::mahKeys[256] = {};

void DEMO_APP::UpdateKeyboardInput(UINT _key, bool _state, bool _toggle){
	if (_state && _toggle){
		return;
	}
	if (!_toggle){
		if (_state)
			KeyStateON.push_back(_key);
		else
			KeyStateOFF.push_back(_key);
	}
	// Toggle
	else{
		mahKeys[_key] ? KeyStateOFF.push_back(_key) : KeyStateON.push_back(_key);
	}
}

void DEMO_APP::UpdateInput(){
	while (!KeyStateON.empty())
	{
		mahKeys[KeyStateON.back()] = true;
		KeyStateON.pop_back();
	}
	while (!KeyStateOFF.empty()){
		mahKeys[KeyStateOFF.back()] = false;
		KeyStateOFF.pop_back();
	}
	
	float sDelt = (float)timeX.SmoothDelta();
	if (mahKeys[VK_A])
		newCamOffset.x -= speed * sDelt;
	if (mahKeys[VK_D])
		newCamOffset.x += speed * sDelt;
	if (mahKeys[VK_W])
		newCamOffset.z += speed * sDelt;
	if (mahKeys[VK_S])
		newCamOffset.z -= speed * sDelt;
	if (mahKeys[VK_Q])
		newCamOffset.y -= speed * sDelt;
	if (mahKeys[VK_E])
		newCamOffset.y += speed * sDelt;

		XMMATRIX temp, INverted;
		XMVECTOR Scale, Rot, Trans;

	if (!mahKeys[VK_T]){
		temp = XMMatrixIdentity();

		if (mahKeys[VK_NUMPAD4])
			temp = XMMatrixRotationY(XMConvertToRadians(-80 * sDelt)) *temp;
		if (mahKeys[VK_NUMPAD6])
			temp = XMMatrixRotationY(XMConvertToRadians(80 * sDelt)) *temp;

		INverted = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_view));
		XMMatrixDecompose(&Scale, &Rot, &Trans, INverted);
		temp = XMMatrixScalingFromVector(Scale) * XMMatrixRotationQuaternion(Rot) * temp;
		temp = temp * XMMatrixTranslationFromVector(Trans);

		if (mahKeys[VK_NUMPAD8])
			temp = XMMatrixRotationX(XMConvertToRadians(-80 * sDelt)) * temp;
		if (mahKeys[VK_NUMPAD2])
			temp = XMMatrixRotationX(XMConvertToRadians(80 * sDelt)) * temp;

		XMFLOAT4 MAX(1, 1, 1, 1), MIN(-1, 0, -1, 0);
		temp.r[1] = XMVectorClamp(temp.r[1], XMLoadFloat4(&MIN), XMLoadFloat4(&MAX));
		XMStoreFloat4x4(&Spinny, temp);

		temp = XMMatrixIdentity();
		temp = temp * XMMatrixTranslation(newCamOffset.x, newCamOffset.y, newCamOffset.z);
		temp = temp * XMLoadFloat4x4(&Spinny);
		XMStoreFloat4x4(&m_view, XMMatrixInverse(NULL, temp));
	}
	else{
		XMMATRIX temp = XMMatrixIdentity();
		INverted = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_view));
		XMMatrixDecompose(&Scale, &Rot, &Trans, INverted);
		XMVECTOR aVector = Trans;
		XMMatrixDecompose(&Scale, &Rot, &Trans, XMLoadFloat4x4(& m_CubeWorld));
		XMStoreFloat3(&Tracker_Tgt, Trans);
		temp = XMMatrixLookAtLH(aVector, XMLoadFloat3(&Tracker_Tgt), XMLoadFloat3(&Tracker_Up));
		temp = temp * XMMatrixInverse(NULL, XMMatrixTranslation(newCamOffset.x, newCamOffset.y, newCamOffset.z));
		XMStoreFloat4x4(&m_view, temp);
	}

	ZeroMemory(&newCamOffset, sizeof(newCamOffset));

	if (MStatus == MouseStatus::FREE && mahKeys[VK_CONTROL]){
		ShowCursor(false);
		MStatus = MouseStatus::LOCKED;
	}
	if (MStatus == MouseStatus::LOCKED && !mahKeys[VK_CONTROL]){
		ShowCursor(true);
		MStatus = MouseStatus::FREE;
	}

}

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

	iDevice->CreateDeferredContext(NULL, &DeffContext_Default);
	iDevice->CreateDeferredContext(NULL, &DeffContext_Zero);
	iDevice->CreateDeferredContext(NULL, &DeffContext_Model);

	iDevice->QueryInterface(IID_PPV_ARGS(&Debuger));

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
	
#pragma region Load
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

	CreateDDSTextureFromFile(iDevice, L"Assets\\Portal\\PortalSkybox.dds", (ID3D11Resource **)&tx_Skybox, &SkyboxView);	

	VERTEX_3D realCube[8] = {
			{ XMFLOAT3(-1, 1, -1), XMFLOAT2(0, 0) }, { XMFLOAT3(1, 1, -1), XMFLOAT2(1, 0) },
			{ XMFLOAT3(-1, -1, -1), XMFLOAT2(0, 1) }, { XMFLOAT3(1, -1, -1), XMFLOAT2(1, 1) },
			{ XMFLOAT3(1, 1, 1), XMFLOAT2(0, 0) }, { XMFLOAT3(-1, 1, 1), XMFLOAT2(1, 0) },
			{ XMFLOAT3(1, -1, 1), XMFLOAT2(0, 1) },{ XMFLOAT3(-1, -1,-1), XMFLOAT2(1, 1) }
	};

	D3D11_BUFFER_DESC box_desc;
	ZeroMemory(&box_desc, sizeof(D3D11_BUFFER_DESC));
	box_desc.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
	box_desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	box_desc.ByteWidth = sizeof(VERTEX_3D) * 8;

	D3D11_SUBRESOURCE_DATA box_data;
	ZeroMemory(&box_data, sizeof(D3D11_SUBRESOURCE_DATA));
	box_data.pSysMem = realCube;

	iDevice->CreateBuffer(&box_desc,&box_data,&vb_Box);

	unsigned int indx = 0, one, two, three;
	vector<unsigned int> box_indx;
	for (unsigned int i = 0; i < 36; i+=4){
		
	}

	box_indx.data();

#if 0
	D3D11_TEXTURE2D_DESC Skybox_Tx_desc;
	ZeroMemory(&Skybox_Tx_desc, sizeof(D3D11_TEXTURE2D_DESC));
	Skybox_Tx_desc.SampleDesc.Count = 1;
	Skybox_Tx_desc.Format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	Skybox_Tx_desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	Skybox_Tx_desc.MiscFlags = D3D11_RESOURCE_MISC_FLAG::D3D11_RESOURCE_MISC_TEXTURECUBE;
	Skybox_Tx_desc.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
	Skybox_Tx_desc.ArraySize = 6;
	Skybox_Tx_desc.Height = 512;
	Skybox_Tx_desc.Width = 512;
	Skybox_Tx_desc.MipLevels = 10;

	D3D11_SHADER_RESOURCE_VIEW_DESC Skybox_view_desc;
	ZeroMemory(&Skybox_view_desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	Skybox_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	Skybox_view_desc.Format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	Skybox_view_desc.TextureCube.MipLevels = 10;
	Skybox_view_desc.TextureCube.MostDetailedMip = 0;


	const unsigned int *Poteto[6];
	Poteto[D3D11_TEXTURECUBE_FACE::D3D11_TEXTURECUBE_FACE_POSITIVE_X] = PORTAL_RT_PIXELS;
	Poteto[D3D11_TEXTURECUBE_FACE::D3D11_TEXTURECUBE_FACE_NEGATIVE_X] = PORTAL_LF_PIXELS;
	Poteto[D3D11_TEXTURECUBE_FACE::D3D11_TEXTURECUBE_FACE_POSITIVE_Y] = PORTAL_UP_PIXELS;
	Poteto[D3D11_TEXTURECUBE_FACE::D3D11_TEXTURECUBE_FACE_NEGATIVE_Y] = PORTAL_DN_PIXELS;
	Poteto[D3D11_TEXTURECUBE_FACE::D3D11_TEXTURECUBE_FACE_POSITIVE_Z] = PORTAL_FT_PIXELS;
	Poteto[D3D11_TEXTURECUBE_FACE::D3D11_TEXTURECUBE_FACE_NEGATIVE_Z] = PORTAL_BK_PIXELS;
	D3D11_SUBRESOURCE_DATA Skybox_Sub[10]; // 10 miplevels, 6 slices
	
	for (unsigned int i = 0; i < 10; ++i){
		ZeroMemory(&Skybox_Sub[i], sizeof(Skybox_Sub[i]));
		Skybox_Sub[i].pSysMem = &Poteto[i][portal_bk_leveloffsets[i]];
		Skybox_Sub[i].SysMemPitch = (portal_bk_width >> i) * sizeof(unsigned int);
		Skybox_Sub[i].SysMemSlicePitch = portal_bk_numpixels * sizeof(unsigned int);
	}
	iDevice->CreateTexture2D(&Skybox_Tx_desc, Skybox_Sub, &tx_Skybox);



#endif // 0
	
#pragma endregion

	ID3D11Resource *iResource;
	ZeroMemory(&iResource, sizeof(ID3D11Resource));
	swapChain->GetBuffer(0, __uuidof(iResource), reinterpret_cast<void**>(&iResource));
	iDevice->CreateRenderTargetView(iResource, NULL, &iRenderTarget);
	iResource->Release(); 

	D3D11_TEXTURE2D_DESC Stencil_Resource_desc;
	ZeroMemory(&Stencil_Resource_desc, sizeof(D3D11_TEXTURE2D_DESC));
	Stencil_Resource_desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
	Stencil_Resource_desc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	Stencil_Resource_desc.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	Stencil_Resource_desc.Height = BACKBUFFER_HEIGHT;
	Stencil_Resource_desc.Width = BACKBUFFER_WIDTH;
	Stencil_Resource_desc.ArraySize = 1;
	Stencil_Resource_desc.SampleDesc.Count = 1;

	iDevice->CreateTexture2D(&Stencil_Resource_desc, NULL, &tx_DepthStencil);

	D3D11_DEPTH_STENCIL_VIEW_DESC STENCIL_DESC;
	ZeroMemory(&STENCIL_DESC, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	STENCIL_DESC.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	STENCIL_DESC.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;

	iDevice->CreateDepthStencilView(tx_DepthStencil, &STENCIL_DESC, &iDepthStencilView);

	swapChain->GetDesc(&chainDesc);
	ZeroMemory(&viewPort, sizeof(D3D11_VIEWPORT));
	viewPort.Height = static_cast<FLOAT>(chainDesc.BufferDesc.Height);
	viewPort.Width = static_cast<FLOAT>(chainDesc.BufferDesc.Width);
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;

#pragma region extraView

// <Mah hovercam>
	ZeroMemory(&hovCam_view, sizeof(hovCam_view));
	hovCam_view.Height = HOVERCAM_HEIGHT;
	hovCam_view.Width = HOVERCAM_WITDH;
	hovCam_view.TopLeftX = BACKBUFFER_WIDTH * 0.65;
	hovCam_view.TopLeftY = BACKBUFFER_HEIGHT * 0.75;
	hovCam_view.MinDepth = 0.0f;
	hovCam_view.MaxDepth = 1.0f;

// <mah hovercam/>

#pragma endregion

// <Mah 3D>

#pragma region DefaultShape
	indx = 0;

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
#pragma endregion

#pragma region DefaultViewProjection

	Tracker_Up = XMFLOAT3(0, 1, 0);
	Tracker_Pos = XMFLOAT3(0, 0, 0);
	Tracker_Tgt = XMFLOAT3(0, 0, 1);

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
	toShader_perspective.view = XMMatrixTranspose( view);	
	XMStoreFloat4x4(&m_view, view);

	// zNear = 0.1;
	// zFar = 10
	// vFOV = 90
	float aspect = BACKBUFFER_WIDTH / BACKBUFFER_HEIGHT;
	m_Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(90.0f), aspect, 0.1f, 1000.0f);
	toShader_perspective.projection = XMMatrixTranspose(m_Projection);


#pragma endregion

	XMMATRIX model = XMLoadFloat4x4(&m_CubeWorld);
	model = XMMatrixTranslation(0.7f, 0, 0);
	toShader_perspective.model = XMMatrixTranspose(model);
	XMStoreFloat4x4(&m_CubeWorld, model); 

#pragma region extraView
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
#pragma endregion

#pragma region RTTview
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
	//toShader_RTT.view = XMLoadFloat4x4(&m_view);


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
#pragma endregion

#pragma region Default
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
#pragma endregion


#pragma region Load
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
#pragma endregion

	turn = 12.0f;
	timeX.Throttle(2);
	ZeroMemory(&newCamOffset, sizeof(newCamOffset));

}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	timeX.Signal();
	UpdateInput();

	iDeviceContext->ClearDepthStencilView(iDepthStencilView,D3D11_CLEAR_DEPTH,1,NULL);
	iDeviceContext->OMSetRenderTargets(1, &iRenderTarget, iDepthStencilView);

	FLOAT DarkBlue[] = { 0.0f, 0.0f, 0.45f, 1.0f };
	FLOAT Black[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	iDeviceContext->RSSetViewports(1, &viewPort);
	iDeviceContext->ClearRenderTargetView(iRenderTarget, DarkBlue);

// <Mah 3d>
	XMMATRIX cubeWorld= XMLoadFloat4x4(&m_CubeWorld);
	cubeWorld = XMMatrixRotationY(-XMConvertToRadians(turn*timeX.Delta()))*cubeWorld;
	toShader_perspective.model = XMMatrixTranspose(cubeWorld);
	XMStoreFloat4x4(&m_CubeWorld, cubeWorld);

	toShader_perspective.view =XMMatrixTranspose( XMLoadFloat4x4(&m_view)) ;

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

	thread step1(&DEMO_APP::RenderDefault,this, DeffContext_Default, XMFLOAT3(0, 1, 0));

	thread step2(&DEMO_APP::RenderDefault,this, DeffContext_Zero, XMFLOAT3(0, 2, 0));

	thread step3(&DEMO_APP::RenderDefault,this, DeffContext_Model, XMFLOAT3(0, 3, 0));



#if 1

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
	iDeviceContext->OMSetRenderTargets(1, &iRenderTarget, iDepthStencilView);

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

#endif

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

	step1.join();
	step2.join();
	step3.join();

	ID3D11CommandList *CmdList;
	ID3D11CommandList *CmdList_Zero000;
	ID3D11CommandList *CmdList_Model000;


	DeffContext_Default->FinishCommandList(false, &CmdList);
	DeffContext_Model->FinishCommandList(false, &CmdList_Model000);
	DeffContext_Zero->FinishCommandList(false, &CmdList_Zero000);
	
	iDeviceContext->ExecuteCommandList(CmdList, false);
	iDeviceContext->ExecuteCommandList(CmdList_Model000, false);
	iDeviceContext->ExecuteCommandList(CmdList_Zero000, false);

	iDeviceContext->Flush();

	CmdList->Release();
	CmdList_Model000->Release();
	CmdList_Zero000->Release();

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

	tx_DepthStencil->Release();
	iDepthStencilView->Release();

	tx_Skybox->Release();
	SkyboxView->Release();
	
	DeffContext_Default->Release();
	DeffContext_Model->Release();
	//DeffCOntext_RTT->Release();
	DeffContext_Zero->Release();
	OutputDebugStringW(L"\n\n\n poteto \n\n\n");

	Debuger->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	Debuger->Release();
	UnregisterClass( L"DirectXApplication", application ); 
	return true;
}

void DEMO_APP::RenderDefault( ID3D11DeviceContext *iDeviceContext , XMFLOAT3 _Offset){


	iDeviceContext->OMSetRenderTargets(1, &iRenderTarget, iDepthStencilView);
	iDeviceContext->RSSetViewports(1, &viewPort);

	XMMATRIX cubeWorld = XMLoadFloat4x4(&m_CubeWorld);
	cubeWorld = XMMatrixRotationY(-XMConvertToRadians(turn*timeX.Delta()))*cubeWorld;
	//XMStoreFloat4x4(&m_CubeWorld, cubeWorld);
	cubeWorld = cubeWorld * XMMatrixTranslationFromVector(XMLoadFloat3(&_Offset));
	toShader_perspective.model = XMMatrixTranspose(cubeWorld);
	toShader_perspective.view = XMMatrixTranspose(XMLoadFloat4x4(&m_view));

	D3D11_MAPPED_SUBRESOURCE map_cube;
	ZeroMemory(&map_cube, sizeof(D3D11_MAPPED_SUBRESOURCE));
	iDeviceContext->Map(cBuff_perspective, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, NULL, &map_cube);
	memcpy(map_cube.pData, &toShader_perspective, sizeof(toShader_perspective));
	iDeviceContext->Unmap(cBuff_perspective, 0);
	iDeviceContext->VSSetConstantBuffers(0, 1, &cBuff_perspective);

	UINT _startSlot = 0;
	UINT _numBuffs = 1;
	UINT _strides = static_cast<UINT>(sizeof(VERTEX_3D));
	UINT _offSets = 0;
	iDeviceContext->IASetVertexBuffers(_startSlot, _numBuffs, &vb_Cube, &_strides, &_offSets);
	iDeviceContext->IASetIndexBuffer(ib_Cube, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	iDeviceContext->PSSetSamplers(0, 1, &SampleState);
	iDeviceContext->PSSetShaderResources(0, 1, &ShaderView);
	iDeviceContext->VSSetShader(VertSha_perspective, NULL, NULL);
	iDeviceContext->PSSetShader(PixSha_perspective, NULL, NULL);
	iDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	iDeviceContext->IASetInputLayout(lay_perspective);
	iDeviceContext->DrawIndexed(12, 0, 0);

}
//#include "Assets\BasicPlatform\Wall.obj
bool DEMO_APP::LoadObjFile(const char *_filename, std::vector<VERTEX_OBJMODEL> &_forVB){
	std::vector<XMFLOAT3> vertexHold;
	std::vector<XMFLOAT3> uvHold;
	std::vector<XMFLOAT3> normalHold;
	std::vector<unsigned int> indx_Vector, indx_UVs, indx_Normal;

	FILE *file = fopen( _filename, "r");
	if (file == NULL){
		printf("Cannot read this thing\n");
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

// ********************************************************************* 
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    if(GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
    switch ( message )
    {
        case ( WM_DESTROY ): { PostQuitMessage( 0 ); }
			break;
		case (WM_KEYDOWN) : 
			switch (wParam)
			{
				case VK_A: DEMO_APP::UpdateKeyboardInput(VK_A, true); break;
				case VK_W: DEMO_APP::UpdateKeyboardInput(VK_W, true); break;
				case VK_S: DEMO_APP::UpdateKeyboardInput(VK_S, true); break;
				case VK_D: DEMO_APP::UpdateKeyboardInput(VK_D, true); break;
				case VK_E: DEMO_APP::UpdateKeyboardInput(VK_E, true); break;
				case VK_Q: DEMO_APP::UpdateKeyboardInput(VK_Q, true); break;
				case VK_T: DEMO_APP::UpdateKeyboardInput(VK_T, true, true); break;
				case VK_CONTROL: DEMO_APP::UpdateKeyboardInput(VK_CONTROL, true, true); break;
				case VK_NUMPAD2: DEMO_APP::UpdateKeyboardInput(VK_NUMPAD2, true); break;
				case VK_NUMPAD4: DEMO_APP::UpdateKeyboardInput(VK_NUMPAD4, true); break;
				case VK_NUMPAD6: DEMO_APP::UpdateKeyboardInput(VK_NUMPAD6, true); break;
				case VK_NUMPAD8: DEMO_APP::UpdateKeyboardInput(VK_NUMPAD8, true); break;
				default:	break;
			}
			break;

		case (WM_KEYUP):
			switch (wParam)
			{
				case VK_A: DEMO_APP::UpdateKeyboardInput(VK_A, false); break;
				case VK_W: DEMO_APP::UpdateKeyboardInput(VK_W, false); break;
				case VK_S: DEMO_APP::UpdateKeyboardInput(VK_S, false); break;
				case VK_D: DEMO_APP::UpdateKeyboardInput(VK_D, false); break;
				case VK_E: DEMO_APP::UpdateKeyboardInput(VK_E, false); break;
				case VK_Q: DEMO_APP::UpdateKeyboardInput(VK_Q, false); break;
				case VK_T: DEMO_APP::UpdateKeyboardInput(VK_T, false, true); break;
				case VK_CONTROL: DEMO_APP::UpdateKeyboardInput(VK_CONTROL, false, true); break;
				case VK_NUMPAD2: DEMO_APP::UpdateKeyboardInput(VK_NUMPAD2, false); break;
				case VK_NUMPAD4: DEMO_APP::UpdateKeyboardInput(VK_NUMPAD4, false); break;
				case VK_NUMPAD6: DEMO_APP::UpdateKeyboardInput(VK_NUMPAD6, false); break;
				case VK_NUMPAD8: DEMO_APP::UpdateKeyboardInput(VK_NUMPAD8, false); break;
				default:		break;
			}
			break;
    }
    return DefWindowProc( hWnd, message, wParam, lParam );
}
//********************* END WARNING ************************//