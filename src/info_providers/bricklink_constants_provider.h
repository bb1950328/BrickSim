#pragma once

#include <map>
#include <string>
#include <vector>
#include "../types.h"

namespace bricksim::bricklink {
    struct ColorFamily {
        const int id;
        const std::string strName;
        const std::string strRGBAHex;
        const std::vector<int> arrColors;
    };

    struct ColorType {
        const int typeColor;
        const std::string strName;
    };

    struct Color {
        const int idColor;
        const std::string strColorName;
        const int typeColor;
        const std::string strColorHex;
        const std::string strColorSort;
    };

    struct Currency {
        const int idCurrency;
        const std::string codeCurrency;
        const std::string sign_long;
        const std::string sign_short;
        const int fraction;
    };

    struct Country {
        const std::string codeCountry;
        const std::string strCountryName;
        const int idCurrency;
        const int idContinent;
        const bool isEUCountry;
    };

    struct Continent {
        const int idContinent;
        const std::string strContinentName;
    };
}

namespace bricksim::info_providers::bricklink_constants {
    void initialize(float* progress = nullptr);

    const uomap_t<int, bricklink::ColorFamily>& getColorFamilies();
    const uomap_t<int, bricklink::ColorType>& getColorTypes();
    const uomap_t<int, bricklink::Color>& getColors();
    const uomap_t<int, bricklink::Currency>& getCurrencies();
    const uomap_t<std::string, bricklink::Country>& getCountries();
    const uomap_t<int, bricklink::Continent>& getContinents();
}
