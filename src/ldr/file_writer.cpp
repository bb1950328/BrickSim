#include "file_writer.h"
#include <filesystem>
#include <fstream>

namespace bricksim::ldr {
    namespace {
        void writeElements(const std::shared_ptr<File>& file, std::ofstream& stream, std::vector<std::pair<std::string, std::shared_ptr<File>>>& submodels) {
            for (const auto &element : file->elements) {
                auto sfElement = std::dynamic_pointer_cast<SubfileReference>(element);
                if (sfElement != nullptr) {
                    const auto& subfile = sfElement->getFile();
                    if (subfile->metaInfo.type==MPD_SUBFILE) {
                        submodels.emplace_back(sfElement->filename, subfile);
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

        std::vector<std::pair<std::string, std::shared_ptr<File>>> submodels;
        writeElements(file, stream, submodels);
        
        for (size_t i = 0; i < submodels.size(); ++i) {
            stream << LDR_NEWLINE << LDR_NEWLINE;

            const auto& subfile = submodels[i].second;
            nameBackup = subfile->metaInfo.name;
            subfile->metaInfo.name = submodels[i].first;
            stream << subfile->metaInfo;
            subfile->metaInfo.name = nameBackup;

            stream << LDR_NEWLINE;

            writeElements(file, stream, submodels);
        }
    }
}
