#include "includes.h"

bool Hooks::ShouldDrawParticles( ) {
	return g_hooks.m_client_mode.GetOldMethod< ShouldDrawParticles_t >( IClientMode::SHOULDDRAWPARTICLES )( this );
}

bool Hooks::ShouldDrawFog( ) {
	// remove fog.
	if ( c_config::get( )->b[ "nofog" ] )
		return false;

	return g_hooks.m_client_mode.GetOldMethod< ShouldDrawFog_t >( IClientMode::SHOULDDRAWFOG )( this );
}

void Hooks::OverrideView( CViewSetup* view ) {
	// damn son.
	g_cl.m_local = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetLocalPlayer( ) );

	// g_grenades.think( );
	g_visuals.ThirdpersonThink( );

	if (g_cl.m_local && g_cl.m_local->m_iHealth() > 0 && g_input.GetKeyState(g_menu.main.antiaim.fakeduck.get()))
		view->m_origin.z = g_cl.m_local->GetAbsOrigin().z + 64.f;

    // call original.
	g_hooks.m_client_mode.GetOldMethod< OverrideView_t >( IClientMode::OVERRIDEVIEW )( this, view );

    // remove scope edge blur.
	if ( c_config::get( )->b[ "noscope" ] ) {
		if( g_cl.m_local && g_cl.m_local->m_bIsScoped( ) )
            view->m_edge_blur = 0;
	}
}

bool Hooks::CreateMove(float time, CUserCmd* cmd) {
	Stack   stack;
	// called from CInput::ExtraMouseSample -> return original.
	if (!cmd || !cmd->m_command_number)
		return g_hooks.m_client_mode.GetOldMethod< CreateMove_t >(IClientMode::CREATEMOVE)(this, time, cmd);

	// if we arrived here, called from -> CInput::CreateMove
	// call EngineClient::SetViewAngles according to what the original returns.
	if (g_hooks.m_client_mode.GetOldMethod< CreateMove_t >(IClientMode::CREATEMOVE)(this, time, cmd))
		g_csgo.m_engine->SetViewAngles(cmd->m_view_angles);

	// random_seed isn't generated in ClientMode::CreateMove yet, we must set generate it ourselves.
	cmd->m_random_seed = g_csgo.MD5_PseudoRandom(cmd->m_command_number) & 0x7fffffff;

	// get bSendPacket off the stack.
	g_cl.m_packet = stack.next().local(0x1c).as< bool* >();

	// get bFinalTick off the stack.
	g_cl.m_final_packet = stack.next().local(0x1b).as< bool* >();

	// keyhandler
	g_visuals.m_thirdperson = c_keyhandler::get( )->auto_check( "thirdperson_key" );

	//manual aa setup
	if ( GetAsyncKeyState( c_config::get()->i["manual_left_key"] ) ) {
		g_hvh.m_left = !g_hvh.m_left;
		g_hvh.m_right = false;
		g_hvh.m_back = false;
	}
	else if ( GetAsyncKeyState( c_config::get( )->i[ "manual_right_key"] ) ) {
		g_hvh.m_left = false;
		g_hvh.m_right = !g_hvh.m_right;
		g_hvh.m_back = false;
	}
	else if ( GetAsyncKeyState( c_config::get( )->i[ "manual_back_key"] ) ) {
		g_hvh.m_left = false;
		g_hvh.m_right = false;
		g_hvh.m_back = !g_hvh.m_back;
	}

	g_cl.OnTick(cmd);

	return false;
}

bool Hooks::DoPostScreenSpaceEffects( CViewSetup* setup ) {
	g_visuals.RenderGlow( );

	return g_hooks.m_client_mode.GetOldMethod< DoPostScreenSpaceEffects_t >( IClientMode::DOPOSTSPACESCREENEFFECTS )( this, setup );
}