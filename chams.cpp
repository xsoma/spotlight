#include "includes.h"

Chams g_chams{ };;

Chams::model_type_t Chams::GetModelType( const ModelRenderInfo_t& info ) {
	// model name.
	//const char* mdl = info.m_model->m_name;

	std::string mdl{ info.m_model->m_name };

	//static auto int_from_chars = [ mdl ]( size_t index ) {
	//	return *( int* )( mdl + index );
	//};

	// little endian.
	//if( int_from_chars( 7 ) == 'paew' ) { // weap
	//	if( int_from_chars( 15 ) == 'om_v' && int_from_chars( 19 ) == 'sled' )
	//		return model_type_t::arms;
	//
	//	if( mdl[ 15 ] == 'v' )
	//		return model_type_t::view_weapon;
	//}

	//else if( int_from_chars( 7 ) == 'yalp' ) // play
	//	return model_type_t::player;

	if ( mdl.find( XOR( "player" ) ) != std::string::npos && info.m_index >= 1 && info.m_index <= 64 )
		return model_type_t::player;

	return model_type_t::invalid;
}
/*IMaterial* CreateMaterial(bool lit, const std::string& material_data)
{
	static auto created = 0;
	std::string type = lit ? XOR("VertexLitGeneric") : XOR("UnlitGeneric");

	auto matname = XOR("cthulhu_") + std::to_string(created);
	++created;

	auto keyValues = new KeyValues(matname.c_str());
	using KeyValuesFn = void(__thiscall*)(void*, const char*);
	reinterpret_cast <KeyValuesFn> (g_csgo.KeyValuesAdress)(keyValues, type.c_str());

	using LoadFromBuffer_t = void(__thiscall*)(void*, const char*, const char*, void*, const char*, void*);
	reinterpret_cast <LoadFromBuffer_t> (g_csgo.LoadFromBuffer)(keyValues, matname.c_str(), material_data.c_str(), nullptr, nullptr, nullptr);

	auto material = g_csgo.m_material_system->CreateMaterial(matname.c_str(), keyValues);
	material->IncrementReferenceCount();

	return material;
}*/
bool Chams::IsInViewPlane( const vec3_t& world ) {
	float w;

	const VMatrix& matrix = g_csgo.m_engine->WorldToScreenMatrix( );

	w = matrix[ 3 ][ 0 ] * world.x + matrix[ 3 ][ 1 ] * world.y + matrix[ 3 ][ 2 ] * world.z + matrix[ 3 ][ 3 ];

	return w > 0.001f;
}

void Chams::SetColor( Color col, IMaterial* mat ) {
	if ( mat )
		mat->ColorModulate( col );

	else
		g_csgo.m_render_view->SetColorModulation( col );
}

void Chams::SetAlpha( float alpha, IMaterial* mat ) {
	if ( mat )
		mat->AlphaModulate( alpha );

	else
		g_csgo.m_render_view->SetBlend( alpha );
}

void Chams::SetupMaterial( IMaterial* mat, Color col, bool z_flag ) {
	SetColor( col );

	// mat->SetFlag( MATERIAL_VAR_HALFLAMBERT, flags );
	mat->SetFlag( MATERIAL_VAR_ZNEARER, z_flag );
	mat->SetFlag( MATERIAL_VAR_NOFOG, z_flag );
	mat->SetFlag( MATERIAL_VAR_IGNOREZ, z_flag );

	g_csgo.m_studio_render->ForcedMaterialOverride( mat );
}

