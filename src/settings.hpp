#pragma once
#include <string>

namespace SJFGame::Settings {
	const std::string FONT_PATH = "fonts/Roboto-Medium.ttf";
	const int32_t IM_GUI_POOL_SIZE = 16;

	inline float FOV_DEGREES{50.f};
	inline float NEAR_PLANE{.1f};
	inline float FAR_PLANE{75.f};

	inline bool VSYNC = false;
	inline bool GAMMA_CORRECTION = false;
	inline bool SHOW_DETAILED_METRICS = false;
}