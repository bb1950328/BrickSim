#include "files.h"
#include "../config.h"
#include "../helpers/stringutil.h"
#include "../helpers/util.h"
#include "../metrics.h"
#include "file_repo.h"
#include <charconv>
#include <fast_float/fast_float.h>
#include <iostream>
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <sstream>

namespace bricksim::ldr {
    WindingOrder inverseWindingOrder(WindingOrder order) {
        return order == WindingOrder::CW ? WindingOrder::CCW : WindingOrder::CW;
    }

    std::shared_ptr<FileElement> FileElement::parseLine(const std::string_view line, BfcState bfcState) {
        std::string_view lineContent = line.length() > 2 ? line.substr(2) : "";
        switch (line[0] - '0') {
            case 0: {
                if (TexmapStartCommand::doesLineMatch(lineContent)) {
                    return std::make_shared<TexmapStartCommand>(lineContent);
                } else {
                    return std::make_shared<CommentOrMetaElement>(lineContent);
                }
            }
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

#ifndef NDEBUG
    FileElement::~FileElement() {
        //todo use std::lock_guard without breaking build
        std::scoped_lock lg(metrics::ldrFileElementInstanceCountMtx);
        --metrics::ldrFileElementInstanceCount;
        //std::cout << metrics::ldrFileElementInstanceCount << std::endl;
    }

    FileElement::FileElement() {
        std::scoped_lock lg(metrics::ldrFileElementInstanceCountMtx);
        ++metrics::ldrFileElementInstanceCount;
        //std::cout << metrics::ldrFileElementInstanceCount << std::endl;
    }
#else
    FileElement::~FileElement() = default;
    FileElement::FileElement() = default;
#endif

    void File::addTextLine(const std::string_view line) {
        auto trimmed = stringutil::trim(line);
        unsigned int currentStep = elements.empty() ? 0 : elements.back()->step;
        if (!trimmed.empty()) {
            auto element = FileElement::parseLine(trimmed, bfcState);
            if (element != nullptr) {
                bfcState.invertNext = false;
                if (element->getType() == 0) {
                    auto metaElement = std::dynamic_pointer_cast<CommentOrMetaElement>(element);
                    if (metaInfo.addLine(metaElement->content)) {
                        element = nullptr;
                    } else if (metaElement->content == "STEP") {
                        currentStep++;
                    } else if (metaElement->content.starts_with("BFC")) {
                        std::string bfcCommand = stringutil::trim(metaElement->content.substr(3));
                        if (bfcCommand.starts_with("CERTIFY")) {
                            std::string order = stringutil::trim(bfcCommand.substr(7));
                            bfcState.windingOrder = order == "CW" ? WindingOrder::CW : WindingOrder::CCW;
                            bfcState.active = true;
                        } else if (bfcCommand.starts_with("CLIP")) {
                            std::string order = stringutil::trim(bfcCommand.substr(4));
                            if (order == "CW") {
                                bfcState.windingOrder = WindingOrder::CW;
                            } else if (order == "CCW") {
                                bfcState.windingOrder = WindingOrder::CCW;
                            }
                            bfcState.active = true;
                        } else if (bfcCommand == "CW") {
                            bfcState.windingOrder = WindingOrder::CW;
                            bfcState.active = true;//todo this is never explicitly stated in the standard
                        } else if (bfcCommand == "CCW") {
                            bfcState.windingOrder = WindingOrder::CCW;
                            bfcState.active = true;//todo this is never explicitly stated in the standard
                        } else if (bfcCommand == "NOCLIP") {
                            bfcState.active = false;
                        } else if (bfcCommand == "INVERTNEXT") {
                            bfcState.invertNext = true;
                        }
                    } else if (metaElement->content.starts_with(META_COMMAND_TEXMAP) && config::get(config::ENABLE_TEXMAP_SUPPORT)) {
                        auto startCommand = std::dynamic_pointer_cast<TexmapStartCommand>(metaElement);
                        if (startCommand != nullptr) {
                            texmapState.startOrNext(startCommand);
                        } else if (texmapState.isActive()) {
                            size_t start = metaElement->content.find_first_not_of(LDR_WHITESPACE, META_COMMAND_TEXMAP_LEN);
                            if (metaElement->content.find("FALLBACK", start) == start) {
                                texmapState.fallback();
                            } else if (metaElement->content.find("END", start) == start) {
                                texmapState.end();
                            }
                        }
                    } else if (texmapState.isActive() && !texmapState.fallbackSectionReached) {
                        if (metaElement->content.starts_with("!:")) {
                            auto realCommand = metaElement->content.substr(metaElement->content.find_first_not_of(LDR_WHITESPACE, 2));
                            element = FileElement::parseLine(realCommand, bfcState);
                        }
                        element->directTexmap = texmapState.startCommand;
                    } else if (metaElement->content.starts_with(LDCAD_META_START)) {
                        parseLdcadMeta(std::string_view(metaElement->content).substr(sizeof(LDCAD_META_START)));
                    }
                } else if (texmapState.isActive()) {
                    if (texmapState.fallbackSectionReached) {
                        element = nullptr;
                    } else {
                        element->directTexmap = texmapState.startCommand;
                    }
                }
                if (element != nullptr) {
                    element->step = currentStep;
                    elements.push_back(element);
                }
            }
        }
    }

    void File::printStructure(int indent) const {
        for (const auto& elem: elements) {
            if (elem->getType() == 1) {
                auto subfileRef = std::dynamic_pointer_cast<SubfileReference>(elem);
                for (int i = 0; i < indent; ++i) {
                    std::cout << "\t";
                }
                std::cout << subfileRef->filename << "\n";
                subfileRef->getFile(nameSpace)->printStructure(indent + 1);
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
            hash = bricksim::hash<std::string>{}(metaInfo.name);
        }
        return hash;
    }
    void File::addShadowContent(const std::string& shadowContent) {
        size_t lineStart = 0;
        size_t lineEnd = 0;
        std::string_view view = shadowContent;
        while (lineEnd < view.size()) {
            lineStart = view.find(LDCAD_META_START, lineStart);
            if (lineStart == std::string::npos) {
                break;
            }
            lineEnd = view.find_first_of("\r\n", lineStart);
            if (lineEnd == std::string::npos) {
                lineEnd = view.size();
            } else {
                ++lineEnd;
            }

            parseLdcadMeta(stringutil::trim(view.substr(lineStart + sizeof(LDCAD_META_START), lineEnd - lineStart)));

            lineStart = lineEnd;
        }
    }
    void File::parseLdcadMeta(const std::string_view& metaContent) {
        const std::string line(metaContent);
        const auto metaCommand = connection::ldcad_meta::Reader::readLine(line);
        if (metaCommand != nullptr) {
            ldcadMetas.push_back(metaCommand);
        } else if (!connection::ldcad_meta::Reader::isUnsupportedCommand(line)) {
            spdlog::warn("unknown/invalid ldcad snap meta in {}: {}", metaInfo.name, line);
        }
    }
    File::~File() = default;

    CommentOrMetaElement::CommentOrMetaElement(const std::string_view line) :
        content(line) {
    }

    inline void parseNextFloat(const std::string_view line, size_t& start, size_t& end, float& result) {
        start = line.find_first_not_of(LDR_WHITESPACE, end);
        end = std::min(line.size(), line.find_first_of(LDR_WHITESPACE, start));
        fast_float::from_chars(line.data() + start, line.data() + end, result);
    }

    inline void parseNextThreeFloats(const std::string_view line, size_t& start, size_t& end, float* result) {
        for (int i = 0; i < 3; ++i) {
            parseNextFloat(line, start, end, result[i]);
        }
    }

    SubfileReference::SubfileReference(const std::string_view line, bool bfcInverted) :
        bfcInverted(bfcInverted) {
        size_t start = line.find_first_not_of(LDR_WHITESPACE);
        size_t end = line.find_first_of(LDR_WHITESPACE, start);
        std::from_chars(&line[start], &line[end], color.code);
        for (int n = 0; n < std::size(numbers); ++n) {
            parseNextFloat(line, start, end, numbers[n]);
        }
        filename = stringutil::trim(line.substr(end + 1));
    }

    Line::Line(const std::string_view line) {
        size_t start = line.find_first_not_of(LDR_WHITESPACE);
        size_t end = line.find_first_of(LDR_WHITESPACE, start);
        std::from_chars(&line[start], &line[end], color.code);
        for (int n = 0; n < std::size(coords); ++n) {
            parseNextFloat(line, start, end, coords[n]);
        }
    }

    Triangle::Triangle(const std::string_view line, WindingOrder order) {
        size_t start = line.find_first_not_of(LDR_WHITESPACE);
        size_t end = line.find_first_of(LDR_WHITESPACE, start);
        std::from_chars(&line[start], &line[end], color.code);

        parseNextThreeFloats(line, start, end, &coords[0]);//p1

        if (order == WindingOrder::CCW) {
            parseNextThreeFloats(line, start, end, &coords[1 * 3]);//p2
            parseNextThreeFloats(line, start, end, &coords[2 * 3]);//p3
        } else {
            parseNextThreeFloats(line, start, end, &coords[2 * 3]);//p3
            parseNextThreeFloats(line, start, end, &coords[1 * 3]);//p2
        }
    }

    Quadrilateral::Quadrilateral(const std::string_view line, WindingOrder order) {
        size_t start = line.find_first_not_of(LDR_WHITESPACE);
        size_t end = line.find_first_of(LDR_WHITESPACE, start);
        std::from_chars(&line[start], &line[end], color.code);

        parseNextThreeFloats(line, start, end, &coords[0]);//p1

        if (order == WindingOrder::CCW) {
            parseNextThreeFloats(line, start, end, &coords[1 * 3]);//p2
            parseNextThreeFloats(line, start, end, &coords[2 * 3]);//p3
            parseNextThreeFloats(line, start, end, &coords[3 * 3]);//p4
        } else {
            parseNextThreeFloats(line, start, end, &coords[3 * 3]);//p4
            parseNextThreeFloats(line, start, end, &coords[2 * 3]);//p3
            parseNextThreeFloats(line, start, end, &coords[1 * 3]);//p2
        }
    }

    OptionalLine::OptionalLine(const std::string_view line) {
        size_t start = line.find_first_not_of(LDR_WHITESPACE);
        size_t end = line.find_first_of(LDR_WHITESPACE, start);
        std::from_chars(&line[start], &line[end], color.code);

        for (int i = 0; i < std::size(coords); ++i) {
            parseNextFloat(line, start, end, coords[i]);
        }
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

    std::shared_ptr<File> SubfileReference::getFile(const std::shared_ptr<FileNamespace>& fileNamespace) {
        if (file == nullptr) {
            file = ldr::file_repo::get().getFile(fileNamespace, filename);
        }
        return file;
    }

    glm::mat4 SubfileReference::getTransformationMatrix() const {
        return {
                a(), b(), c(), x(),
                d(), e(), f(), y(),
                g(), h(), i(), z(),
                0.0f, 0.0f, 0.0f, 1.0f};
    }

    std::string SubfileReference::getLdrLine() const {
        return fmt::format("1 {:d} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:s}", color.code, x(), y(), z(), a(), b(), c(), d(), e(), f(), g(), h(), i(), filename);
    }
    void SubfileReference::setTransformationMatrix(const glm::mat4& matrix) {
        a() = matrix[0][0];
        b() = matrix[1][0];
        c() = matrix[2][0];
        x() = matrix[3][0];
        d() = matrix[0][1];
        e() = matrix[1][1];
        f() = matrix[2][1];
        y() = matrix[3][1];
        g() = matrix[0][2];
        h() = matrix[1][2];
        i() = matrix[2][2];
        z() = matrix[3][2];
    }

    SubfileReference::SubfileReference(ColorReference color, const glm::mat4& transformation, bool bfcInverted) :
        bfcInverted(bfcInverted), color(color) {
        setTransformationMatrix(transformation);
    }

    int Line::getType() const {
        return 2;
    }
    std::string Line::getLdrLine() const {
        return fmt::format("2 {:d} {:g} {:g} {:g} {:g} {:g} {:g}", color.code, x1(), y1(), z1(), x2(), y2(), z2());
    }

    int Triangle::getType() const {
        return 3;
    }
    std::string Triangle::getLdrLine() const {
        return fmt::format("3 {:d} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g}", color.code, x1(), y1(), z1(), x2(), y2(), z2(), x3(), y3(), z3());
    }

    int Quadrilateral::getType() const {
        return 4;
    }
    std::string Quadrilateral::getLdrLine() const {
        return fmt::format("4 {:d} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g}", color.code, x1(), y1(), z1(), x2(), y2(), z2(), x3(), y3(), z3(), x4(), y4(), z4());
    }

    int OptionalLine::getType() const {
        return 5;
    }

    std::string OptionalLine::getLdrLine() const {
        return fmt::format("5 {:d} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g}", color.code, x1(), y1(), z1(), x2(), y2(), z2(), controlX1(), controlY1(), controlZ1(), controlX2(), controlY2(), controlZ2());
    }

    bool ldr::FileMetaInfo::addLine(const std::string& line) {
        if (firstLine) {
            title = stringutil::trim(line);
            firstLine = false;
        } else if (line.starts_with("Name:")) {
            name = stringutil::trim(line.substr(5));
        } else if (line.starts_with("Author:")) {
            author = stringutil::trim(line.substr(7));
        } else if (line.starts_with("!CATEGORY")) {
            headerCategory = stringutil::trim(line.substr(9));
        } else if (line.starts_with("!KEYWORDS")) {
            size_t i = 9;
            while (true) {
                size_t next = line.find(',', i);
                if (next == std::string::npos) {
                    keywords.insert(stringutil::trim(line.substr(i)));
                    break;
                }
                keywords.insert(stringutil::trim(line.substr(i, next - i)));
                i = next + 1;
            }
        } else if (line.starts_with("!HISTORY")) {
            history.push_back(line.substr(line.find_first_not_of(LDR_WHITESPACE, 8)));
        } else if (line.starts_with("!LICENSE")) {
            license = line.substr(line.find_first_not_of(LDR_WHITESPACE, 8));
        } else if (line.starts_with("!THEME")) {
            theme = line.substr(line.find_first_not_of(LDR_WHITESPACE, 6));
        } else if (line.starts_with("!LDRAW_ORG")) {
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
        if (!headerCategory.has_value() || headerCategory->empty()) {
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
                case FileType::MODEL: return "Model";
                case FileType::MPD_SUBFILE: return "Submodel";
                case FileType::PART: return "Part";
                case FileType::SUBPART: return "Subpart";
                case FileType::PRIMITIVE: return "Primitive";
                default: return "File";
            }
        }
    }

    TexmapStartCommand::TexmapStartCommand(const std::string_view line) :
        CommentOrMetaElement(line) {
        size_t start = line.find_first_not_of(LDR_WHITESPACE, META_COMMAND_TEXMAP_LEN);
        size_t end = line.find_first_of(LDR_WHITESPACE, start + 1);
        start = line.find_first_not_of(LDR_WHITESPACE, end);
        end = line.find_first_of(LDR_WHITESPACE, start);
        auto projectionMethodStrView = std::string_view(line).substr(start, end - start);
        projectionMethod = magic_enum::enum_cast<ProjectionMethod>(projectionMethodStrView).value_or(ProjectionMethod::PLANAR);

        parseNextThreeFloats(line, start, end, &coords[0]);    //p1
        parseNextThreeFloats(line, start, end, &coords[1 * 3]);//p2
        parseNextThreeFloats(line, start, end, &coords[2 * 3]);//p3

        if (projectionMethod != ProjectionMethod::PLANAR) {
            parseNextFloat(line, start, end, a());
        } else {
            a() = 0;
        }
        if (projectionMethod == ProjectionMethod::SPHERICAL) {
            parseNextFloat(line, start, end, b());
        } else {
            b() = 0;
        }

        size_t glossmapPos = line.find(" GLOSSMAP", end);//the space is necessary because the path could be something like /home/user/GLOSSMAPS/glossmap123.png
        if (glossmapPos == std::string::npos) {
            glossmapPos = line.find("\tGLOSSMAP", end);
        }
        if (glossmapPos != std::string::npos) {
            textureFilename = stringutil::trim(line.substr(end, glossmapPos - end));
            glossmapFileName = stringutil::trim(line.substr(glossmapPos + 10));
        } else {
            textureFilename = stringutil::trim(line.substr(end));
        }
    }

    bool TexmapStartCommand::doesLineMatch(const std::string_view line) {
        if (line.starts_with(META_COMMAND_TEXMAP)) {
            size_t start = line.find_first_not_of(LDR_WHITESPACE, META_COMMAND_TEXMAP_LEN);
            return line.find("START", start) == start;
        } else {
            return false;
        }
    }

    std::string TexmapStartCommand::getLdrLine() const {
        std::string abStr;
        if (projectionMethod == ProjectionMethod::PLANAR) {
            abStr = "";
        } else if (projectionMethod == ProjectionMethod::CYLINDRICAL) {
            abStr = fmt::format("{:g}", a());
        } else {
            abStr = fmt::format("{:g} {:g}", a(), b());
        }
        std::string glossmapStr = glossmapFileName.has_value() ? fmt::format("GLOSSMAP {}", glossmapFileName.value()) : "";
        return fmt::format("0 !TEXMAP START {} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {} {} {}", magic_enum::enum_name(projectionMethod), x1(), y1(), z1(), x2(), y2(), z2(), x3(), y3(), z3(), abStr, textureFilename, glossmapStr);
    }

    TexmapStartCommand::TexmapStartCommand(const TexmapStartCommand& other) :
        CommentOrMetaElement(other.content),
        projectionMethod(other.projectionMethod),
        coords(other.coords),
        textureFilename(other.textureFilename),
        glossmapFileName(other.glossmapFileName) {
    }

    void TexmapState::startOrNext(const std::shared_ptr<TexmapStartCommand>& command) {
        this->startCommand = command;
        fallbackSectionReached = false;
    }

    void TexmapState::fallback() {
        fallbackSectionReached = true;
    }

    void TexmapState::end() {
        startCommand.reset();
    }

    bool TexmapState::isActive() const {
        return startCommand != nullptr;
    }

    FileNamespace::FileNamespace(std::string name, const std::filesystem::path& searchPath) :
        name(std::move(name)), searchPath(std::filesystem::absolute(searchPath)) {}
    bool FileNamespace::operator==(const FileNamespace& rhs) const {
        return name == rhs.name && searchPath == rhs.searchPath;
    }
    bool FileNamespace::operator!=(const FileNamespace& rhs) const {
        return !(rhs == *this);
    }
}
