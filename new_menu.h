#pragma once
#include "includes.h"
#include <d3dx9.h>
#include "singleton.h"

class c_menu : public singleton<c_menu>
{
public:
	void initialize( IDirect3DDevice9* pDevice );
	void draw_begin( );
	void draw( );
	void draw_end( );

	bool is_menu_initialized( );
	bool is_menu_opened( );
	void set_menu_opened( bool v );

	IDirect3DTexture9* get_texture_data( );
	ImColor get_accent_color( );
	ImFont* verdana_bold;

private:
	bool m_bInitialized;
	bool m_bIsOpened;
	IDirect3DTexture9* m_pTexture;

	int m_nCurrentTab;
};