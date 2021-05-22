#include <mutex>
#include <regex>
#include "price_guide_provider.h"
#include "bricklink_constants_provider.h"
#include "../helpers/util.h"
#include "../db.h"

namespace price_guide_provider {
    namespace {

        std::optional<bricklink::Currency> getCurrencyByCode(const std::string &code) {
            for (const auto &currency : bricklink_constants_provider::getCurrencies()) {
                if (currency.second.codeCurrency == code) {
                    return {currency.second};
                }
            }
            return {};
        }

        std::optional<bricklink::Color> getColorByName(const std::string &name) {
            for (const auto &color : bricklink_constants_provider::getColors()) {
                if (util::equalsAlphanum(color.second.strColorName, name)) {
                    return {color.second};
                }
            }
            return {};
        }

        std::map<const std::string, int> idItems;

        int getIdItem(const std::string &partCode) {
            //todo save in cache db
            static std::mutex lock;
            std::lock_guard<std::mutex> lg(lock);
            auto it = idItems.find(partCode);
            if (it == idItems.end()) {
                auto res = util::requestGET("https://www.bricklink.com/v2/catalog/catalogitem.page?P=" + partCode);
                // todo http error handling
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
        return true;
    }

    PriceGuide getPriceGuide(const std::string &partCode, const std::string &currencyCode, const std::string &colorName, bool forceRefresh) {
        if (!forceRefresh) {
            auto cacheValue = db::priceGuideCache::get(partCode, currencyCode, colorName);
            if (cacheValue.has_value()) {
                return cacheValue.value();
            }
        }
        //https://www.bricklink.com/v2/catalog/catalogitem_pgtab.page?idItem=38562&idColor=11&st=2&gm=1&gc=0&ei=0&prec=2&showflag=0&showbulk=0&currency=136
        const std::string &partIdStr = std::to_string(getIdItem(partCode));
        const std::string &colorCodeStr = std::to_string(getColorByName(colorName)->idColor);
        const std::string &currencyCodeStr = std::to_string(getCurrencyByCode(currencyCode)->idCurrency);
        std::string pgUrl = std::string("https://www.bricklink.com/v2/catalog/catalogitem_pgtab.page?idItem=") + partIdStr + "&idColor=" + colorCodeStr +
                            "&st=2&gm=0&gc=0&ei=0&prec=4&showflag=0&showbulk=0&currency=" + currencyCodeStr;
        auto res = util::requestGET(pgUrl,false, 3500);//don't use cache because relevant numbers are saved in priceGuideCache
        std::regex rgx(
                R"(\s*<TABLE CELLSPACING=0 CELLPADDING=0 CLASS="pcipgSummaryTable"><TR><TD>Total Lots:</TD><TD><b>(\d+)</b></TD></TR><TR><TD>Total Qty:</TD><TD><b>(\d+)</b></TD></TR><TR><TD>Min Price:</TD><TD><b>[A-Za-z]+ ([.\d]+)</b></TD></TR><TR><TD>Avg Price:</TD><TD><b>[A-Za-z]+ ([.\d]+)</b></TD></TR><TR><TD>Qty Avg Price:</TD><TD><b>[A-Za-z]+ ([.\d]+)</b></TD></TR><TR><TD>Max Price:</TD><TD><b>[A-Za-z]+ ([.\d]+)</b></TD></TR></TABLE>\s*)");

        std::stringstream html;
        html << res.second;
        for (std::string line; getline(html, line);) {
            std::smatch matches;
            if (std::regex_search(line, matches, rgx)) {
                PriceGuide value{true,
                                 currencyCode,
                                 std::stoi(matches[1].str()),
                                 std::stoi(matches[2].str()),
                                 std::stof(matches[3].str()),
                                 std::stof(matches[4].str()),
                                 std::stof(matches[5].str()),
                                 std::stof(matches[6].str()),
                };
                db::priceGuideCache::put(partCode, currencyCode, colorName, value);
                return value;
            }
        }

        PriceGuide val;
        val.available = false;
        val.currency = currencyCode;
        db::priceGuideCache::put(partCode, currencyCode, colorName, val);
        return val;
    }

    std::optional<PriceGuide> getPriceGuideIfCached(const std::string &partCode, const std::string &currencyCode, const std::string &colorName) {
        return db::priceGuideCache::get(partCode, currencyCode, colorName);
    }
}