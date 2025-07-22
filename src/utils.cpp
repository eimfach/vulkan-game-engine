#include "utils.hpp"

namespace SJFGame::Utils {
	std::vector<char> read_file(const std::string& filepath) {
		std::ifstream file{ filepath, std::ios::binary };

		if (!file.is_open()) {
			throw std::runtime_error("Utils::read_file: Failed to open file: " + filepath);
		}

		std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});

		file.close();
		return buffer;
	}
}