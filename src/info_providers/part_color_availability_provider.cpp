#include "part_color_availability_provider.h"
#include "../helpers/stringutil.h"
#include "../helpers/util.h"
#include <fstream>
#include <spdlog/spdlog.h>

namespace bricksim::info_providers::part_color_availability {

    namespace {
        uomap_t<std::string, ldr::ColorReference> colorsByName;
        uomap_t<std::string, uoset_t<ldr::ColorReference>> colorsAvailable{};

        bool isDataAvailable = false;

        void ensureDataLoaded() {
            static bool initialized = false;
            if (!initialized) {
                std::ifstream codesFile("codes.txt");
                if (!codesFile.good()) {
                    spdlog::warn("codes.txt not found");
                    isDataAvailable = false;
                } else {
                    isDataAvailable = true;

                    for (const auto& item: ldr::color_repo::getColors()) {
                        colorsByName[item.second->name] = item.second->asReference();
                    }

                    uoset_t<std::string> colorsInCodesTxtButNotInLdrColors;

                    std::string line;
                    std::getline(codesFile, line);//skip header
                    while (std::getline(codesFile, line)) {
                        auto firstTab = line.find('\t');
                        auto secondTab = line.find('\t', firstTab + 1);
                        auto partCode = line.substr(0, firstTab);
                        auto colorName = line.substr(firstTab + 1, secondTab - firstTab - 1);
                        colorName = util::translateBrickLinkColorNameToLDraw(colorName);
                        auto it = colorsByName.find(colorName);
                        if (it != colorsByName.end()) {
                            colorsAvailable[partCode].insert(it->second);
                        } else {
                            colorsInCodesTxtButNotInLdrColors.insert(colorName);
                        }
                    }
                    if (!colorsInCodesTxtButNotInLdrColors.empty()) {
                        spdlog::warn("the following color names were found in codes.txt, bui not in ldr_colors: {}", fmt::join(colorsInCodesTxtButNotInLdrColors, ", "));
                    }
                }
                initialized = true;
            }
        }
    }

    std::optional<uoset_t<ldr::ColorReference>> getAvailableColorsForPart(const std::shared_ptr<ldr::File>& part) {
        ensureDataLoaded();
        std::string partCode = part->metaInfo.name;
        stringutil::replaceAll(partCode, ".dat", "");
        auto it = colorsAvailable.find(partCode);
        if (it == colorsAvailable.end()) {
            return {};
        } else {
            return it->second;
        }
    }

}
