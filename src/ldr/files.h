#pragma once

#include "colors.h"
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <stack>

namespace bricksim::ldr {
    constexpr const char* const LDR_NEWLINE = "\r\n";
    constexpr const char* const LDR_WHITESPACE = " \t";

    constexpr const char* const META_COMMAND_TEXMAP = "!TEXMAP";
    constexpr size_t META_COMMAND_TEXMAP_LEN = std::char_traits<char>::length(META_COMMAND_TEXMAP);

    class FileElement;
    class File;

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
        std::string title;                         //usually the first line in the file
        std::string name;                          //0 Name: xxxxx
        std::string author;                        //0 Author: xxxxx
        oset_t<std::string> keywords;              //0 !KEYWORDS xxx, yyyy, zzzz
        std::vector<std::string> history;          //0 !HISTORY xxxx
        std::string license;                       //0 !LICENSE xxxx
        std::string theme;                         //0 !THEME
        std::string fileTypeLine;                  //0 !LDRAW_ORG
        std::optional<std::string> headerCategory; //0 !CATEGORY xxxx
        FileType type;

        friend std::ostream& operator<<(std::ostream& os, const FileMetaInfo& info);

        bool addLine(const std::string& line);

        [[nodiscard]] const std::string& getCategory();
    private:
        bool firstLine = true;
    };

    class FileElement {
    public:
        static std::shared_ptr<FileElement> parseLine(const std::string& line, BfcState bfcState);
        [[nodiscard]] virtual int getType() const = 0;
        [[nodiscard]] virtual std::string getLdrLine() const = 0;
        virtual ~FileElement();

        unsigned int step = 0;//0 is before the first "0 STEP" line
        bool hidden = false;//hidden elements do not have to be rendered. for example, they are part of fallback sections or come after a meta-command that hides them.
    };

    class CommentOrMetaElement : public FileElement {
    public:
        explicit CommentOrMetaElement(const std::string& line);
        std::string content;

        [[nodiscard]] int getType() const override;
        [[nodiscard]] std::string getLdrLine() const override;
    };

    class SubfileReference : public FileElement {
    public:
        explicit SubfileReference(const std::string& line, bool bfcInverted);
        explicit SubfileReference(ColorReference color, const glm::mat4& transformation, bool bfcInverted);
        bool bfcInverted;
        ColorReference color;
        float x, y, z, a, b, c, d, e, f, g, h, i;
        std::string filename;
        [[nodiscard]] int getType() const override;
        [[nodiscard]] std::string getLdrLine() const override;
        [[nodiscard]] glm::mat4 getTransformationMatrix() const;
        void setTransformationMatrix(const glm::mat4& matrix);
        std::shared_ptr<File> getFile();

    private:
        std::shared_ptr<File> file = nullptr;
    };

    class Line : public FileElement {
    public:
        ColorReference color;
        float x1, y1, z1, x2, y2, z2;

        explicit Line(const std::string& line);

        [[nodiscard]] int getType() const override;
        [[nodiscard]] std::string getLdrLine() const override;
    };

    class Triangle : public FileElement {
    public:
        ColorReference color;
        float x1, y1, z1, x2, y2, z2, x3, y3, z3;

        explicit Triangle(const std::string& line, WindingOrder order);

        [[nodiscard]] int getType() const override;
        [[nodiscard]] std::string getLdrLine() const override;
    };

    class Quadrilateral : public FileElement {
    public:
        ColorReference color;
        float x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;

        explicit Quadrilateral(const std::string& line, WindingOrder order);

        [[nodiscard]] int getType() const override;
        [[nodiscard]] std::string getLdrLine() const override;
    };

    class OptionalLine : public FileElement {
    public:
        ColorReference color;

        float x1, y1, z1, x2, y2, z2, controlX1, controlY1, controlZ1, controlX2, controlY2, controlZ2;

        explicit OptionalLine(const std::string& line);

        [[nodiscard]] int getType() const override;
        [[nodiscard]] std::string getLdrLine() const override;
    };

    class TexmapStartCommand : public CommentOrMetaElement  {
    public:
        enum ProjectionMethod {
            PLANAR,
            CYLINDRICAL,
            SPHERICAL,
        };
        ProjectionMethod projectionMethod;
        float x1, y1, z1, x2, y2, z2, x3, y3, z3, a, b;//a and b may be not used depending on projectionMethod
        std::string textureFilename;
        std::optional<std::string> glossmapFileName;

        explicit TexmapStartCommand(const std::string& line);

        static bool doesLineMatch(const std::string& line);
    };

    struct TexmapState {
        std::weak_ptr<TexmapStartCommand> startCommand;
        bool fallbackSectionReached;
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
        std::stack<TexmapState> texmapStateStack;
    };

    namespace {
        const char* getFileTypeStr(FileType type);
    }
}
