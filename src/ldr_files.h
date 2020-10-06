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

class LdrFileElement;

class LdrColor;

enum LdrFileType {
    MODEL,
    MPD_SUBFILE,
    PART,
    SUBPART,
    PRIMITIVE
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

    LdrFileType type;
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
