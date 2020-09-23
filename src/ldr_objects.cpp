//
// Created by bb1950328 on 19.09.20.
//

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include "ldr_objects.h"
#include "util.h"
#include "config.h"

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

LdrFile* LdrFile::parseFile(const std::string & filename){
    auto mainFile = new LdrFile();
    std::ifstream input = openFile(filename);
    if (!input.good()) {
        throw std::invalid_argument("can't open file \""+filename+"\"");
    }
    bool isMpd = util::ends_with(filename, ".mpd");
    if (isMpd) {
        std::string currentSubFileName = filename;
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
        //todo process files with subfile references at the end or compute dependencies
        for (auto const& entry: fileLines) {
            LdrFile* currentFile;
            if (entry.first==filename) {
                currentFile = mainFile;
            } else {
                if (util::starts_with(entry.second.front(), "0 !: ")) {
                    //todo parse base64 data and store it somewhere
                    continue;
                } else {
                    currentFile = new LdrFile();
                    LdrFileRepository::add_file(entry.first, currentFile);
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
        if (!trimmed.empty()) {
            elements.push_back(LdrFileElement::parse_line(trimmed));
        }
}
std::ifstream LdrFile::openFile(const std::string &filename) {
    auto parts_lib_location = util::extend_home_dir(Configuration::getInstance().get_string(config::KEY_LDRAW_PARTS_LIBRARY));
    //todo make parts search case-insensitive https://forums.ldraw.org/thread-13787.html
    auto locations = {
                util::extend_home_dir(filename),
                util::pathjoin({parts_lib_location, "parts", util::as_lower(filename)}),//parts
                util::pathjoin({parts_lib_location, "p", util::as_lower(filename)}),//primitives
                util::pathjoin({parts_lib_location, "models", util::as_lower(filename)}),//models
        };
    std::ifstream input;
    for (const auto & loc : locations) {
        input = std::ifstream(loc);
        if (input.good()) {
            break;
        }
    }
    return input;
}
void LdrFile::printStructure(int indent) {
    for (auto elem : elements) {
        if (elem->getType()==1) {
            auto *subfileRef = dynamic_cast<LdrSubfileReference *>(elem);
            for (int i = 0; i < indent; ++i) {
                std::cout << "\t";
            }
            std::cout << subfileRef->filename << "\n";
            if (util::starts_with(subfileRef->filename, "42043")) {
                int i = 5;//todo remove
            }
            subfileRef->getFile()->printStructure(indent+1);
        }
    }
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

LdrColor::LdrColor(const std::string& line) {
    std::stringstream linestream(line);
    linestream >> name;
    while (linestream.rdbuf()->in_avail() != 0) {
        std::string keyword;
        linestream >> keyword;
        if (keyword=="CODE") {
            linestream>>code;
        } else if (keyword=="VALUE") {
            std::string valueCode;
            linestream>>valueCode;
            value = RGB(valueCode);
        } else if (keyword=="EDGE") {
            std::string edgeCode;
            linestream>>edgeCode;
            edge = RGB(edgeCode);
        } else if (keyword=="CHROME") {
            finish = CHROME;
        } else if (keyword=="PEARLESCENT") {
            finish = PEARLESCENT;
        } else if (keyword=="RUBBER") {
            finish = RUBBER;
        } else if (keyword=="MATTE_METALLIC") {
            finish = MATTE_METALLIC;
        } else if (keyword=="METAL") {
            finish = METAL;
        } else if (keyword=="MATERIAL") {
            finish = MATERIAL;
            material = new LdrColorMaterial();
            std::string typeStr;
            linestream>>typeStr;
            material->type = typeStr=="GLITTER"?LdrColorMaterial::GLITTER:LdrColorMaterial::SPECKLE;
            while (linestream.rdbuf()->in_avail() != 0) {
                linestream>>keyword;
                if (keyword=="VALUE") {
                    std::string valueCode;
                    linestream>>valueCode;
                    material->value = RGB(valueCode);
                } else if (keyword=="ALPHA") {
                    linestream>>material->alpha;
                } else if (keyword=="LUMINANCE") {
                    linestream>>material->luminance;
                } else if (keyword=="FRACTION") {
                    linestream>>material->fraction;
                } else if (keyword=="VFRACTION") {
                    linestream>>material->vfraction;
                } else if (keyword=="SIZE") {
                    linestream>>material->size;
                } else if (keyword=="MAXSIZE") {
                    linestream>>material->maxsize;
                } else if (keyword=="MINSIZE") {
                    linestream>>material->minsize;
                }
            }
            break;
        }
    }
}
glm::vec3 RGB::asGlmVector() const {
    return glm::vec3(red/255.0f, green/255.0f, blue/255.0f);
}


LdrColor *LdrColorRepository::get_color(const int colorCode) {
    auto iterator = colors.find(colorCode);
    if (iterator == colors.end()) {
        throw std::invalid_argument("unknown color code: " + std::to_string(colorCode));
    }
    return &(iterator->second);
}
LdrColorRepository *LdrColorRepository::instance = nullptr;
LdrColorRepository * LdrColorRepository::getInstance(){
    if (instance == nullptr) {
        instance = new LdrColorRepository();
        instance->initialize();
    }
    return instance;
}
void LdrColorRepository::initialize(){
    auto lib_path = util::extend_home_dir(Configuration::getInstance().get_string(config::KEY_LDRAW_PARTS_LIBRARY));
    auto input = std::ifstream(util::pathjoin({lib_path, "LDConfig.ldr"}));
    for (std::string line; getline(input, line);) {
        auto trimmed = util::trim(line);
        if (!trimmed.empty() && trimmed.rfind("0 !COLOUR", 0)==0) {
            LdrColor col(line.substr(10));
            colors[col.code] = col;
        }
    }
}


std::map<std::string, LdrFile*> LdrFileRepository::files;

LdrFile *LdrFileRepository::get_file(const std::string &filename) {
    auto iterator = files.find(filename);
    if (iterator == files.end()) {
        LdrFile* file = LdrFile::parseFile(filename);
        files[filename] = file;
        return file;
    }
    return (iterator->second);
}
void LdrFileRepository::add_file(const std::string &filename, LdrFile *file){
    files[filename] = file;
}
void LdrFileRepository::clear_cache(){
    files.clear();
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

RGB::RGB(std::string htmlCode){
    auto redChars = new char[2];
    auto greenChars = new char[2];
    auto blueChars = new char[2];

    redChars[0] = htmlCode[1];
    redChars[1] = htmlCode[2];
    greenChars[0] = htmlCode[3];
    greenChars[1] = htmlCode[4];
    blueChars[0] = htmlCode[5];
    blueChars[1] = htmlCode[6];

    red = std::stoi(redChars,nullptr,16);
    green = std::stoi(greenChars,nullptr,16);
    blue = std::stoi(blueChars,nullptr,16);

    delete [] redChars;
    delete [] greenChars;
    delete [] blueChars;
}
