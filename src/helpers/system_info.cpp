#include "system_info.h"
#include "../controller.h"
#include "../graphics/hardware_properties.h"
#include <curl/curlver.h>
#include <glad/glad.h>
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <spdlog/version.h>
#include <stb_image.h>
#include <string>
#include <vector>

#include <cpuinfo.h>
#include <curl/curl.h>
#include <magic_enum.hpp>

namespace bricksim::helpers::system_info {
    std::vector<std::pair<std::string, std::string>> getSystemInfo() {
        std::vector<std::pair<std::string, std::string>> result;
        curl_version_info_data* curlVersionData = curl_version_info(CURLVERSION_NOW);

        result.emplace_back("GPU Vendor:",
                            std::string(reinterpret_cast<const char *>(graphics::getHardwareProperties().vendor)));
        result.emplace_back("GPU Renderer:",
                            std::string(reinterpret_cast<const char *>(graphics::getHardwareProperties().renderer)));
        result.emplace_back("BrickSim Version:", constants::versionString);
        result.emplace_back("Git Commit Hash:", constants::gitCommitHash);
        result.emplace_back("Dear ImGui Version:", ImGui::GetVersion());
        result.emplace_back("Libcurl Version:", LIBCURL_VERSION);
        result.emplace_back("Spdlog Version:", fmt::format("{}.{}.{}", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH));
        result.emplace_back("STBI Version:", std::to_string(STBI_VERSION));
        result.emplace_back("GLFW Version:", fmt::format("{}.{}.{}", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION));
        result.emplace_back("cURL Version:", curlVersionData->version);
        result.emplace_back("cURL Features:", fmt::format("{0:b}", curlVersionData->features));

        result.emplace_back("sizeof(void*):", fmt::format("{} Bytes or {} bits", sizeof(void*), sizeof(void*) * 8));
        result.emplace_back("sizeof(char):", fmt::format("{} Bytes or {} bits", sizeof(char), sizeof(char) * 8));
        result.emplace_back("sizeof(int):", fmt::format("{} Bytes or {} bits", sizeof(int), sizeof(int) * 8));
        result.emplace_back("sizeof(long):", fmt::format("{} Bytes or {} bits", sizeof(long), sizeof(long) * 8));
        result.emplace_back("sizeof(float):", fmt::format("{} Bytes or {} bits", sizeof(float), sizeof(float) * 8));
        result.emplace_back("sizeof(double):", fmt::format("{} Bytes or {} bits", sizeof(double), sizeof(double) * 8));

        cpuinfo_initialize();

        const cpuinfo_processor* const processor = cpuinfo_get_current_processor();
        const cpuinfo_cluster* cluster = processor->cluster;
        result.emplace_back("CPU vendor:", magic_enum::enum_name(cluster->vendor).substr(15));
        result.emplace_back("CPU package name: ", cluster->package->name);
        result.emplace_back("CPU microarchitecture code: ", fmt::format("{:#08x}", static_cast<uint64_t>(cluster->uarch)));

        result.emplace_back("ImGui::GetFontSize():", std::to_string(ImGui::GetFontSize()));
        result.emplace_back("ImGui::GetWindowDpiScale():", std::to_string(ImGui::GetWindowDpiScale()));

        return result;
    }
}
