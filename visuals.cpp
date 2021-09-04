#include "includes.h"

Visuals g_visuals{ };;

void Visuals::ModulateWorld() {
	std::vector< IMaterial* > world, props;

	// iterate material handles.
	for (uint16_t h{ g_csgo.m_material_system->FirstMaterial() }; h != g_csgo.m_material_system->InvalidMaterial(); h = g_csgo.m_material_system->NextMaterial(h)) {
		// get material from handle.
		IMaterial* mat = g_csgo.m_material_system->GetMaterial(h);
		if (!mat)
			continue;

		// store world materials.
		if (FNV1a::get(mat->GetTextureGroupName()) == HASH("World textures"))
			world.push_back(mat);

		// store props.
		else if (FNV1a::get(mat->GetTextureGroupName()) == HASH("StaticProp textures"))
			props.push_back(mat);
	}

	// night
	const float darkness = c_config::get( )->i[ "night_darkness" ] / 100.f;

	if ( c_config::get( )->i[ "world" ] == 1 ) {
		for (const auto& w : world)
			w->ColorModulate(darkness, darkness, darkness);

		// IsUsingStaticPropDebugModes my nigga
		if (g_csgo.r_DrawSpecificStaticProp->GetInt() != 0) {
			g_csgo.r_DrawSpecificStaticProp->SetValue(0);
		}

		for (const auto& p : props)
			p->ColorModulate(0.5f, 0.5f, 0.5f);

		//game::SetSkybox(XOR("sky_csgo_night02"));
	}

	// disable night.
	else {
		for (const auto& w : world)
			w->ColorModulate(1.f, 1.f, 1.f);

		// restore r_DrawSpecificStaticProp.
		if (g_csgo.r_DrawSpecificStaticProp->GetInt() != -1) {
			g_csgo.r_DrawSpecificStaticProp->SetValue(-1);
		}

		for (const auto& p : props)
			p->ColorModulate(1.f, 1.f, 1.f);
	}

	// transparent props.
	if ( c_config::get( )->b[ "transparent_props" ] ) {

		// IsUsingStaticPropDebugModes my nigga
		if ( g_csgo.r_DrawSpecificStaticProp->GetInt( ) != 0 ) {
			g_csgo.r_DrawSpecificStaticProp->SetValue( 0 );
		}

		float alpha = c_config::get( )->i[ "transparent_props_amount" ] / 100.f;
		for ( const auto& p : props )
			p->AlphaModulate( alpha );
	}

	// disable transparent props.
	else {

		// restore r_DrawSpecificStaticProp.
		if (g_csgo.r_DrawSpecificStaticProp->GetInt() != -1) {
			g_csgo.r_DrawSpecificStaticProp->SetValue(-1);
		}

		for (const auto& p : props)
			p->AlphaModulate(1.0f);
	}
}

void Visuals::ThirdpersonThink() {
	ang_t                          offset;
	vec3_t                         origin, forward;
	static CTraceFilterSimple_game filter{ };
	CGameTrace                     tr;

	// for whatever reason overrideview also gets called from the main menu.
	if (!g_csgo.m_engine->IsInGame())
		return;

	// check if we have a local player and he is alive.
	bool alive = g_cl.m_local && g_cl.m_local->alive();

	// camera should be in thirdperson.
	if (m_thirdperson) {

		// if alive and not in thirdperson already switch to thirdperson.
		if (alive && !g_csgo.m_input->CAM_IsThirdPerson())
			g_csgo.m_input->CAM_ToThirdPerson();

		// if dead and spectating in firstperson switch to thirdperson.
		else if (g_cl.m_local->m_iObserverMode() == 4) {

			// if in thirdperson, switch to firstperson.
			// we need to disable thirdperson to spectate properly.
			if (g_csgo.m_input->CAM_IsThirdPerson()) {
				g_csgo.m_input->CAM_ToFirstPerson();
				g_csgo.m_input->m_camera_offset.z = 0.f;
			}

			g_cl.m_local->m_iObserverMode() = 5;
		}
	}

	// camera should be in firstperson.
	else if (g_csgo.m_input->CAM_IsThirdPerson()) {
		g_csgo.m_input->CAM_ToFirstPerson();
		g_csgo.m_input->m_camera_offset.z = 0.f;
	}

	// if after all of this we are still in thirdperson.
	if (g_csgo.m_input->CAM_IsThirdPerson()) {
		// get camera angles.
		g_csgo.m_engine->GetViewAngles(offset);

		// get our viewangle's forward directional vector.
		math::AngleVectors(offset, &forward);

		// cam_idealdist convar.
		offset.z = c_config::get( )->i[ "thirdperson_distance" ];

		// start pos.
		origin = g_cl.m_shoot_pos;

		// setup trace filter and trace.
		filter.SetPassEntity(g_cl.m_local);

		g_csgo.m_engine_trace->TraceRay(
			Ray(origin, origin - (forward * offset.z), { -16.f, -16.f, -16.f }, { 16.f, 16.f, 16.f }),
			MASK_NPCWORLDSTATIC,
			(ITraceFilter*)&filter,
			&tr
		);

		// adapt distance to travel time.
		math::clamp(tr.m_fraction, 0.f, 1.f);
		offset.z *= tr.m_fraction;

		// override camera angles.
		g_csgo.m_input->m_camera_offset = { offset.x, offset.y, offset.z };
	}
}

void Visuals::Hitmarker() 
{
	static auto cross = g_csgo.m_cvar->FindVar(HASH("weapon_debug_spread_show"));
	cross->SetValue( c_config::get( )->b[ "force_xhair" ] && !g_cl.m_local->m_bIsScoped( ) ? 3 : 0 );
	if ( !c_config::get( )->b[ "hitmarker" ] )
		return;

	if (g_csgo.m_globals->m_curtime > m_hit_end)
		return;

	if (m_hit_duration <= 0.f)
		return;

	float complete = (g_csgo.m_globals->m_curtime - m_hit_start) / m_hit_duration;
	int x = g_cl.m_width / 2,
		y = g_cl.m_height / 2,
		alpha = (1.f - complete) * 240;

	constexpr int line{ 6 };

	render::line(x - line, y - line, x - (line / 4), y - (line / 4), { 200, 200, 200, alpha });
	render::line(x - line, y + line, x - (line / 4), y + (line / 4), { 200, 200, 200, alpha });
	render::line(x + line, y + line, x + (line / 4), y + (line / 4), { 200, 200, 200, alpha });
	render::line(x + line, y - line, x + (line / 4), y - (line / 4), { 200, 200, 200, alpha });
}

void Visuals::NoSmoke() {
	if ( !c_config::get( )->b[ "nosmoke" ] )
		return;
	//colado https://www.unknowncheats.me/forum/counterstrike-global-offensive/262635-epic-wireframe-smoke.html
	std::vector<const char*> vistasmoke_mats =
	{
			"particle/vistasmokev1/vistasmokev1_fire",
			"particle/vistasmokev1/vistasmokev1_smokegrenade",
			"particle/vistasmokev1/vistasmokev1_emods",
			"particle/vistasmokev1/vistasmokev1_emods_impactdust",
	};

	for (auto mat_s : vistasmoke_mats)
	{
		IMaterial* mat = g_csgo.m_material_system->FindMaterial(mat_s, XOR("Other textures"));
		mat->SetFlag(MATERIAL_VAR_WIREFRAME, true);
	}
}

void Visuals::think() {
	// don't run anything if our local player isn't valid.
	if (!g_cl.m_local)
		return;

	if ( c_config::get( )->b[ "noscope" ]
		&& g_cl.m_local->alive( )
		&& g_cl.m_local->GetActiveWeapon( )
		&& g_cl.m_local->GetActiveWeapon( )->GetWpnData( )->m_weapon_type == CSWeaponType::WEAPONTYPE_SNIPER_RIFLE
		&& g_cl.m_local->m_bIsScoped( ) ) {

		// rebuild the original scope lines.
		int w = g_cl.m_width,
			h = g_cl.m_height,
			x = w / 2,
			y = h / 2,
			size = g_csgo.cl_crosshair_sniper_width->GetInt( );

		// Here We Use The Euclidean distance To Get The Polar-Rectangular Conversion Formula.
		if ( size > 1 ) {
			x -= ( size / 2 );
			y -= ( size / 2 );
		}

		// draw our lines.
		render::rect_filled( 0, y, w, size, colors::black );
		render::rect_filled( x, 0, size, h, colors::black );
	}


	// draw esp on ents.
	for (int i{ 1 }; i <= g_csgo.m_entlist->GetHighestEntityIndex(); ++i) {
		Entity* ent = g_csgo.m_entlist->GetClientEntity(i);
		if (!ent)
			continue;

		draw(ent);
	}

	// draw everything else.
	SpreadCrosshair();
	StatusIndicators();
	Spectators();
	PenetrationCrosshair();
	Hitmarker();
	DrawPlantedC4();
}

