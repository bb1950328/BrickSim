// price_guide_provider.cpp
// Created by bab21 on 03.11.20.
//

#include <string>
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <set>
#include <regex>
#include <fstream>
#include "price_guide_provider.h"
#include "helpers/util.h"

namespace price_guide_provider {
    namespace {
        struct BRCurrency {
            int id;
            std::string name;
            std::string code;
        };

        struct BRColor {
            int id;
            std::string name;
        };

        std::vector<BRCurrency> currencies;
        std::vector<BRColor> colors;

        const BRCurrency *getCurrencyByCode(const std::string &code) {
            for (const auto &currency : currencies) {
                if (currency.code == code) {
                    return &currency;
                }
            }
            return nullptr;
        }

        const BRColor *getColorByName(const std::string &name) {
            for (const auto &color : colors) {
                if (color.name == name) {
                    return &color;
                }
            }
            return nullptr;
        }

        size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string *data) {
            data->append((char *) ptr, size * nmemb);
            return size * nmemb;
        }

        std::pair<int, std::string> requestGET(const std::string &url, bool useCache = true) {
            std::size_t urlHash = std::hash<std::string>{}(url);
            std::filesystem::path cachePath("./requestCache/" + std::to_string(urlHash) + ".txt");

            if (useCache && std::filesystem::is_regular_file(cachePath)) {
                return {0, util::fileToString(cachePath)};
            } else {
                auto curl = curl_easy_init();
                if (curl) {
                    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
                    curl_easy_setopt(curl, CURLOPT_USERPWD, "user:pass");
                    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/42.0.2311.135 Safari/537.36 Edge/12.10136");//sorry microsoft ;)
                    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
                    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

                    std::string response_string;
                    std::string header_string;
                    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
                    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
                    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);

                    curl_easy_perform(curl);
                    curl_easy_cleanup(curl);

                    char *effectiveUrl;
                    long response_code;
                    double elapsed;
                    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
                    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
                    curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &effectiveUrl);

                    std::filesystem::create_directories(cachePath.parent_path());
                    std::ofstream out(cachePath);
                    out << response_string;

                    return {response_code, response_string};
                }
                return {-1, ""};
            }
        }

        std::map<const std::string, int> idItems;

        int getIdItem(const std::string &partCode) {
            auto it = idItems.find(partCode);
            if (it == idItems.end()) {
                auto res = requestGET("https://www.bricklink.com/v2/catalog/catalogitem.page?P=" + partCode);
                std::regex rgx("idItem:\\s+(\\d+)");
                std::smatch matches;
                if (std::regex_search(res.second, matches, rgx)) {
                    int val = std::stoi(matches[1].str());
                    idItems.emplace(partCode, val);
                    return val;
                } else {
                    return -1;
                }
            } else {
                return it->second;
            }
        }
    }

    bool initialize() {
        auto allVarsJs = requestGET("https://www.bricklink.com/js/allVars.js");
        std::stringstream vars;
        vars << allVarsJs.second;
        std::regex currencyRgx(R"(^_varCurrencyList\.push\( \{ idCurrency: (\d+), strCurrencyName: '([a-zA-Z0-9 ]+)', strCurrencyCode: '([A-Z]+)' \} \);\s*$)");
        std::regex colorRgx(R"(^_varColorList\.push\( \{ idColor: (\d+), strColorName: '(.+)', group: (\d+), rgb: '([a-fA-F0-9]*)' \} \);\s*$)");
        for (std::string line; getline(vars, line);) {
            std::smatch matches;
            if (std::regex_search(line, matches, currencyRgx)) {
                currencies.push_back({
                                             std::stoi(matches[1].str()),
                                             matches[2].str(),
                                             matches[3].str()
                                     });
            } else if (std::regex_search(line, matches, colorRgx)) {
                colors.push_back({
                                         std::stoi(matches[1].str()),
                                         matches[2].str()
                                 });
            }
        }

        return true;
    }

    std::vector<PriceGuide> getPriceGuide(const std::string &partCode, const std::string &currencyCode, const std::string &colorName, bool forceRefresh) {
        std::string pgUrl = std::string("https://www.bricklink.com/v2/catalog/catalogitem_pgtab.page?idItem=") + std::to_string(getIdItem(partCode))
                            + "&idColor=" + std::to_string(getColorByName(colorName)->id)
                            + "&st=2&gm=0&gc=0&ei=0&prec=4&showflag=0&showbulk=0&currency=" + std::to_string(getCurrencyByCode(currencyCode)->id);
        auto res = requestGET(pgUrl, !forceRefresh);

        std::regex rgx(
                R"(\s*<TABLE CELLSPACING=0 CELLPADDING=0 CLASS="pcipgSummaryTable"><TR><TD>Total Lots:</TD><TD><b>(\d+)</b></TD></TR><TR><TD>Total Qty:</TD><TD><b>(\d+)</b></TD></TR><TR><TD>Min Price:</TD><TD><b>[A-Za-z]+ ([.\d]+)</b></TD></TR><TR><TD>Avg Price:</TD><TD><b>[A-Za-z]+ ([.\d]+)</b></TD></TR><TR><TD>Qty Avg Price:</TD><TD><b>[A-Za-z]+ ([.\d]+)</b></TD></TR><TR><TD>Max Price:</TD><TD><b>[A-Za-z]+ ([.\d]+)</b></TD></TR></TABLE>\s*)");

        std::vector<PriceGuide> priceGuides;
        std::stringstream html;
        html << res.second;
        for (std::string line; getline(html, line);) {
            std::smatch matches;
            if (std::regex_search(line, matches, rgx)) {
                priceGuides.push_back({currencyCode,
                                       std::stoi(matches[1].str()),
                                       std::stoi(matches[2].str()),
                                       std::stof(matches[3].str()),
                                       std::stof(matches[4].str()),
                                       std::stof(matches[5].str()),
                                       std::stof(matches[6].str()),
                                      });
                if (priceGuides.size()==2) {
                    break;
                }
            }
        }

        return priceGuides;
    }
}