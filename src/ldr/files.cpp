#include "files.h"
#include "../helpers/util.h"
#include "file_repo.h"
#include <iostream>
#include <spdlog/spdlog.h>
#include <sstream>

namespace bricksim::ldr {
    WindingOrder inverseWindingOrder(WindingOrder order) {
        return order == CW ? CCW : CW;
    }

    std::shared_ptr<ldr::FileElement> ldr::FileElement::parse_line(std::string line, BfcState bfcState) {
        std::string line_content = line.length() > 2 ? line.substr(2) : "";
        switch (line[0] - '0') {
                //@formatter:off
            case 0: return std::make_shared<CommentOrMetaElement>(line_content);
            case 1: return std::make_shared<SubfileReference>(line_content, bfcState.invertNext);
            case 2: return std::make_shared<Line>(line_content);
            case 3: return std::make_shared<Triangle>(line_content, bfcState.windingOrder);
            case 4: return std::make_shared<Quadrilateral>(line_content, bfcState.windingOrder);
            case 5: return std::make_shared<OptionalLine>(line_content);
            default: /*throw std::invalid_argument("The line is not valid: \"" + line + "\"");*/
                spdlog::warn("invalid line: {}", line);
                return nullptr;
                //@formatter:on
        }
    }

    FileElement::~FileElement() = default;

    std::shared_ptr<File> File::parseFile(ldr::FileType fileType, const std::string& name, const std::string& content) {
        auto mainFile = std::make_shared<File>();
        mainFile->metaInfo.type = fileType;
        std::stringstream contentStream;
        //todo refactor this function. i think this is faster without stringstream...
        // also the content is copied twice (content -> fileLines -> addTextLine param)
        contentStream << content;
        if (fileType == MODEL) {
            std::string currentSubFileName = name;
            std::map<std::string, std::string> fileLines;
            bool firstFile = true;
            for (std::string line; getline(contentStream, line);) {
                if (util::startsWith(line, "0 FILE")) {
                    if (!firstFile) {
                        currentSubFileName = util::trim(line.substr(7));
                    } else {
                        firstFile = false;
                    }
                } else if (util::startsWith(line, "0 !DATA")) {
                    currentSubFileName = util::trim(line.substr(8));
                } else if (util::startsWith(line, "0 NOFILE")) {
                    currentSubFileName = "";
                } else if (!currentSubFileName.empty()) {
                    fileLines[currentSubFileName] += ("\n" + line);
                }
            }
            for (auto const& entry: fileLines) {
                std::shared_ptr<File> currentFile;
                if (entry.first == name) {
                    currentFile = mainFile;
                } else {
                    if (util::startsWith(entry.second, "0 !: ")) {
                        //todo parse base64 data and store it somewhere
                        continue;
                    } else {
                        currentFile = ldr::file_repo::get().addFileWithContent(entry.first, MPD_SUBFILE, "");
                        mainFile->mpdSubFiles.insert(currentFile);
                    }
                }

                const std::string& currentFileContent = entry.second;
                std::string::size_type pos;
                std::string::size_type prev = 0;
                while ((pos = currentFileContent.find('\n', prev)) != std::string::npos) {
                    currentFile->addTextLine(currentFileContent.substr(prev, pos - prev));
                    prev = pos + 1;
                }
                currentFile->addTextLine(currentFileContent.substr(prev));
            }
        } else {
            for (std::string line; getline(contentStream, line);) {
                mainFile->addTextLine(line);
            }
        }

        return mainFile;
    }

    void File::addTextLine(const std::string& line) {
        auto trimmed = util::trim(line);
        unsigned int currentStep = elements.empty() ? 0 : elements.back()->step;
        if (!trimmed.empty()) {
            auto element = ldr::FileElement::parse_line(trimmed, bfcState);
            if (element != nullptr) {
                bfcState.invertNext = false;
                if (element->getType() == 0) {
                    auto metaElement = std::dynamic_pointer_cast<CommentOrMetaElement>(element);
                    if (metaInfo.addLine(metaElement->content)) {
                        element = nullptr;
                    } else if (metaElement->content == "STEP") {
                        currentStep++;
                    } else if (util::startsWith(metaElement->content, "BFC")) {
                        std::string bfcCommand = util::trim(metaElement->content.substr(3));
                        if (util::startsWith(bfcCommand, "CERTIFY")) {
                            std::string order = util::trim(bfcCommand.substr(7));
                            bfcState.windingOrder = order == "CW" ? CW : CCW;
                            bfcState.active = true;
                        } else if (util::startsWith(bfcCommand, "CLIP")) {
                            std::string order = util::trim(bfcCommand.substr(4));
                            if (order == "CW") {
                                bfcState.windingOrder = CW;
                            } else if (order == "CCW") {
                                bfcState.windingOrder = CCW;
                            }
                            bfcState.active = true;
                        } else if (bfcCommand == "CW") {
                            bfcState.windingOrder = CW;
                            bfcState.active = true;//todo this is never explicitly stated in the standard
                        } else if (bfcCommand == "CCW") {
                            bfcState.windingOrder = CCW;
                            bfcState.active = true;//todo this is never explicitly stated in the standard
                        } else if (bfcCommand == "NOCLIP") {
                            bfcState.active = false;
                        } else if (bfcCommand == "INVERTNEXT") {
                            bfcState.invertNext = true;
                        }
                    }
                }
                if (element != nullptr) {
                    element->step = currentStep;
                    elements.push_back(element);
                }
            }
        }
    }

