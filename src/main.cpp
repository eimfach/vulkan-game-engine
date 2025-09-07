#include "first_app.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
    SJFGame::FirstApp app{};

    app.run();

    return EXIT_SUCCESS;
}