#define CATCH_CONFIG_RUNNER

#include "catch2/catch.hpp"

int main(int argc, char *argv[]) {
    //initialisation here
    const auto returnCode = Catch::Session().run(argc, argv);
    //cleanup here
    return returnCode;
}