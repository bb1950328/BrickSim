#include "util.h"
#include "../config/read.h"
#include "../constant_data/constants.h"
#include "../db.h"
#include "glm/gtc/matrix_transform.hpp"
#include "platform_detection.h"
#include "stringutil.h"
#include <array>
#include <cstring>
#include <curl/curl.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <magic_enum.hpp>
#include <palanteer.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <stb_image.h>
#include <stb_image_write.h>

#ifdef BRICKSIM_PLATFORM_WINDOWS
    #include <windows.h>
#ifdef min
        #undef min
#endif
#ifdef max
        #undef max
#endif
#elif defined(BRICKSIM_PLATFORM_LINUX) || defined(BRICKSIM_PLATFORM_MACOS)
#include <pthread.h>
#endif

namespace bricksim::util {
    namespace {
        bool isStbiVerticalFlipEnabled = false;
    }

    std::string extendHomeDir(const std::string& input) {
        return replaceSpecialPaths(input).string();
    }

    std::filesystem::path replaceSpecialPaths(const std::string& input) {
        if (!input.empty()) {
            if (input[0] == '~') {
                const auto homeDir = getenv(USER_ENV_VAR);
                if (homeDir != nullptr) {
                    if (input.size() == 1) {
                        return {homeDir};
                    }
                    if (input[1] == '/' || input[1] == '\\') {
                        return std::filesystem::path(homeDir) / std::filesystem::path(input.substr(2));
                    }
                }
            } else if (input.starts_with("{tmp}")) {
                if (input.size() == 5) {
                    return std::filesystem::temp_directory_path();
                } else if (input[5] == '/' || input[5] == '\\') {
                    return std::filesystem::temp_directory_path() / input.substr(6);
                }
            }
        }
        return {input};
    }

    uomap_t<std::string, std::filesystem::path> getSpecialPaths() {
        return {
                {"~", getenv(USER_ENV_VAR)},
                {"{tmp}", std::filesystem::temp_directory_path()},
        };
    }

    void openDefaultBrowser(const std::string& link) {
        spdlog::info("openDefaultBrowser(\"{}\")", link);
        #ifdef BRICKSIM_PLATFORM_WINDOWS
        ShellExecute(nullptr, "open", link.c_str(), nullptr, nullptr, SW_SHOWNORMAL);//todo testing
        #elif defined(BRICKSIM_PLATFORM_MACOS)
        std::string command = std::string("open ") + link;
        #elif defined(BRICKSIM_PLATFORM_LINUX)
        std::string command = std::string("xdg-open ") + link;
        #else
    #warning "openDefaultProwser not supported on this platform"
        #endif
        #if defined(BRICKSIM_PLATFORM_LINUX) || defined(BRICKSIM_PLATFORM_MACOS)
        int exitCode = system(command.c_str());
        if (exitCode != 0) {
            spdlog::warn("command \"{}\" exited with code {}", command, exitCode);
        }
        #endif
    }

    bool memeqzero(const void* data, size_t length) {
        //from https://github.com/rustyrussell/ccan/blob/master/ccan/mem/mem.c#L92
        const auto* p = static_cast<const unsigned char*>(data);

        /* Check first 16 bytes manually */
        for (size_t len = 0; len < 16; len++) {
            if (!length) {
                return true;
            }
            if (*p) {
                return false;
            }
            p++;
            length--;
        }

        /* Now we know that's zero, memcmp with self. */
        return memcmp(data, p, length) == 0;
    }

    std::string translateBrickLinkColorNameToLDraw(std::string colorName) {
        colorName = stringutil::replaceChar(colorName, ' ', '_');
        colorName = stringutil::replaceChar(colorName, '-', '_');
        stringutil::replaceAll(colorName, "Gray", "Grey");
        return colorName;
    }

    std::string translateLDrawColorNameToBricklink(std::string colorName) {
        colorName = stringutil::replaceChar(colorName, '_', ' ');
        stringutil::replaceAll(colorName, "Grey", "Gray");
        return colorName;
    }

