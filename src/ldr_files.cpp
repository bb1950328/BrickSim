//
// Created by bb1950328 on 19.09.20.
//

#include <fstream>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include "ldr_files.h"
#include "util.h"
#include "config.h"
#include "ldr_file_repository.h"
#include "ldr_colors.h"

LdrFileElement *LdrFileElement::parse_line(std::string line) {
    std::string line_content = line.length() > 2 ? line.substr(2) : "";
    switch (line[0] - '0') {
        //@formatter:off
        case 0: return new LdrCommentOrMetaElement(line_content);
        case 1: return new LdrSubfileReference(line_content);
        case 2: return new LdrLine(line_content);
        case 3: return new LdrTriangle(line_content);
        case 4: return new LdrQuadrilateral(line_content);
        case 5: return new LdrOptionalLine(line_content);
        default: throw std::invalid_argument("The line is not valid: \"" + line + "\"");
        //@formatter:off
    }
}
LdrFileElement::~LdrFileElement()= default;

LdrFile* LdrFile::parseFile(LdrFileType fileType, const std::filesystem::path &path){
    auto mainFile = new LdrFile();
    mainFile->metaInfo.type = fileType;
    std::ifstream input(path);
    if (!input.good()) {
        throw std::invalid_argument("can't open file \"" + path.string() + "\"");
    }
    bool isMpd = path.extension() == ".mpd";
    if (isMpd) {
        std::string currentSubFileName = path.string();
        std::map<std::string, std::list<std::string>> fileLines;
        bool firstFile = true;
        for (std::string line; getline(input, line);) {
            if (util::starts_with(line, "0 FILE")) {
                if (!firstFile) {
                    currentSubFileName = util::trim(line.substr(7));
                } else {
                    firstFile = false;
                }
            } else if (util::starts_with(line, "0 !DATA")) {
                currentSubFileName = util::trim(line.substr(8));
            } else {
                fileLines[currentSubFileName].push_back(line);
            }
        }
        for (auto const& entry: fileLines) {
            LdrFile* currentFile;
            if (entry.first == path) {
                currentFile = mainFile;
            } else {
                if (util::starts_with(entry.second.front(), "0 !: ")) {
                    //todo parse base64 data and store it somewhere
                    continue;
                } else {
                    currentFile = new LdrFile();
                    currentFile->metaInfo.type = MPD_SUBFILE;
                    LdrFileRepository::add_file(entry.first, currentFile, MPD_SUBFILE);
                }
            }
            unsigned long lineCount = entry.second.size();
            currentFile->elements.reserve(lineCount);
            for (const auto& line : entry.second) {
                currentFile->addTextLine(line);
            }

        }
    } else {
        for (std::string line; getline(input, line);) {
            mainFile->addTextLine(line);
        }
    }
    
    return mainFile;
}
void LdrFile::addTextLine(const std::string &line) {
    auto trimmed = util::trim(line);
    unsigned int currentStep = elements.empty()?0:elements.back()->step;
    if (!trimmed.empty()) {
        LdrFileElement *element = LdrFileElement::parse_line(trimmed);
        if (element->getType()==0) {
            auto *metaElement = dynamic_cast<LdrCommentOrMetaElement *>(element);
            if (metaInfo.add_line(metaElement->content)) {
                delete element;
                element = nullptr;
            } else if (metaElement->content=="STEP") {
                currentStep++;
            }
        }
        if (element!=nullptr) {
            element->step = currentStep;
            elements.push_back(element);
        }
    }
}
void LdrFile::printStructure(int indent) {
    for (auto elem : elements) {
        if (elem->getType()==1) {
            auto *subfileRef = dynamic_cast<LdrSubfileReference *>(elem);
            for (int i = 0; i < indent; ++i) {
                std::cout << "\t";
            }
            std::cout << subfileRef->filename << "\n";
            subfileRef->getFile()->printStructure(indent+1);
        }
    }
}
void LdrFile::preLoadSubfilesAndEstimateComplexity() {
    if (!subfiles_preloaded_and_complexity_estimated) {
        preLoadSubfilesAndEstimateComplexityInternal();
    }
}
void LdrFile::preLoadSubfilesAndEstimateComplexityInternal(){
    referenceCount++;
    if (!subfiles_preloaded_and_complexity_estimated) {
        for (LdrFileElement *elem: elements) {
            if (elem->getType()==1) {
                LdrFile *subFile = dynamic_cast<LdrSubfileReference *>(elem)->getFile();
                subFile->preLoadSubfilesAndEstimateComplexityInternal();
                estimatedComplexity += subFile->estimatedComplexity;
            } else if (elem->getType()==2) {
                estimatedComplexity += 1;
            } else if (elem->getType()==3) {
                estimatedComplexity += 2;
            } else if (elem->getType()==4) {
                estimatedComplexity += 3;
            }
        }
        subfiles_preloaded_and_complexity_estimated = true;
    }
}
std::string LdrFile::getDescription() const {
    for (auto elem : elements) {
        if (elem->getType()==0) {
            return dynamic_cast<LdrCommentOrMetaElement *>(elem)->content;
        }
    }
    return "?";
}

long LdrFile::instancedMinComplexity = -1;

