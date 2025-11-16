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

namespace nEngine::Utils {
	static std::default_random_engine generator{};
	static std::uniform_real_distribution<float> float_distribution{ -10.f, 10.f };

	// from: https://stackoverflow.com/a/57595105
	template <typename T, typename... Rest>
	void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
		seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hashCombine(seed, rest), ...);
	};

	std::vector<char> read_file(const std::string& filepath);
	ECS::Transform rand_transform();
	std::optional<std::filesystem::path> get_save_dir();
	void write_save_state(ECS::Manager& manager);

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