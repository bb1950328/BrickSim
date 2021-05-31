#define CATCH_CONFIG_RUNNER

#include "catch2/catch.hpp"

int main(int argc, char *argv[]) {
    std::cerr << "Before Catch" << std::endl;
    const auto returnCode = Catch::Session().run(argc, argv);
    std::cerr << "After Catch" << std::endl;
    return returnCode;
}