    void File::printStructure(int indent) {
        for (const auto& elem: elements) {
            if (elem->getType() == 1) {
                auto subfileRef = std::dynamic_pointer_cast<SubfileReference>(elem);
                for (int i = 0; i < indent; ++i) {
                    std::cout << "\t";
                }
                std::cout << subfileRef->filename << "\n";
                subfileRef->getFile()->printStructure(indent + 1);
            }
        }
    }

    void File::preLoadSubfilesAndEstimateComplexity() {
        if (!subfilesPreloadedAndComplexityEstimated) {
            preLoadSubfilesAndEstimateComplexityInternal();
        }
    }

    void File::preLoadSubfilesAndEstimateComplexityInternal() {
        referenceCount++;
        if (!subfilesPreloadedAndComplexityEstimated) {
            for (const auto& elem: elements) {
                if (elem->getType() == 1) {
                    std::shared_ptr<File> subFile = std::dynamic_pointer_cast<SubfileReference>(elem)->getFile();
                    subFile->preLoadSubfilesAndEstimateComplexityInternal();
                    estimatedComplexity += subFile->estimatedComplexity;
                } else if (elem->getType() == 2) {
                    estimatedComplexity += 1;
                } else if (elem->getType() == 3) {
                    estimatedComplexity += 2;
                } else if (elem->getType() == 4) {
                    estimatedComplexity += 3;
                }
            }
            subfilesPreloadedAndComplexityEstimated = true;
        }
    }

    const std::string& File::getDescription() const {
        if (!metaInfo.title.empty()) {
            return metaInfo.title;
        } else if (!metaInfo.name.empty()) {
            return metaInfo.name;
        }
        static std::string unknown = "?";
        return unknown;
    }

    long File::instancedMinComplexity = -1;

    bool File::isComplexEnoughForOwnMesh() const {
        return (metaInfo.type != SUBPART && metaInfo.type != PRIMITIVE);// todo spend more time here, I think there's much more potential here
    }

    const std::size_t& File::getHash() const {
        if (hash == 0) {
            hash = std::hash<std::string>{}(metaInfo.name);
        }
        return hash;
    }

    File::~File() = default;

    CommentOrMetaElement::CommentOrMetaElement(const std::string& line) {
        content = line;
    }

    SubfileReference::SubfileReference(std::string& line, bool bfcInverted) :
        bfcInverted(bfcInverted) {
        char* rest = &line[0];
        char* pch = strtok_r(rest, " \t", &rest);
        color = atoi(pch);
        pch = strtok_r(rest, " \t", &rest);
        x = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        y = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        z = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        a = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        b = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        c = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        d = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        e = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        f = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        g = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        h = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        i = atof(pch);
        filename = util::trim(std::string(rest));
    }

    Line::Line(std::string& line) {
        char* rest = &line[0];
        char* pch = strtok_r(rest, " \t", &rest);
        color = atoi(pch);
        pch = strtok_r(rest, " \t", &rest);
        x1 = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        y1 = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        z1 = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        x2 = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        y2 = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        z2 = atof(pch);
    }

    Triangle::Triangle(std::string& line, WindingOrder order) {
        char* rest = &line[0];
        char* pch = strtok_r(rest, " \t", &rest);
        color = atoi(pch);
        pch = strtok_r(rest, " \t", &rest);
        x1 = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        y1 = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        z1 = atof(pch);
        if (order == CCW) {
            pch = strtok_r(rest, " \t", &rest);
            x2 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            y2 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            z2 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            x3 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            y3 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            z3 = atof(pch);
        } else {
            pch = strtok_r(rest, " \t", &rest);
            x3 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            y3 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            z3 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            x2 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            y2 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            z2 = atof(pch);
        }
    }

