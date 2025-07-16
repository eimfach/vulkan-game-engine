#include "assets.hpp"

#include <stdexcept>

namespace SJFGame::Assets {
    bool import_model(const std::string& filepath) {
        // Create an instance of the Importer class
        Assimp::Importer importer;

        // And have it read the given file with some example postprocessing
        // Usually - if speed is not the most important aspect for you - you'll
        // probably to request more postprocessing than we do in this example.
        const aiScene* scene = importer.ReadFile(filepath,
            aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType);

        // If the import failed, report it
        if (nullptr == scene) {
            //DoTheErrorLogging(importer.GetErrorString());
            throw std::runtime_error("Error loading model: " + filepath);
            return false;
        }

        // Now we can access the file's contents.
        //DoTheSceneProcessing(scene);

        // We're done. Everything will be cleaned up by the importer destructor
        return true;
    }
}