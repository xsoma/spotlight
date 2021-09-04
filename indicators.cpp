#include "includes.h"

hud_indicators g_indicators{ };

void hud_indicators::Indicators()
{
	if ( !g_menu.main.visuals.indicators.get( 0 ) )
		return;

	Color color = g_gui.m_color;
	struct Indicator_t { std::string text; std::string mode; };
	std::vector< Indicator_t > indicators{ };

	if ( g_aimbot.m_baim_toggle )
	{
		Indicator_t ind{};
		ind.text = XOR( "Force Body-Aim" );
		ind.mode = XOR( "[toggled]" );
		indicators.push_back( ind );
	}

	if ( g_aimbot.m_fake_latency )
	{
		Indicator_t ind{};
		ind.text = XOR( "Ping Spike" );
		ind.mode = XOR( "[toggled]" );
		indicators.push_back( ind );
	}

	if ( g_aimbot.m_damage_toggle )
	{
		Indicator_t ind{};
		ind.text = XOR( "Override MinDmg" );
		ind.mode = XOR( "[toggled]" );
		indicators.push_back( ind );
	}
	
	if ( indicators.empty( ) )
		return;

	for ( size_t i{ }; i < indicators.size( ); ++i ) {
		auto& indicator = indicators[ i ];
		auto size = render::indicator.size( indicator.text );

		render::rect_filled( 5, ( g_cl.m_height / 2 + 27 ) + ( i * 15 ), 3, 11, g_gui.m_color );
		render::menu_shade.string( 12, ( g_cl.m_height / 2 + 27 ) + ( i * 15 ), colors::white, indicator.text, render::ALIGN_LEFT );
		render::menu_shade.string( 135, ( g_cl.m_height / 2 + 27 ) + ( i * 15 ), color, indicator.mode, render::ALIGN_CENTER );
	}
}

void hud_indicators::StateIndicators()
{
	if (!g_csgo.m_engine->IsInGame())
		return;

	if (!g_menu.main.misc.indicators_status.get())
		return;

	g_csgo.m_engine->GetScreenSize(m_width, m_height);

	Color color = g_gui.m_color;

	int	x{ 8 };

	// what we are btw trying to draw...
	float dsync_height = g_cl.m_local->m_flPoseParameter()[11] * 58;	// this?
	float choke_value = g_csgo.m_cl->m_choked_commands;

	// dsync

	// outline
	render::rect_filled(x + 10, m_height / 1.6, 97, 6, { 10, 10, 10, 160 });
	// line.
	render::rect_filled(x + 10, m_height / 1.6, dsync_height * 1.67, 6, { color.r(), color.g(), color.b(), 255 });	// 97/58


	// outline
	render::rect_filled(x + 10, m_height / 1.6 + 35, 97, 6, { 10, 10, 10, 160 });
	// line.
	render::rect_filled(x + 10, m_height / 1.6 + 35, choke_value * (97 / 14.f), 6, { color.r(), color.g(), color.b(), 255 });
}
