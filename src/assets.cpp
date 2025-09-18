#include "assets.hpp"

//libs
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>

//std
#include <stdexcept>
#include <filesystem>
#include <iostream>

namespace nEngine::Assets {
    namespace GLTF = fastgltf;

    bool import_model(const std::string& filepath) {
        GLTF::Parser parser{};
        std::filesystem::path system_file{ filepath };

        auto expected_data = GLTF::GltfDataBuffer::FromPath(system_file);
        if (expected_data.error() != GLTF::Error::None) {
            std::cout << "GLTF: could'nt load the given file at path: " << filepath << "\n";
            return false;
        }

        auto expected_asset = parser.loadGltf(expected_data.get(), system_file.parent_path(), GLTF::Options::None);
        if (expected_asset.error() != GLTF::Error::None) {
            std::cout << "GLTF: error occurred while reading the buffer, parsing the JSON, or validating the data of the file at path: " << filepath << "\n";
            return false;
        }

        // The glTF 2.0 asset is now ready to be used. Simply call asset.get(), asset.get_if() or
        // asset-> to get a direct reference to the Asset class. You can then access the glTF data
        // structures, like, for example, with buffers:
        GLTF::Asset& asset = expected_asset.get();
        for (auto& buffer : expected_asset->buffers) {
            // Process the buffers.
        }

        return false;
    }
}