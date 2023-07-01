#include "../ldr/files.h"
#include <catch2/catch_all.hpp>
#include <iostream>

namespace bricksim {
    TEST_CASE("ldr quadrilateral parse") {
        std::string line1 = "4 1 1.2 3.4 5.6 7.8 9.0 10.11 12.13 14.15 16.17 18.19 20.21 22.23";
        std::string line1content = line1.substr(2);
        BENCHMARK("parse 1") {
            auto line = ldr::FileElement::parseLine(line1, {});
            return line;
        };
        BENCHMARK("parse 2") {
            auto line = ldr::Quadrilateral(line1, ldr::WindingOrder::CCW);
            return line;
        };
    }
}
