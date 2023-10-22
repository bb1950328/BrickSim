#include "graphviz_wrapper.h"
#include "platform_detection.h"
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

#ifdef BRICKSIM_PLATFORM_WINDOWS
    #include <windows.h>
    #include <processthreadsapi.h>
    #ifdef min
        #undef min
    #endif
    #ifdef max
        #undef max
    #endif
#endif

namespace bricksim::graphviz_wrapper {

    bool renderDot(const std::filesystem::path& outputPath, std::string_view dot) {
        std::string format = "png";
        const auto outputExtension = outputPath.extension().string();
        if (outputExtension.ends_with("svg")) {
            format = "svg";
        } else if (outputExtension.ends_with("pdf")) {
            format = "pdf";
        } else if (outputExtension.ends_with("bmp")) {
            format = "bmp";
        }

#ifdef BRICKSIM_PLATFORM_WINDOWS
        const auto tmpDotFile = std::filesystem::temp_directory_path() / fmt::format("input.{}.dot", GetCurrentProcessId());
        const auto options = fmt::format("-T{} {} -o {}", format, tmpDotFile.string(), outputPath.string());
        ShellExecute(nullptr, "dot", options.c_str(), nullptr, nullptr, SW_SHOWNORMAL);//todo testing
        std::filesystem::remove(tmpDotFile);
#elif defined(BRICKSIM_PLATFORM_LINUX) || defined(BRICKSIM_PLATFORM_MACOS)
        std::string command = fmt::format("dot -T{} -o {}", format, outputPath.string());
        auto* pipe = popen(command.c_str(), "w");
        if (pipe == nullptr) {
            throw std::invalid_argument("");
        }
        const auto writtenLen = std::fwrite(dot.data(), sizeof(decltype(dot)::value_type), dot.size(), pipe);
        if (writtenLen != dot.size()) {
            spdlog::warn("incorrect number of chars written while calling graphviz: size={}, written={}", dot.size(), writtenLen);
        }

        pclose(pipe);
#else
    #warning "openDefaultProwser not supported on this platform"
#endif
        return std::filesystem::exists(outputPath);
    }
    bool isAvailable() {
        static bool checked = false;
        static bool result;
        if (!checked) {
#ifdef BRICKSIM_PLATFORM_WINDOWS
            result = true;//todo implement
#elif defined(BRICKSIM_PLATFORM_LINUX) || defined(BRICKSIM_PLATFORM_MACOS)
            result = system("which dot > /dev/null 2>&1") == 0;
#endif
        }
        return result;
    }
}
