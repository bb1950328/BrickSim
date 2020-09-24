// test_configuration_reader.cpp
// Created by bab21 on 19.09.20.
//

#include <iostream>
#include "../config.h"

int mainx() {
    std::cout << Configuration::getInstance()->get_double("test");
    return 0;
}