void Chams::init( ) {
	std::ofstream( "csgo\\materials\\simple_flat.vmt" ) << R"#("UnlitGeneric"
{
  "$basetexture" "vgui/white_additive"
  "$ignorez"      "1"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";
	std::ofstream( "csgo\\materials\\simple_ignorez_reflective.vmt" ) << R"#("VertexLitGeneric"
{

  "$basetexture" "vgui/white_additive"
  "$ignorez"      "1"
  "$envmap"       "env_cubemap"
  "$normalmapalphaenvmapmask"  "1"
  "$envmapcontrast"             "1"
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";

	std::ofstream( "csgo\\materials\\simple_regular_reflective.vmt" ) << R"#("VertexLitGeneric"
{

  "$basetexture" "vgui/white_additive"
  "$ignorez"      "0"
  "$envmap"       "env_cubemap"
  "$normalmapalphaenvmapmask"  "1"
  "$envmapcontrast"             "1"
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";

	std::ofstream( "csgo/materials/skeetchams.vmt" ) << R"#("VertexLitGeneric"
{
	  "$basetexture" "vgui/white_additive"
	  "$ignorez" "0"
	  "$additive" "0"
	  "$envmap"  "models/effects/cube_white"
	  "$normalmapalphaenvmapmask" "1"
	  "$envmaptint" "[0.25 0.4 0.88]"
	  "$envmapfresnel" "1"
	  "$envmapfresnelminmaxexp" "[0 1 2]"

	  "$envmapcontrast" "1"
	  "$nofog" "1"
	  "$model" "1"
	  "$nocull" "0"
	  "$selfillum" "1"
	  "$halflambert" "1"
	  "$znearer" "0"
	  "$flat" "1"
}
)#";

	std::ofstream( "csgo/materials/onetap_overlay.vmt" ) << R"#(" VertexLitGeneric "{

			"$additive" "1"
	        "$envmap"  "models/effects/cube_white"
			"$envmaptint" "[0 0 0]"
			"$envmapfresnel" "1"
			"$envmapfresnelminmaxexp" "[0 16 12]"
			"$alpha" "0.8"
})#";

	std::ofstream( "csgo/materials/glowOverlay.vmt" ) << R"#("VertexLitGeneric" {
				"$additive" "1"
				"$envmap" "models/effects/cube_white"
				"$envmaptint" "[0 0.1 0.2]"
				"$envmapfresnel" "1"
				"$envmapfresnelminmaxexp" "[0 1 2]"
				"$ignorez" "1"
				"$alpha" "1"
			})#";

	std::ofstream( "csgo/materials/testnigger.vmt" ) << R"#("VertexLitGeneric" { 

  "$basetexture" "vgui/white_additive"
  "$ignorez"      "1"
  "$envmap"       "env_cubemap"
  "$envmaptint"   "[0.6 0.6 0.6]"
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
	})#";

	std::ofstream( "csgo\\materials\\yeti_flat.vmt" ) << R"#("UnlitGeneric"
{
  "$basetexture" "vgui/white_additive"
  "$ignorez"      "0"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";

	materialMetall = g_csgo.m_material_system->FindMaterial( XOR( "simple_ignorez_reflective" ), nullptr );
	materialMetall->IncrementReferenceCount( );

	materialMetallnZ = g_csgo.m_material_system->FindMaterial( XOR( "simple_regular_reflective" ), nullptr );
	materialMetallnZ->IncrementReferenceCount( );

	// find stupid materials.
	debugambientcube = g_csgo.m_material_system->FindMaterial( XOR( "debug/debugambientcube" ), XOR( "Model textures" ) );
	debugambientcube->IncrementReferenceCount( );

	debugdrawflat = g_csgo.m_material_system->FindMaterial( XOR( "debug/debugdrawflat" ), XOR( "Model textures" ) );
	debugdrawflat->IncrementReferenceCount( );

	materialMetall3 = g_csgo.m_material_system->FindMaterial( XOR( "testnigger" ), XOR( "Model textures" ) );
	materialMetall3->IncrementReferenceCount( );

	skeet = g_csgo.m_material_system->FindMaterial( XOR( "skeetchams" ), XOR( "Model textures" ) );
	skeet->IncrementReferenceCount( );

	onetap = g_csgo.m_material_system->FindMaterial( XOR( "onetap_overlay" ), XOR( "Model textures" ) );
	onetap->IncrementReferenceCount( );

	yeti = g_csgo.m_material_system->FindMaterial( XOR( "yeti_flat.vmt" ), XOR( "Model textures" ) );
	yeti->IncrementReferenceCount( );
}

