#pragma once
#include "entity_manager.hpp"

// std
#include <fstream>
#include <stdexcept>
#include <iterator>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include <optional>
#include <filesystem>
#include <type_traits>

namespace nEngine::Utils {

	using byte = char;

	static std::default_random_engine generator{};
	static std::uniform_real_distribution<float> float_distribution{ -10.f, 10.f };

	// from: https://stackoverflow.com/a/57595105
	template <typename T, typename... Rest>
	void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
		seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hashCombine(seed, rest), ...);
	};

	void _assert(bool assertation, std::string&& fail_message);

	std::vector<byte> read_file(const std::string& filepath);
	ECS::Transform rand_transform();
	std::optional<std::filesystem::path> get_save_dir();
	std::optional<std::filesystem::path> write_save_state(ECS::Manager& manager, std::string& filename);
	std::optional<std::filesystem::path> load_save_state(ECS::Manager& manager, std::string& filename);

	template <typename T>
	void serialize_collected_pods_guarded(std::vector<T>& vec, std::ofstream& file) {
		static_assert(std::is_trivial_v<T> == true);
		static_assert(std::is_standard_layout_v<T> == true);

		size_t vector_size{};

		vector_size = vec.size();
		if (!file.write(reinterpret_cast<const byte*>(&vector_size), sizeof(vector_size))) {
			throw std::runtime_error("Error writing Savestate: Failed to write to file.");
		}

		if (!file.write(reinterpret_cast<const byte*>(vec.data()), vector_size * sizeof(T))) {
			throw std::runtime_error("Error writing Savestate: Failed to write to file.");
		}
	}

	template <typename T>
	void deserialize_collected_pods_guarded(std::vector<T>& vec, std::ifstream& file) {
		static_assert(std::is_trivial_v<T> == true);
		static_assert(std::is_standard_layout_v<T> == true);

		size_t vector_size{};
		if (!file.read(reinterpret_cast<byte*>(&vector_size), sizeof(vector_size))) {
			throw std::runtime_error("Error loading Savestate: Corrupted file.");
		}

		if (vector_size != vec.size()) {
			throw std::runtime_error("Error loading Savestate: State invalid.");
		}

		if (!file.read(reinterpret_cast<byte*>(vec.data()), vector_size * sizeof(T))) {
			throw std::runtime_error("Error loading Savestate: Corrupted file.");
		}
	}

	class Timer {
	public:
		Timer(std::string ref);
		~Timer();
	private:
		std::chrono::time_point<std::chrono::steady_clock> start;
		std::chrono::time_point<std::chrono::steady_clock> end;
		std::chrono::duration<float> duration;
		std::string reference;
	};
}