    Quadrilateral::Quadrilateral(std::string& line, WindingOrder order) {
        char* rest = &line[0];
        char* pch = strtok_r(rest, " \t", &rest);
        color = atoi(pch);
        pch = strtok_r(rest, " \t", &rest);
        x1 = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        y1 = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        z1 = atof(pch);
        if (order == CCW) {
            pch = strtok_r(rest, " \t", &rest);
            x2 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            y2 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            z2 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            x3 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            y3 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            z3 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            x4 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            y4 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            z4 = atof(pch);
        } else {
            pch = strtok_r(rest, " \t", &rest);
            x4 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            y4 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            z4 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            x3 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            y3 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            z3 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            x2 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            y2 = atof(pch);
            pch = strtok_r(rest, " \t", &rest);
            z2 = atof(pch);
        }
    }

    OptionalLine::OptionalLine(std::string& line) {
        char* rest = &line[0];
        char* pch = strtok_r(rest, " \t", &rest);
        color = atoi(pch);
        pch = strtok_r(rest, " \t", &rest);
        x1 = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        y1 = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        z1 = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        x2 = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        y2 = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        z2 = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        controlX1 = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        controlY1 = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        controlZ1 = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        controlX2 = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        controlY2 = atof(pch);
        pch = strtok_r(rest, " \t", &rest);
        controlZ2 = atof(pch);
    }

    int CommentOrMetaElement::getType() const {
        return 0;
    }

    int SubfileReference::getType() const {
        return 1;
    }

    std::shared_ptr<File> SubfileReference::getFile() {
        if (file == nullptr) {
            file = ldr::file_repo::get().getFile(filename);
        }
        return file;
    }

    glm::mat4 SubfileReference::getTransformationMatrix() const {
        return {
                a, b, c, x,
                d, e, f, y,
                g, h, i, z,
                0.0f, 0.0f, 0.0f, 1.0f};
    }

    int Line::getType() const {
        return 2;
    }

    int Triangle::getType() const {
        return 3;
    }

    int Quadrilateral::getType() const {
        return 4;
    }

    int OptionalLine::getType() const {
        return 5;
    }

    bool ldr::FileMetaInfo::addLine(const std::string& line) {
        if (firstLine) {
            title = util::trim(line);
            firstLine = false;
        } else if (util::startsWith(line, "Name:")) {
            name = util::trim(line.substr(5));
        } else if (util::startsWith(line, "Author:")) {
            author = util::trim(line.substr(7));
        } else if (util::startsWith(line, "!CATEGORY")) {
            category = util::trim(line.substr(9));
        } else if (util::startsWith(line, "!KEYWORDS")) {
            size_t i = 9;
            while (true) {
                size_t next = line.find(',', i);
                if (next == std::string::npos) {
                    keywords.insert(util::trim(line.substr(i)));
                    break;
                }
                keywords.insert(util::trim(line.substr(i, next - i)));
                i = next + 1;
            }
        } else if (util::startsWith(line, "!HISTORY")) {
            history.push_back(util::trim(line.substr(8)));
        } else if (util::startsWith(line, "!LICENSE")) {
            license = line.substr(8);
        } else if (util::startsWith(line, "!THEME")) {
            theme = line.substr(6);
        } else {
            return false;
        }
        return true;
    }

    std::ostream& operator<<(std::ostream& os, const ldr::FileMetaInfo& info) {
        if (!info.title.empty()) {
            os << "0 " << info.title << std::endl;
        }
        if (!info.name.empty()) {
            os << "0 Name: " << info.name << std::endl;
        }
        if (!info.author.empty()) {
            os << "0 Author: " << info.author << std::endl;
        }
        if (!info.category.empty()) {
            os << "0 !CATEGORY " << info.category << std::endl;
        }
        if (!info.keywords.empty()) {
            os << "0 !KEYWORDS ";
            size_t lineWidth = 13;
            bool first = true;
            for (const auto& kw: info.keywords) {
                if (!first) {
                    lineWidth += 2;
                }
                lineWidth += kw.size();
                if (lineWidth > 80) {
                    os << std::endl
                       << "0 !KEYWORDS ";
                    lineWidth = 13 + kw.size();
                    first = true;
                }
                if (!first) {
                    os << ", ";
                }
                os << kw;
                first = false;
            }
            os << std::endl;
        }
        if (!info.history.empty()) {
            for (const auto& historyElement: info.history) {
                os << "0 !HISTORY " << historyElement << std::endl;
            }
        }
        if (!info.license.empty()) {
            os << "0 !LICENSE " << info.license << std::endl;
        }
        if (!info.theme.empty()) {
            os << "0 !THEME " << info.theme << std::endl;
        }
        return os;
    }

    const std::string& ldr::FileMetaInfo::getCategory() {
        if (category == "????") {
            const auto firstSpace = title.find(' ');
            auto start = 0;
            while (title[start] == '_' || title[start] == '~' || title[start] == '=') {
                start++;
            }
            category = title.substr(start, firstSpace - start);
        }
        return category;
    }
}