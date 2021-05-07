#include <curl/curl.h>
#include <spdlog/spdlog.h>
#include "system_info.h"
#include "../controller.h"
#include "stb_image.h"

namespace system_info {
    std::vector<std::pair<const char *, std::string>> getSystemInfo() {
        std::vector<std::pair<const char *, std::string>> result;
        const GLubyte *vendor;
        const GLubyte *renderer;
        controller::executeOpenGL([&]() {
            vendor = glGetString(GL_VENDOR);
            renderer = glGetString(GL_RENDERER);
        });

        result.emplace_back("GPU Vendor:", std::string(reinterpret_cast<const char *>(vendor)));
        result.emplace_back("GPU Renderer:", std::string(reinterpret_cast<const char *>(renderer)));
        std::cout << constants::versionMinor << std::endl;
        result.emplace_back("BrickSim Version:", constants::versionString);
        result.emplace_back("Git Commit Hash:", constants::gitCommitHash);
        result.emplace_back("Dear ImGui Version:", ImGui::GetVersion());
        result.emplace_back("Libcurl Version:", LIBCURL_VERSION);
        result.emplace_back("Spdlog Version:",
                            std::to_string(SPDLOG_VER_MAJOR) + '.' + std::to_string(SPDLOG_VER_MINOR) + '.' + std::to_string(SPDLOG_VER_PATCH));
        result.emplace_back("STBI Version:", std::to_string(STBI_VERSION));
        result.emplace_back("GLFW Version:",
                            std::to_string(GLFW_VERSION_MAJOR) + "." + std::to_string(GLFW_VERSION_MINOR) + "." + std::to_string(GLFW_VERSION_REVISION));

        result.emplace_back("sizeof(void*):", std::to_string(sizeof(void *)) + " Bytes or " + std::to_string(sizeof(void *) * 8) + " Bits");
        result.emplace_back("sizeof(char):", std::to_string(sizeof(char)) + " Bytes or " + std::to_string(sizeof(char) * 8) + " Bits");
        result.emplace_back("sizeof(int):", std::to_string(sizeof(int)) + " Bytes or " + std::to_string(sizeof(int) * 8) + " Bits");
        result.emplace_back("sizeof(long):", std::to_string(sizeof(long)) + " Bytes or " + std::to_string(sizeof(long) * 8) + " Bits");
        result.emplace_back("sizeof(float):", std::to_string(sizeof(float)) + " Bytes or " + std::to_string(sizeof(float) * 8) + " Bits");
        result.emplace_back("sizeof(double):", std::to_string(sizeof(double)) + " Bytes or " + std::to_string(sizeof(double) * 8) + " Bits");

        CPUInfo cpuInfo;
        result.emplace_back("CPU Vendor:", cpuInfo.vendor());
        result.emplace_back("CPU Model:", cpuInfo.model());
        result.emplace_back("Number of CPU cores:", std::to_string(cpuInfo.cores()));
        result.emplace_back("Number logical CPUs:", std::to_string(cpuInfo.logicalCpus()));
        std::string cpuFeatures;
        std::map<const char*, bool> features = {
                {"SSE", cpuInfo.isSSE()},
                {"SSE2", cpuInfo.isSSE2()},
                {"SSE3", cpuInfo.isSSE3()},
                {"SSE41", cpuInfo.isSSE41()},
                {"SSE42", cpuInfo.isSSE42()},
                {"AVX", cpuInfo.isAVX()},
                {"AVX2", cpuInfo.isAVX2()},
                {"HyperThreaded", cpuInfo.isHyperThreaded()},
        };
        bool first = true;
        for (const auto &item : features) {
            if (item.second) {
                if (first) {
                    first = false;
                } else {
                    cpuFeatures += ' ';
                }
                cpuFeatures += item.first;
            }
        }
        result.emplace_back("CPU Features:", cpuFeatures);

        result.emplace_back("ImGui::GetFontSize():", std::to_string(ImGui::GetFontSize()));
        result.emplace_back("ImGui::GetWindowDpiScale():", std::to_string(ImGui::GetWindowDpiScale()));

        return result;
    }