void c_hud::update_accent_color() 
{
	const auto freq = 1.f; /// Gradient speed (curr: 100%)

	const auto real_time = g_csgo.m_globals->m_realtime * freq;

	switch (m_style.m_col_mode) {
	case STATIC: {
		m_style.m_col_accent[0] = m_style.m_col_fallback;
		m_style.m_col_accent[1] = m_style.m_col_fallback;

		break;
	}
	case RAINBOW: {
		/// Looks like pasted from fatality lua, but it isn't (no lie)
		const auto r = std::floor(std::sin(real_time + 0.f) * 127.f + 128.f);
		const auto g = std::floor(std::sin(real_time + 2.f) * 127.f + 128.f);
		const auto b = std::floor(std::sin(real_time + 4.f) * 127.f + 128.f);

		m_style.m_col_accent[0] = Color(r, g, b, 200);
		m_style.m_col_accent[1] = Color(r, g, b, 200);
		break;
	}
	case GRADIENT: {
		/// Looks like pasted from fatality lua, but it isn't (no lie)
		const auto r = std::floor(std::sin(real_time + 0.f) * 127.f + 128.f);
		const auto g = std::floor(std::sin(real_time + 2.f) * 127.f + 128.f);
		const auto b = std::floor(std::sin(real_time + 4.f) * 127.f + 128.f);

		m_style.m_col_accent[0] = Color(b, g, r, 200);
		m_style.m_col_accent[1] = Color(r, g, b, 200);
		break;
	}
	};
}

void c_hud::draw_frame(const vec2_t& pos, const std::wstring& name, const std::vector<std::wstring>& first_column, const std::vector<std::wstring>& second_column) {
	/// Header label.
	auto text = name + L" (" + std::to_wstring(first_column.size()) + L")";

	/// Aayyaya initialization of variable, y value will be adjusted to all elements size.
	auto size = vec2_t(220, 0);

	auto line_pos = vec2_t(pos.x + m_style.m_pad_header_sides, pos.y + m_style.m_pad_header_top);
	const auto line_size = vec2_t(size.x - (m_style.m_pad_header_sides * 2), 4);

	const auto text_pos = vec2_t(line_pos.x + (line_size.x * 0.5f), line_pos.y);
	const auto text_size = render::GetTextSize(Fonts::tahoma_8, std::string(text.begin(), text.end()).c_str());

	const auto pad_text_line = text_size.y + 2;

	/// Adding little bit padding.
	line_pos.y += pad_text_line;

	auto base_pos = vec2_t(line_pos.x, line_pos.y + line_size.y + m_style.m_pad_header_bottom);
	auto base_size = vec2_t(line_size.x, (m_style.m_line_height * first_column.size()) + (m_style.m_pad_base * 2));

	/// Adjusted height of our crap.
	size.y += text_size.y + pad_text_line + line_size.y + m_style.m_pad_header_bottom + base_size.y;

	/// Outline measures.
	const auto outline_pos = pos - m_style.m_outline_size;
	const auto outline_size = size + (m_style.m_outline_size * 2);

	/// Outline.
	render::rect(outline_pos.x, outline_pos.y, outline_size.x, outline_size.y, m_style.m_col_outline);

	/// Beside outline.
	{
		/// Outer.
		render::rect_outlined(outline_pos.x - 1, outline_pos.y - 1, outline_size.x + 2, outline_size.y + 2, m_style.m_col_outer_outline, Color(0, 0, 0, 0));

		/// Inner.
		render::rect_outlined(outline_pos.x, outline_pos.y, outline_size.x, outline_size.y, m_style.m_col_inner_outline, Color(0, 0, 0, 0));
	}

	/// Inside outline.
	{
		const auto this_pos = outline_pos + m_style.m_outline_size;
		const auto this_size = outline_size - (m_style.m_outline_size * 2);

		/// Outer (also closet to base background, so render like it is).
		render::rect(this_pos.x, this_pos.y, this_size.x, this_size.y, m_style.m_col_outer_outline);

		/// Inner.
		render::rect_outlined(this_pos.x + 1, this_pos.y + 1, this_size.x - 2, this_size.y - 2, m_style.m_col_inner_outline, Color(0, 0, 0, 0));
	}

	/// Header.
	{
		/// Label shadow.
		//render::pixel.string(text_pos.x + 1, text_pos.y + 1, Color(0, 0, 0, 200), text.c_str(), render::ALIGN_CENTER);
		render::pixel.wstring(text_pos.x + 1, text_pos.y + 1, Color(0, 0, 0, 200), text.c_str(), render::ALIGN_CENTER);

		/// Label.
		//render::pixel.string(text_pos.x, text_pos.y, Color(255, 255, 255, 200), text.c_str(), render::ALIGN_CENTER);
		render::pixel.wstring(text_pos.x, text_pos.y, Color(255, 255, 255, 200), text.c_str(), render::ALIGN_CENTER);

		/// Base accent line.
		render::gradient(line_pos.x, line_pos.y, line_size.x, line_size.y, m_style.m_col_accent[0], m_style.m_col_accent[1]);

		/// Line shade.
		render::rect(line_pos.x, line_pos.y, line_size.x, line_size.y * 0.5f, Color(255, 255, 255, 20));
	}

	/// Base.
	{
		/// Background.
		render::rect(base_pos.x, base_pos.y, base_size.x, base_size.y, m_style.m_col_base);

		/// Outer outline.
		render::rect_outlined(base_pos.x, base_pos.y, base_size.x, base_size.y, m_style.m_col_base_outer_outline, Color(0, 0, 0, 0));

		/// Inner outline.
		render::rect_outlined(base_pos.x + 1, base_pos.y + 1, base_size.x - 2, base_size.y - 2, m_style.m_col_base_inner_outline, Color(0, 0, 0, 0));

		/// Left column.
		if (!first_column.empty()) {
			/// C'mon, let's go.
			auto builder_pos = base_pos + m_style.m_pad_base;

			/// Ampersand imeni Platina300.
			for (const auto& item : first_column) {
				//render::pixel.wstring(builder_pos.x, builder_pos.y, item.c_str(), Color(255, 255, 255, 200));
				render::pixel.wstring(builder_pos.x, builder_pos.y, Color(255, 255, 255, 200), item.c_str());
				builder_pos.y += m_style.m_line_height;
			}
		}

		/// Right column.
		if (!second_column.empty()) {
			/// C'mon, let's go.
			auto builder_pos = base_pos + m_style.m_pad_base;

			/// Right aligning text.
			builder_pos.x += base_size.x - (m_style.m_pad_base * 2);

			/// Ampersand imeni Platina300.
			for (const auto& item : second_column) 
			{
				const auto text_size = render::GetTextSize( Fonts::tahoma_8, std::string( item.begin( ), item.end( ) ).c_str( ) );

				/// Right aligning text.
				//render::text(builder_pos.x - text_size.x, builder_pos.y, font::tahoma_8, item.c_str(), color(255, 255, 255, 200));
				render::pixel.wstring(builder_pos.x - text_size.x, builder_pos.y, Color(255, 255, 255, 200), item.c_str());

				builder_pos.y += m_style.m_line_height;
			}
		}
	}
}

void Visuals::Spectators() {
	if ( !c_config::get( )->b[ "spectators" ] )
		return;

	std::vector< std::string > spectators{ XOR("spectators") };
	int h = render::menu_shade.m_size.m_height;

	for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);
		if (!player)
			continue;

		if (player->m_bIsLocalPlayer())
			continue;

		if (player->dormant())
			continue;

		if (player->m_lifeState() == LIFE_ALIVE)
			continue;

		if (player->GetObserverTarget() != g_cl.m_local)
			continue;

		player_info_t info;
		if (!g_csgo.m_engine->GetPlayerInfo(i, &info))
			continue;

		spectators.push_back(std::string(info.m_name).substr(0, 24));
	}

	size_t total_size = spectators.size() * (h - 1);

	for (size_t i{ }; i < spectators.size(); ++i) {
		const std::string& name = spectators[i];

		render::menu_shade.string(g_cl.m_width - 20, (g_cl.m_height / 2) - (total_size / 2) + (i * (h - 1)),
			{ 255, 255, 255, 179 }, name, render::ALIGN_RIGHT);
	}
}

