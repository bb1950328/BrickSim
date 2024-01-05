#pragma once

// from https://stackoverflow.com/questions/5919996/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor

namespace bricksim::detected_platform {
    enum class Platform {
        LINUX,
        MACOS,
        WINDOWS,
        OTHER,
    };

    enum class Bits {
        BIT32,
        BIT64
    };

    constexpr Bits bits = sizeof(void*) == 4 ? Bits::BIT32 : Bits::BIT64;// NOLINT

    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    constexpr Platform platform = Platform::WINDOWS;
    #define BRICKSIM_PLATFORM_WINDOWS
    #elif __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
    constexpr Platform platform = Platform::OTHER;
    #elif TARGET_OS_IPHONE
    constexpr Platform platform = Platform::OTHER;
    #elif TARGET_OS_MAC
    constexpr Platform platform = Platform::MACOS;
    #define BRICKSIM_PLATFORM_MACOS
    #else
    constexpr Platform platform = Platform::OTHER;
    #endif
    #elif __linux__
    constexpr Platform platform = Platform::LINUX;
    #define BRICKSIM_PLATFORM_LINUX
    #elif __unix__
    //unix but not linux
    constexpr Platform platform = Platform::OTHER;
    #elif defined(_POSIX_VERSION)
    //posix but not unix
    constexpr Platform platform = Platform::OTHER;
    #else
    constexpr Platform platform = Platform::OTHER;
    #endif
    constexpr bool linux_ = platform == Platform::LINUX;   // NOLINT
    constexpr bool macOS = platform == Platform::MACOS;    // NOLINT
    constexpr bool windows = platform == Platform::WINDOWS;// NOLINT
}