bool LdrFile::isComplexEnoughForOwnMesh() const {
    /*if (instancedMinComplexity==-1) {
        instancedMinComplexity = config::get_long(config::INSTANCED_MIN_COMPLEXITY);
    }*/
    return (metaInfo.type!=SUBPART && metaInfo.type!=PRIMITIVE);// todo spend more time here, I think there's much more potential here
}

LdrCommentOrMetaElement::LdrCommentOrMetaElement(const std::string& line) {
    content = line;
}

LdrSubfileReference::LdrSubfileReference(const std::string& line) {
    std::stringstream linestream(line);
    int colorCode;
    char *filenameTmp = new char [MAX_LDR_FILENAME_LENGTH+1];

    linestream >> colorCode;
    linestream >> x;
    linestream >> y;
    linestream >> z;
    linestream >> a;
    linestream >> b;
    linestream >> c;
    linestream >> d;
    linestream >> e;
    linestream >> f;
    linestream >> g;
    linestream >> h;
    linestream >> i;
    linestream.getline(filenameTmp, MAX_LDR_FILENAME_LENGTH+1);

    color = LdrColorRepository::getInstance()->get_color(colorCode);
    filename = util::trim(std::string(filenameTmp));
    delete [] filenameTmp;
}

LdrLine::LdrLine(const std::string& line) {
    std::stringstream linestream(line);
    int colorCode;
    linestream >> colorCode;
    linestream >> x1;
    linestream >> y1;
    linestream >> z1;
    linestream >> x2;
    linestream >> y2;
    linestream >> z2;
    color = LdrColorRepository::getInstance()->get_color(colorCode);
}

LdrTriangle::LdrTriangle(const std::string& line) {
    std::stringstream linestream(line);
    int colorCode;
    linestream >> colorCode;
    linestream >> x1;
    linestream >> y1;
    linestream >> z1;
    linestream >> x2;
    linestream >> y2;
    linestream >> z2;
    linestream >> x3;
    linestream >> y3;
    linestream >> z3;
    color = LdrColorRepository::getInstance()->get_color(colorCode);
}

LdrQuadrilateral::LdrQuadrilateral(const std::string& line) {
    std::stringstream linestream(line);
    int colorCode;
    linestream >> colorCode;
    linestream >> x1;
    linestream >> y1;
    linestream >> z1;
    linestream >> x2;
    linestream >> y2;
    linestream >> z2;
    linestream >> x3;
    linestream >> y3;
    linestream >> z3;
    linestream >> x4;
    linestream >> y4;
    linestream >> z4;
    color = LdrColorRepository::getInstance()->get_color(colorCode);
}

LdrOptionalLine::LdrOptionalLine(const std::string& line) {
    std::stringstream linestream(line);
    int colorCode;
    linestream >> colorCode;
    linestream >> x1;
    linestream >> y1;
    linestream >> z1;
    linestream >> x2;
    linestream >> y2;
    linestream >> z2;
    linestream >> control_x1;
    linestream >> control_y1;
    linestream >> control_z1;
    linestream >> control_x2;
    linestream >> control_y2;
    linestream >> control_z2;
    color = LdrColorRepository::getInstance()->get_color(colorCode);
}

int LdrCommentOrMetaElement::getType() const{
    return 0;
}

int LdrSubfileReference::getType() const{
    return 1;
}
LdrFile * LdrSubfileReference::getFile() {
    if (file==nullptr) {
        file = LdrFileRepository::get_file(filename);
    }
    return file;
}
glm::mat4 LdrSubfileReference::getTransformationMatrix() const {
    return {
        a, b, c, x * 1,//todo check if the *1 is necessary
        d, e, f, y * 1,
        g, h, i, z * 1,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}
int LdrLine::getType() const{
    return 2;
}
int LdrTriangle::getType() const{
    return 3;
}
int LdrQuadrilateral::getType() const{
    return 4;
}
int LdrOptionalLine::getType() const{
    return 5;
}

bool LdrFileMetaInfo::add_line(std::string line){
    static bool first = true;
    if (first) {
        title = util::trim(line);
        first = false;
    } else if (util::starts_with(line, "Name:")) {
        name = util::trim(line.substr(5));
    } else if (util::starts_with(line, "Author:")) {
        author = util::trim(line.substr(7));
    } else if (util::starts_with(line, "!CATEGORY")) {
        category = util::trim(line.substr(9));
    } else if (util::starts_with(line, "!KEYWORDS")) {
        size_t i = 9;
        while (true) {
            size_t next = line.find(',', i);
            if (next==std::string::npos) {
                keywords.insert(util::trim(line.substr(i)));
                break;
            }
            keywords.insert(util::trim(line.substr(i, next-i)));
            i = next+1;
        }
    } else if (util::starts_with(line, "!HISTORY")) {
        history.push_back(util::trim(line.substr(8)));
    } else if (util::starts_with(line, "!LICENSE")) {
        license = line.substr(8);
    } else if (util::starts_with(line, "!THEME")) {
        theme = line.substr(6);
    } else {
        return false;
    }
    return true;
}
std::ostream & operator<<(std::ostream & os, const LdrFileMetaInfo & info) {
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
        for (const auto &kw : info.keywords) {
            if (!first) {
                lineWidth += 2;
            }
            lineWidth += kw.size();
            if (lineWidth > 80) {
                os << std::endl << "0 !KEYWORDS ";
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
        for (const auto &historyElement : info.history) {
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