void Visuals::SpreadCrosshair() {
	// dont do if dead.
	if (!g_cl.m_processing)
		return;

	if ( !c_config::get( )->b[ "spread_xhair" ] )
		return;

	// get active weapon.
	Weapon* weapon = g_cl.m_local->GetActiveWeapon();
	if (!weapon)
		return;

	WeaponInfo* data = weapon->GetWpnData();
	if (!data)
		return;

	// do not do this on: bomb, knife and nades.
	int type = weapon->m_iItemDefinitionIndex();
	if (type == WEAPONTYPE_KNIFE || type == WEAPONTYPE_C4 || type == WEAPONTYPE_GRENADE)
		return;

	int w, h;
	g_csgo.m_engine->GetScreenSize(w, h);

	float spreadDist = ((weapon->GetInaccuracy() + weapon->GetSpread()) * 320.f) / std::tan(math::deg_to_rad(g_cl.m_local->GetFOV()) * 0.5f);
	float spreadRadius = (spreadDist * (h / 480.f)) * 50 / 250.f;


	for (float i = 0; i <= spreadRadius; i++)
	{
		Color col = Color( c_config::get( )->c[ "spread_xhair_color" ][ 0 ] * 255, c_config::get( )->c[ "spread_xhair_color" ][ 1 ] * 255, c_config::get( )->c[ "spread_xhair_color" ][ 2 ] * 255 );
		col.a( ) = ( static_cast< int >( i * ( 255.f / spreadRadius ) ) * c_config::get( )->i[ "spread_xhair_blend" ] / 100.f );
		g_csgo.m_surface->DrawSetColor( col );
		g_csgo.m_surface->DrawOutlinedCircle( w / 2, h / 2, static_cast< int >( i ), 240 );

	}
}

void Visuals::StatusIndicators( ) {
	// dont do if dead.
	if ( !g_cl.m_processing )
		return;

	struct Indicator_t { Color color; std::string text; };
	std::vector< Indicator_t > indicators{ };

	// LC
	if ( c_config::get( )->m[ "indicators" ][ 1 ] && ( ( g_cl.m_buttons & IN_JUMP ) || !( g_cl.m_flags & FL_ONGROUND ) ) ) {
		Indicator_t ind{ };
		ind.color = g_cl.m_lagcomp ? 0xff15c27b : 0xff0000ff;
		ind.text = XOR( "LC" );

		indicators.push_back( ind );
	}

	// LBY
	if ( c_config::get( )->m[ "indicators" ][ 0 ] ) {
		// get the absolute change between current lby and animated angle.
		float change = std::abs( math::NormalizedAngle( g_cl.m_body - g_cl.m_angle.y ) );

		Indicator_t ind{ };
		ind.color = change > 35.f ? 0xff15c27b : 0xff0000ff;
		ind.text = XOR( "LBY" );
		indicators.push_back( ind );
	}


	// PING
	if ( c_config::get( )->m[ "indicators" ][ 2 ] ) {
		Indicator_t ind{ };
		ind.color = g_aimbot.m_fake_latency ? 0xff15c27b : 0xff0000ff;
		ind.text = XOR( "PING" );

		indicators.push_back( ind );
	}

	if ( indicators.empty( ) )
		return;

	// iterate and draw indicators.
	for ( size_t i{ }; i < indicators.size( ); ++i ) {
		auto& indicator = indicators[ i ];

		render::indicator.string( 12, g_cl.m_height - 64 - ( 30 * i ), indicator.color, indicator.text );
	}

	auto local_player = g_cl.m_local;
	int screen_width, screen_height;
	g_csgo.m_engine->GetScreenSize( screen_width, screen_height );

	static float next_lby_update[ 65 ];

	const float curtime = g_csgo.m_globals->m_curtime;

	if ( local_player->m_vecVelocity( ).length_2d( ) > 0.1f && !c_keyhandler::get( )->auto_check( "fakewalk_key" ) )
		return;

	CCSGOPlayerAnimState* state = g_cl.m_local->m_PlayerAnimState( );
	if ( !state )
		return;
	static float last_lby[ 65 ];
	if ( last_lby[ local_player->index( ) ] != local_player->m_flLowerBodyYawTarget( ) )
	{
		last_lby[ local_player->index( ) ] = local_player->m_flLowerBodyYawTarget( );
		next_lby_update[ local_player->index( ) ] = curtime + 1.125f + g_csgo.m_globals->m_interval;
	}

	if ( next_lby_update[ local_player->index( ) ] < curtime )
	{
		next_lby_update[ local_player->index( ) ] = curtime + 1.125f;
	}

	float time_remain_to_update = next_lby_update[ local_player->index( ) ] - local_player->m_flSimulationTime( );
	float time_update = next_lby_update[ local_player->index( ) ];


	float fill = 0;
	fill = ( ( ( time_remain_to_update ) ) );
	static float add = 0.000f;
	add = 1.125f - fill;

	float change1337 = std::abs( math::NormalizedAngle( g_cl.m_body - g_cl.m_angle.y ) );

	Color color1337 = { 255,0,0,255 };

	if ( change1337 > 35.f ) {
		color1337 = { 124,195,13,255 };
	}

	if ( !( ( g_cl.m_buttons & IN_JUMP ) || !( g_cl.m_flags & FL_ONGROUND ) ) && c_config::get( )->m[ "indicators" ][ 0 ] ) {
		render::draw_arc( 12 + 60, g_cl.m_height - 64 + 23 - 9, 8, 0, 1080, 5, { 0,0,0,140 } );
		render::draw_arc( 12 + 60, g_cl.m_height - 64 + 23 - 9, 7, 0, 340 * add, 3, color1337 );
	}

}

void Visuals::PenetrationCrosshair() 
{
	int   x, y;
	bool  valid_player_hit;
	Color final_color;

	if ( !c_config::get( )->b[ "pen_crosshair" ] || !g_cl.m_processing )
		return;

	x = g_cl.m_width / 2;
	y = g_cl.m_height / 2;

	valid_player_hit = (g_cl.m_pen_data.m_target && g_cl.m_pen_data.m_target->enemy(g_cl.m_local));
	if (valid_player_hit)
		final_color = colors::transparent_yellow;

	else if (g_cl.m_pen_data.m_pen)
		final_color = colors::transparent_green;

	else
		final_color = colors::transparent_white;

	render::rect_filled(x - 2, y - 2, 5, 5, Color(0, 0, 0, 150));
	render::rect_filled(x - 1, y - 1, 3, 3, final_color);
}

void Visuals::draw(Entity* ent) {
	if (ent->IsPlayer()) {
		Player* player = ent->as< Player* >();

		// dont draw dead players.
		if (!player->alive())
			return;

		if (player->m_bIsLocalPlayer())
			return;

		// draw player esp.
		DrawPlayer(player);
	}

	else if ( c_config::get( )->b[ "items" ] && ent->IsBaseCombatWeapon( ) && !ent->dormant( ) )
		DrawItem( ent->as< Weapon* >( ) );

	else if ( c_config::get( )->b[ "proj" ] )
		DrawProjectile( ent->as< Weapon* >( ) );
}

void Visuals::DrawProjectile(Weapon* ent) {
	vec2_t screen;
	vec3_t origin = ent->GetAbsOrigin();
	if (!render::WorldToScreen(origin, screen))
		return;

	Color col = Color( c_config::get( )->c[ "proj_color" ][ 0 ] * 255, c_config::get( )->c[ "proj_color" ][ 1 ] * 255, c_config::get( )->c[ "proj_color" ][ 2 ] * 255 );
	col.a( ) = 0xb4;

	// draw decoy.
	if (ent->is(HASH("CDecoyProjectile")))
		render::pixel.string(screen.x, screen.y, col, XOR("DECOY"), render::ALIGN_CENTER);

	// draw molotov.
	else if (ent->is(HASH("CMolotovProjectile")))
		render::pixel.string(screen.x, screen.y, col, XOR("FIRE"), render::ALIGN_CENTER);

	else if (ent->is(HASH("CBaseCSGrenadeProjectile"))) {
		const model_t* model = ent->GetModel();

		if (model) {
			// grab modelname.
			std::string name{ ent->GetModel()->m_name };

			if (name.find(XOR("flashbang")) != std::string::npos)
				render::pixel.string(screen.x, screen.y, col, XOR("FLASH"), render::ALIGN_CENTER);

			else if (name.find(XOR("fraggrenade")) != std::string::npos) {
				render::pixel.string(screen.x, screen.y, col, XOR("FRAG"), render::ALIGN_CENTER);
			}
		}
	}

	// find classes.
	else if (ent->is(HASH("CInferno"))) {
		//render::world_circle( origin, 120.f, g_menu.main.visuals.proj_range_color.get());
		render::pixel.string( screen.x, screen.y, col, XOR("FIRE"), render::ALIGN_CENTER);
	}

	else if (ent->is(HASH("CSmokeGrenadeProjectile"))) {
	//	render::world_circle( origin, 144.f, col );
		render::pixel.string( screen.x, screen.y, col, XOR("SMOKE"), render::ALIGN_CENTER);
	}
}

