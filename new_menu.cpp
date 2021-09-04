#include "includes.h"
#include "imgui/bgtexture.h"
#include "imgui/bindsfont.h"
#include "tabsfont.h"

IDirect3DTexture9* imgxd = nullptr;
ImFont* tabs;
ImFont* def;

void SkinUpdate( ) {
	g_skins.m_update = true;
}

static int MenuTabs = 0;
static int RageSubTabs = 1, AntiAimsSubTabs = 1, LegitSubTabs = 1, VisualsSubTabs = 1, MiscSubTabs = 1, ScriptsSubTabs = 1;
static int lua_selected = 0;

std::vector<std::string> cfgList;
void ReadDirectory( const std::string& name, std::vector<std::string>& v )
{
	v.clear( );
	auto pattern( name );
	pattern.append( "\\*.ini" );
	WIN32_FIND_DATAA data;
	HANDLE hFind;
	if ( ( hFind = FindFirstFileA( pattern.c_str( ), &data ) ) != INVALID_HANDLE_VALUE )
	{
		do
		{
			v.emplace_back( data.cFileName );
		} while ( FindNextFileA( hFind, &data ) != 0 );
		FindClose( hFind );
	}
}

void c_menu::initialize( IDirect3DDevice9* pDevice ) {
	if ( this->m_bInitialized )
		return;

	ui::CreateContext( );
	auto io = ui::GetIO( );
	auto style = &ui::GetStyle( );

	style->WindowRounding = 0.f;
	style->AntiAliasedLines = true;
	style->AntiAliasedFill = true;
	style->ScrollbarRounding = 0.f;
	style->ScrollbarSize = 6.f;
	style->WindowPadding = ImVec2( 0, 0 );
	style->Colors[ ImGuiCol_ScrollbarBg ] = ImVec4( 45 / 255.f, 45 / 255.f, 45 / 255.f, 1.f );
	style->Colors[ ImGuiCol_ScrollbarGrab ] = ImVec4( 65 / 255.f, 65 / 255.f, 65 / 255.f, 1.f );

	IDirect3DSwapChain9* pChain = nullptr;
	D3DPRESENT_PARAMETERS pp = {};
	D3DDEVICE_CREATION_PARAMETERS param = {};
	pDevice->GetCreationParameters( &param );
	pDevice->GetSwapChain( 0, &pChain );

	if ( pChain )
		pChain->GetPresentParameters( &pp );

	ImGui_ImplWin32_Init( param.hFocusWindow );
	ImGui_ImplDX9_Init( pDevice );

	D3DXCreateTextureFromFileInMemoryEx(
		pDevice, texture, sizeof( texture ), 4096, 4096, D3DX_DEFAULT, NULL,
		pp.BackBufferFormat, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, NULL, NULL, NULL, &this->m_pTexture );

	ImFontConfig cfg;
	io.Fonts->AddFontFromMemoryCompressedTTF( menuFont, sizeof menuFont, 12, &cfg, io.Fonts->GetGlyphRangesCyrillic( ) );
	io.Fonts->AddFontFromMemoryCompressedTTF( boldMenuFont, sizeof boldMenuFont, 11, &cfg, io.Fonts->GetGlyphRangesCyrillic( ) );
	io.Fonts->AddFontFromMemoryTTF( keybinds_font, 25600, 10.f, &cfg, io.Fonts->GetGlyphRangesCyrillic( ) );
	c_menu::get( )->verdana_bold = io.Fonts->AddFontFromMemoryCompressedTTF( boldMenuFont, sizeof boldMenuFont, 13, &cfg, io.Fonts->GetGlyphRangesCyrillic( ) );
	io.Fonts->AddFontDefault( );

	ImGuiFreeType::BuildFontAtlas( io.Fonts, 0x00 );

	this->m_bInitialized = true;
}

void c_menu::draw_begin( ) {
	if ( !this->m_bInitialized )
		return;

	ImGui_ImplDX9_NewFrame( );
	ImGui_ImplWin32_NewFrame( );
	ui::NewFrame( );
}

