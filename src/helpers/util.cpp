
#include <algorithm>
#include <numeric>
#include <iostream>
#include <filesystem>
#include <glm/gtx/string_cast.hpp>
#include <curl/curl.h>
#include "util.h"
#include "../constant_data/git_stats.h"
#include "../config.h"
#include "../lib/stb_image_write.h"
#include "../lib/stb_image.h"
#include "../controller.h"
#include "platform_detection.h"
#include "../db.h"

#ifdef BRICKSIM_PLATFORM_WIN32_OR_64

#include <windows.h>

#endif

#include <cstdlib>
#include <imgui.h>
#include <mutex>
#include <spdlog/spdlog.h>
#include <fcntl.h>

namespace util {
    namespace {

        bool isStbiVerticalFlipEnabled = false;
    }

    std::string extendHomeDir(const std::string &input) {
        return extendHomeDirPath(input).string();
    }

    std::string replaceHomeDir(const std::string &input) {
        const std::string homeDir = getenv(USER_ENV_VAR);
        if (startsWith(input, homeDir)) {
            return homeDir + input.substr(homeDir.size());
        }
        return input;
    }

    std::filesystem::path extendHomeDirPath(const std::string &input) {
        if (input[0] == '~' && (input[1] == '/' || input[1] == '\\')) {
            return std::filesystem::path(getenv(USER_ENV_VAR)) / std::filesystem::path(input.substr(2));
        } else if (input[0] == '~' && input.size()==1) {
            return std::filesystem::path(getenv(USER_ENV_VAR));
        } else {
            return std::filesystem::path(input);
        }
    }

    std::string trim(const std::string &input) {
        auto wsbefore = std::find_if_not(input.begin(), input.end(), [](int c) { return std::isspace(c); });
        auto wsafter = std::find_if_not(input.rbegin(), input.rend(), [](int c) { return std::isspace(c); }).base();
        return (wsafter <= wsbefore ? std::string() : std::string(wsbefore, wsafter));
    }

    std::string pathjoin(const std::list<std::string> &parts) {
        auto result = std::string();
        for (const auto &part : parts) {
            result.append(part);
            if (result.back() != PATH_SEPARATOR) {
                result.push_back(PATH_SEPARATOR);
            }
        }
        result.pop_back();//remove last separator
        std::replace(result.begin(), result.end(), PATH_SEPARATOR_FOREIGN, PATH_SEPARATOR);
        return result;
    }

    std::string asLower(const std::string &string) {
        auto result = std::string();
        for (const auto &ch: string) {
            result.push_back(std::tolower(ch));
        }
        return result;
    }

