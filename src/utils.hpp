#pragma once
#include "entity_manager.hpp"

// std
#include <fstream>
#include <stdexcept>
#include <iterator>
#include <string>
#include <vector>
#include <random>

namespace SJFGame::Utils {
	static std::default_random_engine generator{};
	static std::uniform_real_distribution<float> float_distribution{ -10.f, 10.f };

	// from: https://stackoverflow.com/a/57595105
	template <typename T, typename... Rest>
	void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
		seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hashCombine(seed, rest), ...);
	};

	std::vector<char> read_file(const std::string& filepath);
	ECS::Transform randTransform();
}