    std::filesystem::path withoutBasePath(const std::filesystem::path& path, const std::filesystem::path& basePath) {
        auto itPath = path.begin();
        auto itBase = basePath.begin();
        while (itPath != path.end() && itBase != basePath.end() && *itPath == *itBase) {
            ++itPath;
            ++itBase;
        }
        std::filesystem::path result;
        std::for_each(itPath, path.end(), [&result](auto part) {
            result /= part;
        });
        return result;
    }

    size_t oldWriteFunction(void* ptr, size_t size, size_t nmemb, std::string* data) {
        data->append((char*)ptr, size * nmemb);
        return size * nmemb;
    }

    size_t oldWriteFunction4kb(void* ptr, size_t size, size_t nmemb, std::string* data) {
        data->append((char*)ptr, size * nmemb);
        if (data->size() > 4096) {
            return 0;
        }
        return size * nmemb;
    }

    std::pair<int, std::string> requestGET(const std::string& url, bool useCache, size_t sizeLimit, int (*progressFunc)(void*, long, long, long, long)) {
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
        }
        char errorBuffer[CURL_ERROR_SIZE + 1];
        errorBuffer[0] = '\0';

        curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_USERPWD, "user:pass");
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/42.0.2311.135 Safari/537.36 Edge/12.10136");//sorry microsoft ;)
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_AUTOREFERER, true);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 10'000L);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);

        /*curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [sizeLimit](void *ptr, size_t size, size_t nmemb, std::string *data) -> size_t {
            data->append((char *) ptr, size * nmemb);
            if (sizeLimit > 0 && data->size()>sizeLimit) {
                return 0;
            }
            return size * nmemb;
        });*/
        //todo build something threadsafe which can pass sizeLimit to writeFunction
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (0 < sizeLimit && sizeLimit < 4096) ? oldWriteFunction4kb : oldWriteFunction);

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

        const CURLcode result = curl_easy_perform(curl);

        if (result != CURLE_OK) {
            spdlog::error("cURL error on GET request to {}: {} | {}", url, magic_enum::enum_name(result), errorBuffer);
        }

        char* effectiveUrl;
        long response_code;
        double elapsed;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
        curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &effectiveUrl);

        if (useCache && 100 <= response_code && response_code < 400 && result == CURLE_OK && !response_string.empty()) {
            db::requestCache::put(url, response_string);
        }

        curl_easy_cleanup(curl);

        return {response_code, response_string};
    }

    size_t writeFunctionToFile(void* ptr, size_t size, size_t nmemb, FILE* stream) {
        size_t written = fwrite(ptr, size, nmemb, stream);
        return written;
    }

    std::pair<int, std::string> downloadFile(const std::string& url, const std::filesystem::path targetFile, int (*progressFunc)(void*, long, long, long, long)) {
        CURL* curl = curl_easy_init();
        if (curl) {
            FILE* fp = fopen(targetFile.string().c_str(), "wb");
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunctionToFile);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

            if (progressFunc != nullptr) {
                curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
                curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressFunc);
            } else {
                curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
            }

            CURLcode res = curl_easy_perform(curl);

            curl_easy_cleanup(curl);
            fclose(fp);
        }
        return {234, "TODO"};
    }

    std::string readFileToString(const std::filesystem::path& path) {
        FILE* f = fopen(path.string().c_str(), "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            unsigned long length = ftell(f);
            fseek(f, 0, SEEK_SET);
            std::string buffer;
            buffer.resize(length);
            size_t bytesRead = fread(&buffer[0], 1, length, f);
            if (bytesRead != length) {
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

    DecomposedTransformation decomposeTransformationToStruct(const glm::mat4& transformation) {
        DecomposedTransformation result{};
        glm::decompose(transformation, result.scale, result.orientation, result.translation, result.skew, result.perspective);
        return result;
    }

    bool isUvInsideImage(const glm::vec2& uv) {
        return 0 <= uv.x && uv.x <= 1 && 0 <= uv.y && uv.y <= 1;
    }

    std::string randomAlphanumString(uint64_t length) {
        std::string result;
        result.reserve(length);

        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<> distribution(0, static_cast<int>(ALPHANUM_CHARS.size()) - 1);

        for (size_t i = 0; i < length; ++i) {
            result += ALPHANUM_CHARS[distribution(rng)];
        }

        return result;
    }

    std::string escapeFilename(const std::string& original) {
        std::string result = original;
        auto idx = result.find_first_of(constants::DISALLOWED_FILENAME_CHARS);
        while (idx != std::string::npos) {
            result[idx] = '_';
            idx = result.find_first_of(constants::DISALLOWED_FILENAME_CHARS, idx);
        }
        return result;
    }

    void setThreadName(const char* threadName) {
        #ifdef BRICKSIM_PLATFORM_LINUX
        pthread_setname_np(pthread_self(), threadName);
        #elif defined(BRICKSIM_PLATFORM_MACOS)
        pthread_setname_np(threadName);
        #endif
        #ifdef USE_PL
        plDeclareThreadDyn(threadName);
        #endif
    }

    UtfType determineUtfTypeFromBom(const std::string_view text) {
        if (text.starts_with(constants::UTF8_BOM)) {
            return {8,
                    static_cast<std::uint8_t>(std::strlen(constants::UTF8_BOM)),
                    std::endian::native};
        } else if (text.starts_with(constants::UTF16BE_BOM)) {
            return {16,
                    static_cast<std::uint8_t>(std::strlen(constants::UTF16BE_BOM)),
                    std::endian::big};
        } else if (text.starts_with(constants::UTF16LE_BOM)) {
            return {16,
                    static_cast<std::uint8_t>(std::strlen(constants::UTF16LE_BOM)),
                    std::endian::little};
        } else if (text.starts_with(constants::UTF32BE_BOM)) {
            return {32,
                    static_cast<std::uint8_t>(std::strlen(constants::UTF32BE_BOM)),
                    std::endian::big};
        } else if (text.starts_with(constants::UTF32LE_BOM)) {
            return {32,
                    static_cast<std::uint8_t>(std::strlen(constants::UTF32LE_BOM)),
                    std::endian::little};
        } else {
            return {0,
                    0,
                    std::endian::native};
        }
    }

    bool isStbiFlipVertically() {
        return isStbiVerticalFlipEnabled;
    }

    void setStbiFlipVertically(bool value) {
        isStbiVerticalFlipEnabled = value;
        stbi_set_flip_vertically_on_load(value ? 1 : 0);
    }

    unsigned char* RawImage::getPixel(uint16_t x, uint16_t y) {
        return data.data() + x + width * y;
    }

    RawImage readImage(std::span<const uint8_t> fileData) {
        const auto vFlipBackup = isStbiFlipVertically();
        setStbiFlipVertically(false);

        RawImage result;
        auto* rawData = stbi_load_from_memory(fileData.data(), fileData.size(), &result.width, &result.height, &result.channels, 0);
        result.data.assign(rawData, rawData + result.width * result.height * result.channels);

        stbi_image_free(rawData);
        setStbiFlipVertically(vFlipBackup);
        return result;
    }

    bool writeImage(const char* path, const RawImage& image) {
        return writeImage(path, image.data.data(), image.width, image.height, image.channels);
    }

    bool writeImage(const char* path, const unsigned char* pixels, int width, int height, int channels) {
        auto path_lower = stringutil::asLower(std::string_view(path));
        stbi_flip_vertically_on_write(true);
        if (path_lower.ends_with(".png")) {
            return stbi_write_png(path, width, height, channels, pixels, width * channels) != 0;
        } else if (path_lower.ends_with(".jpg") || path_lower.ends_with(".jpeg")) {
            const int quality = std::min(100, std::max(5, config::get().graphics.jpgScreenshotQuality));
            return stbi_write_jpg(path, width, height, channels, pixels, quality) != 0;
        } else if (path_lower.ends_with(".bmp")) {
            return stbi_write_bmp(path, width, height, channels, pixels) != 0;
        } else if (path_lower.ends_with(".tga")) {
            return stbi_write_tga(path, width, height, channels, pixels) != 0;
        } else {
            return false;
        }
    }

    glm::mat4 DecomposedTransformation::orientationAsMat4() const {
        return glm::toMat4(orientation);
    }

    glm::mat4 DecomposedTransformation::translationAsMat4() const {
        return glm::translate(glm::mat4(1.0f), translation);
    }

    glm::mat4 DecomposedTransformation::scaleAsMat4() const {
        return glm::scale(glm::mat4(1.0f), scale);
    }
}