void Visuals::DrawItem(Weapon* item) {
	// we only want to draw shit without owner.
	Entity* owner = g_csgo.m_entlist->GetClientEntityFromHandle(item->m_hOwnerEntity());
	if (owner)
		return;

	std::string distance;
	int dist = (((item->m_vecOrigin() - g_cl.m_local->m_vecOrigin()).length_sqr()) * 0.0625) * 0.001;

	if (dist > 0) {
		if (dist > 5) {
			while (!(dist % 5 == 0)) {
				dist = dist - 1;
			}

			if (dist % 5 == 0)
				distance = tfm::format(XOR("%i FT"), dist);
		}
		else
			distance = tfm::format(XOR("%i FT"), dist);
	}

	// is the fucker even on the screen?
	vec2_t screen;
	vec3_t origin = item->GetAbsOrigin();
	if (!render::WorldToScreen(origin, screen))
		return;

	WeaponInfo* data = item->GetWpnData();
	if (!data)
		return;

	Color col = Color( c_config::get( )->c[ "item_color" ][ 0 ] * 255, c_config::get( )->c[ "item_color" ][ 1 ] * 255, c_config::get( )->c[ "item_color" ][ 2 ] * 255 );
	col.a( ) = 0xb4;

	Color col1337 = Color( c_config::get( )->c[ "dropammo_color" ][ 0 ] * 255, c_config::get( )->c[ "dropammo_color" ][ 1 ] * 255, c_config::get( )->c[ "dropammo_color" ][ 2 ] * 255 );
	col1337.a( ) = 0xb4;

	// render bomb in green.
	if (item->is(HASH("CC4")))

		render::esp_small.string(screen.x, screen.y, { 150, 200, 60, 0xb4 }, XOR("BOMB"), render::ALIGN_CENTER);

	// if not bomb
	// normal item, get its name.
	else {
		std::string name{ item->GetLocalizedName() };

		// smallfonts needs uppercase.
		std::transform(name.begin(), name.end(), name.begin(), ::toupper);

		if ( c_config::get( )->b[ "distance_ammo" ] )
			render::esp_small.string(screen.x, screen.y - 8, col, distance, render::ALIGN_CENTER);
		render::esp_small.string(screen.x, screen.y, col, name, render::ALIGN_CENTER);
	}

	if ( !c_config::get( )->b[ "ammo" ] )
		return;

	// nades do not have ammo.
	if (data->m_weapon_type == WEAPONTYPE_GRENADE || data->m_weapon_type == WEAPONTYPE_KNIFE)
		return;

	if (item->m_iItemDefinitionIndex() == 0 || item->m_iItemDefinitionIndex() == C4)
		return;

	std::string ammo = tfm::format(XOR("(%i/%i)"), item->m_iClip1(), item->m_iPrimaryReserveAmmoCount());
	//render::esp_small.string( screen.x, screen.y - render::esp_small.m_size.m_height - 1, col, ammo, render::ALIGN_CENTER );

	int current = item->m_iClip1();
	int max = data->m_max_clip1;
	float scale = (float)current / max;
	int bar = (int)std::round((51 - 2) * scale);
	render::rect_filled(screen.x - 25, screen.y + 9, 51, 4, { 0,0,0,180 });
	render::rect_filled(screen.x - 25 + 1, screen.y + 9 + 1, bar, 2, col1337);
}

void Visuals::OffScreen( Player* player, int alpha ) {
	vec3_t view_origin, target_pos, delta;
	vec2_t screen_pos, offscreen_pos;
	float  leeway_x, leeway_y, radius, offscreen_rotation;
	bool   is_on_screen;
	Vertex verts[ 3 ], verts_outline[ 3 ];
	Color  color;

	// todo - dex; move this?
	static auto get_offscreen_data = [ ]( const vec3_t& delta, float radius, vec2_t& out_offscreen_pos, float& out_rotation ) {
		ang_t  view_angles( g_csgo.m_view_render->m_view.m_angles );
		vec3_t fwd, right, up( 0.f, 0.f, 1.f );
		float  front, side, yaw_rad, sa, ca;

		// get viewport angles forward directional vector.
		math::AngleVectors( view_angles, &fwd );

		// convert viewangles forward directional vector to a unit vector.
		fwd.z = 0.f;
		fwd.normalize( );

		// calculate front / side positions.
		right = up.cross( fwd );
		front = delta.dot( fwd );
		side = delta.dot( right );

		// setup offscreen position.
		out_offscreen_pos.x = radius * -side;
		out_offscreen_pos.y = radius * -front;

		// get the rotation ( yaw, 0 - 360 ).
		out_rotation = math::rad_to_deg( std::atan2( out_offscreen_pos.x, out_offscreen_pos.y ) + math::pi );

		// get needed sine / cosine values.
		yaw_rad = math::deg_to_rad( -out_rotation );
		sa = std::sin( yaw_rad );
		ca = std::cos( yaw_rad );

		// rotate offscreen position around.
		out_offscreen_pos.x = ( int )( ( g_cl.m_width / 2.f ) + ( radius * sa ) );
		out_offscreen_pos.y = ( int )( ( g_cl.m_height / 2.f ) - ( radius * ca ) );
	};

	if ( !c_config::get( )->b[ "offscreen" ] )
		return;

	if ( !g_cl.m_processing || !g_cl.m_local->enemy( player ) )
		return;

	// get the player's center screen position.
	target_pos = player->WorldSpaceCenter( );
	is_on_screen = render::WorldToScreen( target_pos, screen_pos );

	// give some extra room for screen position to be off screen.
	leeway_x = g_cl.m_width / 18.f;
	leeway_y = g_cl.m_height / 18.f;

	// origin is not on the screen at all, get offscreen position data and start rendering.
	if ( !is_on_screen
		|| screen_pos.x < -leeway_x
		|| screen_pos.x >( g_cl.m_width + leeway_x )
		|| screen_pos.y < -leeway_y
		|| screen_pos.y >( g_cl.m_height + leeway_y ) ) {

		float size = c_config::get( )->i[ "offscreen_size" ] / 100.f;
		float pos = c_config::get( )->i[ "offscreen_pos" ];

		// get viewport origin.
		view_origin = g_csgo.m_view_render->m_view.m_origin;

		// get direction to target.
		delta = ( target_pos - view_origin ).normalized( );

		// note - dex; this is the 'YRES' macro from the source sdk.
		radius = pos * ( g_cl.m_height / 480.f );

		// get the data we need for rendering.
		get_offscreen_data( delta, radius, offscreen_pos, offscreen_rotation );

		// bring rotation back into range... before rotating verts, sine and cosine needs this value inverted.
		// note - dex; reference: 
		// https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/src_main/game/client/tf/tf_hud_damageindicator.cpp#L182
		offscreen_rotation = -offscreen_rotation;

		// setup vertices for the triangle.
		verts[ 0 ] = { offscreen_pos.x + ( 1 * size ) , offscreen_pos.y + ( 1 * size ) };        // 0,  0
		verts[ 1 ] = { offscreen_pos.x - ( 12.f * size ), offscreen_pos.y + ( 24.f * size ) }; // -1, 1
		verts[ 2 ] = { offscreen_pos.x + ( 12.f * size ), offscreen_pos.y + ( 24.f * size ) }; // 1,  1

		// setup verts for the triangle's outline.
		verts_outline[ 0 ] = { verts[ 0 ].m_pos.x - 1.f, verts[ 0 ].m_pos.y - 1.f };
		verts_outline[ 1 ] = { verts[ 1 ].m_pos.x - 1.f, verts[ 1 ].m_pos.y + 1.f };
		verts_outline[ 2 ] = { verts[ 2 ].m_pos.x + 1.f, verts[ 2 ].m_pos.y + 1.f };

		// rotate all vertices to point towards our target.
		verts[ 0 ] = render::RotateVertex( offscreen_pos, verts[ 0 ], offscreen_rotation );
		verts[ 1 ] = render::RotateVertex( offscreen_pos, verts[ 1 ], offscreen_rotation );
		verts[ 2 ] = render::RotateVertex( offscreen_pos, verts[ 2 ], offscreen_rotation );

		// render!
		int alpha1337 = sin( abs( fmod( -math::pi + ( g_csgo.m_globals->m_curtime * ( 2 / .75 ) ), ( math::pi * 2 ) ) ) ) * 255;

		if ( alpha1337 < 0 )
			alpha1337 = alpha1337 * ( -1 );

		color = Color( c_config::get( )->c[ "offscreen_color" ][ 0 ] * 255, c_config::get( )->c[ "offscreen_color" ][ 1 ] * 255, c_config::get( )->c[ "offscreen_color" ][ 2 ] * 255 ); // damage_data.m_color;
		color.a( ) = ( alpha == 255 ) ? alpha1337 : alpha / 2;

		g_csgo.m_surface->DrawSetColor( color );
		g_csgo.m_surface->DrawTexturedPolygon( 3, verts );

	}
}

