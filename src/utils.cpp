#include "utils.hpp"
#include "settings.hpp"

//lib
#ifdef _WIN32
#include "shlobj.h"
#include "combaseapi.h"
#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

// std
#include <iostream>
#include <cstdlib>

namespace nEngine::Utils {
	std::vector<char> read_file(const std::string& filepath) {
		std::ifstream file{ filepath, std::ios::binary };

		if (!file.is_open()) {
			throw std::runtime_error("Utils::read_file: Failed to open file: " + filepath);
		}

		std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});

		file.close();
		return buffer;
	}

	ECS::Transform rand_transform() {
		float x = float_distribution(generator);
		float y = float_distribution(generator);
		float z = float_distribution(generator);
		ECS::Transform t{};
		t.translation = { x, y, z };
		return t;
	}

	std::optional<std::filesystem::path> get_save_dir() {
		#ifdef _WIN32
		wchar_t* path;
		auto r = SHGetKnownFolderPath(FOLDERID_SavedGames, 0, NULL, &path);
		if (r == E_FAIL) {
			return {};
		} if (r == E_INVALIDARG) {
			return {};
		}

		std::filesystem::path p{ path };
		p = p / Settings::SAVEGAME_FOLDER / "saves";
		CoTaskMemFree(path);
		return p;

		#else
		struct passwd* pw = getpwuid(getuid());
		const char* homedir = pw->pw_dir;

		std::filesystem::path p{ homedir };
		p = p / "." << Settings::SAVEGAME_FOLDER / "saves";
		return p;
		#endif
	}

	std::optional<std::filesystem::path> write_save_state(ECS::Manager& manager) {
		if (auto path = get_save_dir()) {
			std::filesystem::create_directories(path.value());
			std::string filename = "save" + Settings::SAVEGAME_EXT;
			std::filesystem::path savestate = path.value() / filename;
			std::ofstream file{ savestate, std::ios::binary | std::ios::out };

			size_t size{};

			auto& vc = manager.getComponents<ECS::Visibility>();
			size = vc.size();
			file.write(reinterpret_cast<const char*>(&size), sizeof(size));
			for (ECS::Visibility v : vc) {
				file.write(reinterpret_cast<const char*>(&v), sizeof(v));
			}

			auto& vt = manager.getComponents<ECS::Transform>();
			size = vt.size();
			file.write(reinterpret_cast<const char*>(&size), sizeof(size));
			for (ECS::Transform t : vt) {
				file.write(reinterpret_cast<const char*>(&t), sizeof(t));
			}

			file.close();

			return savestate;
		}

		return {};
	}

	std::optional<std::filesystem::path> load_save_state(ECS::Manager& manager) {
		if (auto path = get_save_dir()) {
			std::string filename = "save" + Settings::SAVEGAME_EXT;
			std::filesystem::path savestate = path.value() / filename;
			std::ifstream file{ savestate, std::ios::binary | std::ios::in};

			size_t vector_size{};
			file.read(reinterpret_cast<char*>(&vector_size), sizeof(vector_size));
			auto& vc = manager.getComponents<ECS::Visibility>();
			for (size_t i = 0; i < vector_size; i++) {
				ECS::Visibility v{};
				file.read(reinterpret_cast<char*>(&v), sizeof(v));
				vc[i] = v;
			}

			vector_size = 0;
			file.read(reinterpret_cast<char*>(&vector_size), sizeof(vector_size));
			auto& vt = manager.getComponents<ECS::Transform>();
			for (size_t i = 0; i < vector_size; i++) {
				ECS::Transform t{};
				file.read(reinterpret_cast<char*>(&t), sizeof(t));
				vt[i] = t;
			}

			return savestate;
		}
		
		return {};
	}

	Timer::Timer(std::string ref) : reference{ref} {
		start = std::chrono::high_resolution_clock::now();
	}

	Timer::~Timer() {
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;

		float ms = duration.count() * 1000.f;
		std::cout << "[" << reference << "] " << ms << " ms\n";
	}
}