bool Chams::OverridePlayer( int index ) {
	Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( index );
	if ( !player )
		return false;

	// always skip the local player in DrawModelExecute.
	// this is because if we want to make the local player have less alpha
	// the static props are drawn after the players and it looks like aids.
	// therefore always process the local player in scene end.
	if ( player->m_bIsLocalPlayer( ) )
		return true;

	// see if this player is an enemy to us.
	bool enemy = g_cl.m_local && player->enemy( g_cl.m_local );

	// we have chams on enemies.
	if ( enemy && c_config::get( )->b[ "chams_enemy_visible" ] )
		return true;

	return false;
}

bool Chams::GenerateLerpedMatrix( int index, BoneArray* out ) {
	LagRecord* current_record;
	AimPlayer* data;

	Player* ent = g_csgo.m_entlist->GetClientEntity< Player* >( index );
	if ( !ent )
		return false;

	if ( !g_aimbot.IsValidTarget( ent ) )
		return false;

	data = &g_aimbot.m_players[ index - 1 ];
	if ( !data || data->m_records.empty( ) )
		return false;

	if ( data->m_records.size( ) < 2 )
		return false;

	auto* channel_info = g_csgo.m_engine->GetNetChannelInfo( );
	if ( !channel_info )
		return false;

	static float max_unlag = 0.2f;
	static auto sv_maxunlag = g_csgo.m_cvar->FindVar( HASH( "sv_maxunlag" ) );
	if ( sv_maxunlag ) {
		max_unlag = sv_maxunlag->GetFloat( );
	}

	for ( auto it = data->m_records.rbegin( ); it != data->m_records.rend( ); it++ ) {
		current_record = it->get( );

		bool end = it + 1 == data->m_records.rend( );

		if ( current_record && current_record->valid( ) && ( !end && ( ( it + 1 )->get( ) ) ) ) {
			if ( current_record->m_origin.dist_to( ent->GetAbsOrigin( ) ) < 1.f ) {
				return false;
			}

			vec3_t next = end ? ent->GetAbsOrigin( ) : ( it + 1 )->get( )->m_origin;
			float  time_next = end ? ent->m_flSimulationTime( ) : ( it + 1 )->get( )->m_sim_time;

			float total_latency = channel_info->GetAvgLatency( 0 ) + channel_info->GetAvgLatency( 1 );
			std::clamp( total_latency, 0.f, max_unlag );

			float correct = total_latency + g_cl.m_lerp;
			float time_delta = time_next - current_record->m_sim_time;
			float add = end ? 1.f : time_delta;
			float deadtime = current_record->m_sim_time + correct + add;

			float curtime = g_csgo.m_globals->m_curtime;
			float delta = deadtime - curtime;

			float mul = 1.f / add;
			vec3_t lerp = math::Interpolate( next, current_record->m_origin, std::clamp( delta * mul, 0.f, 1.f ) );

			matrix3x4_t ret[ 128 ];

			std::memcpy( ret,
				current_record->m_bones,
				sizeof( ret ) );

			for ( size_t i{ }; i < 128; ++i ) {
				vec3_t matrix_delta = current_record->m_bones[ i ].GetOrigin( ) - current_record->m_origin;
				ret[ i ].SetOrigin( matrix_delta + lerp );
			}

			std::memcpy( out,
				ret,
				sizeof( ret ) );

			return true;
		}
	}

	return false;
}