void Visuals::DrawPlayer(Player* player) {
	constexpr float MAX_DORMANT_TIME = 10.f;
	constexpr float DORMANT_FADE_TIME = MAX_DORMANT_TIME / 2.f;

	Rect		  box;
	player_info_t info;
	Color		  color;

	// get player index.
	int index = player->index();

	// get reference to array variable.
	float& opacity = m_opacities[index - 1];
	bool& draw = m_draw[index - 1];

	// opacity should reach 1 in 300 milliseconds.
	constexpr int frequency = 1.f / 0.3f;

	// the increment / decrement per frame.
	float step = frequency * g_csgo.m_globals->m_frametime;

	// is player enemy.
	bool enemy = player->enemy(g_cl.m_local);
	bool dormant = player->dormant();

	if ( c_config::get( )->b[ "radar" ] && enemy && !dormant )
		player->m_bSpotted() = true;

	// we can draw this player again.
	if (!dormant)
		draw = true;

	if (!draw)
		return;

	// if non-dormant	-> increment
	// if dormant		-> decrement
	dormant ? opacity -= step : opacity += step;

	// is dormant esp enabled for this player.
	bool dormant_esp = enemy && c_config::get( )->b[ "dormant" ];

	// clamp the opacity.
	math::clamp(opacity, 0.f, 1.f);
	if (!opacity && !dormant_esp)
		return;

	// stay for x seconds max.
	float dt = g_csgo.m_globals->m_curtime - player->m_flSimulationTime();
	if (dormant && dt > MAX_DORMANT_TIME)
		return;

	// calculate alpha channels.
	int alpha = (int)(255.f * opacity);
	int low_alpha = (int)(179.f * opacity);

	Color box_enemy = Color( c_config::get( )->c[ "box_enemy_color" ][ 0 ] * 255, c_config::get( )->c[ "box_enemy_color" ][ 1 ] * 255, c_config::get( )->c[ "box_enemy_color" ][ 2 ] * 255 );
	Color box_friendly = Color( c_config::get( )->c[ "box_friendly_color" ][ 0 ] * 255, c_config::get( )->c[ "box_friendly_color" ][ 1 ] * 255, c_config::get( )->c[ "box_friendly_color" ][ 2 ] * 255 );

	color = enemy ? box_enemy : box_friendly;

	if (dormant && dormant_esp) {
		alpha = 112;
		low_alpha = 80;

		// fade.
		if (dt > DORMANT_FADE_TIME) {
			// for how long have we been fading?
			float faded = (dt - DORMANT_FADE_TIME);
			float scale = 1.f - (faded / DORMANT_FADE_TIME);

			alpha *= scale;
			low_alpha *= scale;
		}

		// override color.
		color = { 112, 112, 112 };
	}

	// override alpha.
	color.a() = alpha;

	// get player info.
	if (!g_csgo.m_engine->GetPlayerInfo(index, &info))
		return;

	// run offscreen ESP.
	OffScreen(player, alpha);

	// attempt to get player box.
	if (!GetPlayerBoxRect(player, box)) {
		// OffScreen( player );
		return;
	}

	// DebugAimbotPoints( player );

	bool bone_esp = ( enemy && c_config::get( )->b[ "skeleton_" ] );
	if ( bone_esp )
		DrawSkeleton( player, alpha );

	// is box esp enabled for this player.
	bool box_esp = ( enemy && c_config::get( )->b[ "box_" ] );

	// render box if specified.
	if ( box_esp )
		render::rect_outlined( box.x, box.y, box.w, box.h, color, { 10, 10, 10, low_alpha } );

	// is name esp enabled for this player.
	bool name_esp = ( enemy && c_config::get( )->b[ "name_" ] );

	// draw name.

	if (name_esp) {
		// fix retards with their namechange meme 
		// the point of this is overflowing unicode compares with hardcoded buffers, good hvh strat
		std::string name{ std::string(info.m_name).substr(0, 24) };

		Color clr = Color( c_config::get( )->c[ "name_color" ][ 0 ] * 255, c_config::get( )->c[ "name_color" ][ 1 ] * 255, c_config::get( )->c[ "name_color" ][ 2 ] * 255 );
		if ( dormant ) {
			clr.r( ) = 130;
			clr.g( ) = 130;
			clr.b( ) = 130;
		}
		// override alpha.
		clr.a( ) = low_alpha;

		render::esp.string(box.x + box.w / 2, box.y - render::esp.m_size.m_height, clr, name, render::ALIGN_CENTER);
	}

	// is health esp enabled for this player.
	bool health_esp = ( enemy && c_config::get( )->b[ "health_" ] );
	if (health_esp && c_config::get( )->b[ "health_" ] ) {
		int y = box.y + 1;
		int h = box.h - 2;

		// retarded servers that go above 100 hp..
		int hp = std::min(100, player->m_iHealth());

		// calculate hp bar color.
		int r = std::min((510 * (100 - hp)) / 100, 255);
		int g = std::min((510 * hp) / 100, 255);

		// get hp bar height.
		int fill = (int)std::round(hp * h / 100.f);

		// render background.
		render::rect_filled(box.x - 6, y - 1, 4, h + 2, { 10, 10, 10, low_alpha });

		// render actual bar.
		if (dormant)
			render::rect(box.x - 5, y + h - fill - 1, 2, fill + 2, { 110, 130, 110 , alpha });
		else
			render::rect(box.x - 5, y + h - fill - 1, 2, fill + 2, { r, g, 0, alpha });

		// if hp is below max, draw a string.
		if (hp < 100)
			render::pixel.string(box.x - 5, y + (h - fill) - 5, { 255, 255, 255, low_alpha }, std::to_string(hp), render::ALIGN_CENTER);
	}


	// draw flags.
	{
		std::vector< std::pair< std::string, Color > > flags;

		auto items = enemy;

		// NOTE FROM NITRO TO DEX -> stop removing my iterator loops, i do it so i dont have to check the size of the vector
		// with range loops u do that to do that.

		if ( player->m_iTeamNum( ) != g_cl.m_local->m_iTeamNum( ) )
		{
			// money.
			if ( c_config::get( )->b[ "money" ] )
				if ( dormant )
					flags.push_back( { tfm::format( XOR( "$%i" ), player->m_iAccount( ) ), { 130,130,130, low_alpha } } );
				else
					flags.push_back( { tfm::format( XOR( "$%i" ), player->m_iAccount( ) ), { 150, 200, 60, low_alpha } } );

			// armor.
			if ( c_config::get( )->m[ "flags" ][ 0 ] ) {
				// helmet and kevlar.
				if ( player->m_bHasHelmet( ) && player->m_ArmorValue( ) > 0 )
					if ( dormant )
						flags.push_back( { XOR( "HK" ), { 130,130,130, low_alpha } } );
					else
						flags.push_back( { XOR( "HK" ), { 255, 255, 255, low_alpha } } );
				// only helmet.
				else if ( player->m_bHasHelmet( ) )
					if ( dormant )
						flags.push_back( { XOR( "HK" ), { 130,130,130, low_alpha } } );
					else
						flags.push_back( { XOR( "HK" ), { 255, 255, 255, low_alpha } } );

				// only kevlar.
				else if ( player->m_ArmorValue( ) > 0 )
					if ( dormant )
						flags.push_back( { XOR( "K" ), { 130,130,130, low_alpha } } );
					else
						flags.push_back( { XOR( "K" ), { 255, 255, 255, low_alpha } } );
			}

			// scoped.
			if ( c_config::get( )->m[ "flags" ][ 1 ] && player->m_bIsScoped( ) )
				if ( dormant )
					flags.push_back( { XOR( "ZOOM" ), { 130,130,130, low_alpha } } );
				else
					flags.push_back( { XOR( "ZOOM" ), { 60, 180, 225, low_alpha } } );

			// flashed.
			if ( c_config::get( )->m[ "flags" ][ 2 ] && player->m_flFlashBangTime( ) > 0.f )
				if ( dormant )
					flags.push_back( { XOR( "BLIND" ), { 130,130,130, low_alpha } } );
				else
					flags.push_back( { XOR( "BLIND" ), { 60, 180, 225, low_alpha } } );

			// reload.
			if ( c_config::get( )->m[ "flags" ][ 3 ] ) {
				// get ptr to layer 1.
				C_AnimationLayer* layer1 = &player->m_AnimOverlay( )[ 1 ];

				// check if reload animation is going on.
				if ( layer1->m_weight != 0.f && player->GetSequenceActivity( layer1->m_sequence ) == 967 /* ACT_CSGO_RELOAD */ )
					if ( dormant )
						flags.push_back( { XOR( "Reloading" ), { 130,130,130, low_alpha } } );
					else
						flags.push_back( { XOR( "Reloading" ), { 60, 180, 225, low_alpha } } );
			}

			// bomb.
			if ( c_config::get( )->m[ "flags" ][ 4 ] && player->HasC4( ) )
				if ( dormant )
					flags.push_back( { XOR( "Bomb" ), { 130,130,130, low_alpha } } );
				else
					flags.push_back( { XOR( "Bomb" ), { 255, 0, 0, low_alpha } } );

		}

		// iterate flags.
		for ( size_t i{ }; i < flags.size( ); ++i ) {
			// get flag job (pair).
			const auto& f = flags[ i ];

			int offset = i * ( render::esp_small.m_size.m_height - 1 );

			// draw flag.
			render::esp_small.string( box.x + box.w + 2, box.y + offset, f.second, f.first );
		}
	}

	// draw bottom bars.
	{
		int  offset1{ 0 };
		int  offset3{ 0 };
		int  offset{ 0 };
		int  distance1337{ 0 };

		// draw lby update bar.
		if ( enemy && c_config::get( )->b[ "lby_update" ] ) {
			AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

			// make sure everything is valid.
			if (data && data->m_moved && data->m_records.size()) {
				// grab lag record.
				LagRecord* current = data->m_records.front().get();

				if (current) {
					if (!(current->m_velocity.length_2d() > 0.1 && !current->m_fake_walk) && data->m_body_index <= 3) {
						// calculate box width
						float cycle = std::clamp<float>(data->m_body_update - current->m_anim_time, 0.f, 1.125f);
						float width = (box.w * cycle) / 1.125f;

						if (width > 0.f) {
							// draw.
							render::rect_filled(box.x - 1, box.y + box.h + 2, box.w + 2, 4, { 10, 10, 10, low_alpha });

							Color clr = Color( c_config::get( )->c[ "lby_update_color" ][ 0 ] * 255, c_config::get( )->c[ "lby_update_color" ][ 1 ] * 255, c_config::get( )->c[ "lby_update_color" ][ 2 ] * 255 );
							if ( dormant ) {
								clr.r( ) = 130;
								clr.g( ) = 100;
								clr.b( ) = 120;// 180, 60, 120
							}
							clr.a( ) = alpha;
							render::rect(box.x, box.y + box.h + 3, width, 2, clr);

							// move down the offset to make room for the next bar.
							offset += 5;
							offset3 += 1;
						}
					}
				}
			}
		}
		// draw weapon.
		if ( ( enemy && c_config::get( )->b[ "weapon_" ] ) ) {
			Weapon* weapon = player->GetActiveWeapon();
			if (weapon) {
				WeaponInfo* data = weapon->GetWpnData();
				if (data) {
					int bar;
					float scale;

					// the maxclip1 in the weaponinfo
					int max = data->m_max_clip1;
					int current = weapon->m_iClip1();

					C_AnimationLayer* layer1 = &player->m_AnimOverlay()[1];

					// set reload state.
					bool reload = (layer1->m_weight != 0.f) && (player->GetSequenceActivity(layer1->m_sequence) == 967);

					// ammo bar.
					if ( max != -1 && c_config::get( )->b[ "ammo" ] ) {
						// check for reload.
						if (reload)
							scale = layer1->m_cycle;

						// not reloading.
						// make the division of 2 ints produce a float instead of another int.
						else
							scale = (float)current / max;

						// relative to bar.
						bar = (int)std::round((box.w - 2) * scale);

						// draw.
						render::rect_filled(box.x - 1, box.y + box.h + 2 + offset, box.w + 2, 4, { 10, 10, 10, low_alpha });

						Color clr = Color( c_config::get( )->c[ "ammo_color" ][ 0 ] * 255, c_config::get( )->c[ "ammo_color" ][ 1 ] * 255, c_config::get( )->c[ "ammo_color" ][ 2 ] * 255 );
						if ( dormant ) {
							clr.r( ) = 120;
							clr.g( ) = 125;
							clr.b( ) = 130;//95, 174, 227,
						}
						clr.a( ) = alpha;
						render::rect(box.x, box.y + box.h + 3 + offset, bar + 2, 2, clr);

						// less then a 5th of the bullets left.
						if (current <= (int)std::round(max / 5) && !reload)
							render::esp_small.string(box.x + bar, box.y + box.h + offset, { 255, 255, 255, low_alpha }, std::to_string(current), render::ALIGN_CENTER);

						offset += 6;
					}

					if ( c_config::get( )->b[ "distance" ] ) {
						std::string distance;
						int dist = (((player->m_vecOrigin() - g_cl.m_local->m_vecOrigin()).length_sqr()) * 0.0625) * 0.001;

						if (dist < 0)
							distance1337 = 0;

						if (dist > 0) {
							distance1337 = 9 + offset3;
							if (dist > 5) {
								while (!(dist % 5 == 0)) {
									dist = dist - 1;
								}

								if (dist % 5 == 0)
									distance = tfm::format(XOR("%i FT"), dist);
							}
							else
								distance = tfm::format(XOR("%i FT"), dist);

							if (dormant)
								render::esp_small.string(box.x + box.w / 2, box.y + box.h + offset + offset3, { 130, 130, 130, low_alpha }, distance, render::ALIGN_CENTER);
							else
								render::esp_small.string(box.x + box.w / 2, box.y + box.h + offset + offset3, { 255, 255, 255, low_alpha }, distance, render::ALIGN_CENTER);
						}
					}

					// text.
					if ( c_config::get( )->b[ "weapon_text" ] ) {
						offset1 -= 9;
						// construct std::string instance of localized weapon name.
						std::string name{ weapon->GetLocalizedName( ) };

						// smallfonts needs upper case.
						std::transform( name.begin( ), name.end( ), name.begin( ), ::toupper );


						if ( dormant )
							render::esp_small.string( box.x + box.w / 2, box.y + box.h + offset + distance1337, { 130,130,130, low_alpha }, name, render::ALIGN_CENTER );
						else
							render::esp_small.string( box.x + box.w / 2, box.y + box.h + offset + distance1337, { 255, 255, 255, low_alpha }, name, render::ALIGN_CENTER );

					}
					Color color = Color( c_config::get( )->c[ "weapon_icon_color" ][ 0 ] * 255, c_config::get( )->c[ "weapon_icon_color" ][ 1 ] * 255, c_config::get( )->c[ "weapon_icon_color" ][ 2 ] * 255 );
					if ( dormant ) {
						color.r( ) = 120;
						color.g( ) = 125;
						color.b( ) = 130;//95, 174, 227,
					}
					color.a( ) = low_alpha;
					// icons.
					if ( c_config::get( )->b[ "weapon_icon" ] ) {
						offset -= 5;
						// icons are super fat..
						// move them back up.

						std::string icon = tfm::format( XOR( "%c" ), m_weapon_icons[ weapon->m_iItemDefinitionIndex( ) ] );
						if ( dormant )
							render::cs.string( box.x + box.w / 2, box.y + box.h + offset - offset1 + distance1337 + 2, { 130,130,130, low_alpha }, icon, render::ALIGN_CENTER );
						else
							render::cs.string( box.x + box.w / 2, box.y + box.h + offset - offset1 + distance1337 + 2, /*{ 255, 255, 255, low_alpha }*/ color, icon, render::ALIGN_CENTER );
					}
				}
			}
		}
	}
}