void c_menu::draw( ) {
	if ( !this->m_bIsOpened && ui::GetStyle( ).Alpha > 0.f ) {
		float fc = 255.f / 0.2f * ui::GetIO( ).DeltaTime;
		ui::GetStyle( ).Alpha = std::clamp( ui::GetStyle( ).Alpha - fc / 255.f, 0.f, 1.f );
	}

	if ( this->m_bIsOpened && ui::GetStyle( ).Alpha < 1.f ) {
		float fc = 255.f / 0.2f * ui::GetIO( ).DeltaTime;
		ui::GetStyle( ).Alpha = std::clamp( ui::GetStyle( ).Alpha + fc / 255.f, 0.f, 1.f );
	}

	if ( !this->m_bIsOpened && ui::GetStyle( ).Alpha == 0.f )
		return;

	ui::SetNextWindowSizeConstraints( ImVec2( 478, 612 ), ImVec2( 4096, 4096 ) );
	ui::Begin( "gideonproject", 0, ImGuiWindowFlags_NoTitleBar );

	ui::TabButton( "ragebot", &this->m_nCurrentTab, 0, 5 );
	ui::TabButton( "antiaim", &this->m_nCurrentTab, 1, 5 );
	ui::TabButton( "visuals", &this->m_nCurrentTab, 2, 5 );
	ui::TabButton( "misc", &this->m_nCurrentTab, 3, 5 );
	ui::TabButton( "skinchanger", &this->m_nCurrentTab, 4, 5 );
	auto cfg = c_config::get;

	//position backwend
	static auto child_pos_backwend = [ ]( int num ) -> ImVec2 {
		return ImVec2( ui::GetWindowPos( ).x + 20 + ( ui::GetWindowSize( ).x / 2 - 31 ) * num + 20 * num, ui::GetWindowPos( ).y + 52 );
	};
	static auto child_pos_other = [ ]( int num ) -> ImVec2 {
		return ImVec2( ui::GetWindowPos( ).x + 20 + ( ui::GetWindowSize( ).x / 2 - 31 ) * num + 20 * num, ui::GetWindowPos( ).y + 380 );
	};

	//visuals backwend
	static auto child_pos_backwend_colored_models = [ ]( int num ) -> ImVec2 {
		return ImVec2( ui::GetWindowPos( ).x + 20 + ( ui::GetWindowSize( ).x / 2 - 31 ) * num + 20 * num, ui::GetWindowPos( ).y + 380 );
	};

	static auto child_pos_backwend_world_esp = [ ]( int num ) -> ImVec2 {
		return ImVec2( ui::GetWindowPos( ).x + 20 + ( ui::GetWindowSize( ).x / 2 - 31 ) * num + 20 * num, ui::GetWindowPos( ).y + 400 );
	};

	//sizea
	auto child_size = ImVec2( ui::GetWindowSize( ).x / 2 - 31, ui::GetWindowSize( ).y - 80 );
	auto child_size_accuracy = ImVec2( ui::GetWindowSize( ).x / 2 - 31, 310 );
	auto child_size_other = ImVec2( ui::GetWindowSize( ).x / 2 - 31, ui::GetWindowSize( ).y - 408 );

	//visuals size
	auto child_size_player_esp = ImVec2( ui::GetWindowSize( ).x / 2 - 31, 310 );
	auto child_size_colored_models = ImVec2( ui::GetWindowSize( ).x / 2 - 31, ui::GetWindowSize( ).y - 408 );
	auto child_size_other_esp = ImVec2( ui::GetWindowSize( ).x / 2 - 31, 330 );
	auto child_size_world_esp = ImVec2( ui::GetWindowSize( ).x / 2 - 31, ui::GetWindowSize( ).y - 428 );

	//MENU
	if ( this->m_nCurrentTab == 0 ) {
		ui::SetNextWindowPos( child_pos_backwend( 0 ) );
		ui::BeginChild( "Aimbot", child_size ); {
			ui::Checkbox( "  Enabled", &cfg( )->b[ "aimbot_enable" ] );
			ui::Checkbox( "  Silent aim", &cfg( )->b[ "silent" ] );
			ui::Checkbox( "  Fov limit", &cfg( )->b[ "fov" ] );
			if ( cfg( )->b[ "fov" ] ) {
				ui::SliderInt( " Maximum FOV", &cfg( )->i[ "fov_amount" ], 0, 180, "%d°" );
			}		
			ui::Checkbox( "  Antiaim resolver", &cfg( )->b[ "correct" ] );
			ui::Text( " Resolver override" );
			ui::Keybind( "nn", &cfg( )->i[ "override_key" ], &cfg( )->i[ "override_keystyle" ] );
			ui::Checkbox( "  Fakelag correction", &cfg( )->b[ "lagfix" ] );
			ui::Checkbox( "  Automatic penetration", &cfg( )->b[ "penetrate" ] );
			ui::Checkbox( "  Compensate spread", &cfg( )->b[ "hitchance" ] );
			if ( cfg( )->b[ "hitchance" ] ) {
				ui::SliderInt( "Minimum hit chance", &cfg( )->i[ "hitchance_amount" ], 0, 100, "%d%%" );
			}
			ui::SliderInt( " Minimum damage - Visible", &cfg( )->i[ "minimal_damage" ], 0, 100, "%d" );
			ui::SliderInt( " Minimum damage - Penetration", &cfg( )->i[ "penetrate_minimal_damage" ], 0, 100, "%d" );
			ui::Checkbox( "  Minimum damage override", &cfg( )->b[ "minimum_damage_override_enable" ] );
			if ( cfg( )->b[ "minimum_damage_override_enable" ] ) {
				ui::Keybind( "##sdf", &cfg( )->i[ "damage_override_key" ], &cfg( )->i[ "damage_override_keystyle" ] );
				ui::SliderInt( " Minimum damage - Override", &cfg( )->i[ "override_damage_amount" ], 0, 100, "%d" );
			}
			ui::SingleSelect( " Automatic scope", &cfg( )->i[ "zoom" ], { "Off" , "Always" , "Hitchance fail" } );
			ui::Checkbox( "  Automatic stop", &cfg( )->b[ "quickstop" ] );
		} ui::EndChild( );
		ui::SetNextWindowPos( child_pos_backwend( 1 ) );
		ui::BeginChild( "Hitscan", child_size_accuracy ); {
			ui::SingleSelect( " Target selection", &cfg( )->i[ "selection" ], { "Distance" ,  "Crosshair" , "Damage"  ,  "Health" ,  "Lag" ,  "Height" } );
			ui::Checkbox( "  Ignore limbs on move", &cfg( )->b[ "ignore_limbs" ] );
			ui::MultiSelect( " Hitboxes", &cfg( )->m[ "hitbox" ], { "Head", "Chest" , "Body" , "Arms", "Legs" , "Feet" } );
			ui::MultiSelect( " Multipoint", &cfg( )->m[ "multipoint" ], { "Head", "Chest"  ,"Body"  ,  "Legs" } );
			ui::MultiSelect( " Multipoint safety", &cfg( )->m[ "hitbox_history" ], { "Head" , "Chest"  , "Body"  , "Arms"  , "Legs"  , "Feet" } );
			ui::SliderInt( " Hitbox scale", &cfg( )->i[ "hitbox_scale" ], 0, 100, "%d%%" );
			ui::SliderInt( " Chest hitbox scale", &cfg( )->i[ "body_scale" ], 0, 100, "%d%%" );
			ui::Text( " Force body aim" );
			ui::Keybind( "##Force body aim", &cfg( )->i[ "baim_key" ], &cfg( )->i[ "baimkey_keystyle" ] );
			ui::MultiSelect( " Prefer head", &cfg( )->m[ "head1" ], { "Always", "Running", "Resolved"  ,"In Air" } );
			ui::MultiSelect( " Prefer body", &cfg( )->m[ "baim1" ], { "Always" ,  "Lethal" , "Lethal x2" ,  "Fake"  ,  "In air"  , "After x misses" } );
			ui::MultiSelect( " Only body", &cfg( )->m[ "baim2" ], { "Always" , "Health"  , "Fake"  ,  "In air"  , "After x misses" } );
			if ( c_config::get( )->m[ "baim1" ][ 5 ] || c_config::get( )->m[ "baim2" ][ 4 ] )
			{
				ui::SliderInt( "##Misses", &cfg( )->i[ "misses" ], 1, 10, "%d" );
			}

			if ( c_config::get( )->m[ "baim2" ][ 1 ] )
			{
				ui::SliderInt( "##Health", &cfg( )->i[ "baim_hp" ], 1, 50, "%dhp" );
			}
		} ui::EndChild( );
		ui::SetNextWindowPos( child_pos_other( 1 ) );
		ui::BeginChild( "Other", child_size_other ); {
			ui::Checkbox( "  Remove spread (!)", &cfg( )->b[ "nospread" ] );
			ui::Checkbox( "  Remove recoil", &cfg( )->b[ "norecoil" ] );
			ui::Checkbox( "  Knife bot", &cfg( )->b[ "knifebot" ] );
			ui::Checkbox( "  Zeus bot", &cfg( )->b[ "zeusbot" ] );			
		} ui::EndChild( );
	}
	if ( this->m_nCurrentTab == 1 ) {
		ui::SetNextWindowPos( child_pos_backwend( 0 ) );
		ui::BeginChild( "Anti-Aimbot", child_size ); {
			ui::Checkbox( "  Enabled", &cfg( )->b[ "aa_enable" ] );
			ui::Checkbox( "  Manual", &cfg( )->b[ "manual_aa_enable" ] );
			if ( cfg( )->b[ "manual_aa_enable" ] ) {
				ui::Text( " Left" );
				ui::Keybind( "##leftkey", &cfg( )->i[ "manual_left_key" ], &cfg( )->i[ "manual_left_keystyle" ] );

				ui::Text( " Right" );
				ui::Keybind( "##rigtkey", &cfg( )->i[ "manual_right_key" ], &cfg( )->i[ "manual_right_keystyle" ] );

				ui::Text( " Back" );
				ui::Keybind( "##backeky", &cfg( )->i[ "manual_back_key" ], &cfg( )->i[ "manual_back_keystyle" ] );
			}
			ui::SingleSelect( " Pitch", &cfg( )->i[ "pitch" ], { "Off" ,  "Down" , "Up", "Random" } );
			ui::SingleSelect( " Yaw base", &cfg( )->i[ "base_angle" ], { "Off",  "Static",  "Away crosshair",  "Away distance" } );
			ui::SingleSelect( " Yaw", &cfg( )->i[ "yaw_stand" ], { "Off", "Direction", "Jitter" , "Rotate" , "Random" } );
			if ( cfg( )->i[ "yaw_stand" ] == 3 ) {
				ui::SliderInt( " Rotate offset", &cfg( )->i[ "rotate_range" ], 0.f, 360.f, "%d°%" );
				ui::SliderInt( " Rotate speed", &cfg( )->i[ "rotate_speed" ], 1.f, 100.f, "%d%%" );
			}
			if ( cfg( )->i[ "yaw_stand" ] == 2 ) {
				ui::SliderInt( " Jitter offset", &cfg( )->i[ "jitter_range" ], 1.f, 180.f, "%d°%" );
			}
			if ( cfg( )->i[ "yaw_stand" ] == 4 ) {
				ui::SliderInt( " Random update", &cfg( )->i[ "rotate_update" ], 0.1, 1.f, "%d%%" );
			}
			ui::SingleSelect( " Direction", &cfg( )->i[ "direction" ], { "Freestanding" , "Backwards" , "Left" , "Right" ,  "Custom" } );
			if ( cfg( )->i[ "direction" ] == 4 ) {
				ui::SliderInt( " Custom direction", &cfg( )->i[ "dir_custom" ], -180.f, 180.f, "%d%%" );
			}
			if ( cfg( )->i[ "direction" ] == 0 ) {
				ui::SliderInt( " Freestanding time", &cfg( )->i[ "dir_time" ], 0.f, 10.f, "%ds%" );
			}
			ui::Checkbox( "  Lock direction", &cfg( )->b[ "dir_lock" ] );
			ui::SingleSelect( " Fake yaw", &cfg( )->i[ "fake_yaw" ], { "Off" ,"Default", "Relative" ,  "Jitter" ,  "Rotate", "Random", "Local view", "Oposite", "Sprinter" } );
		} ui::EndChild( );
		ui::SetNextWindowPos( child_pos_backwend( 1 ) );
		ui::BeginChild( "Fakelag", child_size_accuracy ); {
			ui::Checkbox( "  Enabled", &cfg( )->b[ "lag_enable" ] );
			ui::Checkbox( "  Customize conditions", &cfg( )->b[ "lag_customize" ] );
			if ( cfg( )->b[ "lag_customize" ] ) {
				ui::MultiSelect( " Conditions", &cfg( )->m[ "lag_active" ], { "Move" , "Air" , "Crouch" } );
			}
			ui::Checkbox( "  Latency compensation", &cfg( )->b[ "lag_breaker" ] );
			ui::SingleSelect( " Mode", &cfg( )->i[ "lag_mode" ], { "Maximum", "Break" ,  "Random" , "Break step", "Fluctuate" } );
			ui::SliderInt( " Limit", &cfg( )->i[ "lag_limit" ], 0.f, 14.f, "%d" );
			ui::SliderInt( " Variance", &cfg( )->i[ "lag_variance" ], 0.f, 100.f, "%d%%" );
			ui::Checkbox( "  Fake lag while shooting", &cfg( )->b[ "lag_shoot" ] );
			ui::Checkbox( "  Reset on bunny hop", &cfg( )->b[ "lag_land" ] );
		} ui::EndChild( );
		ui::SetNextWindowPos( child_pos_other( 1 ) );
		ui::BeginChild( "Other", child_size_other ); {

		} ui::EndChild( );
	}
	if ( this->m_nCurrentTab == 2 ) {
		ui::SetNextWindowPos( child_pos_backwend( 0 ) );
		ui::BeginChild( "Player ESP", child_size_player_esp ); {
			ui::Checkbox( "  Enabled", &cfg( )->b[ "esp_enable_checkbox" ] );
			ui::Checkbox( "  Dormant fade", &cfg( )->b[ "dormant" ] );
			ui::Checkbox( "  Box", &cfg( )->b[ "box_" ] );
			ui::ColorEdit4( "##BOXEcolor", cfg( )->c[ "box_enemy_color" ] );
			ui::Checkbox( "  Name", &cfg( )->b[ "name_" ] );
			ui::ColorEdit4( "##NAMEcolor", cfg( )->c[ "name_color" ] );
			ui::Checkbox( "  Health", &cfg( )->b[ "health_" ] );
			ui::Checkbox( "  Weapon", &cfg( )->b[ "weapon_" ] );
			ui::Checkbox( "  Weapon text", &cfg( )->b[ "weapon_text" ] );
			ui::Checkbox( "  Weapon icon", &cfg( )->b[ "weapon_icon" ] );
			ui::ColorEdit4( "##ICONcolor", cfg( )->c[ "weapon_icon_color" ] );
			ui::Checkbox( "  Distance", &cfg( )->b[ "distance" ] );
			ui::Checkbox( "  Ammo bar", &cfg( )->b[ "ammo" ] );
			ui::ColorEdit4( "##AMMOcolor", cfg( )->c[ "ammo_color" ] );
			ui::Checkbox( "  LBY timer", &cfg( )->b[ "lby_update" ] );
			ui::ColorEdit4( "##LBYTIMERcolor", cfg( )->c[ "lby_update_color" ] );
			ui::MultiSelect( " Flags", &cfg( )->m[ "flags" ], { "Armor" ,  "Scoped", "Flashed", "Reload", "Bomb" } );
			ui::Checkbox( "  Money", &cfg( )->b[ "money" ] );
			ui::Checkbox( "  Skeleton", &cfg( )->b[ "skeleton_" ] );
			ui::ColorEdit4( "###Enemyskelecolor", cfg( )->c[ "skeleton_enemy_color" ] );
		} ui::EndChild( );
		ui::SetNextWindowPos( child_pos_backwend_colored_models( 0 ) );
		ui::BeginChild( "Colored Models", child_size_colored_models ); {
			ui::Checkbox( "  Player", &cfg( )->b[ "chams_enemy_visible" ] );
			ui::ColorEdit4( "##PLAYERcolor", cfg( )->c[ "chams_enemy_vis" ] );
			ui::Checkbox( "  Player behind wall", &cfg( )->b[ "chams_enemy_invisible" ] );
			ui::ColorEdit4( "##PLAYERBEHINDWALLcolor", cfg( )->c[ "chams_enemy_invis" ] );
			ui::SingleSelect( "##Material", &cfg( )->i[ "chamstype" ], { "Texture" , "Flat" , "Metallic" ,  "Glow" ,  "Wireframe" } );
			if ( cfg( )->b[ "chams_enemy_visible" ] ) {
				ui::SliderInt( " Player blend", &cfg( )->i[ "chams_enemy_blend" ], 0, 100, "%d%%" );
			}
			
			ui::Checkbox( "  Local player", &cfg( )->b[ "chams_local" ] );
			ui::ColorEdit4( "##localplayercolor", cfg( )->c[ "chams_local_col" ] );
			if ( cfg( )->b[ "chams_local" ] ) {
				ui::SliderInt( "Local player blend", &cfg( )->i[ "chams_local_blend" ], 0, 100, "%d%%" );
			}
			ui::SingleSelect( "Material", &cfg( )->i[ "chamstype_local" ], { "Texture" ,"Flat" , "Metallic" , "Glow" ,  "Wireframe" } );

			ui::Checkbox( "  Local fake shadow", &cfg( )->b[ "fake_chams" ] );
			ui::ColorEdit4( "##fakechamscolor", cfg( )->c[ "fake_chams_col" ] );
			if ( cfg( )->b[ "fake_chams" ] ) {
				ui::SliderInt( " Local fake blend", &cfg( )->i[ "fake_chams_blend" ], 0, 100, "%d%%" );
			}

			ui::Checkbox( "  History", &cfg( )->b[ "chams_enemy_history" ] );
			ui::ColorEdit4( "##Recordcolor", cfg( )->c[ "chams_enemy_history_col" ] );
			if ( cfg( )->b[ "chams_enemy_history" ] ) {
				ui::SliderInt( " Shadow blend", &cfg( )->i[ "chams_enemy_history_blend" ], 0, 100, "%d%%" );
			}
			ui::Checkbox( "  Blend while scoped", &cfg( )->b[ "chams_local_scope" ] );
		} ui::EndChild( );
		ui::SetNextWindowPos( child_pos_backwend( 1 ) );
		ui::BeginChild( "Other ESP", child_size_other_esp ); {
			ui::Checkbox( "  Dropped weapons", &cfg( )->b[ "items" ] );
			ui::ColorEdit4( "##DROPPEDcolor", cfg( )->c[ "item_color" ] );
			ui::Checkbox( "  Grenades", &cfg( )->b[ "proj" ] );
			ui::ColorEdit4( "##PROJcolor", cfg( )->c[ "proj_color" ] );
			ui::Checkbox( "  Grenade simulation", &cfg( )->b[ "tracers" ] );
			ui::Checkbox( "  Out of FOV arrow", &cfg( )->b[ "offscreen" ] );
			ui::ColorEdit4( "##OUTOFFOVcolor", cfg( )->c[ "offscreen_color" ] );
			if ( cfg( )->b[ "offscreen" ] )
			{
				ui::SliderInt( " Offset", &cfg( )->i[ "offscreen_pos" ], 0, 480, "%dpx" );
				ui::SliderInt( " Size", &cfg( )->i[ "offscreen_size" ], 20, 58, "%dpx" );
			}
			ui::Checkbox( "  Hit marker", &cfg( )->b[ "hitmarker" ] );
			ui::Checkbox( "  Visualize sounds", &cfg( )->b[ "footstep" ] );
			ui::ColorEdit4( "##FOOTSTEPcolor", cfg( )->c[ "footstep_color" ] );
			ui::Checkbox( "  Force crosshair", &cfg( )->b[ "force_xhair" ] );
			ui::Checkbox( "  Remove flashbang effects", &cfg( )->b[ "noflash" ] );
			ui::Checkbox( "  Remove smoke grenades", &cfg( )->b[ "nosmoke" ] );
			ui::Checkbox( "  Remove scope overlay", &cfg( )->b[ "noscope" ] );
			ui::Checkbox( "  Remove fog", &cfg( )->b[ "nofog" ] );
			ui::Checkbox( "  Remove visual recoil", &cfg( )->b[ "novisrecoil" ] );
			ui::Checkbox( "  Disable post processing", &cfg( )->b[ "nopostprocess" ] );
			ui::Checkbox( "  Disable rendering of teammates", &cfg( )->b[ "disableteam" ] );
			ui::Text( "Thirdperson key" );
			ui::Keybind( " Thirdperson", &cfg( )->i[ "thirdperson_key" ], &cfg( )->i[ "thirdperson_keystyle" ] );
			ui::SliderInt( " Thirdperson distance", &cfg( )->i[ "thirdperson_distance" ], 50, 300, "%d%%" );
			ui::Checkbox( "  Spectator list", &cfg( )->b[ "spectators" ] );
			ui::MultiSelect( " Indicators", &cfg( )->m[ "indicators" ], { "Lower body yaw" , "Lag compensation" ,  "Fake latency" } );
			ui::Checkbox( "  Penetration reticle", &cfg( )->b[ "pen_crosshair" ] );
			ui::Checkbox( "  Penetration damage", &cfg( )->b[ "pen_damage" ] );
		} ui::EndChild( );
		ui::SetNextWindowPos( child_pos_backwend_world_esp( 1 ) );
		ui::BeginChild( "World ESP", child_size_world_esp ); {
			ui::Checkbox( "  Skybox change", &cfg( )->b[ "skybox_change" ] );
			ui::SingleSelect( "##nns", &cfg( )->i[ "skybox" ], { "Tibet" , "Embassy" , "Italy" , "Daylight" , "Cloudy" , "Night 1" , "Night 2" , "Night Flat" , "Day HD" , "Day" , "Rural" , "Vertigo HD" , "Vertigo Blue HD" , "Vertigo" , "Vietnam" ,"Dusty Sky","Jungle" , "Nuke" ,"Office" } );
			ui::SingleSelect( " Brightness adjustment", &cfg( )->i[ "world" ], { "Fullbright" , "Nightmode" } );
			ui::SliderInt( "##brightness", &cfg( )->i[ "night_darkness" ], 0, 100, "%d%%" );
			if ( ui::Button( " Update World" ) )
			{
				g_visuals.ModulateWorld( );
			}
			ui::Checkbox( "  Transparent props", &cfg( )->b[ "transparent_props" ] );
			ui::SliderInt( " Props transparency", &cfg( )->i[ "transparent_props_amount" ], 0, 100, "%d%%" );
		} ui::EndChild( );
	}
	if ( this->m_nCurrentTab == 3 ) {
		ui::SetNextWindowPos( child_pos_backwend( 0 ) );
		ui::BeginChild( "Miscellaneous", child_size ); {
			ui::Checkbox( "  Bunny hop", &cfg( )->b[ "bhop" ] );
			ui::Checkbox( "  Air strafe", &cfg( )->b[ "autostrafe" ] );
			ui::Checkbox( "  Clan tag spammer", &cfg( )->b[ "clantag" ] );
			ui::Checkbox( "  Log weapon purchases", &cfg( )->b[ "log_purchase" ] );
			ui::Checkbox( "  Log damage", &cfg( )->b[ "log_damage" ] );
			ui::Checkbox( "  Persistent kill feed", &cfg( )->b[ "killfeed" ] );
			ui::Text( "Ping spike" );
			ui::SliderInt( "##pingspikegay", &cfg( )->i[ "fake_latency_amt" ], 1.f, 1000.f, "%dms%" );
			ui::Keybind( "Ping spike", &cfg( )->i[ "fake_latency" ], &cfg( )->i[ "fakelatency_keystyle" ] );
		} ui::EndChild( );
		ui::SetNextWindowPos( child_pos_backwend( 1 ) );
		ui::BeginChild( "Settings", child_size_accuracy ); {
			ui::SingleSelect( " Safety mode", &cfg( )->i[ "config_mode" ], { XOR( "Matchmaking" ), XOR( "No-spread" ) } );
			ui::Checkbox( "  Unlock inventory in-game", &cfg( )->b[ "unlock" ] );
			ui::Text( "Menu key" );
			ui::Keybind( "###menukey", &cfg( )->i[ "misc_menukey" ] );
			ui::Text( "Menu color" );
			ui::ColorEdit4( "###menucolor", cfg( )->c[ "menu_color" ] );
			ui::Checkbox( "  Override FOV", &cfg( )->b[ "fov_visual" ] );
			ui::SliderInt( "##overridefov", &cfg( )->i[ "fov_amt" ], 60.f, 140.f, "%d°%" );
			ui::Checkbox( "  Override zoom FOV", &cfg( )->b[ "fov_scoped" ] );
			ui::Checkbox( "  Override viewmodel FOV", &cfg( )->b[ "viewmodel_fov" ] );
			ui::SliderInt( "##overrideVIEWMODELfov", &cfg( )->i[ "viewmodel_fov_amt" ], 60.f, 140.f, "%d°%" );
			if ( ui::Button( "Unlock Hidden Cvars" ) )
			{
				g_cl.UnlockHiddenConvars( );
			}
		} ui::EndChild( );
		ui::SetNextWindowPos( child_pos_other( 1 ) );
		ui::BeginChild( "Config", child_size_other ); {
			ui::SingleSelect( " Preset", &cfg( )->i[ "_preset" ], c_config::get( )->presets );
			ui::Keybind( "##presetkey", &cfg( )->i[ "_preset_" + std::to_string( cfg( )->i[ "_preset" ] ) ] );

			if ( ui::Button( "Load" ) )
			{
				c_config::get( )->load( );
				g_aimbot.m_fake_latency = false;

			}

			if ( ui::Button( "Save" ) )
			{
				c_config::get( )->save( );
				g_aimbot.m_fake_latency = false;
			}

			if ( ui::Button( "Reset" ) )
			{
				c_config::get( )->load_defaults( );
				g_aimbot.m_fake_latency = false;

			}
		} ui::EndChild( );

	}
	if ( this->m_nCurrentTab == 4 ) {
		ui::SetNextWindowPos( child_pos_backwend( 0 ) );
		ui::BeginChild( "Weapon skin", child_size ); {
			ui::Checkbox( "  Enabled", &cfg( )->b[ "skins" ] );
			if ( g_cl.m_weapon_id != -1 )
			{
				std::string base_string = XOR( "skins_" );

				switch ( g_cl.m_weapon_id ) {
				case  DEAGLE:
					base_string.append( XOR( "DEAGLE" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_deagle" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_deagle" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_deagle" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_deagle" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_deagle" ], 0, 0 );
					if ( cfg( )->i[ "id_deagle" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  ELITE:
					base_string.append( XOR( "ELITE" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_elite" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_elite" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_elite" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_elite" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_elite" ], 0, 0 );
					if ( cfg( )->i[ "id_elite" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  FIVESEVEN:
					base_string.append( XOR( "FIVESEVEN" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_fiveseven" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_fiveseven" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_fiveseven" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_fiveseven" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_fiveseven" ], 0, 0 );
					if ( cfg( )->i[ "id_fiveseven" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  GLOCK:
					base_string.append( XOR( "GLOCK" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_glock" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_glock" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_glock" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_glock" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_glock" ], 0, 0 );
					if ( cfg( )->i[ "id_glock" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  AK47:
					base_string.append( XOR( "AK47" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_ak47" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_ak47" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_ak47" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_ak47" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_ak47" ], 0, 0 );
					if ( cfg( )->i[ "id_ak47" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  AUG:
					base_string.append( XOR( "AUG" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_aug" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_aug" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_aug" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_aug" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_aug" ], 0, 0 );
					if ( cfg( )->i[ "id_aug" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case AWP:
					base_string.append( XOR( "AWP" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_awp" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_awp" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_awp" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_awp" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_awp" ], 0, 0 );
					if ( cfg( )->i[ "id_awp" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  FAMAS:
					base_string.append( XOR( "FAMAS" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_famas" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_famas" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_famas" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_famas" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_famas" ], 0, 0 );
					if ( cfg( )->i[ "id_famas" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  G3SG1:
					base_string.append( XOR( "G3SG1" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_g3sg1" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_g3sg1" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_g3sg1" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_g3sg1" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_g3sg1" ], 0, 0 );
					if ( cfg( )->i[ "id_g3sg1" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  GALIL:
					base_string.append( XOR( "GALIL" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_galil" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_galil" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_galil" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_galil" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_galil" ], 0, 0 );
					if ( cfg( )->i[ "id_galil" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  M249:
					base_string.append( XOR( "M249" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_m249" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_m249" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_m249" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_m249" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_m249" ], 0, 0 );
					if ( cfg( )->i[ "id_m249" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  M4A4:
					base_string.append( XOR( "M4A4" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_m4a4" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_m4a4" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_m4a4" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_m4a4" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_m4a4" ], 0, 0 );
					if ( cfg( )->i[ "id_m4a4" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  MAC10:
					base_string.append( XOR( "MAC10" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_mac10" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_mac10" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_mac10" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_mac10" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_mac10" ], 0, 0 );
					if ( cfg( )->i[ "id_mac10" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  P90:
					base_string.append( XOR( "P90" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_p90" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_p90" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_p90" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_p90" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_p90" ], 0, 0 );
					if ( cfg( )->i[ "id_p90" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  UMP45:
					base_string.append( XOR( "UMP45" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_ump45" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_ump45" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_ump45" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_ump45" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_ump45" ], 0, 0 );
					if ( cfg( )->i[ "id_ump45" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  XM1014:
					base_string.append( XOR( "XM1014" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_xm1014" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_xm1014" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_xm1014" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_xm1014" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_xm1014" ], 0, 0 );
					if ( cfg( )->i[ "id_xm1014" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  BIZON:
					base_string.append( XOR( "BIZON" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_bizon" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_bizon" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_bizon" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_bizon" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_bizon" ], 0, 0 );
					if ( cfg( )->i[ "id_bizon" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  MAG7:
					base_string.append( XOR( "MAG7" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_mag_7" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_mag_7" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_mag_7" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_mag_7" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_mag7" ], 0, 0 );
					if ( cfg( )->i[ "id_mag7" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  NEGEV:
					base_string.append( XOR( "NEGEV" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_negev" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_negev" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_negev" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_negev" ], 0, 1000 );
					if ( cfg( )->i[ "id_negev" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  SAWEDOFF:
					base_string.append( XOR( "SAWEDOFF" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_sawedoff" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_sawedoff" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_sawedoff" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_sawedoff" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_sawedoff" ], 0, 0 );
					break;
				case  TEC9:
					base_string.append( XOR( "TEC9" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_mac10" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_mac10" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_mac10" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_mac10" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_mac10" ], 0, 0 );
					break;
				case  P2000:
					base_string.append( XOR( "P2000" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_p2000" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_p2000" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_p2000" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_p2000" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_p2000" ], 0, 0 );
					break;
				case  MP7:
					base_string.append( XOR( "MP7" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_mp7" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_mp7" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_mp7" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_mp7" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_mp7" ], 0, 0 );
					break;
				case  MP9:
					base_string.append( XOR( "MP9" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_mp9" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_mp9" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_mp9" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_mp9" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_mp9" ], 0, 0 );
					break;
				case  NOVA:
					base_string.append( XOR( "NOVA" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_nova" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_nova" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_nova" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_nova" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_nova" ], 0, 0 );
					break;
				case  P250:
					base_string.append( XOR( "P250" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_p250" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_p250" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_p250" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_p250" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_p250" ], 0, 0 );
					break;
				case  SCAR20:
					base_string.append( XOR( "SCAR20" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_scar20" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_scar20" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_scar20" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_scar20" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_scar20" ], 0, 0 );
					if ( cfg( )->i[ "id_scar20" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  SG553:
					base_string.append( XOR( "SG553" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_sg553" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_sg553" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_sg553" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_sg553" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_sg553" ], 0, 0 );
					break;
				case  SSG08:
					base_string.append( XOR( "SSG08" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_ssg08" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_ssg08" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_ssg08" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_ssg08" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_ssg08" ], 0, 0 );
					if ( cfg( )->i[ "id_ssg08" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  M4A1S:
					base_string.append( XOR( "M4A1S" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_m4a1s" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_m4a1s" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_m4a1s" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_m4a1s" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_m4a1s" ], 0, 0 );
					break;
				case  USPS:
					base_string.append( XOR( "USPS" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_usps" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_usps" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_usps" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_usps" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_usps" ], 0, 0 );
					break;
				case  CZ75A:
					base_string.append( XOR( "CZ75A" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_cz75a" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_cz75a" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_cz75a" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_cz75a" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_cz75a" ], 0, 0 );
					break;
				case  REVOLVER:
					base_string.append( XOR( "REVOLVER" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_revolver" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_revolver" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_revolver" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_revolver" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_revolver" ], 0, 0 );
					if ( cfg( )->i[ "id_revolver" ] > 1 )
					{
						SkinUpdate( );
					}
					break;


				default:
					break;
				}
			}
		} ui::EndChild( );
		ui::SetNextWindowPos( child_pos_backwend( 1 ) );
		ui::BeginChild( "Knife & Gloves", child_size_accuracy ); {
			ui::SingleSelect( "##knife", &cfg( )->i[ "knife" ], { "Off", "Bayonet", "Bowie", "Butterfly", "Falchion", "Flip", "Gut", "Huntsman", "Karambit", "M9 Bayonet" , "Daggers" } );
			if ( g_cl.m_weapon_id != -1 )
			{
				std::string base_string = XOR( "skins_" );

				switch ( g_cl.m_weapon_id ) {
				case  KNIFE_BAYONET:
					base_string.append( XOR( "KNIFE_BAYONET" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_bayonet" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_bayonet" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_bayonet" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_bayonet" ], 0, 1000 );
					ui::InputInt( "Paintkit id", &cfg( )->i[ "id_bayonet" ], 1, 5 );
					if ( cfg( )->i[ "id_bayonet" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  KNIFE_FLIP:
					base_string.append( XOR( "KNIFE_FLIP" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_flip" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_flip" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_flip" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_flip" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_flip" ], 0, 0 );
					if ( cfg( )->i[ "id_flip" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case KNIFE_GUT:
					base_string.append( XOR( "KNIFE_GUT" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_gut" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_gut" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_gut" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_gut" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_gut" ], 0, 0 );
					if ( cfg( )->i[ "id_gut" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  KNIFE_KARAMBIT:
					base_string.append( XOR( "KNIFE_KARAMBIT" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_karambit" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_karambit" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_karambit" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_karambit" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_karambit" ], 0, 0 );
					if ( cfg( )->i[ "id_karambit" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  KNIFE_M9_BAYONET:
					base_string.append( XOR( "KNIFE_M9_BAYONET" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_m9" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_m9" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_m9" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_m9" ], 0, 1000 );
					if ( cfg( )->i[ "id_m9" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case KNIFE_HUNTSMAN:
					base_string.append( XOR( "KNIFE_HUNTSMAN" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_huntsman" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_huntsman" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_huntsman" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_huntsman" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_huntsman" ], 0, 0 );
					if ( cfg( )->i[ "id_huntsman" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  KNIFE_FALCHION:
					base_string.append( XOR( "KNIFE_FALCHION" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_falchion" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_falchion" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_falchion" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_falchion" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_falchion" ], 0, 0 );
					if ( cfg( )->i[ "id_falchion" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  KNIFE_BOWIE:
					base_string.append( XOR( "KNIFE_BOWIE" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_bowie" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_bowie" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_bowie" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_bowie" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_bowie" ], 0, 0 );
					if ( cfg( )->i[ "id_bowie" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case  KNIFE_BUTTERFLY:
					base_string.append( XOR( "KNIFE_BUTTERFLY" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_butterfly" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_butterfly" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_butterfly" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_butterfly" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "butterfly" ], 0, 0 );
					if ( cfg( )->i[ "id_butterfly" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				case KNIFE_SHADOW_DAGGERS:
					base_string.append( XOR( "KNIFE_SHADOW_DAGGERS" ) );
					ui::SliderInt( " Paintkit id", &cfg( )->i[ "id_daggers" ], 0, 1000 );
					ui::Checkbox( " Stattrak", &cfg( )->b[ "stattrak_daggers" ] );
					ui::SliderInt( " Quality", &cfg( )->i[ "quality_daggers" ], 0, 100 );
					ui::SliderInt( " Seed", &cfg( )->i[ "seed_daggers" ], 0, 1000 );
					ui::InputInt( "", &cfg( )->i[ "id_daggers" ], 0, 0 );
					if ( cfg( )->i[ "id_daggers" ] > 1 )
					{
						SkinUpdate( );
					}
					break;
				default:
					break;
				}

				ui::SingleSelect( "##glove", &cfg( )->i[ "glove" ], { "Off", "Bloodhound", "Sport", "Driver", "Handwraps", "Moto", "Specialist" } );
				ui::SliderInt( "  Paintkit gloves", &cfg( )->i[ "glove_id" ], 0, 26 );
				ui::InputInt( "", &cfg( )->i[ "glove_id" ], 0, 0 );

				if ( cfg( )->i[ "glove_id" ] > 0 )
					SkinUpdate( );
			}
		} ui::EndChild( );
		ui::SetNextWindowPos( child_pos_other( 1 ) );
		ui::BeginChild( "Update", child_size_other ); {
			if ( ui::Button( "Skin Update" ) )
				SkinUpdate( );
		} ui::EndChild( );
	}
	ui::End( );
}

void c_menu::draw_end( ) {
	if ( !this->m_bInitialized )
		return;

	ui::EndFrame( );
	ui::Render( );
	ImGui_ImplDX9_RenderDrawData( ui::GetDrawData( ) );
}

bool c_menu::is_menu_initialized( ) {
	return this->m_bInitialized;
}

bool c_menu::is_menu_opened( ) {
	return this->m_bIsOpened;
}

void c_menu::set_menu_opened( bool v ) {
	this->m_bIsOpened = v;
}

IDirect3DTexture9* c_menu::get_texture_data( ) {
	return this->m_pTexture;
}

ImColor c_menu::get_accent_color( ) {
	return ImColor(
		c_config::get( )->c[ "menu_color" ][ 0 ],
		c_config::get( )->c[ "menu_color" ][ 1 ],
		c_config::get( )->c[ "menu_color" ][ 2 ],
		ui::GetStyle( ).Alpha
	);
}