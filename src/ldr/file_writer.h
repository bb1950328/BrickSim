#pragma once

#include "files.h"
#include <filesystem>
namespace bricksim::ldr {
    namespace {
        void writeElements(const std::shared_ptr<File>& file, std::ofstream& stream, std::vector<std::pair<std::string, std::shared_ptr<File>>>& submodels, uoset_t<std::string>& foundSubmodelNames);
    }

    void writeFile(const std::shared_ptr<File>& file, const std::filesystem::path& path);
}