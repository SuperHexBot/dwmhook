#include "includes.hpp"
#include "render.hpp"
#include "ReflectiveLoader.h"
ID3D11Device* pD3DXDevice = nullptr;
ID3D11DeviceContext* pD3DXDeviceCtx = nullptr;
ID3D11Texture2D* pBackBuffer = nullptr;
ID3D11RenderTargetView* pRenderTarget = nullptr;

IFW1Factory* pFontFactory = nullptr;
IFW1FontWrapper* pFontWrapper = nullptr;

BOOL bDataCompare( const BYTE* pData, const BYTE* bMask, const char* szMask )
{
	for ( ; *szMask; ++szMask, ++pData, ++bMask )
	{
		if ( *szMask == 'x' && *pData != *bMask )
			return FALSE;
	}
	return ( *szMask ) == NULL;
}

DWORD64 FindPattern( const char* szModule, BYTE* bMask, const char* szMask )
{
	MODULEINFO mi{ };
	GetModuleInformation( GetCurrentProcess(), GetModuleHandleA( szModule ), &mi, sizeof( mi ) );

	DWORD64 dwBaseAddress = DWORD64( mi.lpBaseOfDll );
	const auto dwModuleSize = mi.SizeOfImage;

	for ( auto i = 0ul; i < dwModuleSize; i++ )
	{
		if ( bDataCompare( PBYTE( dwBaseAddress + i ), bMask, szMask ) )
			return DWORD64( dwBaseAddress + i );
	}
	return NULL;
}