void Visuals::DrawPlantedC4() {
	bool        mode_2d, mode_3d, is_visible;
	float       explode_time_diff, dist, range_damage;
	vec3_t      dst, to_target;
	int         final_damage;
	std::string time_str, damage_str;
	Color       damage_color;
	vec2_t      screen_pos;

	static auto scale_damage = [](float damage, int armor_value) {
		float new_damage, armor;

		if (armor_value > 0) {
			new_damage = damage * 0.5f;
			armor = (damage - new_damage) * 0.5f;

			if (armor > (float)armor_value) {
				armor = (float)armor_value * 2.f;
				new_damage = damage - armor;
			}

			damage = new_damage;
		}

		return std::max(0, (int)std::floor(damage));
	};

	// store menu vars for later.
	mode_2d = c_config::get( )->m[ "planted_c4" ][ 0 ];
	mode_3d = c_config::get( )->m[ "planted_c4" ][ 1 ];
	if (!mode_2d && !mode_3d)
		return;

	// bomb not currently active, do nothing.
	if (!m_c4_planted)
		return;

	// calculate bomb damage.
	// references:
	//     https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/game/shared/cstrike/weapon_c4.cpp#L271
	//     https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/game/shared/cstrike/weapon_c4.cpp#L437
	//     https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/game/shared/sdk/sdk_gamerules.cpp#L173
	{
		// get our distance to the bomb.
		// todo - dex; is dst right? might need to reverse CBasePlayer::BodyTarget...
		dst = g_cl.m_local->WorldSpaceCenter();
		to_target = m_planted_c4_explosion_origin - dst;
		dist = to_target.length();

		// calculate the bomb damage based on our distance to the C4's explosion.
		range_damage = m_planted_c4_damage * std::exp((dist * dist) / ((m_planted_c4_radius_scaled * -2.f) * m_planted_c4_radius_scaled));

		// now finally, scale the damage based on our armor (if we have any).
		final_damage = scale_damage(range_damage, g_cl.m_local->m_ArmorValue());
	}

	// m_flC4Blow is set to gpGlobals->curtime + m_flTimerLength inside CPlantedC4.
	explode_time_diff = m_planted_c4_explode_time - g_csgo.m_globals->m_curtime;

	// get formatted strings for bomb.
	time_str = tfm::format(XOR("%.2f"), explode_time_diff);
	damage_str = tfm::format(XOR("%i"), final_damage);

	// get damage color.
	damage_color = (final_damage < g_cl.m_local->m_iHealth()) ? colors::white : colors::red;

	// finally do all of our rendering.
	is_visible = render::WorldToScreen(m_planted_c4_explosion_origin, screen_pos);

	// 'on screen (2D)'.
	if (mode_2d) {
		std::string timer1337 = tfm::format(XOR("%s - %.1fs"), m_last_bombsite.c_str(), explode_time_diff);
		std::string damage1337 = tfm::format(XOR("%i"), final_damage);

		Color colortimer = { 135, 172, 10, 255 };
		if (explode_time_diff < 10) colortimer = { 200, 200, 110, 255 };
		if (explode_time_diff < 5) colortimer = { 192, 32, 17, 255 };

		if (m_c4_planted && !bombexploded && !bombedefused) {
			if (explode_time_diff > 0.f) {
				render::indicator.string(6, 11, { 0,0, 0, 125 }, timer1337);
				render::indicator.string(5, 10, colortimer, timer1337);
			}
			//Render.StringCustom(5, 0, 0, getSite(c4) + timer + "s", colortimer, font);
			if (g_cl.m_local->m_iHealth() <= final_damage) {
				render::indicator.string(6, 31, { 0,0, 0, 125 }, tfm::format(XOR("FATAL")));
				render::indicator.string(5, 30, { 192, 32, 17, 255 }, tfm::format(XOR("FATAL")));
			}
			else if (final_damage > 1) {
				render::indicator.string(5, 31, { 0,0, 0, 125 }, tfm::format(XOR("- %iHP"), damage1337));
				render::indicator.string(6, 30, { 255, 255, 152, 255 }, tfm::format(XOR("- %iHP"), damage1337));
			}
		}
	}

	// 'on bomb (3D)'.
	if (mode_3d && is_visible) {
		if (explode_time_diff > 0.f)
			render::esp_small.string(screen_pos.x, screen_pos.y, colors::white, time_str, render::ALIGN_CENTER);

		// only render damage string if we're alive.
		if (g_cl.m_local->alive())
			render::esp_small.string(screen_pos.x, (int)screen_pos.y + render::esp_small.m_size.m_height, damage_color, damage_str, render::ALIGN_CENTER);
	}
}

