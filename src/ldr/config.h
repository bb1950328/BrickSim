#pragma once

#include "files.h"
#include <memory>
namespace bricksim::ldr {
    struct Avatar {
        Avatar(std::string_view line);
        std::string category;
        std::string description;
        std::array<float, 9> rotation;
        std::string fileName;
    };

    class LDConfig {
        std::weak_ptr<File> file;
        std::string updateDate;
        std::vector<std::shared_ptr<Color>> colors;
        std::vector<Avatar> avatars;
    public:
        explicit LDConfig(const std::shared_ptr<File>& file);
        std::shared_ptr<File> getFile() const;
        const std::string& getUpdateDate() const;
        const std::vector<std::shared_ptr<Color>>& getColors() const;
        const std::vector<Avatar>& getAvatars() const;
    };

    const LDConfig& getConfig();
}