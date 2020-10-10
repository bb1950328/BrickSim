//
// Created by bb1950328 on 19.09.20.
//

#ifndef BRICKSIM_LDR_FILES_H
#define BRICKSIM_LDR_FILES_H

static const int MAX_LDR_FILENAME_LENGTH = 255;

#include <vector>
#include <map>
#include <filesystem>
#include <glm/glm.hpp>
#include <set>
#include <ostream>

class LdrFileElement;

class LdrColor;

enum LdrFileType {
    MODEL,
    MPD_SUBFILE,
    PART,
    SUBPART,
    PRIMITIVE
};

class LdrFileMetaInfo {
public:
    std::string title;//usually the first line in the file
    std::string name;//0 Name: xxxxx
    std::string author;//0 Author: xxxxx
    std::string category;//0 !CATEGORY xxxx
    std::set<std::string> keywords;//0 !KEYWORDS xxx, yyyy, zzzz
    std::vector<std::string> history;//0 !HISTORY xxxx
    std::string license;//0 !LICENSE xxxx
    std::string theme;//0 !THEME
    LdrFileType type;

    friend std::ostream &operator<<(std::ostream &os, const LdrFileMetaInfo &info);

    bool add_line(std::string line);
};

class LdrFile {
public:
    unsigned long long estimatedComplexity = 0;
    unsigned int referenceCount = 0;
    static LdrFile *parseFile(LdrFileType fileType, const std::filesystem::path &path);

    LdrFile() = default;

    std::vector<LdrFileElement *> elements;

    void printStructure(int indent=0);

    void preLoadSubfilesAndEstimateComplexity();

    [[nodiscard]] std::string getDescription() const ;

    [[nodiscard]] bool isComplexEnoughForOwnMesh() const;

    LdrFileMetaInfo metaInfo;
private:
    bool subfiles_preloaded_and_complexity_estimated = false;

    void addTextLine(const std::string &line);

    void preLoadSubfilesAndEstimateComplexityInternal();

    static long instancedMinComplexity;
};

class LdrFileElement {
public:
    static LdrFileElement *parse_line(std::string line);

    [[nodiscard]] virtual int getType() const = 0;

    virtual ~LdrFileElement();

    unsigned int step = 0;//0 is before the first "0 STEP" line
};

class LdrCommentOrMetaElement : public LdrFileElement {

public:
    explicit LdrCommentOrMetaElement(const std::string &line);

    std::string content;

    [[nodiscard]] int getType() const override;
};

class LdrSubfileReference : public LdrFileElement {

public:
    explicit LdrSubfileReference(const std::string &line);

    LdrColor *color;
    double x, y, z, a, b, c, d, e, f, g, h, i;
    std::string filename;
    [[nodiscard]] int getType() const override;
    [[nodiscard]] glm::mat4 getTransformationMatrix() const;
    LdrFile *getFile();

private:
    LdrFile *file = nullptr;
};

class LdrLine : public LdrFileElement {
public:
    LdrColor *color;
    double x1, y1, z1, x2, y2, z2;

    explicit LdrLine(const std::string &line);

    [[nodiscard]] int getType() const override;
};

class LdrTriangle : public LdrFileElement {
public:
    LdrColor *color;
    double x1, y1, z1, x2, y2, z2, x3, y3, z3;

    explicit LdrTriangle(const std::string &line);

    [[nodiscard]] int getType() const override;
};

class LdrQuadrilateral : public LdrFileElement {
public:
    LdrColor *color;
    double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;

    explicit LdrQuadrilateral(const std::string &line);

    [[nodiscard]] int getType() const override;
};

class LdrOptionalLine : public LdrFileElement {
public:
    LdrColor *color;

    double x1, y1, z1, x2, y2, z2, control_x1, control_y1, control_z1, control_x2, control_y2, control_z2;

    explicit LdrOptionalLine(const std::string &line);

    [[nodiscard]] int getType() const override;
};



#endif //BRICKSIM_LDR_FILES_H