// test_ldr_file_read.cpp
// Created by bb1950328 on 20.09.20.
//

#include <chrono>
#include <iostream>
#include "../ldr_files.h"
#include "../ldr_file_repository.h"

static const int run_count = 100;

int mainldr() {
    auto before = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < run_count; ++i) {
        LdrFile *pyramid = ldr_file_repo::get_file("~/ldraw/models/car.ldr");
        ldr_file_repo::clear_cache();
    }
    auto after = std::chrono::high_resolution_clock::now();
    long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count();
    std::cout << microseconds / 1000000.0 / run_count  << " seconds";
    return 0;
}
