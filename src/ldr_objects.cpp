//
// Created by bb1950328 on 19.09.20.
//

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <glm/gtx/string_cast.hpp>
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

LdrFile* LdrFile::parseFile(const std::filesystem::path & path){
    auto mainFile = new LdrFile();
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
        //todo process files with subfile references at the end or compute dependencies
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
                    LdrFileRepository::add_file(entry.first, currentFile, SUBPART);
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
std::string LdrFile::getDescription() {
    for (auto elem : elements) {
        if (elem->getType()==0) {
            return dynamic_cast<LdrCommentOrMetaElement *>(elem)->content;
        }
    }
    return "?";
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
LdrInstanceDummyColor LdrColorRepository::instDummyColor;
LdrColorRepository * LdrColorRepository::getInstance(){
    if (instance == nullptr) {
        instance = new LdrColorRepository();
        instance->initialize();
    }
    return instance;
}
void LdrColorRepository::initialize(){
    auto lib_path = util::extend_home_dir(Configuration::getInstance()->get_string(config::KEY_LDRAW_PARTS_LIBRARY));
    auto input = std::ifstream(util::pathjoin({lib_path, "LDConfig.ldr"}));
    for (std::string line; getline(input, line);) {
        auto trimmed = util::trim(line);
        if (!trimmed.empty() && trimmed.rfind("0 !COLOUR", 0)==0) {
            LdrColor col(line.substr(10));
            colors[col.code] = col;
        }
    }
    colors[instDummyColor.code] = instDummyColor;
}


std::map<std::string, std::pair<LdrFileType, LdrFile*>> LdrFileRepository::files;
std::filesystem::path LdrFileRepository::ldrawDirectory;
std::filesystem::path LdrFileRepository::partsDirectory;
std::filesystem::path LdrFileRepository::subpartsDirectory;
std::filesystem::path LdrFileRepository::primitivesDirectory;
std::filesystem::path LdrFileRepository::modelsDirectory;
bool LdrFileRepository::namesInitialized;
std::map<std::string, std::filesystem::path> LdrFileRepository::primitiveNames;
std::map<std::string, std::filesystem::path> LdrFileRepository::subpartNames;
std::map<std::string, std::filesystem::path> LdrFileRepository::partNames;
std::map<std::string, std::filesystem::path> LdrFileRepository::modelNames;

LdrFile *LdrFileRepository::get_file(const std::string &filename) {
    auto iterator = files.find(filename);
    if (iterator == files.end()) {
        auto typeNamePair = resolve_file(filename);
        LdrFile* file = LdrFile::parseFile(typeNamePair.second);
        files[filename] = std::make_pair(typeNamePair.first, file);
        return file;
    }
    return (iterator->second.second);
}

LdrFileType LdrFileRepository::get_file_type(const std::string &filename) {
    auto iterator = files.find(filename);
    if (iterator == files.end()) {
        throw std::invalid_argument("this file is unknown!");
    }
    return iterator->second.first;
}

void LdrFileRepository::add_file(const std::string &filename, LdrFile *file, LdrFileType type){
    files[filename] = std::make_pair(type, file);
}
void LdrFileRepository::clear_cache(){
    files.clear();
}
void LdrFileRepository::initializeNames() {
    if (!namesInitialized) {
        auto before = std::chrono::high_resolution_clock::now();
        ldrawDirectory = util::extend_home_dir_path(Configuration::getInstance()->get_string(config::KEY_LDRAW_PARTS_LIBRARY));
        partsDirectory = ldrawDirectory / std::filesystem::path("parts");
        subpartsDirectory = partsDirectory / std::filesystem::path("s");
        primitivesDirectory = ldrawDirectory / std::filesystem::path("p");
        modelsDirectory = ldrawDirectory / std::filesystem::path("models");
        std::cout << "ldraw dir: " << ldrawDirectory << std::endl;
        //todo code duplication
        for (const auto & entry : std::filesystem::directory_iterator(partsDirectory)) {
            if (entry.is_regular_file()) {
                auto fname = entry.path().filename();
                partNames[util::as_lower(fname.string())] = fname;
            }
        }
        for (const auto & entry : std::filesystem::directory_iterator(subpartsDirectory)) {
            if (entry.is_regular_file()) {
                auto fname = entry.path().filename();
                subpartNames[util::as_lower(fname.string())] = fname;
            }
        }
        for (const auto & entry : std::filesystem::recursive_directory_iterator(primitivesDirectory)) {
            if (entry.is_regular_file()) {
                const auto& fname = std::filesystem::relative(entry.path(), primitivesDirectory);
                primitiveNames[util::as_lower(fname.string())] = fname;
            }
        }
        for (const auto & entry : std::filesystem::recursive_directory_iterator(modelsDirectory)) {
            if (entry.is_regular_file()) {
                const auto& fname = std::filesystem::relative(entry.path(), modelsDirectory);
                modelNames[util::as_lower(fname.string())] = fname;
            }
        }
        namesInitialized = true;
        auto after = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(after-before).count();
        std::cout << "Initialized name lists in " << duration << "ms. found:" << std::endl;
        std::cout << "\t" << partNames.size() << " parts in " << partsDirectory.string() << "(first is " << partNames.begin()->second << ")" << std::endl;
        std::cout << "\t" << subpartNames.size() << " subparts in " << subpartsDirectory.string()  << "(first is " << subpartNames.begin()->second << ")"<< std::endl;
        std::cout << "\t" << primitiveNames.size() << " primitives in " << primitivesDirectory.string()  << "(first is " << primitiveNames.begin()->second << ")"<< std::endl;
        std::cout << "\t" << modelNames.size() << " files in " << modelsDirectory.string()  << "(first is " << modelNames.begin()->second << ")"<< std::endl;
    }
}
std::pair<LdrFileType, std::filesystem::path> LdrFileRepository::resolve_file(const std::string & filename) {
    initializeNames();
    if (util::starts_with(filename, "s\\")) {
        auto itSubpart = subpartNames.find(util::as_lower(filename.substr(2)));
        if (itSubpart != subpartNames.end()) {
            auto fullPath = subpartsDirectory / itSubpart->second;
            return std::make_pair(LdrFileType::SUBPART, fullPath);
        }
    }
    auto itPart = partNames.find(util::as_lower(filename));
    if (partNames.end()!=itPart) {
        auto fullPath = partsDirectory / itPart->second;
        return std::make_pair(LdrFileType::PART, fullPath);
    }
    auto itPrimitive = primitiveNames.find(util::as_lower(filename));
    if (primitiveNames.end() != itPrimitive) {
        auto fullPath = primitivesDirectory / itPrimitive->second;
        return std::make_pair(LdrFileType::PRIMITIVE, fullPath);
    }
    auto itModel = modelNames.find(util::as_lower(filename));
    if (modelNames.end() != itModel) {
        auto fullPath = modelsDirectory / itModel->second;
        return std::make_pair(LdrFileType::MODEL, fullPath);
    }
    return std::make_pair(LdrFileType::MODEL, util::extend_home_dir_path(filename));
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
LdrInstanceDummyColor::LdrInstanceDummyColor(const std::string & line): LdrInstanceDummyColor() {

}
LdrInstanceDummyColor::LdrInstanceDummyColor() {
    name = "Instance Dummy Color";
    code = -1;
    value = edge = RGB("#FFB39B");
}