    CPUID::CPUID(unsigned int funcId, unsigned int subFuncId) {
#ifdef _WIN32
        __cpuidex((int *) regs, (int) funcId, (int) subFuncId);

#else
        asm volatile
            ("cpuid" : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3])
             : "a" (funcId), "c" (subFuncId));
        // ECX is set to zero for CPUID function 4
#endif
    }

    CPUInfo::CPUInfo() {
        // Get vendor name EAX=0
        CPUID cpuID0(0, 0);
        uint32_t HFS = cpuID0.EAX();
        mVendorId += std::string((const char *) &cpuID0.EBX(), 4);
        mVendorId += std::string((const char *) &cpuID0.EDX(), 4);
        mVendorId += std::string((const char *) &cpuID0.ECX(), 4);
        // Get SSE instructions availability
        CPUID cpuID1(1, 0);
        mIsHTT = cpuID1.EDX() & AVX_POS;
        mIsSSE = cpuID1.EDX() & SSE_POS;
        mIsSSE2 = cpuID1.EDX() & SSE2_POS;
        mIsSSE3 = cpuID1.ECX() & SSE3_POS;
        mIsSSE41 = cpuID1.ECX() & SSE41_POS;
        mIsSSE42 = cpuID1.ECX() & SSE41_POS;
        mIsAVX = cpuID1.ECX() & AVX_POS;
        // Get AVX2 instructions availability
        CPUID cpuID7(7, 0);
        mIsAVX2 = cpuID7.EBX() & AVX2_POS;

        std::string upVId = mVendorId;
        for_each(upVId.begin(), upVId.end(), [](char &in) { in = ::toupper(in); });
        // Get num of cores
        if (upVId.find("INTEL") != std::string::npos) {
            if (HFS >= 11) {
                for (int lvl = 0; lvl < 4; ++lvl) {
                    CPUID cpuID4(0x0B, lvl);
                    uint32_t currLevel = (LVL_TYPE & cpuID4.ECX()) >> 8;
                    switch (currLevel) {
                        case 0x01:
                            mNumSMT = LVL_CORES & cpuID4.EBX();
                            break;
                        case 0x02:
                            mNumLogCpus = LVL_CORES & cpuID4.EBX();
                            break;
                        default:
                            break;
                    }
                }
                mNumCores = mNumLogCpus / mNumSMT;
            } else {
                if (HFS >= 1) {
                    mNumLogCpus = (cpuID1.EBX() >> 16u) & 0xFF;
                    if (HFS >= 4) {
                        mNumCores = 1u + (CPUID(4, 0).EAX() >> 26) & 0x3F;
                    }
                }
                if (mIsHTT) {
                    if (!(mNumCores > 1)) {
                        mNumCores = 1;
                        mNumLogCpus = (mNumLogCpus >= 2 ? mNumLogCpus : 2);
                    }
                } else {
                    mNumCores = mNumLogCpus = 1;
                }
            }
        } else if (upVId.find("AMD") != std::string::npos) {
            if (HFS >= 1) {
                mNumLogCpus = (cpuID1.EBX() >> 16u) & 0xFF;
                if (CPUID(0x80000000, 0).EAX() >= 8) {
                    mNumCores = 1u + (CPUID(0x80000008, 0).ECX() & 0xFF);
                }
            }
            if (mIsHTT) {
                if (!(mNumCores > 1)) {
                    mNumCores = 1;
                    mNumLogCpus = (mNumLogCpus >= 2 ? mNumLogCpus : 2);
                }
            } else {
                mNumCores = mNumLogCpus = 1;
            }
        } else {
            spdlog::warn("Unexpected vendor id");
        }
        // Get processor brand string
        // This seems to be working for both Intel & AMD vendors
        for (int i = 0x80000002; i < 0x80000005; ++i) {
            CPUID cpuID(i, 0);
            mModelName += std::string((const char *) &cpuID.EAX(), 4);
            mModelName += std::string((const char *) &cpuID.EBX(), 4);
            mModelName += std::string((const char *) &cpuID.ECX(), 4);
            mModelName += std::string((const char *) &cpuID.EDX(), 4);
        }
    }
}