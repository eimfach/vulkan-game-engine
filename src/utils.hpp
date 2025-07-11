#pragma once

// std
#include <fstream>
#include <stdexcept>
#include <iterator>
#include <string>
#include <vector>

namespace SJFGame {
	// from: https://stackoverflow.com/a/57595105
	template <typename T, typename... Rest>
	void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
		seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hashCombine(seed, rest), ...);
	};

	std::vector<char> read_file(const std::string& filepath, const std::char_traits<char> traits) {
		std::ifstream file{ filepath, std::ios::binary };

		if (!file.is_open()) {
			throw std::runtime_error("Utils::read_file: Failed to open file: " + filepath);
		}

		std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});

		file.close();
		return buffer;
	}
}