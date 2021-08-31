#include "files.h"
#include "../helpers/util.h"
#include "file_repo.h"
#include <fast_float/fast_float.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <sstream>

namespace bricksim::ldr {
    WindingOrder inverseWindingOrder(WindingOrder order) {
        return order == CW ? CCW : CW;
    }

    std::shared_ptr<FileElement> FileElement::parseLine(const std::string& line, BfcState bfcState) {
        std::string lineContent = line.length() > 2 ? line.substr(2) : "";
        switch (line[0] - '0') {
            case 0: return std::make_shared<CommentOrMetaElement>(lineContent);
            case 1: return std::make_shared<SubfileReference>(lineContent, bfcState.invertNext);
            case 2: return std::make_shared<Line>(lineContent);
            case 3: return std::make_shared<Triangle>(lineContent, bfcState.windingOrder);
            case 4: return std::make_shared<Quadrilateral>(lineContent, bfcState.windingOrder);
            case 5: return std::make_shared<OptionalLine>(lineContent);
            default: /*throw std::invalid_argument("The line is not valid: \"" + line + "\"");*/
                spdlog::warn("invalid line: {}", line);
                return nullptr;
        }
    }

    FileElement::~FileElement() = default;

    void File::addTextLine(const std::string& line) {
        auto trimmed = util::trim(line);
        unsigned int currentStep = elements.empty() ? 0 : elements.back()->step;
        if (!trimmed.empty()) {
            auto element = ldr::FileElement::parseLine(trimmed, bfcState);
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

    const std::string& File::getDescription() const {
        if (!metaInfo.title.empty()) {
            return metaInfo.title;
        } else if (!metaInfo.name.empty()) {
            return metaInfo.name;
        }
        static std::string unknown = "?";
        return unknown;
    }

    const std::size_t& File::getHash() const {
        if (hash == 0) {
            hash = robin_hood::hash<std::string>{}(metaInfo.name);
        }
        return hash;
    }

    File::~File() = default;

    CommentOrMetaElement::CommentOrMetaElement(const std::string& line) {
        content = line;
    }

    inline void parseNextFloat(const std::string& line, size_t& start, size_t& end, float& result) {
        start = line.find_first_not_of(LDR_WHITESPACE, end);
        end = line.find_first_of(LDR_WHITESPACE, start);
        fast_float::from_chars(&line[start], &line[end], result);
    }

    SubfileReference::SubfileReference(const std::string& line, bool bfcInverted) :
        bfcInverted(bfcInverted) {
        size_t start = line.find_first_not_of(LDR_WHITESPACE);
        size_t end = line.find_first_of(LDR_WHITESPACE, start);
        color = std::atoi(line.c_str());
        parseNextFloat(line, start, end, x);
        parseNextFloat(line, start, end, y);
        parseNextFloat(line, start, end, z);
        parseNextFloat(line, start, end, a);
        parseNextFloat(line, start, end, b);
        parseNextFloat(line, start, end, c);
        parseNextFloat(line, start, end, d);
        parseNextFloat(line, start, end, e);
        parseNextFloat(line, start, end, f);
        parseNextFloat(line, start, end, g);
        parseNextFloat(line, start, end, h);
        parseNextFloat(line, start, end, i);
        filename = util::trim(line.substr(end + 1));
    }

    Line::Line(const std::string& line) {
        size_t start = line.find_first_not_of(LDR_WHITESPACE);
        size_t end = line.find_first_of(LDR_WHITESPACE, start);
        color = std::atoi(line.c_str());
        parseNextFloat(line, start, end, x1);
        parseNextFloat(line, start, end, y1);
        parseNextFloat(line, start, end, z1);
        parseNextFloat(line, start, end, x2);
        parseNextFloat(line, start, end, y2);
        parseNextFloat(line, start, end, z2);
    }

    Triangle::Triangle(const std::string& line, WindingOrder order) {
        size_t start = line.find_first_not_of(LDR_WHITESPACE);
        size_t end = line.find_first_of(LDR_WHITESPACE, start);
        color = std::atoi(line.c_str());
        parseNextFloat(line, start, end, x1);
        parseNextFloat(line, start, end, y1);
        parseNextFloat(line, start, end, z1);
        if (order == CCW) {
            parseNextFloat(line, start, end, x2);
            parseNextFloat(line, start, end, y2);
            parseNextFloat(line, start, end, z2);
            parseNextFloat(line, start, end, x3);
            parseNextFloat(line, start, end, y3);
            parseNextFloat(line, start, end, z3);
        } else {
            parseNextFloat(line, start, end, x3);
            parseNextFloat(line, start, end, y3);
            parseNextFloat(line, start, end, z3);
            parseNextFloat(line, start, end, x2);
            parseNextFloat(line, start, end, y2);
            parseNextFloat(line, start, end, z2);
        }
    }

    Quadrilateral::Quadrilateral(const std::string& line, WindingOrder order) {
        size_t start = line.find_first_not_of(LDR_WHITESPACE);
        size_t end = line.find_first_of(LDR_WHITESPACE, start);
        color = std::atoi(line.c_str());
        parseNextFloat(line, start, end, x1);
        parseNextFloat(line, start, end, y1);
        parseNextFloat(line, start, end, z1);

        if (order == CCW) {
            parseNextFloat(line, start, end, x2);
            parseNextFloat(line, start, end, y2);
            parseNextFloat(line, start, end, z2);
            parseNextFloat(line, start, end, x3);
            parseNextFloat(line, start, end, y3);
            parseNextFloat(line, start, end, z3);
            parseNextFloat(line, start, end, x4);
            parseNextFloat(line, start, end, y4);
            parseNextFloat(line, start, end, z4);
        } else {
            parseNextFloat(line, start, end, x4);
            parseNextFloat(line, start, end, y4);
            parseNextFloat(line, start, end, z4);
            parseNextFloat(line, start, end, x3);
            parseNextFloat(line, start, end, y3);
            parseNextFloat(line, start, end, z3);
            parseNextFloat(line, start, end, x2);
            parseNextFloat(line, start, end, y2);
            parseNextFloat(line, start, end, z2);
        }
    }

    OptionalLine::OptionalLine(const std::string& line) {
        size_t start = line.find_first_not_of(LDR_WHITESPACE);
        size_t end = line.find_first_of(LDR_WHITESPACE, start);
        color = std::atoi(line.c_str());
        parseNextFloat(line, start, end, x1);
        parseNextFloat(line, start, end, y1);
        parseNextFloat(line, start, end, z1);
        parseNextFloat(line, start, end, x2);
        parseNextFloat(line, start, end, y2);
        parseNextFloat(line, start, end, z2);
        parseNextFloat(line, start, end, controlX1);
        parseNextFloat(line, start, end, controlY1);
        parseNextFloat(line, start, end, controlZ1);
        parseNextFloat(line, start, end, controlX2);
        parseNextFloat(line, start, end, controlY2);
        parseNextFloat(line, start, end, controlZ2);
    }

    int CommentOrMetaElement::getType() const {
        return 0;
    }

    std::string CommentOrMetaElement::getLdrLine() const {
        return "0 " + content;
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

    std::string SubfileReference::getLdrLine() const {
        return fmt::format("1 {:d} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:s}", color.code, x, y, z, a, b, c, d, e, f, g, h, i, filename);
    }
    void SubfileReference::setTransformationMatrix(const glm::mat4& matrix) {
        a = matrix[0][0];
        b = matrix[1][0];
        c = matrix[2][0];
        x = matrix[3][0];
        d = matrix[0][1];
        e = matrix[1][1];
        f = matrix[2][1];
        y = matrix[3][1];
        g = matrix[0][2];
        h = matrix[1][2];
        i = matrix[2][2];
        z = matrix[3][2];
    }

    SubfileReference::SubfileReference(ColorReference color, const glm::mat4& transformation, bool bfcInverted) : color(color), bfcInverted(bfcInverted) {
        setTransformationMatrix(transformation);
    }

    int Line::getType() const {
        return 2;
    }
    std::string Line::getLdrLine() const {
        return fmt::format("2 {:d} {:g} {:g} {:g} {:g} {:g} {:g}", color.code, x1, y1, z1, x2, y2, z2);
    }

    int Triangle::getType() const {
        return 3;
    }
    std::string Triangle::getLdrLine() const {
        return fmt::format("3 {:d} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g}", color.code, x1, y1, z1, x2, y2, z2, x3, y3, z3);
    }

    int Quadrilateral::getType() const {
        return 4;
    }
    std::string Quadrilateral::getLdrLine() const {
        return fmt::format("4 {:d} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g}", color.code, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4);
    }

    int OptionalLine::getType() const {
        return 5;
    }

    std::string OptionalLine::getLdrLine() const {
        return fmt::format("5 {:d} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g}", color.code, x1, y1, z1, x2, y2, z2, controlX1, controlY1, controlZ1, controlX2, controlY2, controlZ2);
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
            headerCategory = util::trim(line.substr(9));
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
            history.push_back(line.substr(line.find_first_not_of(LDR_WHITESPACE, 8)));
        } else if (util::startsWith(line, "!LICENSE")) {
            license = line.substr(line.find_first_not_of(LDR_WHITESPACE, 8));
        } else if (util::startsWith(line, "!THEME")) {
            theme = line.substr(line.find_first_not_of(LDR_WHITESPACE, 6));
        } else if (util::startsWith(line, "!LDRAW_ORG")) {
            fileTypeLine = line.substr(line.find_first_not_of(LDR_WHITESPACE, 10));//standard says "In general, parsers should consider this line to be case-insensitive and free-format."
        } else {
            return false;
        }
        return true;
    }

    std::ostream& operator<<(std::ostream& os, const ldr::FileMetaInfo& info) {
        if (!info.title.empty()) {
            os << "0 " << info.title << LDR_NEWLINE;
        }

        if (!info.name.empty()) {
            os << "0 Name: " << info.name << LDR_NEWLINE;
        }

        if (!info.author.empty()) {
            os << "0 Author: " << info.author << LDR_NEWLINE;
        }

        os << "0 !LDRAW_ORG ";
        if (info.fileTypeLine.empty()) {
            os << getFileTypeStr(info.type);
        } else {
            os << info.fileTypeLine;
        }
        os << LDR_NEWLINE;

        if (info.license.empty()) {
            os << "0 !LICENSE Not redistributable : see NonCAreadme.txt" << LDR_NEWLINE;
        } else {
            os << "0 !LICENSE " << info.license << LDR_NEWLINE;
        }

        if (info.headerCategory.has_value() && !info.headerCategory->empty()) {
            os << "0 !CATEGORY " << info.headerCategory.value() << LDR_NEWLINE;
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
                    os << LDR_NEWLINE
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
            os << LDR_NEWLINE;
        }

        if (!info.history.empty()) {
            for (const auto& historyElement: info.history) {
                os << "0 !HISTORY " << historyElement << LDR_NEWLINE;
            }
        }

        if (!info.theme.empty()) {
            os << "0 !THEME " << info.theme << LDR_NEWLINE;
        }

        return os;
    }

    const std::string& ldr::FileMetaInfo::getCategory() {
        if (headerCategory->empty()) {
            const auto firstSpace = title.find(' ');
            auto start = 0;
            while (title[start] == '_' || title[start] == '~' || title[start] == '=') {
                start++;
            }
            headerCategory = title.substr(start, firstSpace - start);
        }
        return headerCategory.value();
    }

    namespace {
        const char* getFileTypeStr(FileType type) {
            switch (type) {
                case MODEL: return "Model";
                case MPD_SUBFILE: return "Submodel";
                case PART: return "Part";
                case SUBPART: return "Subpart";
                case PRIMITIVE: return "Primitive";
                default: return "File";
            }
        }
    }
}