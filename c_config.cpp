#include "includes.h"
std::string color_to_string(float col[4]) {
	return std::to_string((int)(col[0] * 255)) + "," + std::to_string((int)(col[1] * 255)) + "," + std::to_string((int)(col[2] * 255)) + "," + std::to_string((int)(col[3] * 255));
}

float* string_to_color(std::string s) {
	static auto split = [](std::string str, const char* del) -> std::vector<std::string>
	{
		char* pTempStr = _strdup(str.c_str());
		char* pWord = strtok(pTempStr, del);
		std::vector<std::string> dest;

		while (pWord != NULL)
		{
			dest.push_back(pWord);
			pWord = strtok(NULL, del);
		}

		free(pTempStr);

		return dest;
	};

	std::vector<std::string> col = split(s, ",");
	return new float[4]{
		(float)std::stoi(col.at(0)) / 255.f,
		(float)std::stoi(col.at(1)) / 255.f,
		(float)std::stoi(col.at(2)) / 255.f,
		(float)std::stoi(col.at(3)) / 255.f
	};
}

void c_config::save() {
	char file_path[MAX_PATH] = { 0 };
	sprintf(file_path, "C:/spotlight/csgo_%s.ini", presets.at(i["_preset"]));

	for (auto e : b) {
		if (!std::string(e.first).find("_")) continue;
		char buffer[8] = { 0 }; _itoa(e.second, buffer, 10);
		WritePrivateProfileStringA("b", e.first.c_str(), std::string(buffer).c_str(), file_path);
	}

	for (auto e : i) {
		if (!std::string(e.first).find("_")) continue;
		char buffer[32] = { 0 }; _itoa(e.second, buffer, 10);
		WritePrivateProfileStringA("i", e.first.c_str(), std::string(buffer).c_str(), file_path);
	}

	for (auto e : f) {
		if (!std::string(e.first).find("_")) continue;
		char buffer[64] = { 0 }; sprintf(buffer, "%f", e.second);
		WritePrivateProfileStringA("f", e.first.c_str(), std::string(buffer).c_str(), file_path);
	}

	for (auto e : c) {
		if (!std::string(e.first).find("_")) continue;
		WritePrivateProfileStringA("c", e.first.c_str(), color_to_string(e.second).c_str(), file_path);
	}

	for (auto e : m) {
		if (!std::string(e.first).find("_")) continue;

		std::string vs = "";
		for (auto v : e.second)
			vs += std::to_string(v.first) + ":" + std::to_string(v.second) + "|";

		WritePrivateProfileStringA("m", e.first.c_str(), vs.c_str(), file_path);
	}

	this->save_keys();
}

