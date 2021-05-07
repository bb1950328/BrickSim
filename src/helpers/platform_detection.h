#ifndef BRICKSIM_PLATFORM_DETECTION_H
#define BRICKSIM_PLATFORM_DETECTION_H

// from https://stackoverflow.com/questions/5919996/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor

namespace detected_platform {
    enum Platform {
        LINUX,
        MACOS,
        WINDOWS,
        OTHER,
    };
    enum Bits {
        BIT32,
        BIT64
    };

    constexpr Bits bits = sizeof(void *) == 4 ? BIT32 : BIT64;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    constexpr Platform platform = WINDOWS;
#define BRICKSIM_PLATFORM_WINDOWS
#elif __APPLE__
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
    constexpr Platform platform = OTHER;
#elif TARGET_OS_IPHONE
    constexpr Platform platform = OTHER;
#elif TARGET_OS_MAC
    constexpr Platform platform = MACOS;
#define BRICKSIM_PLATFORM_MACOS
#else
    constexpr Platform platform = OTHER;
#endif
#elif __linux__
    constexpr Platform platform = LINUX;
#define BRICKSIM_PLATFORM_LINUX
#elif __unix__
    //unix but not linux
    constexpr Platform platform = OTHER;
#elif defined(_POSIX_VERSION)
    //posix but not unix
    constexpr Platform platform = OTHER;
#else
    constexpr Platform platform = OTHER;
#endif
    constexpr bool linux = platform==LINUX;
    constexpr bool macOS = platform==MACOS;
    constexpr bool windows = platform==WINDOWS;
}


#endif //BRICKSIM_PLATFORM_DETECTION_H
