#include "config.h"
#include "fast_float/fast_float.h"
#include "file_repo.h"
#include "spdlog/spdlog.h"

namespace bricksim::ldr {
    namespace {
        std::unique_ptr<LDConfig> config;
        std::shared_ptr<File> getCurrentFileFromRepo() {
            return file_repo::get().getFile((std::shared_ptr<FileNamespace>)nullptr, constants::LDRAW_CONFIG_FILE_NAME);
        }

    }

    const LDConfig& getConfig() {
        std::shared_ptr<File> currentFile;
        if (config!= nullptr) {
            currentFile = getCurrentFileFromRepo();
            if (currentFile != config->getFile()) {
                config = nullptr;
            }
        }
        if (config == nullptr) {
            if (currentFile == nullptr) {
                currentFile = getCurrentFileFromRepo();
            }
            config = std::make_unique<LDConfig>(currentFile);
        }
        return *config;
    }
    LDConfig::LDConfig(const std::shared_ptr<File>& file) :
            file(file) {
        const std::string_view updateLine = file->metaInfo.fileTypeLine;
        const auto startIdx = updateLine.find_first_not_of(LDR_WHITESPACE, updateLine.find("UPDATE")+6);
        const auto endIdx = updateLine.find_first_of(LDR_WHITESPACE, startIdx);
        updateDate = updateLine.substr(startIdx, endIdx);

        for (const auto& element: file->elements) {
            if (element->getType() == 0) {
                const auto& content = std::dynamic_pointer_cast<CommentOrMetaElement>(element)->content;
                const auto contentTrimmed = stringutil::trim(std::string_view(content));
                if (contentTrimmed.starts_with("!COLOUR")) {
                    colors.push_back(std::make_shared<Color>(stringutil::trim(contentTrimmed.substr(strlen("!COLOUR")))));
                } else if (contentTrimmed.starts_with("!LDRAW_ORG Configuration UPDATE")) {

                } else if (contentTrimmed.starts_with("!AVATAR")) {
                    try {
                        avatars.emplace_back(stringutil::trim(contentTrimmed.substr(strlen("!AVATAR"))));
                    } catch (std::invalid_argument) {
                        spdlog::warn("invalid avatar found in ldraw config file: {}", contentTrimmed);
                    }
                }
            }
        }
    }
    std::shared_ptr<File> LDConfig::getFile() const {
        return file.lock();
    }
    const std::string& LDConfig::getUpdateDate() const {
        return updateDate;
    }
    const std::vector<std::shared_ptr<Color>>& LDConfig::getColors() const {
        return colors;
    }
    const std::vector<Avatar>& LDConfig::getAvatars() const {
        return avatars;
    }

    Avatar::Avatar(std::string_view line) {
        std::size_t start;
        std::size_t end = 0;

        if (!line.starts_with("CATEGORY")) {
            throw std::invalid_argument("");
        }
        start = line.find('"')+1;
        end = stringutil::findClosingQuote(line, start);
        if (end==std::string_view::npos) {
            throw std::invalid_argument("");
        }
        category = line.substr(start, end);

        start = line.find_first_not_of(LDR_WHITESPACE, end+1);
        if (!line.substr(start).starts_with("DESCRIPTION")) {
            throw std::invalid_argument("");
        }
        start = line.find('"', start)+1;
        end = stringutil::findClosingQuote(line, start);
        if (end==std::string_view::npos) {
            throw std::invalid_argument("");
        }
        description = line.substr(start, end);

        start = line.find_first_not_of(LDR_WHITESPACE, end+1);
        if (!line.substr(start).starts_with("PART")) {
            throw std::invalid_argument("");
        }
        end = line.find_first_of(LDR_WHITESPACE, start);
        for (int i = 0; i < 9; ++i) {
            start = line.find_first_not_of(LDR_WHITESPACE, end);
            end = std::min(line.size(), line.find_first_of(LDR_WHITESPACE, start));
            fast_float::from_chars(line.data()+start, line.data()+end, rotation[i]);
        }

        start = line.find('"', end)+1;
        end = std::min(line.size(), stringutil::findClosingQuote(line, start));
        fileName = line.substr(start, end);
    }
}