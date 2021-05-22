#include <spdlog/spdlog.h>
#include <rapidjson/document.h>
#include "bricklink_constants_provider.h"
#include "../helpers/util.h"

namespace bricklink_constants_provider {
    namespace {
        bool initialized = false;

        constexpr long fileSizeEstimated = 1056551;
        float* initialisationProgress;
        int globalConstantsDownloadProgress(void *ptr, long downTotal, long downNow, long upTotal, long upNow) {
            if (initialisationProgress!= nullptr) {
                *initialisationProgress = 0.99f * std::min(downNow, fileSizeEstimated) / fileSizeEstimated;
            }
            return 0;
        }

        std::map<int, bricklink::ColorFamily> colorFamilies;
        std::map<int, bricklink::ColorType> colorTypes;
        std::map<int, bricklink::Color> colors;
        std::map<int, bricklink::Currency> currencies;
        std::map<std::string, bricklink::Country> countries;
        std::map<int, bricklink::Continent> continents;
    }

    void initialize(float *progress) {
        if (initialized) {
            spdlog::warn("bricklink_constants_provider::initialize() already called");
        }

        initialisationProgress = progress;
        *progress = 0.0f;
        auto response = util::requestGET("https://www.bricklink.com/_file/global_constants.js", true, 0, globalConstantsDownloadProgress);

        if (response.first != util::RESPONSE_CODE_FROM_CACHE && (response.first < 200 || response.first >= 300)) {
            throw std::runtime_error(std::string("can't download global bricklink constants. HTTP Status: ") + std::to_string(response.first));
        }

        int startPos = 0;

        if (util::startsWith(response.second, "var _blvarGlobalConstantsNew = {")) {//todo make a better solution with regex
            startPos = 31;
        }

        response.second = response.second.substr(startPos);

        while (response.second.back()!=';') {
            response.second.pop_back();
        }
        response.second.pop_back();


        rapidjson::Document d;
        d.Parse(response.second.c_str());

        assert(d.IsObject());

        for (const auto &colorFamily : d["color_families"].GetArray()) {
            std::vector<int> arrColorsVector;
            const auto arrColors = colorFamily["arrColors"].GetArray();
            for (const auto &arrColor : arrColors) {
                arrColorsVector.push_back(arrColor.GetInt());
            }
            auto id = colorFamily["idColorFamily"].GetInt();
            colorFamilies.insert({id, {id,
                                       colorFamily["strName"].GetString(),
                                       colorFamily["strRGBAHex"].GetString(),
                                       arrColorsVector}});
        }

        for (const auto &item : d["color_types"].GetArray()) {
            auto typeColor = item["typeColor"].GetInt();
            colorTypes.insert({typeColor, {typeColor, item["strName"].GetString()}});
        }

        for (const auto &color : d["colors"].GetArray()) {
            auto idColor = color["idColor"].GetInt();
            const char *strColorName = color["strColorName"].GetString();
            int typeColor = color["typeColor"].GetInt();
            const char *strColorHex = color["strColorHex"].GetString();
            const char *strColorSort = color["strColorSort"].IsNull()?"":color["strColorSort"].GetString();
            colors.insert({idColor, {
                    idColor,
                    strColorName,
                    typeColor,
                    strColorHex,
                    strColorSort}
            });
        }

        for (const auto &currency : d["currencies"].GetArray()) {
            auto idCurrency = currency["idCurrency"].GetInt();
            currencies.insert({idCurrency, {
                idCurrency,
                currency["codeCurrency"].GetString(),
                currency["sign_long"].GetString(),
                currency["sign_short"].GetString(),
                currency["fraction"].GetInt()
            }});
        }

        for (const auto &country : d["countries"].GetArray()) {
            auto codeCountry = country["codeCountry"].GetString();
            countries.insert({codeCountry, {
                codeCountry,
                country["strCountryName"].GetString(),
                country["idCurrency"].GetInt(),
                country["idContinent"].GetInt(),
                country["isEUCountry"].GetBool()
            }});
        }

        for (const auto &continent : d["continents"].GetArray()) {
            auto idContinent = continent["idContinent"].GetInt();
            continents.insert({idContinent, {
                idContinent,
                continent["strContinentName"].GetString()
            }});
        }

        *progress = 1.0f;
        initialized = true;
    }

    const std::map<int, bricklink::ColorFamily> &getColorFamilies() {
        return colorFamilies;
    }

    const std::map<int, bricklink::ColorType> &getColorTypes() {
        return colorTypes;
    }

    const std::map<int, bricklink::Color> &getColors() {
        return colors;
    }

    const std::map<int, bricklink::Currency> &getCurrencies() {
        return currencies;
    }

    const std::map<std::string, bricklink::Country> &getCountries() {
        return countries;
    }

    const std::map<int, bricklink::Continent> &getContinents() {
        return continents;
    }
}