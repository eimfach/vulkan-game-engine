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
		ECS::Transform t{ {}, { 1.f, 1.f, 1.f }, {} };
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

	std::optional<std::filesystem::path> write_save_state(ECS::Manager& manager, std::string& name) {
		if (auto path = get_save_dir()) {
			std::filesystem::create_directories(path.value());
			std::string filename = name + Settings::SAVEGAME_EXT;
			std::filesystem::path savestate = path.value() / filename;

			std::vector<char> file_copy = read_file(savestate.string());
			std::ofstream file{ savestate, std::ios::binary | std::ios::out };

			//TODO: write OS type flag and savestate compat version
			try {
				serialize_collected_pods_guarded(manager.getComponents<ECS::Visibility>(), file);
				serialize_collected_pods_guarded(manager.getComponents<ECS::Transform>(), file);
				file.close();
			}
			catch (const std::exception& e) {
				file.close();
				std::ofstream file_rollback{ savestate, std::ios::binary | std::ios::out };
				file_rollback.write(file_copy.data(), file_copy.size() * sizeof(char));
			}

			return savestate;
		}

		return {};
	}

	std::optional<std::filesystem::path> load_save_state(ECS::Manager& manager, std::string& name) {
		if (auto path = get_save_dir()) {
			std::string filename = name + Settings::SAVEGAME_EXT;
			std::filesystem::path savestate = path.value() / filename;
			std::ifstream file{ savestate, std::ios::binary | std::ios::in};

			try {
				deserialize_collected_pods_guarded(manager.getComponents<ECS::Visibility>(), file);
				deserialize_collected_pods_guarded(manager.getComponents<ECS::Transform>(), file);
			}
			catch (const std::exception& e) {
				std::cout << "Error loading save state";
			}

			return savestate;
		}
		
		return {};
	}

	void _assert(bool assertation, std::string&& fail_message) {
		#ifndef NDEBUG
		using namespace std::string_literals;

		if (!assertation) {
			std::cout << "Debug Assertion Abort: "s << fail_message << "\n";
			std::abort();
		}
		#endif
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