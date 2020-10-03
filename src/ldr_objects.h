//
// Created by bb1950328 on 19.09.20.
//

#ifndef BRICKSIM_LDR_OBJECTS_H
#define BRICKSIM_LDR_OBJECTS_H

static const int MAX_LDR_FILENAME_LENGTH = 255;

#include <vector>
#include <map>
#include <filesystem>
#include <glm/glm.hpp>

class LdrFileElement;

class LdrColor;

struct RGB {
    RGB() = default;

    explicit RGB(std::string htmlCode);

    unsigned char red, green, blue;
    [[nodiscard]] glm::vec3 asGlmVector() const;
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
    unsigned long long estimatedComplexity = 0;
    unsigned int referenceCount = 0;
    static LdrFile* parseFile(const std::filesystem::path &path);

    LdrFile() = default;

    std::vector<LdrFileElement *> elements;

    void printStructure(int indent=0);

    void preLoadSubfilesAndEstimateComplexity();

    std::string getDescription() const ;

    bool isComplexEnoughForOwnMesh() const;
private:
    bool subfiles_preloaded_and_complexity_estimated = false;

    void addTextLine(const std::string &line);

    void preLoadSubfilesAndEstimateComplexityInternal();
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

class LdrColor {
public:
    enum Finish {
        NONE, CHROME, PEARLESCENT, RUBBER, MATTE_METALLIC, METAL, MATERIAL
    };

    LdrColor() = default;

    explicit LdrColor(const std::string &line);

    std::string name;
    int code;
    RGB value;
    RGB edge;
    unsigned char alpha = 255;
    unsigned char luminance = 0;
    Finish finish = NONE;
    LdrColorMaterial *material = nullptr;
};

class LdrInstanceDummyColor : public LdrColor {
public:
    LdrInstanceDummyColor();
    explicit LdrInstanceDummyColor(const std::string &line);
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

    static LdrInstanceDummyColor instDummyColor;

};

enum LdrFileType {
    MODEL,
    MPD_SUBFILE,
    PART,
    SUBPART,
    PRIMITIVE
};

class LdrFileRepository {
public:
    static LdrFile *get_file(const std::string &filename);

    static LdrFileType get_file_type(const std::string &filename);

    static std::pair<LdrFileType, std::filesystem::path> resolve_file(const std::string &filename);

    static void clear_cache();

    static std::map<std::string, std::pair<LdrFileType, LdrFile*>> files;

    static void add_file(const std::string &filename, LdrFile *file, LdrFileType type);

    static void initializeNames();
private:
    static std::filesystem::path ldrawDirectory;
    static std::filesystem::path partsDirectory;
    static std::filesystem::path subpartsDirectory;
    static std::filesystem::path primitivesDirectory;
    static std::filesystem::path modelsDirectory;
    static bool namesInitialized;

    //keys: name as lowercase values: name as original case
    static std::map<std::string, std::filesystem::path> primitiveNames;
    static std::map<std::string, std::filesystem::path> subpartNames;
    static std::map<std::string, std::filesystem::path> partNames;
    static std::map<std::string, std::filesystem::path> modelNames;
};

#endif //BRICKSIM_LDR_OBJECTS_H
