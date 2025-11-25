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

	using namespace std::string_literals;

	std::vector<byte> read_file(const std::string& filepath) {
		std::ifstream file{ filepath, std::ios::binary };

		if (!file.is_open()) {
			throw std::runtime_error("Utils::read_file: Failed to open file: " + filepath);
		}

		std::vector<byte> buffer(std::istreambuf_iterator<byte>(file), {});

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
		p = p / ("."s + Settings::SAVEGAME_FOLDER) / "saves";
		return p;
		#endif
	}

	std::optional<std::filesystem::path> write_save_state(ECS::Manager& manager, std::string& name) {
		if (auto path = get_save_dir()) {
			std::filesystem::create_directories(path.value());
			std::string filename = name + Settings::SAVEGAME_EXT;
			std::filesystem::path save_state_path = path.value() / filename;

			std::vector<byte> file_copy{};
			if (std::filesystem::exists(save_state_path)) {
				file_copy = read_file(save_state_path.string());
			}

			std::ofstream file{ save_state_path, std::ios::binary | std::ios::out };

			//TODO: write OS type flag and savestate compat version
			//TODO: Compress with LZ4
			try {
				serialize_collected_pods_guarded(manager.getComponents<ECS::Visibility>(), file);
				serialize_collected_pods_guarded(manager.getComponents<ECS::Transform>(), file);
				file.close();
			}
			catch (const std::exception& e) {
				file.close();
				std::cerr << e.what() << " while writing save data, rolling back. \n";

				if (file_copy.size() > 0) {
					std::ofstream file_rollback{ save_state_path, std::ios::binary | std::ios::out };
					file_rollback.write(file_copy.data(), file_copy.size() * sizeof(byte));
				}

				return {};
			}

			return save_state_path;
		}

		return {};
	}

	std::optional<std::filesystem::path> load_save_state(ECS::Manager& manager, std::string& name) {
		if (auto path = get_save_dir()) {
			std::string filename = name + Settings::SAVEGAME_EXT;
			std::filesystem::path save_state_path = path.value() / filename;

			if (!std::filesystem::exists(save_state_path)) {
				return {};
			}

			std::ifstream file{ save_state_path, std::ios::binary | std::ios::in};

			try {
				//TODO: Make copies of the component vectors, which are overwritten with the binary data, 
				// if nothing throws, reassign them to the ecs manager at the end
				const std::vector<ECS::Visibility>& v1 = deserialize_collected_pods_guarded(manager.getComponents<ECS::Visibility>(), file);
				const std::vector<ECS::Transform>& v2 = deserialize_collected_pods_guarded(manager.getComponents<ECS::Transform>(), file);

				manager.loadIntoBuffer(v1);
				manager.loadIntoBuffer(v2);
			}
			catch (const std::exception& e) {
				std::cerr << e.what() << "\n";
				return {};
			}

			return save_state_path;
		}
		
		return {};
	}

	void _assert(bool assertation, std::string&& fail_message) {
		#ifndef NDEBUG
		using namespace std::string_literals;

		if (!assertation) {
			std::cerr << "Debug Assertion Abort: "s << fail_message << "\n";
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