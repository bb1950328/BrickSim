

#include <fstream>
#include <iostream>
#include <spdlog/spdlog.h>
#include "part_color_availability_provider.h"

namespace part_color_availability_provider {

    namespace {
        std::map<std::string, LdrColorReference> colorsByName;
        std::map<const std::string, std::set<LdrColorReference>> colorsAvailable;

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

                    for (const auto &item : ldr_color_repo::getColors()) {
                        colorsByName[item.second->name] = item.second->asReference();
                    }

                    std::set<std::string> colorsInCodesTxtButNotInLdrColors;

                    std::string line;
                    std::getline(codesFile, line);//skip header
                    while (std::getline(codesFile, line)) {
                        auto firstTab = line.find('\t');
                        auto secondTab = line.find('\t', firstTab+1);
                        auto partCode = line.substr(0, firstTab);
                        auto colorName = line.substr(firstTab+1, secondTab-firstTab-1);
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

    std::optional<std::set<LdrColorReference>> getAvailableColorsForPart(const std::shared_ptr<LdrFile>& part) {
        ensureDataLoaded();
        std::string partCode = part->metaInfo.name;
        util::replaceAll(partCode, ".dat", "");
        auto it = colorsAvailable.find(partCode);
        if (it==colorsAvailable.end()) {
            return {};
        } else {
            return it->second;
        }
    }

}
