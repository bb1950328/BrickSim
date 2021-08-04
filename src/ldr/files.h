#pragma once

#include "colors.h"
#include <memory>
#include <set>
#include <string>
#include <vector>
namespace bricksim::ldr {
    class FileElement;

    enum FileType {
        MODEL,
        MPD_SUBFILE,
        PART,
        SUBPART,
        PRIMITIVE
    };

    enum WindingOrder {
        CW,
        CCW
    };

    WindingOrder inverseWindingOrder(WindingOrder order);

    struct BfcState {
        bool active = false;
        bool invertNext = false;
        WindingOrder windingOrder = CCW;
    };

    class FileMetaInfo {
    public:
        std::string title;               //usually the first line in the file
        std::string name;                //0 Name: xxxxx
        std::string author;              //0 Author: xxxxx
        oset_t<std::string> keywords;    //0 !KEYWORDS xxx, yyyy, zzzz
        std::vector<std::string> history;//0 !HISTORY xxxx
        std::string license;             //0 !LICENSE xxxx
        std::string theme;               //0 !THEME
        FileType type;
        friend std::ostream& operator<<(std::ostream& os, const FileMetaInfo& info);

        bool addLine(const std::string& line);

        [[nodiscard]] const std::string& getCategory();

    private:
        std::optional<std::string> category;//0 !CATEGORY xxxx
        bool firstLine = true;
    };

    class File {
    public:
        File() = default;
        virtual ~File();

        std::vector<std::shared_ptr<FileElement>> elements;
        uoset_t<std::shared_ptr<File>> mpdSubFiles;
        FileMetaInfo metaInfo;

        void printStructure(int indent = 0);
        [[nodiscard]] const std::string& getDescription() const;
        [[nodiscard]] const std::size_t& getHash() const;

        void addTextLine(const std::string& line);

    private:
        mutable std::size_t hash = 0;
        BfcState bfcState;
    };

    class FileElement {
    public:
        static std::shared_ptr<FileElement> parseLine(const std::string& line, BfcState bfcState);
        [[nodiscard]] virtual int getType() const = 0;
        virtual ~FileElement();

        unsigned int step = 0;//0 is before the first "0 STEP" line
    };

    class CommentOrMetaElement : public FileElement {
    public:
        explicit CommentOrMetaElement(const std::string& line);

        std::string content;

        [[nodiscard]] int getType() const override;
    };

    class SubfileReference : public FileElement {
    public:
        explicit SubfileReference(std::string& line, bool bfcInverted);
        bool bfcInverted;
        ColorReference color;
        float x, y, z, a, b, c, d, e, f, g, h, i;
        std::string filename;
        [[nodiscard]] int getType() const override;
        [[nodiscard]] glm::mat4 getTransformationMatrix() const;
        std::shared_ptr<File> getFile();

    private:
        std::shared_ptr<File> file = nullptr;
    };

    class Line : public FileElement {
    public:
        ColorReference color;
        float x1, y1, z1, x2, y2, z2;

        explicit Line(std::string& line);

        [[nodiscard]] int getType() const override;
    };

    class Triangle : public FileElement {
    public:
        ColorReference color;
        float x1, y1, z1, x2, y2, z2, x3, y3, z3;

        explicit Triangle(std::string& line, WindingOrder order);

        [[nodiscard]] int getType() const override;
    };

    class Quadrilateral : public FileElement {
    public:
        ColorReference color;
        float x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;

        explicit Quadrilateral(std::string& line, WindingOrder order);

        [[nodiscard]] int getType() const override;
    };

    class OptionalLine : public FileElement {
    public:
        ColorReference color;

        float x1, y1, z1, x2, y2, z2, controlX1, controlY1, controlZ1, controlX2, controlY2, controlZ2;

        explicit OptionalLine(std::string& line);

        [[nodiscard]] int getType() const override;
    };
}