void AddToLog( const char* fmt, ... )
{
	
	/*va_list va;
	va_start( va, fmt );

	char buff[ 1024 ]{ };
	vsnprintf_s( buff, sizeof( buff ), fmt, va );

	va_end( va );

	FILE* f = nullptr;
	fopen_s( &f, LOG_FILE_PATH, "a" );

	if ( !f )
	{
		char szDst[ 256 ];
		sprintf_s( szDst, "Failed to create file %d", GetLastError() );
		MessageBoxA( 0, szDst, 0, 0 );
		return;
	}

	OutputDebugStringA( buff );
	fprintf_s( f, buff );
	fclose( f );*/
	
}
HANDLE hMapFile;
struct PlayerInfo
{
	int feet_x;
	int feet_y;
	int head_x;
	int head_y;
	int dis;
	char name[20];
	char weapon[20];
};
struct GlobalInfo 
{
	int width;
	int height;
	int left;
	int top;
	int PlayerCount;
	PlayerInfo infos[100];
	GlobalInfo()
	{
		memset(this, 0, sizeof(GlobalInfo));
	}
};
unsigned int fun_ARGB(int a, int r, int g, int b)
{
	return a << 24 | r << 16 | g << 8 | b;
}
#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
void DrawEverything( IDXGISwapChain* pDxgiSwapChain )
{
	static bool b = true;
	if ( b )
	{
		pDxgiSwapChain->GetDevice( __uuidof( ID3D11Device ), ( void** )&pD3DXDevice );
		pDxgiSwapChain->AddRef();
		pD3DXDevice->GetImmediateContext( &pD3DXDeviceCtx );

		pDxgiSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&backbuffer_ptr );
		pD3DXDevice->CreateRenderTargetView( *backbuffer_ptr, NULL, &rtview_ptr );

		D3D11_RASTERIZER_DESC raster_desc;
		ZeroMemory( &raster_desc, sizeof( raster_desc ) );
		raster_desc.FillMode = D3D11_FILL_SOLID; // D3D11_FILL_WIREFRAME;
		raster_desc.CullMode = D3D11_CULL_BACK; //D3D11_CULL_NONE;
		pD3DXDevice->CreateRasterizerState( &raster_desc, &rasterizer_state_ov );

		// shader

		D3D_SHADER_MACRO shader_macro[] = { NULL, NULL };
		ID3DBlob* vs_blob_ptr = NULL, * ps_blob_ptr = NULL, * error_blob = NULL;

		D3DCompile( shader_code, strlen( shader_code ), NULL, shader_macro, NULL, "VS", "vs_4_0", 0, 0, &vs_blob_ptr, &error_blob );
		D3DCompile( shader_code, strlen( shader_code ), NULL, shader_macro, NULL, "PS", "ps_4_0", 0, 0, &ps_blob_ptr, &error_blob );

		pD3DXDevice->CreateVertexShader( vs_blob_ptr->GetBufferPointer(), vs_blob_ptr->GetBufferSize(), NULL, &vertex_shader_ptr );
		pD3DXDevice->CreatePixelShader( ps_blob_ptr->GetBufferPointer(), ps_blob_ptr->GetBufferSize(), NULL, &pixel_shader_ptr );

		// layout

		D3D11_INPUT_ELEMENT_DESC element_desc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		pD3DXDevice->CreateInputLayout( element_desc, ARRAYSIZE( element_desc ), vs_blob_ptr->GetBufferPointer(), vs_blob_ptr->GetBufferSize(), &input_layout_ptr );

		// buffers

		SimpleVertex vertices[] =
		{
			{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) },
			{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
			{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT4( 0.0f, 1.0f, 1.0f, 1.0f ) },
			{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ) },
			{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 0.0f, 1.0f, 1.0f ) },
			{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ) },
			{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ) },
			{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f ) },
		};

		WORD indices[] =
		{
			3,1,0,
			2,1,3,

			0,5,4,
			1,5,0,

			3,4,7,
			0,4,3,

			1,6,5,
			2,6,1,

			2,7,6,
			3,7,2,

			6,4,5,
			7,4,6,
		};

		D3D11_BUFFER_DESC bd = {};
		D3D11_SUBRESOURCE_DATA data = {};

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( SimpleVertex ) * 8;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		data.pSysMem = vertices;
		pD3DXDevice->CreateBuffer( &bd, &data, &vertex_buffer_ptr );

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( WORD ) * 36;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		data.pSysMem = indices;
		pD3DXDevice->CreateBuffer( &bd, &data, &index_buffer_ptr );

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( ConstantBuffer );
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		pD3DXDevice->CreateBuffer( &bd, NULL, &const_buffer_ptr );

		t0 = GetTickCount64();

		FW1CreateFactory( FW1_VERSION, &pFontFactory );
		pFontFactory->CreateFontWrapper( pD3DXDevice, L"Arial", &pFontWrapper );
		SAFE_RELEASE( pFontFactory );

		render.Initialize( pFontWrapper );

		hMapFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			sizeof(GlobalInfo),
			"45648a4s5dasdka65d4as5d748asd"
		);
		b = false;
	}
	else
	{
		fix_renderstate();

		static int lastkeydown;
		static bool NoDraw;
		if (GetTickCount() - lastkeydown > 1000 && KEYDOWN(VK_F7))
		{
			lastkeydown = GetTickCount();
			NoDraw = !NoDraw;
		}
		render.BeginScene();
		//render.FillRect( 10.f, 10.f, 100.f, 100.f, 0xFFFF1010 );
		//render.OutlineRect( 9.f, 9.f, 501.f, 501.f, -1 );
		//render.DrawLine( XMFLOAT2( 10.f, 50.f ), XMFLOAT2( 25.f, 75.f ), -1 );
		//render.RenderText(L"she been bouncing on my lap lap lap", 10.f, 10.f, -1, false, false);
		//render.RenderText(L"catjpg on top", 700.f, 600.f, 0xFFFFFFFF, false, true);
		if (!NoDraw)
		{
			if (hMapFile != NULL)
			{
				LPVOID lpBase1 = MapViewOfFile(
					hMapFile,
					FILE_MAP_ALL_ACCESS,
					0,
					0,
					sizeof(GlobalInfo)
				);
				if (lpBase1)
				{
					GlobalInfo* g = new GlobalInfo();
					memcpy(g, lpBase1, sizeof(GlobalInfo));
					if (g->PlayerCount == 0)
					{
						render.RenderText(L"Waiting...", 10.f, 50.f, -1, false, true);
					}
					else
					{
						char buf[50] = { 0 };
						sprintf(buf, u8"��������:%d", g->PlayerCount - 1);
						render.RenderText(buf, g->left + g->width / 2.0, g->top + 50.0f, fun_ARGB(255, 0, 255, 0), false, true);
					}
					for (auto i = 1; i < g->PlayerCount; i++)
					{
						auto box_height = g->infos[i].feet_y - g->infos[i].head_y;
						auto box_width = box_height / 2.5f;
						auto point1 = XMFLOAT2(g->left + g->infos[i].head_x - box_width / 2.0, g->top + g->infos[i].head_y);
						auto point2 = XMFLOAT2(g->left + g->infos[i].head_x + box_width / 2.0, g->top + g->infos[i].head_y);
						auto point3 = XMFLOAT2(g->left + g->infos[i].feet_x + box_width / 2.0, g->top + g->infos[i].feet_y);
						auto point4 = XMFLOAT2(g->left + g->infos[i].feet_x - box_width / 2.0, g->top + g->infos[i].feet_y);

						if (point1.x < g->left + 5 || point1.y < g->top + 5 || point1.x >(g->left + g->width -5) || point1.y >(g->top + g->height -5))
						{
							continue;
						}
						if (point2.x < g->left + 5 || point2.y < g->top + 5 || point2.x >(g->left + g->width - 5) || point2.y >(g->top + g->height - 5))
						{
							continue;
						}
						if (point3.x < g->left + 5 || point3.y < g->top + 5 || point3.x >(g->left + g->width - 5) || point3.y >(g->top + g->height - 5))
						{
							continue;
						}
						if (point4.x < g->left + 5 || point4.y < g->top + 5 || point4.x >(g->left + g->width - 5) || point4.y >(g->top + g->height - 5))
						{
							continue;
						}
						/*Line*/
						render.DrawLine(XMFLOAT2(g->left + g->width / 2.0, g->top), XMFLOAT2(g->left + g->infos[i].head_x, g->top + g->infos[i].head_y), fun_ARGB(255, 255, 0, 0));


						/*Box*/
						render.DrawLine(point1, point2, fun_ARGB(255, 255, 0, 0));
						render.DrawLine(point2, point3, fun_ARGB(255, 255, 0, 0));
						render.DrawLine(point3, point4, fun_ARGB(255, 255, 0, 0));
						render.DrawLine(point4, point1, fun_ARGB(255, 255, 0, 0));

						/*Name*/
						render.RenderText(g->infos[i].name, point2.x, point2.y, fun_ARGB(255, 255, 255, 0), false, true);
						render.RenderText(g->infos[i].weapon, point2.x, (point2.y + point3.y) / 2, fun_ARGB(255, 0, 255, 255), false, true);
						char buf[20] = { 0 };
						sprintf(buf, u8"[����:%d m]", g->infos[i].dis);
						render.RenderText(buf, point2.x, point3.y, fun_ARGB(255, 0, 255, 120), false, true);
					}
					delete g;
					memset(lpBase1, 0, sizeof(GlobalInfo));
					UnmapViewOfFile(lpBase1);
				}
			}
		}
		render.EndScene();
	}
}

