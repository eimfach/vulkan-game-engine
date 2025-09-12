#include "first_app.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
    nEngine::FirstApp app{};

    app.run();

    return EXIT_SUCCESS;
}