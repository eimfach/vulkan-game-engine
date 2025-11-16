#pragma once
#include <string>

namespace nEngine::Settings {
	const std::string FONT_PATH = "fonts/Roboto-Medium.ttf";
	const int32_t IM_GUI_POOL_SIZE = 16;

	inline float FOV_DEGREES{50.f};
	inline float NEAR_PLANE{.1f};
	inline float FAR_PLANE{30.f};

	inline int VOXEL_GRID_EXTENT = 20;

	inline bool VSYNC = false;
	inline bool GAMMA_CORRECTION = false;
	inline bool SHOW_DETAILED_METRICS = false;
	const int MAX_LIGHTS{ 10 };

	inline std::string SAVEGAME_FOLDER{ "nEngine" };
	inline std::string SAVEGAME_EXT{ ".nEngine" };

	struct KeyMappings {
		int moveLeft = GLFW_KEY_A;
		int moveRight = GLFW_KEY_D;
		int moveForward = GLFW_KEY_W;
		int moveBackward = GLFW_KEY_S;
		int moveUp = GLFW_KEY_E;
		int moveDown = GLFW_KEY_Q;
		int lookLeft = GLFW_KEY_LEFT;
		int lookRight = GLFW_KEY_RIGHT;
		int lookUp = GLFW_KEY_UP;
		int lookDown = GLFW_KEY_DOWN;

		int saveGame = GLFW_KEY_F5;
		int loadGame = GLFW_KEY_F7;
	};
}