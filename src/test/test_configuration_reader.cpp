// test_configuration_reader.cpp
// Created by bb1950328 on 19.09.20.
//

#include <iostream>
#include "../config.h"

int mainx() {
    std::cout << Configuration::getInstance()->get_double("test");
    return 0;
}