using PresentMPO_ = __int64( __fastcall* )( void*, IDXGISwapChain*, unsigned int, unsigned int, int, __int64, __int64, int );
PresentMPO_ oPresentMPO = NULL;

__int64 __fastcall hkPresentMPO( void* thisptr, IDXGISwapChain* pDxgiSwapChain, unsigned int a3, unsigned int a4, int a5, __int64 a6, __int64 a7, int a8 )
{
	DrawEverything( pDxgiSwapChain );
	return oPresentMPO( thisptr, pDxgiSwapChain, a3, a4, a5, a6, a7, a8 );
}

using PresentDWM_ = __int64( __fastcall* )( void*, IDXGISwapChain*, unsigned int, unsigned int, const struct tagRECT*, unsigned int, const struct DXGI_SCROLL_RECT*, unsigned int, struct IDXGIResource*, unsigned int );
PresentDWM_ oPresentDWM = NULL;

__int64 __fastcall hkPresentDWM( void* thisptr, IDXGISwapChain* pDxgiSwapChain, unsigned int a3, unsigned int a4, const struct tagRECT* a5, unsigned int a6, const struct DXGI_SCROLL_RECT* a7, unsigned int a8, struct IDXGIResource* a9, unsigned int a10 )
{
	DrawEverything( pDxgiSwapChain );
	return oPresentDWM( thisptr, pDxgiSwapChain, a3, a4, a5, a6, a7, a8, a9, a10 );
}

UINT WINAPI MainThread( PVOID )
{
	MH_Initialize();

	while ( !GetModuleHandleA( "dwmcore.dll" ) )
		Sleep( 150 );

	//
	// [ E8 ? ? ? ? ] the relative addr will be converted to absolute addr
	auto ResolveCall = []( DWORD_PTR sig )
	{
		return sig = sig + *reinterpret_cast< PULONG >( sig + 1 ) + 5;
	};

	//
	// [ 48 8D 05 ? ? ? ? ] the relative addr will be converted to absolute addr
	auto ResolveRelative = []( DWORD_PTR sig )
	{
		return sig = sig + *reinterpret_cast< PULONG >( sig + 0x3 ) + 0x7;
	};

	auto dwRender = FindPattern( "d2d1.dll", PBYTE( "\x48\x8D\x05\x00\x00\x00\x00\x33\xED\x48\x8D\x71\x08" ), "xxx????xxxxxx" );

	if ( dwRender )
	{
		dwRender = ResolveRelative( dwRender );

		PDWORD_PTR Vtbl = PDWORD_PTR( dwRender );

		AddToLog( "table 0x%llx\n", dwRender );

		MH_CreateHook( PVOID( Vtbl[ 6 ] ), PVOID( &hkPresentDWM ), reinterpret_cast< PVOID* >( &oPresentDWM ) );
		MH_CreateHook( PVOID( Vtbl[ 7 ] ), PVOID( &hkPresentMPO ), reinterpret_cast< PVOID* >( &oPresentMPO ) );
		MH_EnableHook( MH_ALL_HOOKS );

		AddToLog( "hooked!\n" );
	}

	return 0;
}

BOOL WINAPI DllMain( HMODULE hDll, DWORD dwReason, PVOID )
{
	if ( dwReason == DLL_PROCESS_ATTACH )
	{
		DeleteFileA( LOG_FILE_PATH );
		_beginthreadex( nullptr, NULL, MainThread, nullptr, NULL, nullptr );
	}
	return true;
}