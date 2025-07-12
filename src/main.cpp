#include "first_app.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
    SJFGame::FirstApp app{};

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << "[======= ERROR =======] " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}