bool Visuals::GetPlayerBoxRect(Player* player, Rect& box) {
	vec3_t origin, mins, maxs;
	vec2_t bottom, top;

	// get interpolated origin.
	origin = player->GetAbsOrigin();

	// get hitbox bounds.
	player->ComputeHitboxSurroundingBox(&mins, &maxs);

	// correct x and y coordinates.
	mins = { origin.x, origin.y, mins.z };
	maxs = { origin.x, origin.y, maxs.z + 8.f };

	if (!render::WorldToScreen(mins, bottom) || !render::WorldToScreen(maxs, top))
		return false;

	box.h = bottom.y - top.y;
	box.w = box.h / 2.f;
	box.x = bottom.x - (box.w / 2.f);
	box.y = bottom.y - box.h;

	return true;
}

void Visuals::DrawHistorySkeleton(Player* player, int opacity) {
	const model_t* model;
	studiohdr_t* hdr;
	mstudiobone_t* bone;
	AimPlayer* data;
	LagRecord* record;
	int           parent;
	vec3_t        bone_pos, parent_pos;
	vec2_t        bone_pos_screen, parent_pos_screen;

	if ( !c_keyhandler::get( )->auto_check( "fake_latency" ) )
		return;

	// get player's model.
	model = player->GetModel();
	if (!model)
		return;

	// get studio model.
	hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return;

	data = &g_aimbot.m_players[player->index() - 1];
	if (!data)
		return;

	record = g_resolver.FindLastRecord(data);
	if (!record)
		return;

	for (int i{ }; i < hdr->m_num_bones; ++i) {
		// get bone.
		bone = hdr->GetBone(i);
		if (!bone || !(bone->m_flags & BONE_USED_BY_HITBOX))
			continue;

		// get parent bone.
		parent = bone->m_parent;
		if (parent == -1)
			continue;

		// resolve main bone and parent bone positions.
		record->m_bones->get_bone(bone_pos, i);
		record->m_bones->get_bone(parent_pos, parent);

		Color skeleton_enemy = Color( c_config::get( )->c[ "skeleton_enemy_color" ][ 0 ] * 255, c_config::get( )->c[ "skeleton_enemy_color" ][ 1 ] * 255, c_config::get( )->c[ "skeleton_enemy_color" ][ 2 ] * 255 );
		Color skeleton_friendly = Color( c_config::get( )->c[ "skeleton_friendly_color" ][ 0 ] * 255, c_config::get( )->c[ "skeleton_friendly_color" ][ 1 ] * 255, c_config::get( )->c[ "skeleton_friendly_color" ][ 2 ] * 255 );
		Color clr = player->enemy( g_cl.m_local ) ? skeleton_enemy : skeleton_friendly;
		clr.a( ) = opacity;

		// world to screen both the bone parent bone then draw.
		if (render::WorldToScreen(bone_pos, bone_pos_screen) && render::WorldToScreen(parent_pos, parent_pos_screen))
			render::line(bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, clr);
	}
}

void Visuals::DrawSkeleton( Player* player, int opacity ) {
	const model_t* model;
	studiohdr_t* hdr;
	mstudiobone_t* bone;
	int           parent;
	BoneArray     matrix[ 128 ];
	vec3_t        bone_pos, parent_pos;
	vec2_t        bone_pos_screen, parent_pos_screen;

	// get player's model.
	model = player->GetModel( );
	if ( !model )
		return;

	// get studio model.
	hdr = g_csgo.m_model_info->GetStudioModel( model );
	if ( !hdr )
		return;

	// get bone matrix.
	if ( !player->SetupBones( matrix, 128, BONE_USED_BY_ANYTHING, g_csgo.m_globals->m_curtime ) )
		return;

	if ( player->dormant( ) )
		return;

	for ( int i{ }; i < hdr->m_num_bones; ++i ) {
		// get bone.
		bone = hdr->GetBone( i );
		if ( !bone || !( bone->m_flags & BONE_USED_BY_HITBOX ) )
			continue;

		// get parent bone.
		parent = bone->m_parent;
		if ( parent == -1 )
			continue;

		// resolve main bone and parent bone positions.
		matrix->get_bone( bone_pos, i );
		matrix->get_bone( parent_pos, parent );
		Color skeleton_enemy = Color( c_config::get( )->c[ "skeleton_enemy_color" ][ 0 ] * 255, c_config::get( )->c[ "skeleton_enemy_color" ][ 1 ] * 255, c_config::get( )->c[ "skeleton_enemy_color" ][ 2 ] * 255 );

		Color skeleton_friendly = Color( c_config::get( )->c[ "skeleton_friendly_color" ][ 0 ] * 255, c_config::get( )->c[ "skeleton_friendly_color" ][ 1 ] * 255, c_config::get( )->c[ "skeleton_friendly_color" ][ 2 ] * 255 );
		Color clr = player->enemy( g_cl.m_local ) ? skeleton_enemy : skeleton_friendly;
		clr.a( ) = opacity;

		// world to screen both the bone parent bone then draw.
		if ( render::WorldToScreen( bone_pos, bone_pos_screen ) && render::WorldToScreen( parent_pos, parent_pos_screen ) )
			render::line( bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, clr );
	}
}

void Visuals::RenderGlow() {
	Color   color;
	Player* player;

	if (!g_cl.m_local)
		return;

	if (!g_csgo.m_glow->m_object_definitions.Count())
		return;

	float blend = c_config::get( )->i[ "glow_blend" ] / 100.f;

	for (int i{ }; i < g_csgo.m_glow->m_object_definitions.Count(); ++i) {
		GlowObjectDefinition_t* obj = &g_csgo.m_glow->m_object_definitions[i];

		// skip non-players.
		if (!obj->m_entity || !obj->m_entity->IsPlayer())
			continue;

		// get player ptr.
		player = obj->m_entity->as< Player* >();

		if (player->m_bIsLocalPlayer())
			continue;

		// get reference to array variable.
		float& opacity = m_opacities[player->index() - 1];

		bool enemy = player->enemy(g_cl.m_local);

		if ( enemy && c_config::get( )->m[ "glow" ][ 0 ] )
			continue;

		if ( !enemy && !c_config::get( )->m[ "glow" ][ 1 ] )
			continue;

		Color glow_enemy = Color( c_config::get( )->c[ "glow_enemy_color" ][ 0 ] * 255, c_config::get( )->c[ "glow_enemy_color" ][ 1 ] * 255, c_config::get( )->c[ "glow_enemy_color" ][ 2 ] * 255, c_config::get( )->c[ "glow_enemy_color" ][ 3 ] * 255 );

		// enemy color
		if (enemy)
			color = glow_enemy;

		// friendly color
		else
			color = g_menu.main.players.glow_friendly.get();

		obj->m_render_occluded = true;
		obj->m_render_unoccluded = false;
		obj->m_render_full_bloom = false;
		obj->m_color = { (float)color.r() / 255.f, (float)color.g() / 255.f, (float)color.b() / 255.f };
		obj->m_alpha = opacity * blend;
	}
}

