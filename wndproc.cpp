#include "includes.h"

DWORD dwOld_D3DRS_COLORWRITEENABLE;
IDirect3DVertexDeclaration9* vertDec;
IDirect3DVertexShader9* vertShader;

HRESULT __stdcall Hooks::Present( IDirect3DDevice9* pDevice, RECT* pRect1, const RECT* pRect2, HWND hWnd, const RGNDATA* pRGNData )
{
	IDirect3DVertexDeclaration9* vert_dec;
	if ( pDevice->GetVertexDeclaration( &vert_dec ) )
		return g_hooks.m_device.GetOldMethod<decltype( &Present )>( 17 )( pDevice, pRect1, pRect2, hWnd, pRGNData );

	IDirect3DVertexShader9* vert_shader;
	if ( pDevice->GetVertexShader( &vert_shader ) )
		return g_hooks.m_device.GetOldMethod<decltype( &Present )>( 17 )( pDevice, pRect1, pRect2, hWnd, pRGNData );



	static auto wanted_ret_address = _ReturnAddress( );
	if ( _ReturnAddress( ) == wanted_ret_address )
	{


		DWORD colorwrite, srgbwrite;
		pDevice->GetRenderState( D3DRS_COLORWRITEENABLE, &colorwrite );
		pDevice->GetRenderState( D3DRS_SRGBWRITEENABLE, &srgbwrite );
		pDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 0xffffffff );
		pDevice->SetRenderState( D3DRS_SRGBWRITEENABLE, false );

		static bool once{ false };
		if ( !once ) {
			c_menu::get( )->initialize( pDevice );

			c_config::get( )->load_defaults( );
			c_config::get( )->load_keys( );
			once = true;
		}

		c_menu::get( )->draw_begin( );

		int w, h = 0;
		g_csgo.m_engine->GetScreenSize( w, h );

		ui::SetNextWindowPos( ImVec2( 0, 0 ) );
		ui::SetNextWindowSize( ImVec2( w, h ) );
		ui::Begin( "##visuals", 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar );

		ui::End( );

		c_menu::get( )->draw( );
		c_menu::get( )->draw_end( );

		pDevice->SetRenderState( D3DRS_COLORWRITEENABLE, colorwrite );
		pDevice->SetRenderState( D3DRS_SRGBWRITEENABLE, srgbwrite );
	}


	pDevice->SetVertexShader( vert_shader );
	pDevice->SetVertexDeclaration( vert_dec );

	return g_hooks.m_device.GetOldMethod<decltype( &Present )>( 17 )( pDevice, pRect1, pRect2, hWnd, pRGNData );
}

HRESULT __stdcall Hooks::Reset( IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentParameters )
{
	ImGui_ImplDX9_InvalidateDeviceObjects( );
	auto result = g_hooks.m_device.GetOldMethod<decltype( &Reset )>( 16 )( pDevice, pPresentParameters );

	if ( result >= 0 )
		ImGui_ImplDX9_CreateDeviceObjects( );

	return result;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
LRESULT WINAPI Hooks::WndProc( HWND wnd, uint32_t msg, WPARAM wParam, LPARAM lp )
{

	if ( msg == WM_KEYDOWN )
	{
		if ( wParam == c_config::get( )->i[ "misc_menukey" ] )
			c_menu::get( )->set_menu_opened( !c_menu::get( )->is_menu_opened( ) );

	}

	if ( c_menu::get( )->is_menu_opened( ) ) {
		ImGui_ImplWin32_WndProcHandler( wnd, msg, wParam, lp );

		if ( wParam != 'W' && wParam != 'A' && wParam != 'S' && wParam != 'D' && wParam != VK_SHIFT && wParam != VK_CONTROL && wParam != VK_TAB && wParam != VK_SPACE || ui::GetIO( ).WantTextInput )
			return true;
	}

	return g_winapi.CallWindowProcA( g_hooks.m_old_wndproc, wnd, msg, wParam, lp );
}