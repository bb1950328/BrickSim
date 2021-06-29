#pragma once

#include <map>
#include <string>
#include <vector>

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

    const std::map<int, bricklink::ColorFamily>& getColorFamilies();
    const std::map<int, bricklink::ColorType>& getColorTypes();
    const std::map<int, bricklink::Color>& getColors();
    const std::map<int, bricklink::Currency>& getCurrencies();
    const std::map<std::string, bricklink::Country>& getCountries();
    const std::map<int, bricklink::Continent>& getContinents();
}