void Chams::RenderHistoryChams( int index ) {
	AimPlayer* data;
	LagRecord* record;

	Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( index );
	if ( !player )
		return;

	if ( !g_aimbot.IsValidTarget( player ) )
		return;

	bool enemy = g_cl.m_local && player->enemy( g_cl.m_local );
	if ( enemy ) {
		data = &g_aimbot.m_players[ index - 1 ];
		//if (!data || data->m_records.empty())
			//return;

		//record = g_resolver.FindLastRecord(data);
		//if (!record)
		//	return;

		// override blend.
		SetAlpha( c_config::get( )->i[ "chams_enemy_history_blend" ] / 100.f );

		Color clr = Color( c_config::get( )->c[ "chams_enemy_history_col" ][ 0 ] * 255, c_config::get( )->c[ "chams_enemy_history_col" ][ 1 ] * 255, c_config::get( )->c[ "chams_enemy_history_col" ][ 2 ] * 255, c_config::get( )->c[ "chams_enemy_history_col" ][ 3 ] * 255 );
		// set material and color.
		SetupMaterial( debugdrawflat, clr, true );

		// was the matrix properly setup?
		BoneArray arr[ 128 ];
		if ( Chams::GenerateLerpedMatrix( index, arr ) ) {
			// backup the bone cache before we fuck with it.
			auto backup_bones = player->m_BoneCache( ).m_pCachedBones;

			// replace their bone cache with our custom one.
			player->m_BoneCache( ).m_pCachedBones = arr;

			// manually draw the model.
			player->DrawModel( );

			// reset their bone cache to the previous one.
			player->m_BoneCache( ).m_pCachedBones = backup_bones;
		}
	}
}

bool Chams::DrawModel( uintptr_t ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone ) {
	// store and validate model type.
	model_type_t type = GetModelType( info );
	if ( type == model_type_t::invalid )
		return true;

	// is a valid player.
	if ( type == model_type_t::player ) {
		// do not cancel out our own calls from SceneEnd
		// also do not cancel out calls from the glow.
		if ( !m_running && !g_csgo.m_studio_render->m_pForcedMaterial && OverridePlayer( info.m_index ) )
			return false;
	}

	return true;
}


void Chams::SceneEnd( ) {
	// store and sort ents by distance.
	if ( SortPlayers( ) ) {
		// iterate each player and render them.
		for ( const auto& p : m_players ) {
			RenderPlayer( p );
		}
		RenderFake( );
	}

	// restore.
	g_csgo.m_studio_render->ForcedMaterialOverride( nullptr );
	g_csgo.m_render_view->SetColorModulation( colors::white );
	g_csgo.m_render_view->SetBlend( 1.f );
}

void Chams::RenderFake( ) {
	if ( c_config::get( )->b[ "fake_chams" ] ) { // check if active
		if ( g_cl.m_local ) {
			SetAlpha( c_config::get( )->i[ "fake_chams_blend" ] / 100.f );
			Color clr = Color( c_config::get( )->c[ "fake_chams_col" ][ 0 ] * 255, c_config::get( )->c[ "fake_chams_col" ][ 1 ] * 255, c_config::get( )->c[ "fake_chams_col" ][ 2 ] * 255, c_config::get( )->c[ "fake_chams_col" ][ 3 ] * 255 );
			SetupMaterial( debugdrawflat, clr, false );
			ang_t fake1337( 0, g_cl.m_radar.y, 0 );
			g_cl.m_local->SetAbsAngles( fake1337 );
			g_cl.m_local->DrawModel( );
		}
	}
}

