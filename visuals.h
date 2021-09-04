#pragma once
#include "includes.h"

struct OffScreenDamageData_t {
    float m_time, m_color_step;
    Color m_color;

    __forceinline OffScreenDamageData_t() : m_time{ 0.f }, m_color{ colors::white } {}
    __forceinline OffScreenDamageData_t(float time, float m_color_step, Color color) : m_time{ time }, m_color{ color } {}
};

class Visuals {
public:
    std::array< bool, 64 >                  m_draw;
    std::array< float, 2048 >               m_opacities;
    std::array< OffScreenDamageData_t, 64 > m_offscreen_damage;
    vec2_t                                  m_crosshair;
    bool                                    m_thirdperson;
    bool                                    fakeangels;;
    float					                m_hit_start, m_hit_end, m_hit_duration;

    // info about planted c4.
    bool                                    planted;
    bool                                    bombexploded;
    bool                                    bombedefused;
    std::string                             m_bombsite;
    bool                                    bombeindefuse;
    float                                   m_planted_c4_defuse_time;
    bool        m_c4_planted;
    Entity* m_planted_c4;
    float       m_planted_c4_explode_time;
    vec3_t      m_planted_c4_explosion_origin;
    float       m_planted_c4_damage;
    float       m_planted_c4_radius;
    float       m_planted_c4_radius_scaled;
    float       miss;
    float       miss1;
    float       miss2;
    std::string m_last_bombsite;

    IMaterial* smoke1;
    IMaterial* smoke2;
    IMaterial* smoke3;
    IMaterial* smoke4;

    std::unordered_map< int, char > m_weapon_icons = {
        { DEAGLE, 'F' },
        { ELITE, 'S' },
        { FIVESEVEN, 'U' },
        { GLOCK, 'C' },
        { AK47, 'B' },
        { AUG, 'E' },
        { AWP, 'R' },
        { FAMAS, 'T' },
        { G3SG1, 'I' },
        { GALIL, 'V' },
        { M249, 'Z' },
        { M4A4, 'W' },
        { MAC10, 'L' },
        { P90, 'M' },
        { UMP45, 'Q' },
        { XM1014, ']' },
        { BIZON, 'D' },
        { MAG7, 'K' },
        { NEGEV, 'Z' },
        { SAWEDOFF, 'K' },
        { TEC9, 'C' },
        { ZEUS, 'Y' },
        { P2000, 'Y' },
        { MP7, 'X' },
        { MP9, 'D' },
        { NOVA, 'K' },
        { P250, 'Y' },
        { SCAR20, 'I' },
        { SG553, '[' },
        { SSG08, 'N' },
        { KNIFE_CT, 'J' },
        { FLASHBANG, 'G' },
        { HEGRENADE, 'H' },
        { SMOKE, 'P' },
        { MOLOTOV, 'H' },
        { DECOY, 'G' },
        { FIREBOMB, 'H' },
        { C4, '\\' },
        { KNIFE_T, 'J' },
        { M4A1S, 'W' },
        { USPS, 'Y' },
        { CZ75A, 'Y' },
        { REVOLVER, 'F' },
        { KNIFE_BAYONET, 'J' },
        { KNIFE_FLIP, 'J' },
        { KNIFE_GUT, 'J' },
        { KNIFE_KARAMBIT, 'J' },
        { KNIFE_M9_BAYONET, 'J' },
        { KNIFE_HUNTSMAN, 'J' },
        { KNIFE_FALCHION, 'J' },
        { KNIFE_BOWIE, 'J' },
        { KNIFE_BUTTERFLY, 'J' },
        { KNIFE_SHADOW_DAGGERS, 'J' },
    };

public:
    static void ModulateWorld();
    void ThirdpersonThink();
    void Hitmarker();
    void NoSmoke();
    void think();
    void Spectators();
    void StatusIndicators();
    void IndicatorCrosshair();
    void SpreadCrosshair();
    void PenetrationCrosshair();
    void DrawPlantedC4();
    void draw(Entity* ent);
    void DrawProjectile(Weapon* ent);
    void DrawItem(Weapon* item);
    void OffScreen(Player* player, int alpha);
    void DrawPlayer(Player* player);
    bool GetPlayerBoxRect(Player* player, Rect& box);
    void DrawHistorySkeleton(Player* player, int opacity);
    void DrawSkeleton(Player* player, int opacity);
    void RenderGlow();
    void DrawHitboxMatrix(LagRecord* record, Color col, float time);
    void DrawBeams();
    void DebugAimbotPoints(Player* player);
};

extern Visuals g_visuals;

class c_hud
{
private:

    /// Elements accent color mode.
    enum e_color_mode {
        STATIC = 1, /// Just accent color
        RAINBOW,        /// Dynamic color
        GRADIENT    /// Gradient dynamic color
    };

    /// Color stuff.

    /// <summary> Returns color, based on current style. </summary>
    void update_accent_color();

    struct style_t {
        e_color_mode      m_col_mode = RAINBOW; /// Elements accent color mode.
        Color             m_col_fallback = Color(210, 150, 20, 200); /// Change your accent color here.
        Color             m_col_accent[2]; /// Final color, will be used in elements rendering.

        /// Default elements stuff.
        size_t            m_outline_size = 4; /// Main outline size (in pixels).
        size_t            m_line_height = render::GetTextSize(Fonts::tahoma_8, "A").y;

        size_t            m_pad_base = 5;
        size_t            m_pad_header_top = 6;
        size_t            m_pad_header_sides = 6;
        size_t            m_pad_header_bottom = 7;

        Color             m_col_base = Color(0, 0, 0);
        Color             m_col_base_outer_outline = Color(0, 0, 0, 220);
        Color             m_col_base_inner_outline = Color(255, 255, 255, 4);

        Color             m_col_outline = Color(0, 0, 0, 200);
        Color             m_col_outer_outline = Color(0, 0, 0, 220);
        Color             m_col_inner_outline = Color(255, 255, 255, 8);
    } m_style;

    //void draw_frame(const vec2_t& pos, const std::wstring& name, const std::vector<std::wstring>& first_column, const std::vector<std::wstring>& second_column = {});
    void draw_frame(const vec2_t& pos, const std::wstring& name, const std::vector<std::wstring>& first_column, const std::vector<std::wstring>& second_column = {});
    //const vec2_t& pos, const wchar_t* name, const std::vector<std::wstring>& first_column, const std::vector<std::wstring>& second_column

};
