#include "file_writer.h"
#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>

namespace bricksim::ldr {
    namespace {
        void writeElements(const std::shared_ptr<File>& file, std::ofstream& stream, std::vector<std::pair<std::string, std::shared_ptr<File>>>& submodels, uoset_t<std::string>& foundSubmodelNames) {
            for (const auto &element : file->elements) {
                auto sfElement = std::dynamic_pointer_cast<SubfileReference>(element);
                if (sfElement != nullptr) {
                    const auto& subfile = sfElement->getFile();
                    if (subfile->metaInfo.type==MPD_SUBFILE && foundSubmodelNames.find(sfElement->filename)== foundSubmodelNames.end()) {
                        submodels.emplace_back(sfElement->filename, subfile);
                        foundSubmodelNames.insert(sfElement->filename);
                    }
                }
                stream << element->getLdrLine() << LDR_NEWLINE;
            }
        }
    }
    void writeFile(const std::shared_ptr<File>& file, const std::filesystem::path& path) {
        std::ofstream stream(path, std::ios::out | std::ios::binary);//not really a binary file, but we don't want the OS to mess with newlines and stuff
        auto nameBackup = file->metaInfo.name;
        file->metaInfo.name = path.filename();
        stream << file->metaInfo;
        file->metaInfo.name = nameBackup;

        stream << LDR_NEWLINE;

        uoset_t<std::string> foundSubmodelNames;
        std::vector<std::pair<std::string, std::shared_ptr<File>>> submodels;
        writeElements(file, stream, submodels, foundSubmodelNames);
        
        for (size_t i = 0; i < submodels.size(); ++i) {
            stream << LDR_NEWLINE << LDR_NEWLINE;

            const auto& subfile = submodels[i].second;
            spdlog::debug("writing submodel {}", subfile->metaInfo.title);

            stream << "0 FILE " << subfile->metaInfo.name << LDR_NEWLINE;

            nameBackup = subfile->metaInfo.name;
            subfile->metaInfo.name = submodels[i].first;
            stream << subfile->metaInfo;
            subfile->metaInfo.name = nameBackup;

            stream << LDR_NEWLINE;

            writeElements(subfile, stream, submodels, foundSubmodelNames);
        }
    }
}