void c_config::load() {
	this->load_defaults();

	char file_path[MAX_PATH] = { 0 };
	sprintf(file_path, "C:/spotlight/csgo_%s.ini", presets.at(i["_preset"]));

	char b_buffer[65536], i_buffer[65536], f_buffer[65536], c_buffer[65536], m_buffer[65536] = { 0 };

	int b_read = GetPrivateProfileSectionA("b", b_buffer, 65536, file_path);
	int i_read = GetPrivateProfileSectionA("i", i_buffer, 65536, file_path);
	int f_read = GetPrivateProfileSectionA("f", f_buffer, 65536, file_path);
	int c_read = GetPrivateProfileSectionA("c", c_buffer, 65536, file_path);
	int m_read = GetPrivateProfileSectionA("m", m_buffer, 65536, file_path);

	if ((0 < b_read) && ((65536 - 2) > b_read)) {
		const char* pSubstr = b_buffer;

		while ('\0' != *pSubstr) {
			size_t substrLen = strlen(pSubstr);

			const char* pos = strchr(pSubstr, '=');
			if (NULL != pos) {
				char name[256] = "";
				char value[256] = "";

				strncpy_s(name, _countof(name), pSubstr, pos - pSubstr);
				strncpy_s(value, _countof(value), pos + 1, substrLen - (pos - pSubstr));

				b[name] = atoi(value);
			}

			pSubstr += (substrLen + 1);
		}
	}

	if ((0 < i_read) && ((65536 - 2) > i_read)) {
		const char* pSubstr = i_buffer;

		while ('\0' != *pSubstr) {
			size_t substrLen = strlen(pSubstr);

			const char* pos = strchr(pSubstr, '=');
			if (NULL != pos) {
				char name[256] = "";
				char value[256] = "";

				strncpy_s(name, _countof(name), pSubstr, pos - pSubstr);
				strncpy_s(value, _countof(value), pos + 1, substrLen - (pos - pSubstr));

				i[name] = atoi(value);
			}

			pSubstr += (substrLen + 1);
		}
	}

	if ((0 < f_read) && ((65536 - 2) > f_read)) {
		const char* pSubstr = f_buffer;

		while ('\0' != *pSubstr) {
			size_t substrLen = strlen(pSubstr);

			const char* pos = strchr(pSubstr, '=');
			if (NULL != pos) {
				char name[256] = "";
				char value[256] = "";

				strncpy_s(name, _countof(name), pSubstr, pos - pSubstr);
				strncpy_s(value, _countof(value), pos + 1, substrLen - (pos - pSubstr));

				f[name] = atof(value);
			}

			pSubstr += (substrLen + 1);
		}
	}

	if ((0 < c_read) && ((65536 - 2) > c_read)) {
		const char* pSubstr = c_buffer;

		while ('\0' != *pSubstr) {
			size_t substrLen = strlen(pSubstr);

			const char* pos = strchr(pSubstr, '=');
			if (NULL != pos) {
				char name[256] = "";
				char value[256] = "";

				strncpy_s(name, _countof(name), pSubstr, pos - pSubstr);
				strncpy_s(value, _countof(value), pos + 1, substrLen - (pos - pSubstr));

				auto col = string_to_color(value);
				c[name][0] = col[0];
				c[name][1] = col[1];
				c[name][2] = col[2];
				c[name][3] = col[3];
			}

			pSubstr += (substrLen + 1);
		}
	}

	static auto split = [](std::string str, const char* del) -> std::vector<std::string>
	{
		char* pTempStr = _strdup(str.c_str());
		char* pWord = strtok(pTempStr, del);
		std::vector<std::string> dest;

		while (pWord != NULL)
		{
			dest.push_back(pWord);
			pWord = strtok(NULL, del);
		}

		free(pTempStr);

		return dest;
	};

	if ((0 < m_read) && ((65536 - 2) > m_read)) {
		const char* pSubstr = m_buffer;

		while ('\0' != *pSubstr) {
			size_t substrLen = strlen(pSubstr);

			const char* pos = strchr(pSubstr, '=');
			if (NULL != pos) {
				char name[256] = "";
				char value[256] = "";

				strncpy_s(name, _countof(name), pSubstr, pos - pSubstr);
				strncpy_s(value, _countof(value), pos + 1, substrLen - (pos - pSubstr));

				std::vector<std::string> kvpa = split(value, "|");
				std::unordered_map<int, bool> vl = {};
				for (auto kvp : kvpa) {
					if (kvp == "")
						continue; // ало глухой

					std::vector<std::string> kv = split(kvp, ":");
					vl[std::stoi(kv.at(0))] = std::stoi(kv.at(1));
				}

				m[name] = vl;
			}

			pSubstr += (substrLen + 1);
		}
	}

	// хуй

	this->load_keys();
}

