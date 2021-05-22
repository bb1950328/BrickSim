#ifndef BRICKSIM_SYSTEM_INFO_H
#define BRICKSIM_SYSTEM_INFO_H

#ifdef _WIN32

#include <climits>
#include <intrin.h>
#include <string>
#include <vector>

typedef unsigned __int32 uint32_t;
#else
#include <stdint.h>
#include <string>
#include <vector>

#endif

namespace system_info {
    ///https://gist.github.com/9prady9/a5e1e8bdbc9dc58b3349
    class CPUID {
        uint32_t regs[4];

    public:
        explicit CPUID(unsigned funcId, unsigned subFuncId);

        [[nodiscard]] const uint32_t &EAX() const { return regs[0]; }

        [[nodiscard]] const uint32_t &EBX() const { return regs[1]; }

        [[nodiscard]] const uint32_t &ECX() const { return regs[2]; }

        [[nodiscard]] const uint32_t &EDX() const { return regs[3]; }
    };

    class CPUInfo {
    public:
        CPUInfo();

        [[nodiscard]] std::string vendor() const { return mVendorId; }

        [[nodiscard]] std::string model() const { return mModelName; }

        [[nodiscard]] int cores() const { return mNumCores; }

        [[nodiscard]] bool isSSE() const { return mIsSSE; }

        [[nodiscard]] bool isSSE2() const { return mIsSSE2; }

        [[nodiscard]] bool isSSE3() const { return mIsSSE3; }

        [[nodiscard]] bool isSSE41() const { return mIsSSE41; }

        [[nodiscard]] bool isSSE42() const { return mIsSSE42; }

        [[nodiscard]] bool isAVX() const { return mIsAVX; }

        [[nodiscard]] bool isAVX2() const { return mIsAVX2; }

        [[nodiscard]] bool isHyperThreaded() const { return mIsHTT; }

        [[nodiscard]] int logicalCpus() const { return mNumLogCpus; }

    private:
        // Bit positions for data extractions
        static const uint32_t SSE_POS = 0x02000000;
        static const uint32_t SSE2_POS = 0x04000000;
        static const uint32_t SSE3_POS = 0x00000001;
        static const uint32_t SSE41_POS = 0x00080000;
        static const uint32_t SSE42_POS = 0x00100000;
        static const uint32_t AVX_POS = 0x10000000;
        static const uint32_t AVX2_POS = 0x00000020;
        static const uint32_t LVL_NUM = 0x000000FF;
        static const uint32_t LVL_TYPE = 0x0000FF00;
        static const uint32_t LVL_CORES = 0x0000FFFF;

        // Attributes
        std::string mVendorId;
        std::string mModelName;
        int mNumSMT;
        int mNumCores;
        int mNumLogCpus;
        bool mIsHTT;
        bool mIsSSE;
        bool mIsSSE2;
        bool mIsSSE3;
        bool mIsSSE41;
        bool mIsSSE42;
        bool mIsAVX;
        bool mIsAVX2;
    };

    std::vector<std::pair<const char *, std::string>> getSystemInfo();
}

#endif //BRICKSIM_SYSTEM_INFO_H