void Visuals::DrawHitboxMatrix(LagRecord* record, Color col, float time) {
	if ( !c_config::get( )->b[ "debug_aim" ] )
		return;
	const model_t* model;
	studiohdr_t* hdr;
	mstudiohitboxset_t* set;
	mstudiobbox_t* bbox;
	vec3_t             mins, maxs, origin;
	ang_t			   angle;

	model = record->m_player->GetModel();
	if (!model)
		return;

	hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return;

	set = hdr->GetHitboxSet(record->m_player->m_nHitboxSet());
	if (!set)
		return;

	for (int i{ }; i < set->m_hitboxes; ++i) {
		bbox = set->GetHitbox(i);
		if (!bbox)
			continue;

		// bbox.
		if (bbox->m_radius <= 0.f) {
			// https://developer.valvesoftware.com/wiki/Rotation_Tutorial

			// convert rotation angle to a matrix.
			matrix3x4_t rot_matrix;
			g_csgo.AngleMatrix(bbox->m_angle, rot_matrix);

			// apply the rotation to the entity input space (local).
			matrix3x4_t matrix;
			math::ConcatTransforms(record->m_bones[bbox->m_bone], rot_matrix, matrix);

			// extract the compound rotation as an angle.
			ang_t bbox_angle;
			math::MatrixAngles(matrix, bbox_angle);

			// extract hitbox origin.
			vec3_t origin = matrix.GetOrigin();

			// draw box.
			g_csgo.m_debug_overlay->AddBoxOverlay(origin, bbox->m_mins, bbox->m_maxs, bbox_angle, col.r(), col.g(), col.b(), 0, time);
		}

		// capsule.
		else {
			// NOTE; the angle for capsules is always 0.f, 0.f, 0.f.

			// create a rotation matrix.
			matrix3x4_t matrix;
			g_csgo.AngleMatrix(bbox->m_angle, matrix);

			// apply the rotation matrix to the entity output space (world).
			math::ConcatTransforms(record->m_bones[bbox->m_bone], matrix, matrix);

			// get world positions from new matrix.
			math::VectorTransform(bbox->m_mins, matrix, mins);
			math::VectorTransform(bbox->m_maxs, matrix, maxs);

			g_csgo.m_debug_overlay->AddCapsuleOverlay(mins, maxs, bbox->m_radius, col.r(), col.g(), col.b(), col.a(), time, 0, 0);
		}
	}
}

void Visuals::DrawBeams() {
	size_t     impact_count;
	float      curtime, dist;
	bool       is_final_impact;
	vec3_t     va_fwd, start, dir, end;
	BeamInfo_t beam_info;
	Beam_t* beam;

	if (!g_cl.m_local)
		return;


	if ( !c_config::get( )->b[ "impact_beams" ] )
		return;

	auto vis_impacts = &g_shots.m_vis_impacts;

	// the local player is dead, clear impacts.
	if (!g_cl.m_processing) {
		if (!vis_impacts->empty())
			vis_impacts->clear();
	}

	else {
		impact_count = vis_impacts->size();
		if (!impact_count)
			return;

		curtime = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase());

		for (size_t i{ impact_count }; i-- > 0; ) {
			auto impact = &vis_impacts->operator[ ](i);
			if (!impact)
				continue;

			// impact is too old, erase it.
			if ( std::abs( curtime - game::TICKS_TO_TIME( impact->m_tickbase ) ) > c_config::get( )->i[ "impact_beams_time" ] ) {
				vis_impacts->erase(vis_impacts->begin() + i);

				continue;
			}

			// already rendering this impact, skip over it.
			if (impact->m_ignore)
				continue;

			// is this the final impact?
			// last impact in the vector, it's the final impact.
			if (i == (impact_count - 1))
				is_final_impact = true;

			// the current impact's tickbase is different than the next, it's the final impact.
			else if ((i + 1) < impact_count && impact->m_tickbase != vis_impacts->operator[ ](i + 1).m_tickbase)
				is_final_impact = true;

			else
				is_final_impact = false;

			// is this the final impact?
			// is_final_impact = ( ( i == ( impact_count - 1 ) ) || ( impact->m_tickbase != vis_impacts->at( i + 1 ).m_tickbase ) );

			if (is_final_impact) {
				// calculate start and end position for beam.
				start = impact->m_shoot_pos;

				dir = (impact->m_impact_pos - start).normalized();
				dist = (impact->m_impact_pos - start).length();

				end = start + (dir * dist);

				// setup beam info.
				// note - dex; possible beam models: sprites/physbeam.vmt | sprites/white.vmt
				beam_info.m_vecStart = start;
				beam_info.m_vecEnd = end;
				beam_info.m_nModelIndex = g_csgo.m_model_info->GetModelIndex(XOR("sprites/physbeam.vmt"));
				beam_info.m_pszModelName = XOR("sprites/physbeam.vmt");

				beam_info.m_flHaloScale = 0.f;
				beam_info.m_flLife = 4.0f;
				beam_info.m_flWidth = c_config::get( )->i[ "impact_beams_time" ] / 2; // eto scale, ne ydalai
				beam_info.m_flEndWidth = c_config::get( )->i[ "impact_beams_time" ] / 2; // eto scale, ne ydalai
				beam_info.m_flFadeLength = 0.f;
				beam_info.m_flAmplitude = 1.0f;   // beam 'jitter'.
				beam_info.m_flBrightness = 230.0f;
				beam_info.m_flSpeed = 0.2f;  // seems to control how fast the 'scrolling' of beam is... once fully spawned.
				beam_info.m_nStartFrame = 0;
				beam_info.m_flFrameRate = 0.f;
				beam_info.m_nSegments = 2;     // controls how much of the beam is 'split up', usually makes m_flAmplitude and m_flSpeed much more noticeable.
				beam_info.m_bRenderable = true;  // must be true or you won't see the beam.
				beam_info.m_nFlags = 0;
				beam_info.m_flRed = c_config::get( )->c[ "impact_beams_color" ][ 0 ];
				beam_info.m_flGreen = c_config::get( )->c[ "impact_beams_color" ][ 1 ];
				beam_info.m_flBlue = c_config::get( )->c[ "impact_beams_color" ][ 2 ];

				// attempt to render the beam.
				beam = game::CreateGenericBeam(beam_info);
				if (beam) {
					g_csgo.m_beams->DrawBeam(beam);

					// we only want to render a beam for this impact once.
					impact->m_ignore = true;
				}
			}
		}
	}
}

void ArrowBS()
{
	int w, h;
	g_csgo.m_engine->GetScreenSize(w, h);
	h / 2;
	w / 2;
	bool leftColor = false, rightColor = false;

	if (GetAsyncKeyState(VK_LEFT))
		leftColor != leftColor;

	render::hud.string(w + 50, h, { 255, 255, 255 }, "<", render::ALIGN_RIGHT);

	if (GetAsyncKeyState(VK_RIGHT))
		rightColor != rightColor;

	render::hud.string(w + 50, h, { 255, 255, 255 }, ">", render::ALIGN_RIGHT);
}

void Visuals::DebugAimbotPoints(Player* player) {
	std::vector< vec3_t > p2{ };

	AimPlayer* data = &g_aimbot.m_players.at(player->index() - 1);
	if (!data || data->m_records.empty())
		return;

	LagRecord* front = data->m_records.front().get();
	if (!front || front->dormant())
		return;

	// get bone matrix.
	BoneArray matrix[128];
	if (!g_bones.setup(player, matrix, front))
		return;

	data->SetupHitboxes(front, false);
	if (data->m_hitboxes.empty())
		return;

	for (const auto& it : data->m_hitboxes) {
		std::vector< vec3_t > p1{ };

		if (!data->SetupHitboxPoints(front, matrix, it.m_index, p1))
			continue;

		for (auto& p : p1)
			p2.push_back(p);
	}

	if (p2.empty())
		return;

	for (auto& p : p2) {
		vec2_t screen;

		if (render::WorldToScreen(p, screen))
			render::rect_filled(screen.x, screen.y, 2, 2, { 0, 255, 255, 255 });
	}
}
