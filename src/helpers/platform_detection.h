//
// Created by Bader on 05.01.2021.
//

#ifndef BRICKSIM_PLATFORM_DETECTION_H
#define BRICKSIM_PLATFORM_DETECTION_H

// from https://stackoverflow.com/questions/5919996/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define BRICKSIM_PLATFORM_WIN32_OR_64
#ifdef _WIN64
#define BRICKSIM_PLATFORM_WIN64
#else
#define BRICKSIM_PLATFORM_WIN32
#endif
#elif __APPLE__
#include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
         #define BRICKSIM_PLATFORM_IOS_SIMULATOR
    #elif TARGET_OS_IPHONE
        #define BRICKSIM_PLATFORM_IOS_DEVICE
    #elif TARGET_OS_MAC
        #define BRICKSIM_PLATFORM_MACOS
    #else
    #   error "Unknown Apple platform"
    #endif
    #define BRICKSIM_PLATFORM_SOME_APPLE
#elif __linux__
    #define BRICKSIM_PLATFORM_LINUX
#elif __unix__ // all unices not caught above
    #define BRICKSIM_PLATFORM_UNIX_NOT_LINUX
#elif defined(_POSIX_VERSION)
    #define BRICKSIM_PLATFORM_POSIX_NOT_UNIX
#else
#define BRICKSIM_PLATFORM_UNKNOWN
#endif


#endif //BRICKSIM_PLATFORM_DETECTION_H