    bool endsWith(std::string const &fullString, std::string const &ending) {
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
        } else {
            return false;
        }
    }

    bool startsWith(std::string const &fullString, std::string const &start) {
        return fullString.rfind(start, 0) == 0;
    }

    void coutMat4(glm::mat4 mat) {/*
        std::cout << "⌈" << glm::to_string(mat[0]) << "⌉\n";
        std::cout << "|" << glm::to_string(mat[1]) << "|\n";
        std::cout << "|" << glm::to_string(mat[2]) << "|\n";
        std::cout << "⌊" << glm::to_string(mat[3]) << "⌋\n";*/
        printf("%8.4f, %8.4f, %8.4f, %8.4f\n", mat[0][0], mat[0][1], mat[0][2], mat[0][3]);
        printf("%8.4f, %8.4f, %8.4f, %8.4f\n", mat[1][0], mat[1][1], mat[1][2], mat[1][3]);
        printf("%8.4f, %8.4f, %8.4f, %8.4f\n", mat[2][0], mat[2][1], mat[2][2], mat[2][3]);
        printf("%8.4f, %8.4f, %8.4f, %8.4f\n", mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
    }

    void replaceAll(std::string &str, const std::string &from, const std::string &to) {
        //https://stackoverflow.com/a/3418285/8733066
        if (from.empty())
            return;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }

    std::string replaceChar(const std::string &str, char from, char to) {
        std::string result(str);
        if (from != to) {
            for (char &i : result) {
                if (i == from) {
                    i = to;
                }
            }
        }
        return result;
    }

    float biggestValue(glm::vec2 vector) {
        return std::max(vector.x, vector.y);
    }

    float biggestValue(glm::vec3 vector) {
        return std::max(std::max(vector.x, vector.y), vector.z);
    }

    float biggestValue(glm::vec4 vector) {
        return std::max(std::max(vector.x, vector.y), std::max(vector.z, vector.w));
    }

    void openDefaultBrowser(const std::string &link) {
        spdlog::info("openDefaultBrowser(\"{}\")", link);
#ifdef BRICKSIM_PLATFORM_WIN32_OR_64
        ShellExecute(nullptr, "open", link.c_str(), nullptr, nullptr, SW_SHOWNORMAL);//todo testing
#elif defined(BRICKSIM_PLATFORM_SOME_APPLE)
        std::string command = std::string("open ") + link;
#elif defined(BRICKSIM_PLATFORM_LINUX)
        std::string command = std::string("xdg-open ") + link;
#endif
#if defined(BRICKSIM_PLATFORM_LINUX) || defined(BRICKSIM_PLATFORM_SOME_APPLE)
        int exitCode = system(command.c_str());
        if (exitCode != 0) {
            spdlog::warn("command \"{}\" exited with code {}", command, exitCode);
        }
#endif
    }

    RGBcolor::RGBcolor(const std::string &htmlCode) {
        std::sscanf(htmlCode.c_str(), "#%2hhx%2hhx%2hhx", &red, &green, &blue);
    }

    glm::vec3 triangleCentroid(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3) {
        return (p1 + p2 + p3) / 3.0f;
    }

    glm::vec3 quadrilateralCentroid(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const glm::vec3 &p4) {
        return (p1 + p2 + p3 + p4) / 4.0f;
    }

    bool doesTransformationInverseWindingOrder(const glm::mat4 &transformation) {
        glm::vec3 vec1 = transformation[0];
        glm::vec3 vec2 = transformation[1];
        glm::vec3 vec3 = transformation[2];
        glm::vec3 cross = glm::cross(vec1, vec2);
        return glm::dot(cross, vec3) < 0.0f;
    }

    glm::vec3 convertIntToColorVec3(unsigned int value) {
        unsigned char bluePart = value & 0xffu;//blue first is intended
        value >>= 8u;
        unsigned char greenPart = value & 0xffu;
        value >>= 8u;
        unsigned char redPart = value & 0xffu;
        return glm::vec3(redPart / 255.0f, greenPart / 255.0f, bluePart / 255.0f);
    }

    unsigned int getIntFromColor(unsigned char red, unsigned char green, unsigned char blue) {
        unsigned int result = ((unsigned int) red) << 16u | ((unsigned int) green) << 8u | blue;
        return result;
    }

    std::vector<std::pair<const char *, std::string>> getSystemInfo() {
        std::vector<std::pair<const char *, std::string>> result;
        const GLubyte *vendor = glGetString(GL_VENDOR);
        const GLubyte *renderer = glGetString(GL_RENDERER);
        result.emplace_back("sizeof(void*):",  std::to_string(sizeof(void *)) + " Bytes or " + std::to_string(sizeof(void *) * 8) + " Bits");
        result.emplace_back("sizeof(char):",  std::to_string(sizeof(char)) + " Bytes or " + std::to_string(sizeof(char) * 8) + " Bits");
        result.emplace_back("sizeof(int):",  std::to_string(sizeof(int)) + " Bytes or " + std::to_string(sizeof(int) * 8) + " Bits");
        result.emplace_back("sizeof(long):",  std::to_string(sizeof(long)) + " Bytes or " + std::to_string(sizeof(long) * 8) + " Bits");
        result.emplace_back("sizeof(float):",  std::to_string(sizeof(float)) + " Bytes or " + std::to_string(sizeof(float) * 8) + " Bits");
        result.emplace_back("sizeof(double):",  std::to_string(sizeof(double)) + " Bytes or " + std::to_string(sizeof(double) * 8) + " Bits");
        result.emplace_back("GPU Vendor:",  std::string(reinterpret_cast<const char *>(vendor)));
        result.emplace_back("GPU Renderer:",  std::string(reinterpret_cast<const char *>(renderer)));
        result.emplace_back("Git Commit Hash:", git_stats::lastCommitHash);
        result.emplace_back("Dear ImGui Version:", IMGUI_VERSION);
        result.emplace_back("Libcurl Version:", LIBCURL_VERSION);
        result.emplace_back("Spdlog Version:", std::to_string(SPDLOG_VER_MAJOR)+'.'+std::to_string(SPDLOG_VER_MINOR)+'.'+std::to_string(SPDLOG_VER_PATCH));
        result.emplace_back("STBI Version:", std::to_string(STBI_VERSION));
        result.emplace_back("GLFW Version:", std::to_string(GLFW_VERSION_MAJOR)+"."+std::to_string(GLFW_VERSION_MINOR)+"."+std::to_string(GLFW_VERSION_REVISION));
        return result;
    }

    float vectorSum(glm::vec2 vector) {
        return vector.x + vector.y;
    }

    float vectorSum(glm::vec3 vector) {
        return vector.x + vector.y + vector.z;
    }

    float vectorSum(glm::vec4 vector) {
        return vector.x + vector.y + vector.z + vector.w;
    }

    std::string formatBytesValue(size_t bytes) {
        double doubleBytes = bytes;
        static std::string bytePrefixes[] = {"B", "KB", "MB", "GB", "TB"};
        size_t prefixIndex = 0;
        while (doubleBytes > 1024) {
            prefixIndex++;
            doubleBytes /= 1024;
        }
        std::stringstream resultStream;
        resultStream << /*resultStream.precision(3) <<*/ doubleBytes << bytePrefixes[prefixIndex];
        return resultStream.str();
    }

    std::string RGBcolor::asHtmlCode() const {
        char buffer[8];
        snprintf(buffer, 8, "#%02x%02x%02x", red, green, blue);
        auto result = std::string(buffer);
        return result;
    }

    RGBcolor::RGBcolor(glm::vec3 vector) {
        red = vector.x * 255;
        green = vector.y * 255;
        blue = vector.z * 255;
    }

    glm::vec3 RGBcolor::asGlmVector() const {
        return glm::vec3(red / 255.0f, green / 255.0f, blue / 255.0f);
    }

    RGBcolor::RGBcolor(const HSVcolor& hsv) {
        if (hsv.saturation == 0) {
            red = hsv.value;
            green = hsv.value;
            blue = hsv.value;
        } else {
            float h = hsv.hue / 255.0f;
            float s = hsv.saturation / 255.0f;
            float v = hsv.value / 255.0f;
            auto i = (int) std::floor(h * 6);
            auto f = h * 6 - i;
            auto p = v * (1.0f - s);
            auto q = v * (1.0f - s * f);
            auto t = v * (1.0f - s * (1.0f - f));
            switch (i % 6) {
                case 0:
                    red = v * 255;
                    green = t * 255;
                    blue = p * 255;
                    break;
                case 1:
                    red = q * 255;
                    green = v * 255;
                    blue = p * 255;
                    break;
                case 2:
                    red = p * 255;
                    green = v * 255;
                    blue = t * 255;
                    break;
                case 3:
                    red = p * 255;
                    green = q * 255;
                    blue = v * 255;
                    break;
                case 4:
                    red = t * 255;
                    green = p * 255;
                    blue = v * 255;
                    break;
                case 5:
                    red = v * 255;
                    green = p * 255;
                    blue = q * 255;
                    break;
                default:
                    break;//shouldn't get here
            }
        }
    }

    RGBcolor::RGBcolor(color_component_t red, color_component_t green, color_component_t blue) : red(red), green(green), blue(blue) {

    }

    HSVcolor::HSVcolor(glm::vec3 vector) {
        hue = vector.x * 255;
        saturation = vector.y * 255;
        value = vector.z * 255;
    }

    glm::vec3 HSVcolor::asGlmVector() const {
        return glm::vec3(hue / 255.0f, saturation / 255.0f, value / 255.0f);
    }

    HSVcolor::HSVcolor(RGBcolor rgb) {
        auto maxc = std::max(std::max(rgb.red, rgb.green), rgb.blue);
        auto minc = std::min(std::min(rgb.red, rgb.green), rgb.blue);
        value = maxc;
        if (maxc != minc) {
            const auto maxmindiff = maxc - minc;
            saturation = maxmindiff * 1.0f / maxc;
            auto rc = (maxc - rgb.red) * 1.0f / maxmindiff;
            auto gc = (maxc - rgb.green) * 1.0f / maxmindiff;
            auto bc = (maxc - rgb.blue) * 1.0f / maxmindiff;
            float h;
            if (rgb.red == maxc) {
                h = bc - gc;
            } else if (rgb.green == maxc) {
                h = 2.0f + rc - bc;
            } else {
                h = 4.0f + gc - rc;
            }
            hue = (((h / 255 / 6.0f) - (int) (h / 255 / 6.0f)) * 255.0f);
        }
    }


    bool memeqzero(const void *data, size_t length) {
        //from https://github.com/rustyrussell/ccan/blob/master/ccan/mem/mem.c#L92
        const auto *p = static_cast<const unsigned char *>(data);
        size_t len;

        /* Check first 16 bytes manually */
        for (len = 0; len < 16; len++) {
            if (!length)
                return true;
            if (*p)
                return false;
            p++;
            length--;
        }

        /* Now we know that's zero, memcmp with self. */
        return memcmp(data, p, length) == 0;
    }

    std::string fileToString(const std::filesystem::path& path) {
        https://stackoverflow.com/a/2602258/8733066
        std::ifstream t(path);
        std::stringstream buffer;
        buffer << t.rdbuf();
        return buffer.str();
    }

    glm::vec2 minForEachComponent(const glm::vec2 &a, const glm::vec2 &b) {
        return {std::min(a.x, b.x), std::min(a.y, b.y)};
    }

    glm::vec3 minForEachComponent(const glm::vec3 &a, const glm::vec3 &b) {
        return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
    }

    glm::vec4 minForEachComponent(const glm::vec4 &a, const glm::vec4 &b) {
        return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w)};
    }

    std::string translateBrickLinkColorNameToLDraw(std::string colorName) {
        colorName = util::replaceChar(colorName, ' ', '_');
        colorName = util::replaceChar(colorName, '-', '_');
        util::replaceAll(colorName, "Gray", "Grey");
        return colorName;
    }

    std::string translateLDrawColorNameToBricklink(std::string colorName) {
        colorName = util::replaceChar(colorName, '_', ' ');
        util::replaceAll(colorName, "Grey", "Gray");
        return colorName;
    }

    bool equalsAlphanum(std::string a, std::string b) {
        auto itA = a.cbegin();
        auto itB = b.cbegin();
        while (itA != a.cend() && itB != b.cend()) {
            while (itA != a.cend() && !std::isalnum(*itA)) {
                ++itA;
            }
            while (itB != b.cend() && !std::isalnum(*itB)) {
                ++itB;
            }
            if ((itA==a.cend())!=(itB==b.cend())) {//
                return false;
            }
            if (itA!=a.cend() && *itA != *itB) {
                return false;
            }
            ++itA;
            ++itB;
        }
        return true;
    }

    std::filesystem::path withoutBasePath(const std::filesystem::path &path, const std::filesystem::path &basePath) {
        //todo this can be more efficient
        std::string result = path.string();
        replaceAll(result, basePath.string(), "");
        if (result[0]==PATH_SEPARATOR) {
            result.erase(0, 1);
        }
        return result;
    }

    bool writeImage(const char *path, unsigned char* pixels, unsigned int width, unsigned int height, int channels) {
        auto path_lower = util::asLower(path);
        stbi_flip_vertically_on_write(true);
        if (util::endsWith(path_lower, ".png")) {
            return stbi_write_png(path, width, height, channels, pixels, width * channels)!=0;
        } else if (util::endsWith(path_lower, ".jpg") || util::endsWith(path, ".jpeg")) {
            const int quality = std::min(100, std::max(5, (int) config::getInt(config::JPG_SCREENSHOT_QUALITY)));
            return stbi_write_jpg(path, width, height, channels, pixels, quality)!=0;
        } else if (util::endsWith(path_lower, ".bmp")) {
            return stbi_write_bmp(path, width, height, channels, pixels)!=0;
        } else if (util::endsWith(path_lower, ".tga")) {
            return stbi_write_tga(path, width, height, channels, pixels)!=0;
        } else {
            return false;
        }
    }

    bool containsIgnoreCase(const std::string &full, const std::string &sub) {
        return std::search(
                full.begin(), full.end(),
                sub.begin(), sub.end(),
                [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
        ) != full.end();
    }

    bool isStbiFlipVertically() {
        return isStbiVerticalFlipEnabled;
    }

    void setStbiFlipVertically(bool value) {
        isStbiVerticalFlipEnabled = value;
        stbi_set_flip_vertically_on_load(value?1:0);
    }

    size_t oldWriteFunction(void *ptr, size_t size, size_t nmemb, std::string *data) {
        data->append((char *) ptr, size * nmemb);
        return size * nmemb;
    }

    size_t oldWriteFunction4kb(void *ptr, size_t size, size_t nmemb, std::string *data) {
        data->append((char *) ptr, size * nmemb);
        if (data->size()>4096) {
            return 0;
        }
        return size * nmemb;
    }



    std::pair<int, std::string> requestGET(const std::string &url, bool useCache, size_t sizeLimit, int (*progressFunc)(void*, long, long, long, long)) {
        if (useCache) {
            auto fromCache = db::requestCache::get(url);
            if (fromCache.has_value()) {
                spdlog::info("Request GET {} cache hit", url);
                return {RESPONSE_CODE_FROM_CACHE, fromCache.value()};
            }
        }
        spdlog::info("Request GET {} {}", url, useCache ? " cache miss" : " without cache");
        auto curl = curl_easy_init();
        if (!curl) {
            return {-1, ""};
        };

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_USERPWD, "user:pass");
        curl_easy_setopt(curl, CURLOPT_USERAGENT,"Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/42.0.2311.135 Safari/537.36 Edge/12.10136");//sorry microsoft ;)
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        /*curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [sizeLimit](void *ptr, size_t size, size_t nmemb, std::string *data) -> size_t {
            data->append((char *) ptr, size * nmemb);
            if (sizeLimit > 0 && data->size()>sizeLimit) {
                return 0;
            }
            return size * nmemb;
        });*///todo build something threadsafe which can pass sizeLimit to writeFunction
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (0<sizeLimit&&sizeLimit<4096)?oldWriteFunction4kb:oldWriteFunction);

        if (progressFunc != nullptr) {
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
            curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressFunc);
        } else {
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        }

        std::string response_string;
        std::string header_string;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

        curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        char *effectiveUrl;
        long response_code;
        double elapsed;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
        curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &effectiveUrl);

        if (useCache) {
            db::requestCache::put(url, response_string);
        }

        return {response_code, response_string};
    }

    std::string readFileToString(const std::filesystem::path &path) {
        FILE *f = fopen(path.string().c_str(), "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            long length = ftell(f);
            fseek(f, 0, SEEK_SET);
            std::string buffer;
            buffer.resize(length);
            size_t bytesRead = fread(&buffer[0], 1, length, f);
            if (bytesRead!=length) {
                spdlog::warn("reading file {}: {} bytes read, but reported size is {} bytes.", path.string(), bytesRead, length);
                buffer.resize(bytesRead);
            }
            fclose(f);
            return buffer;
        } else {
            spdlog::error("can't read file {}: ", path.string(), strerror(errno));
            throw std::invalid_argument(strerror(errno));
        }
    }

    void toUpperInPlace(char *string) {
        for (int i = 0; string[i]!=0; ++i) {
            string[i] = toupper(string[i]);
        }
    }
}