//
// Created by bb1950328 on 19.09.20.
//

#ifndef BRICKSIM_LDR_OBJECTS_H
#define BRICKSIM_LDR_OBJECTS_H

static const int MAX_LDR_FILENAME_LENGTH = 255;

#include <vector>
#include <map>
#include <glm/glm.hpp>

class LdrFileElement;

class LdrColor;

struct RGB {
    RGB() = default;

    explicit RGB(std::string htmlCode);

    unsigned char red, green, blue;
};

struct LdrColorMaterial {
    enum Type {
        GLITTER, SPECKLE
    };
    Type type;
    RGB value;
    unsigned char alpha;
    unsigned char luminance;
    double fraction, vfraction;
    int size = 0;
    int minsize = 0;
    int maxsize = 0;
};

class LdrFile {
public:
    static LdrFile* parseFile(const std::string &filename);

    LdrFile() = default;

    std::vector<LdrFileElement *> elements;
private:

    static std::ifstream openFile(const std::string &filename);

    void addTextLine(const std::string &line);
};

class LdrFileElement {
public:
    static LdrFileElement *parse_line(std::string line);

    virtual int getType() const = 0;
};

class LdrCommentOrMetaElement : public LdrFileElement {

public:
    explicit LdrCommentOrMetaElement(const std::string &line);

    std::string content;

    int getType() const override;
};

class LdrSubfileReference : public LdrFileElement {

public:
    explicit LdrSubfileReference(const std::string &line);

    LdrColor *color;
    double x, y, z, a, b, c, d, e, f, g, h, i;
    std::string filename;
    int getType() const override;

    LdrFile *getFile();

private:
    LdrFile *file = nullptr;
};

class LdrLine : public LdrFileElement {
public:
    LdrColor *color;
    double x1, y1, z1, x2, y2, z2;

    explicit LdrLine(const std::string &line);

    int getType() const override;
};

class LdrTriangle : public LdrFileElement {
public:
    LdrColor *color;
    double x1, y1, z1, x2, y2, z2, x3, y3, z3;

    explicit LdrTriangle(const std::string &line);

    int getType() const override;
};

class LdrQuadrilateral : public LdrFileElement {
public:
    LdrColor *color;
    double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;

    explicit LdrQuadrilateral(const std::string &line);

    int getType() const override;
};

class LdrOptionalLine : public LdrFileElement {
public:
    LdrColor *color;

    double x1, y1, z1, x2, y2, z2, control_x1, control_y1, control_z1, control_x2, control_y2, control_z2;

    explicit LdrOptionalLine(const std::string &line);

    int getType() const override;
};

class LdrColor {
public:
    enum Finish {
        NONE, CHROME, PEARLESCENT, RUBBER, MATTE_METALLIC, METAL, MATERIAL
    };

    LdrColor() = default;

    explicit LdrColor(const std::string &line);

    glm::vec4 asGlmVector() const;

    std::string name;
    int code;
    RGB value;
    RGB edge;
    unsigned char alpha = 255;
    unsigned char luminance = 0;
    Finish finish = NONE;
    LdrColorMaterial *material = nullptr;
};

class LdrColorRepository {
private:
    std::map<int, LdrColor> colors;
    static LdrColorRepository *instance;
public:
    static LdrColorRepository *getInstance();

public:

    void initialize();

    LdrColor *get_color(int colorCode);
};

class LdrFileRepository {
private:
    static std::map<std::string, LdrFile> files;
public:
    static LdrFile *get_file(const std::string &filename);

    static void clear_cache();

    static void add_file(const std::string &filename, const LdrFile *file);
};

#endif //BRICKSIM_LDR_OBJECTS_H