void c_config::load_defaults() {
	int _preset = this->i["_preset"];

	b = std::unordered_map<std::string, bool>();
	i = std::unordered_map<std::string, int>();
	f = std::unordered_map<std::string, float>();
	c = std::unordered_map<std::string, float[4]>();

	i["_preset"] = _preset;

	c["menu_color"][0] = 0.937f;
	c["menu_color"][1] = 0.035f;
	c["menu_color"][2] = 0.373f;
	c["menu_color"][3] = 1.00f;

	i["misc_menukey"] = VK_INSERT;


	b["skins"] = false;
	i["knife"] = 0;
	i["glove"] = 0;
	i["glove_id"] = 0;

	i["id_deagle"] = 0;
	b["stattrak_deagle"] = false;
	i["seed_deagle"] = 0;
	i["quality_deagle"] = 100;

	i["id_elite"] = 0;
	b["stattrak_elite"] = false;
	i["seed_elite"] = 0;
	i["quality_elite"] = 100;

	i["id_fiveseven"] = 0;
	b["stattrak_fiveseven"] = false;
	i["seed_fiveseven"] = 0;
	i["quality_fiveseven"] = 100;

	i["id_glock"] = 0;
	b["stattrak_glock"] = false;
	i["seed_glock"] = 0;
	i["quality_glock"] = 100;

	i["id_ak47"] = 0;
	b["stattrak_ak47"] = false;
	i["seed_ak47"] = 0;
	i["quality_ak47"] = 100;

	i["id_aug"] = 0;
	b["stattrak_aug"] = false;
	i["seed_aug"] = 0;
	i["quality_aug"] = 100;

	i["id_aug"] = 0;
	b["stattrak_aug"] = false;
	i["seed_aug"] = 0;
	i["quality_aug"] = 100;

	i["id_awp"] = 0;
	b["stattrak_awp"] = false;
	i["seed_awp"] = 0;
	i["quality_awp"] = 100;

	i["id_famas"] = 0;
	b["stattrak_famas"] = false;
	i["seed_famas"] = 0;
	i["quality_famas"] = 100;

	i["id_g3sg1"] = 0;
	b["stattrak_g3sg1"] = false;
	i["seed_g3sg1"] = 0;
	i["quality_g3sg1"] = 100;

	i["id_galil"] = 0;
	b["stattrak_galil"] = false;
	i["seed_galil"] = 0;
	i["quality_galil"] = 100;

	i["id_m249"] = 0;
	b["stattrak_m249"] = false;
	i["seed_m249"] = 0;
	i["quality_m249"] = 100;

	i["id_m4a4"] = 0;
	b["stattrak_m4a4"] = false;
	i["seed_m4a4"] = 0;
	i["quality_m4a4"] = 100;

	i["id_mac10"] = 0;
	b["stattrak_mac10"] = false;
	i["seed_mac10"] = 0;
	i["quality_mac10"] = 100;

	i["id_p90"] = 0;
	b["stattrak_p90"] = false;
	i["seed_p90"] = 0;
	i["quality_p90"] = 100;

	i["id_ump45"] = 0;
	b["stattrak_ump45"] = false;
	i["seed_ump45"] = 0;
	i["quality_ump45"] = 100;

	i["id_xm1014"] = 0;
	b["stattrak_xm1014"] = false;
	i["seed_xm1014"] = 0;
	i["quality_xm1014"] = 100;

	i["id_bizon"] = 0;
	b["stattrak_bizon"] = false;
	i["seed_bizon"] = 0;
	i["quality_bizon"] = 100;

	i["id_mag7"] = 0;
	b["stattrak_mag7"] = false;
	i["seed_mag7"] = 0;
	i["quality_mag7"] = 100;

	i["id_negev"] = 0;
	b["stattrak_negev"] = false;
	i["seed_negev"] = 0;
	i["quality_negev"] = 100;

	i["id_sawedoff"] = 0;
	b["stattrak_sawedoff"] = false;
	i["seed_sawedoff"] = 0;
	i["quality_sawedoff"] = 100;

	i["id_tec9"] = 0;
	b["stattrak_tec9"] = false;
	i["seed_tec9"] = 0;
	i["quality_tec9"] = 100;

	i["id_p2000"] = 0;
	b["stattrak_p2000"] = false;
	i["seed_p2000"] = 0;
	i["quality_p2000"] = 100;

	i["id_mp7"] = 0;
	b["stattrak_mp7"] = false;
	i["seed_mp7"] = 0;
	i["quality_mp7"] = 100;

	i["id_mp9"] = 0;
	b["stattrak_mp9"] = false;
	i["seed_mp9"] = 0;
	i["quality_mp9"] = 100;

	i["id_nova"] = 0;
	b["stattrak_nova"] = false;
	i["seed_nova"] = 0;
	i["quality_nova"] = 100;

	i["id_scar20"] = 0;
	b["stattrak_scar20"] = false;
	i["seed_scar20"] = 0;
	i["quality_scar20"] = 100;

	i["id_sg553"] = 0;
	b["stattrak_sg553"] = false;
	i["seed_sg553"] = 0;
	i["quality_sg553"] = 100;

	i["id_ssg08"] = 0;
	b["stattrak_ssg08"] = false;
	i["seed_ssg08"] = 0;
	i["quality_ssg08"] = 100;

	i["id_m4a1s"] = 0;
	b["stattrak_m4a1s"] = false;
	i["seed_m4a1s"] = 0;
	i["quality_m4a1s"] = 100;

	i["id_usps"] = 0;
	b["stattrak_usps"] = false;
	i["seed_usps"] = 0;
	i["quality_usps"] = 100;

	i["id_cz75a"] = 0;
	b["stattrak_cz75a"] = false;
	i["seed_cz75a"] = 0;
	i["quality_cz75a"] = 100;

	i["id_revolver"] = 0;
	b["stattrak_revolver"] = false;
	i["seed_revolver"] = 0;
	i["quality_revolver"] = 100;

	i["id_bayonet"] = 0;
	b["stattrak_bayonet"] = false;
	i["seed_bayonet"] = 0;
	i["quality_bayonet"] = 100;

	i["id_flip"] = 0;
	b["stattrak_flip"] = false;
	i["seed_flip"] = 0;
	i["quality_flip"] = 100;

	i["id_gut"] = 0;
	b["stattrak_gut"] = false;
	i["seed_gut"] = 0;
	i["quality_gut"] = 100;

	i["id_karambit"] = 0;
	b["stattrak_karambit"] = false;
	i["seed_karambit"] = 0;
	i["quality_karambit"] = 100;

	i["id_m9"] = 0;
	b["stattrak_m9"] = false;
	i["seed_m9"] = 0;
	i["quality_m9"] = 100;

	i["id_huntsman"] = 0;
	b["stattrak_huntsman"] = false;
	i["seed_huntsman"] = 0;
	i["quality_huntsman"] = 100;

	i["id_falchion"] = 0;
	b["stattrak_falchion"] = false;
	i["seed_falchion"] = 0;
	i["quality_falchion"] = 100;

	i["id_bowie"] = 0;
	b["stattrak_bowie"] = false;
	i["seed_bowie"] = 0;
	i["quality_bowie"] = 100;

	i["id_butterfly"] = 0;
	b["stattrak_butterfly"] = false;
	i["seed_butterfly"] = 0;
	i["quality_butterfly"] = 100;

	i["id_daggers"] = 0;
	b["stattrak_daggers"] = false;
	i["seed_daggers"] = 0;
	i["quality_daggers"] = 100;

	c["footstep_color"][0] = 1.f;
	c["footstep_color"][1] = 1.f;
	c["footstep_color"][2] = 1.f;
	c["footstep_color"][3] = 1.f;
	b["footstep"] = false;


	i["transparent_props_amount"] = 100;
	i["thirdperson_distance"] = 100;
	i["thirdperson_distance"] = 100;
	i["spread_xhair_blend"] = 100;

	c["spread_xhair_color"][0] = 1.f;
	c["spread_xhair_color"][1] = 1.f;
	c["spread_xhair_color"][2] = 1.f;
	c["spread_xhair_color"][3] = 1.f;

	c["proj_color"][0] = 1.f;
	c["proj_color"][1] = 1.f;
	c["proj_color"][2] = 1.f;
	c["proj_color"][3] = 1.f;

	c["hostname_color"][0] = 1.f;
	c["hostname_color"][1] = 1.f;
	c["hostname_color"][2] = 1.f;
	c["hostname_color"][3] = 1.f;

	c["hostdist_color"][0] = 1.f;
	c["hostdist_color"][1] = 1.f;
	c["hostdist_color"][2] = 1.f;
	c["hostdist_color"][3] = 1.f;

	c["item_color"][0] = 1.f;
	c["item_color"][1] = 1.f;
	c["item_color"][2] = 1.f;
	c["item_color"][3] = 1.f;

	c["grenade_trail_color"][0] = 1.f;
	c["grenade_trail_color"][1] = 1.f;
	c["grenade_trail_color"][2] = 1.f;
	c["grenade_trail_color"][3] = 1.f;

	c["grenade_radius_color"][0] = 1.f;
	c["grenade_radius_color"][1] = 1.f;
	c["grenade_radius_color"][2] = 1.f;
	c["grenade_trail_color"][3] = 1.f;


	c["dropammo_color"][0] = 1.f;
	c["dropammo_color"][1] = 1.f;
	c["dropammo_color"][2] = 1.f;
	c["dropammo_color"][3] = 1.f;

	c["bomb_color"][0] = 150.f / 255.f;
	c["bomb_color"][1] = 200.f / 255.f;
	c["bomb_color"][2] = 60.f / 255.f;
	c["bomb_color"][3] = 1.f;


	c["offscreen_color"][0] = 1.f;
	c["offscreen_color"][1] = 1.f;
	c["offscreen_color"][2] = 1.f;
	c["offscreen_color"][3] = 1.f;

	c["name_color"][0] = 1.f;
	c["name_color"][1] = 1.f;
	c["name_color"][2] = 1.f;
	c["name_color"][3] = 1.f;

	c["lby_update_color"][0] = 215.f / 255.f;
	c["lby_update_color"][1] = 20.f / 255.f;
	c["lby_update_color"][2] = 215.f / 255.f;
	c["lby_update_color"][3] = 1.f;

	c["ammo_color"][0] = 80.f / 255.f;
	c["ammo_color"][1] = 130.f / 255.f;
	c["ammo_color"][2] = 180.f / 255.f;
	c["ammo_color"][3] = 1.f;

	c["weapon_icon_color"][0] = 1.f;
	c["weapon_icon_color"][1] = 1.f;
	c["weapon_icon_color"][2] = 1.f;
	c["weapon_icon_color"][3] = 1.f;


	c["skeleton_enemy_color"][0] = 1.f;
	c["skeleton_enemy_color"][1] = 1.f;
	c["skeleton_enemy_color"][2] = 1.f;
	c["skeleton_enemy_color"][3] = 1.f;


	c["skeleton_friendly_color"][0] = 1.f;
	c["skeleton_friendly_color"][1] = 1.f;
	c["skeleton_friendly_color"][2] = 1.f;
	c["skeleton_friendly_color"][3] = 1.f;


	c["glow_enemy_color"][0] = 160.f / 255.f;
	c["glow_enemy_color"][1] = 60.f / 255.f;
	c["glow_enemy_color"][2] = 110.f / 255.f;
	c["glow_enemy_color"][3] = 1.f;


	c["glow_friendly_color"][0] = 1.f;
	c["glow_friendly_color"][1] = 1.f;
	c["glow_friendly_color"][2] = 1.f;
	c["glow_friendly_color"][3] = 1.f;


	c["impact_beams_color"][0] = 1.f;
	c["impact_beams_color"][1] = 1.f;
	c["impact_beams_color"][2] = 1.f;

	c["impact_beams_hurt_color"][0] = 1.f;
	c["impact_beams_hurt_color"][1] = 1.f;
	c["impact_beams_hurt_color"][2] = 1.f;

	i["cstrafe_keystyle"] = 1;
	i["zstrafe_keystyle"] = 1;
	i["astrafe_keystyle"] = 1;
	i["fakewalk_keystyle"] = 1;
	i["override_keystyle"] = 2;
	i["fakelatency_keystyle"] = 1;
	i["baimkey_keystyle"] = 1;
	i["last_tick_defuse_keystyle"] = 1;
	i["thirdperson_keystyle"] = 2;
	i[ "manual_left_keystyle" ] = 1;
	i[ "manual_right_keystyle" ] = 1;
	i[ "manual_back_keystyle" ] = 1;
	i["chams_enemy_history_blend"] = 100;
	i["fake_chams_blend"] = 100;
	i["chams_local_blend"] = 100;
	i["chams_enemy_blend"] = 100;
	i["hitchance_amount"] = 50;
	i["accuracy_boost"] = 50;

	i["body_scale"] = 50;
	i["minimal_damage"] = 15;
	i["fov_amount"] = 180;
	i["fov_amt"] = 90;
	i["viewmodel_fov_amt"] = 90;
	i["penetrate_minimal_damage"] = 10;
	c["box_friendly_color"][0] = 1.f;
	c["box_friendly_color"][1] = 1.f;
	c["box_friendly_color"][2] = 1.f;
	c["box_friendly_color"][3] = 1.f;

	c["box_enemy_color"][0] = 1.f;
	c["box_enemy_color"][1] = 1.f;
	c["box_enemy_color"][2] = 1.f;
	c["box_enemy_color"][3] = 1.f;

	c["fake_chams_col"][0] = 1.f;
	c["fake_chams_col"][1] = 1.f;
	c["fake_chams_col"][2] = 1.f;
	c["fake_chams_col"][3] = 1.f;

	c["chams_local_col"][0] = 1.f;
	c["chams_local_col"][1] = 1.f;
	c["chams_local_col"][2] = 1.f;
	c["chams_local_col"][3] = 1.f;


	c["chams_enemy_history_col"][0] = 1.f;
	c["chams_enemy_history_col"][1] = 1.f;
	c["chams_enemy_history_col"][2] = 1.f;
	c["chams_enemy_history_col"][3] = 1.f;

	c["chams_enemy_vis"][0] = 173 / 255.f;
	c["chams_enemy_vis"][1] = 244 / 255.f;
	c["chams_enemy_vis"][2] = 5 / 255.f;
	c["chams_enemy_vis"][3] = 255;

	c["chams_enemy_invis"][0] = 0 / 255.f;
	c["chams_enemy_invis"][1] = 170 / 255.f;
	c["chams_enemy_invis"][2] = 255 / 255.f;
	c["chams_enemy_invis"][3] = 255;

	i["head_scale"] = 10;
	i["chest_scale"] = 15;
	i["body_scale"] = 20;
	i["legs_scale"] = 30;
	i["world"] = 100;
}

void c_config::load_keys() {
	for (int k = 0; k < presets.size(); k++) {
		char buffer[32] = { 0 }; sprintf(buffer, "_preset_%i", k);
		i[buffer] = GetPrivateProfileIntA("k", buffer, 0, "C:/spotlight/csgo_keys.ini");
	}
}

void c_config::save_keys() {
	for (int k = 0; k < presets.size(); k++) {
		char buffer[32] = { 0 }; sprintf(buffer, "_preset_%i", k);
		char value[32] = { 0 }; sprintf(value, "%i", i[buffer]);
		WritePrivateProfileStringA("k", buffer, value, "C:/spotlight/csgo_keys.ini");
	}
}