void Chams::DrawChams( void* ecx, uintptr_t ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone ) {
	Player* m_entity = g_csgo.m_entlist->GetClientEntity<Player*>( info.m_index );

	// or crash >_< and prevents dead bodies from being colored
	if ( !m_entity || m_entity->m_iHealth( ) < 1 )
		return;

	// we always draw the local player manually in drawmodel.
	if ( m_entity->m_bIsLocalPlayer( ) )
	{
		if ( c_config::get( )->b[ "chams_local_scope" ] && m_entity->m_bIsScoped( ) )
			SetAlpha( 0.5f );

		else if ( c_config::get( )->b[ "chams_local" ] )
		{

			// orig mdl
			if ( m_entity->m_bIsLocalPlayer( ) )
			{
				g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >( IVModelRender::DRAWMODELEXECUTE )( ecx, ctx, state, info, bone );
			}
			Color clr = Color( c_config::get( )->c[ "chams_local_col" ][ 0 ] * 255, c_config::get( )->c[ "chams_local_col" ][ 1 ] * 255, c_config::get( )->c[ "chams_local_col" ][ 2 ] * 255 );
			// x ray, this btw overrides everything
			if ( m_entity->m_bIsLocalPlayer( ) )
			{
				// override blend.

				SetAlpha( c_config::get( )->c[ "chams_local_col" ][ 3 ] / 100.f );

				// TODO separation
				SetupMaterial( yeti, clr, false );
				g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >( IVModelRender::DRAWMODELEXECUTE )( ecx, ctx, state, info, bone );
			}

			// wire start
			SetAlpha( c_config::get( )->i[ "chams_local_blend" ] / 100.f );

			SetupMaterial( debugambientcube, clr, false );

			g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >( IVModelRender::DRAWMODELEXECUTE )( ecx, ctx, state, info, bone );

		}
	}
}

void Chams::RenderPlayer( Player* player ) {
	// prevent recruisive model cancelation.
	m_running = true;

	// restore.
	g_csgo.m_studio_render->ForcedMaterialOverride( nullptr );
	g_csgo.m_render_view->SetColorModulation( colors::white );
	g_csgo.m_render_view->SetBlend( 1.f );

	// this is the local player.
	// we always draw the local player manually in drawmodel.
	if ( player->m_bIsLocalPlayer( ) ) {
		if ( c_config::get( )->b[ "chams_local_scope" ] && player->m_bIsScoped( ) )
			SetAlpha( 0.5f );

		else if ( c_config::get( )->b[ "chams_local" ] ) {
			// override blend.
			SetAlpha( c_config::get( )->i[ "chams_local_blend" ] / 100.f );

			Color clr = Color( c_config::get( )->c[ "chams_local_col" ][ 0 ] * 255, c_config::get( )->c[ "chams_local_col" ][ 1 ] * 255, c_config::get( )->c[ "chams_local_col" ][ 2 ] * 255, c_config::get( )->c[ "chams_local_col" ][ 3 ] * 255 );

			// set material and color.
			if ( c_config::get( )->i[ "chamstype_local" ] == 0 ) {
				SetupMaterial( debugambientcube, clr, false );
			}
			if ( c_config::get( )->i[ "chamstype_local" ] == 1 ) {
				SetupMaterial( debugdrawflat, clr, false );
			}
			if ( c_config::get( )->i[ "chamstype_local" ] == 2 ) {
				SetupMaterial( materialMetall, clr, false );
			}
			if ( c_config::get( )->i[ "chamstype_local" ] == 3 ) {
				SetupMaterial( skeet, clr, false );
			}
			if ( c_config::get( )->i[ "chamstype_local" ] == 4 ) {
				SetupMaterial( onetap, clr, false );
			}
			if ( c_config::get( )->i[ "chamstype_local" ] == 5 ) {
				SetupMaterial( materialMetall3, clr, false );
			}
		}
		// manually draw the model.
		player->DrawModel( );
	}
	// check if is an enemy.
	bool enemy = g_cl.m_local && player->enemy( g_cl.m_local );

	if ( enemy && c_config::get( )->b[ "chams_enemy_history" ] ) {
		RenderHistoryChams( player->index( ) );
	}
	Color clr_vis = Color( c_config::get( )->c[ "chams_enemy_vis" ][ 0 ] * 255, c_config::get( )->c[ "chams_enemy_vis" ][ 1 ] * 255, c_config::get( )->c[ "chams_enemy_vis" ][ 2 ] * 255 );
	Color clr_xqz = Color( c_config::get( )->c[ "chams_enemy_invis" ][ 0 ] * 255, c_config::get( )->c[ "chams_enemy_invis" ][ 1 ] * 255, c_config::get( )->c[ "chams_enemy_invis" ][ 2 ] * 255 );

	if ( enemy && c_config::get( )->b[ "chams_enemy_visible" ] ) {
		if ( c_config::get( )->b[ "chams_enemy_invisible" ] ) {

			SetAlpha( c_config::get( )->i[ "chams_enemy_blend" ] / 100.f );

			if ( c_config::get( )->i[ "chamstype" ] == 0 ) {
				SetupMaterial( debugambientcube, clr_xqz, true );
			}
			if ( c_config::get( )->i[ "chamstype" ] == 1 ) {
				SetupMaterial( debugdrawflat, clr_xqz, true );
			}
			if ( c_config::get( )->i[ "chamstype" ] == 2 ) {
				SetupMaterial( materialMetallnZ, clr_xqz, true );
			}
			if ( c_config::get( )->i[ "chamstype" ] == 3 ) {
				SetupMaterial( skeet, clr_xqz, true );
			}
			if ( c_config::get( )->i[ "chamstype" ] == 4 ) {
				SetupMaterial( onetap, clr_xqz, true );
			}
			if ( c_config::get( )->i[ "chamstype" ] == 5 ) {
				SetupMaterial( materialMetall3, clr_xqz, true );
			}
			player->DrawModel( );
		}

		SetAlpha( c_config::get( )->i[ "chams_enemy_blend" ] / 100.f );

		if ( c_config::get( )->i[ "chamstype" ] == 0 ) {
			SetupMaterial( debugambientcube, clr_vis, false );
		}
		if ( c_config::get( )->i[ "chamstype" ] == 1 ) {
			SetupMaterial( debugdrawflat, clr_vis, false );
		}
		if ( c_config::get( )->i[ "chamstype" ] == 2 ) {
			SetupMaterial( materialMetall, clr_vis, false );
		}
		if ( c_config::get( )->i[ "chamstype" ] == 3 ) {
			SetupMaterial( skeet, clr_vis, false );
		}
		if ( c_config::get( )->i[ "chamstype" ] == 4 ) {
			SetupMaterial( onetap, clr_vis, false );
		}
		if ( c_config::get( )->i[ "chamstype" ] == 5 ) {
			SetupMaterial( materialMetall3, clr_vis, false );
		}

		player->DrawModel( );
	}


	m_running = false;
}

