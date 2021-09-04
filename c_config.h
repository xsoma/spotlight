#pragma once
#include <unordered_map>
#include <array>
#include <algorithm>

#include "includes.h"
#include "singleton.h"

class c_config : public singleton<c_config> {
public:
	std::unordered_map<std::string, bool> b;
	std::unordered_map<std::string, int> i;
	std::unordered_map<std::string, float> f;
	std::unordered_map<std::string, float[4]> c;
	std::unordered_map<std::string, std::unordered_map<int, bool>> m;

	std::vector<const char*> presets = { "Rage", "Legit", "HvH", "Body-aim", "Secret", "Headshot", "Test" };

	void save();
	void save_keys();
	void load();
	void load_defaults();
	void load_keys();

	Color imcolor_to_ccolor(float* col) {
		return Color((int)(col[0] * 255.f), (int)(col[1] * 255.f), (int)(col[2] * 255.f), (int)(col[3] * 255.f));
	}

	Color imcolor_to_ccolor(float* col, float alpha_override) {
		return Color((int)(col[0] * 255.f), (int)(col[1] * 255.f), (int)(col[2] * 255.f), (int)(alpha_override * 255.f));
	}
};