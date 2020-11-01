// part_color_availability_provider.cpp
// Created by bab21 on 01.11.20.
//

#include <fstream>
#include <iostream>
#include "part_color_availability_provider.h"

namespace part_color_availability_provider {

    namespace {
        std::map<std::string, const LdrColor*> colorsByName;
        std::map<const std::string, std::set<const LdrColor*>> colorsAvailable;

        bool isDataAvailable = false;
        void ensureDataLoaded() {
            static bool initialized = false;
            if (!initialized) {
                std::ifstream codesFile("codes.txt");
                if (!codesFile.good()) {
                    std::cout << "WARNING: codes.txt not found" << std::endl;
                    isDataAvailable = false;
                } else {
                    isDataAvailable = true;

                    for (const auto &item : ldr_color_repo::getColors()) {
                        colorsByName[item.second.name] = &item.second;
                    }

                    std::string line;
                    std::getline(codesFile, line);//skip header
                    while (std::getline(codesFile, line)) {
                        auto firstTab = line.find('\t');
                        auto secondTab = line.find('\t', firstTab+1);
                        auto partCode = line.substr(0, firstTab);
                        auto colorName = line.substr(firstTab+1, secondTab-firstTab-1);
                        auto it = colorsByName.find(colorName);
                        if (it != colorsByName.end()) {
                            colorsAvailable[partCode].insert(it->second);
                        } else {
                            std::cout << "WARNING: found color \"" << colorName << "\" in codes.txt, but not in ldr_colors" << std::endl;
                        }
                    }

                    for (const auto &col : colorsAvailable["3001"]) {
                        std::cout << col->name << std::endl;
                    }
                }
                initialized = true;
            }
        }
    }

    std::optional<std::set<const LdrColor *>> getAvailableColorsForPart(LdrFile *part) {
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