bool Chams::SortPlayers( ) {
	// lambda-callback for std::sort.
	// to sort the players based on distance to the local-player.
	static auto distance_predicate = [ ]( Entity* a, Entity* b ) {
		vec3_t local = g_cl.m_local->GetAbsOrigin( );

		// note - dex; using squared length to save out on sqrt calls, we don't care about it anyway.
		float len1 = ( a->GetAbsOrigin( ) - local ).length_sqr( );
		float len2 = ( b->GetAbsOrigin( ) - local ).length_sqr( );

		return len1 < len2;
	};

	// reset player container.
	m_players.clear( );

	// find all players that should be rendered.
	for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
		// get player ptr by idx.
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );

		// validate.
		if ( !player || !player->IsPlayer( ) || !player->alive( ) )
			continue;

		// do not draw players occluded by view plane.
		if ( !IsInViewPlane( player->WorldSpaceCenter( ) ) )
			continue;

		// this player was not skipped to draw later.
		// so do not add it to our render list.
		if ( !OverridePlayer( i ) )
			continue;

		m_players.push_back( player );
	}

	// any players?
	if ( m_players.empty( ) )
		return false;

	// sorting fixes the weird weapon on back flickers.
	// and all the other problems regarding Z-layering in this shit game.
	std::sort( m_players.begin( ), m_players.end( ), distance_predicate );

	return true;
}