#pragma once

#include "../connection/ldcad_snap_meta/base.h"
#include "../helpers/util.h"
#include "colors.h"
#include <array>
#include <memory>
#include <set>
#include <stack>
#include <string>
#include <vector>

namespace bricksim::ldr {
    constexpr const char* const LDR_NEWLINE = "\r\n";
    constexpr const char* const LDR_WHITESPACE = " \t";

    constexpr const char* const META_COMMAND_TEXMAP = "!TEXMAP";
    constexpr size_t META_COMMAND_TEXMAP_LEN = std::char_traits<char>::length(META_COMMAND_TEXMAP);

    class FileElement;
    class File;
    class TexmapStartCommand;

    enum class FileType {
        MODEL,
        MPD_SUBFILE,
        PART,
        SUBPART,
        PRIMITIVE
    };

    enum class WindingOrder {
        CW,
        CCW
    };

    WindingOrder inverseWindingOrder(WindingOrder order);

    struct BfcState {
        bool active = false;
        bool invertNext = false;
        WindingOrder windingOrder = WindingOrder::CCW;
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
        static std::shared_ptr<FileElement> parseLine(std::string_view line, BfcState bfcState);
        [[nodiscard]] virtual int getType() const = 0;
        [[nodiscard]] virtual std::string getLdrLine() const = 0;
        FileElement();
        virtual ~FileElement();

        unsigned int step = 0;//0 is before the first "0 STEP" line

        ///hidden elements are not rendered. for example, they are part of fallback sections or come after a meta-command that hides them.
        bool hidden = false;

        ///this is only non-nullptr when !TEXMAP START appeared *in the same file* a few lines above and !TEXMAP FALLBACK|END didn't appear yet.
        ///this line therefore counts to the <geometry1> or <geometry2> section
        ///in all other cases this is nullptr
        std::shared_ptr<TexmapStartCommand> directTexmap;
    };

    class CommentOrMetaElement : public FileElement {
    public:
        explicit CommentOrMetaElement(std::string_view line);
        std::string content;

        [[nodiscard]] int getType() const override;
        [[nodiscard]] std::string getLdrLine() const override;
    };

    class SubfileReference : public FileElement {
    public:
        explicit SubfileReference(std::string_view line, bool bfcInverted);
        explicit SubfileReference(ColorReference color, const glm::mat4& transformation, bool bfcInverted);
        bool bfcInverted;
        ColorReference color;
        std::array<float, 12> numbers;
        std::string filename;
        [[nodiscard]] int getType() const override;
        [[nodiscard]] std::string getLdrLine() const override;
        [[nodiscard]] glm::mat4 getTransformationMatrix() const;
        void setTransformationMatrix(const glm::mat4& matrix);
        std::shared_ptr<File> getFile();

        inline float& x() { return numbers[0]; }
        inline float& y() { return numbers[1]; }
        inline float& z() { return numbers[2]; }
        inline float& a() { return numbers[3]; }
        inline float& b() { return numbers[4]; }
        inline float& c() { return numbers[5]; }
        inline float& d() { return numbers[6]; }
        inline float& e() { return numbers[7]; }
        inline float& f() { return numbers[8]; }
        inline float& g() { return numbers[9]; }
        inline float& h() { return numbers[10]; }
        inline float& i() { return numbers[11]; }

        [[nodiscard]] inline const float& x() const { return numbers[0]; }
        [[nodiscard]] inline const float& y() const { return numbers[1]; }
        [[nodiscard]] inline const float& z() const { return numbers[2]; }
        [[nodiscard]] inline const float& a() const { return numbers[3]; }
        [[nodiscard]] inline const float& b() const { return numbers[4]; }
        [[nodiscard]] inline const float& c() const { return numbers[5]; }
        [[nodiscard]] inline const float& d() const { return numbers[6]; }
        [[nodiscard]] inline const float& e() const { return numbers[7]; }
        [[nodiscard]] inline const float& f() const { return numbers[8]; }
        [[nodiscard]] inline const float& g() const { return numbers[9]; }
        [[nodiscard]] inline const float& h() const { return numbers[10]; }
        [[nodiscard]] inline const float& i() const { return numbers[11]; }

    private:
        std::shared_ptr<File> file = nullptr;
    };

    class Line : public FileElement {
    public:
        ColorReference color;
        std::array<float, 6> coords;

        explicit Line(std::string_view line);

        [[nodiscard]] int getType() const override;
        [[nodiscard]] std::string getLdrLine() const override;

        inline float& x1() { return coords[0]; }
        inline float& y1() { return coords[1]; }
        inline float& z1() { return coords[2]; }
        inline float& x2() { return coords[3]; }
        inline float& y2() { return coords[4]; }
        inline float& z2() { return coords[5]; }

        [[nodiscard]] inline const float& x1() const { return coords[0]; }
        [[nodiscard]] inline const float& y1() const { return coords[1]; }
        [[nodiscard]] inline const float& z1() const { return coords[2]; }
        [[nodiscard]] inline const float& x2() const { return coords[3]; }
        [[nodiscard]] inline const float& y2() const { return coords[4]; }
        [[nodiscard]] inline const float& z2() const { return coords[5]; }
    };

    class Triangle : public FileElement {
    public:
        ColorReference color;
        std::array<float, 9> coords;

        explicit Triangle(std::string_view line, WindingOrder order);

        [[nodiscard]] int getType() const override;
        [[nodiscard]] std::string getLdrLine() const override;

        inline float& x1() { return coords[0]; }
        inline float& y1() { return coords[1]; }
        inline float& z1() { return coords[2]; }
        inline float& x2() { return coords[3]; }
        inline float& y2() { return coords[4]; }
        inline float& z2() { return coords[5]; }
        inline float& x3() { return coords[6]; }
        inline float& y3() { return coords[7]; }
        inline float& z3() { return coords[8]; }

        [[nodiscard]] inline const float& x1() const { return coords[0]; }
        [[nodiscard]] inline const float& y1() const { return coords[1]; }
        [[nodiscard]] inline const float& z1() const { return coords[2]; }
        [[nodiscard]] inline const float& x2() const { return coords[3]; }
        [[nodiscard]] inline const float& y2() const { return coords[4]; }
        [[nodiscard]] inline const float& z2() const { return coords[5]; }
        [[nodiscard]] inline const float& x3() const { return coords[6]; }
        [[nodiscard]] inline const float& y3() const { return coords[7]; }
        [[nodiscard]] inline const float& z3() const { return coords[8]; }
    };

    class Quadrilateral : public FileElement {
    public:
        ColorReference color;
        std::array<float, 12> coords;

        explicit Quadrilateral(std::string_view line, WindingOrder order);

        [[nodiscard]] int getType() const override;
        [[nodiscard]] std::string getLdrLine() const override;

        inline float& x1() { return coords[0]; }
        inline float& y1() { return coords[1]; }
        inline float& z1() { return coords[2]; }
        inline float& x2() { return coords[3]; }
        inline float& y2() { return coords[4]; }
        inline float& z2() { return coords[5]; }
        inline float& x3() { return coords[6]; }
        inline float& y3() { return coords[7]; }
        inline float& z3() { return coords[8]; }
        inline float& x4() { return coords[9]; }
        inline float& y4() { return coords[10]; }
        inline float& z4() { return coords[11]; }

        [[nodiscard]] inline const float& x1() const { return coords[0]; }
        [[nodiscard]] inline const float& y1() const { return coords[1]; }
        [[nodiscard]] inline const float& z1() const { return coords[2]; }
        [[nodiscard]] inline const float& x2() const { return coords[3]; }
        [[nodiscard]] inline const float& y2() const { return coords[4]; }
        [[nodiscard]] inline const float& z2() const { return coords[5]; }
        [[nodiscard]] inline const float& x3() const { return coords[6]; }
        [[nodiscard]] inline const float& y3() const { return coords[7]; }
        [[nodiscard]] inline const float& z3() const { return coords[8]; }
        [[nodiscard]] inline const float& x4() const { return coords[9]; }
        [[nodiscard]] inline const float& y4() const { return coords[10]; }
        [[nodiscard]] inline const float& z4() const { return coords[11]; }
    };

    class OptionalLine : public FileElement {
    public:
        ColorReference color;

        std::array<float, 12> coords;

        explicit OptionalLine(std::string_view line);

        [[nodiscard]] int getType() const override;
        [[nodiscard]] std::string getLdrLine() const override;

        inline float& x1() { return coords[0]; }
        inline float& y1() { return coords[1]; }
        inline float& z1() { return coords[2]; }
        inline float& x2() { return coords[3]; }
        inline float& y2() { return coords[4]; }
        inline float& z2() { return coords[5]; }
        inline float& controlX1() { return coords[6]; }
        inline float& controlY1() { return coords[7]; }
        inline float& controlZ1() { return coords[8]; }
        inline float& controlX2() { return coords[9]; }
        inline float& controlY2() { return coords[10]; }
        inline float& controlZ2() { return coords[11]; }

        [[nodiscard]] inline const float& x1() const { return coords[0]; }
        [[nodiscard]] inline const float& y1() const { return coords[1]; }
        [[nodiscard]] inline const float& z1() const { return coords[2]; }
        [[nodiscard]] inline const float& x2() const { return coords[3]; }
        [[nodiscard]] inline const float& y2() const { return coords[4]; }
        [[nodiscard]] inline const float& z2() const { return coords[5]; }
        [[nodiscard]] inline const float& controlX1() const { return coords[6]; }
        [[nodiscard]] inline const float& controlY1() const { return coords[7]; }
        [[nodiscard]] inline const float& controlZ1() const { return coords[8]; }
        [[nodiscard]] inline const float& controlX2() const { return coords[9]; }
        [[nodiscard]] inline const float& controlY2() const { return coords[10]; }
        [[nodiscard]] inline const float& controlZ2() const { return coords[11]; }
    };

    class TexmapStartCommand : public CommentOrMetaElement {
    public:
        enum class ProjectionMethod {
            PLANAR,
            CYLINDRICAL,
            SPHERICAL,
        };
        ProjectionMethod projectionMethod;
        std::array<float, 11> coords;//x1, y1, z1, x2, y2, z2, x3, y3, z3, a, b;//a and b may be not used depending on projectionMethod
        std::string textureFilename;
        std::optional<std::string> glossmapFileName;

        explicit TexmapStartCommand(std::string_view line);
        TexmapStartCommand(const TexmapStartCommand& other);

        static bool doesLineMatch(std::string_view line);

        [[nodiscard]] std::string getLdrLine() const override;

        inline float& x1() { return coords[0]; }
        inline float& y1() { return coords[1]; }
        inline float& z1() { return coords[2]; }
        inline float& x2() { return coords[3]; }
        inline float& y2() { return coords[4]; }
        inline float& z2() { return coords[5]; }
        inline float& x3() { return coords[6]; }
        inline float& y3() { return coords[7]; }
        inline float& z3() { return coords[8]; }
        inline float& a() { return coords[9]; }
        inline float& b() { return coords[10]; }

        [[nodiscard]] inline const float& x1() const { return coords[0]; }
        [[nodiscard]] inline const float& y1() const { return coords[1]; }
        [[nodiscard]] inline const float& z1() const { return coords[2]; }
        [[nodiscard]] inline const float& x2() const { return coords[3]; }
        [[nodiscard]] inline const float& y2() const { return coords[4]; }
        [[nodiscard]] inline const float& z2() const { return coords[5]; }
        [[nodiscard]] inline const float& x3() const { return coords[6]; }
        [[nodiscard]] inline const float& y3() const { return coords[7]; }
        [[nodiscard]] inline const float& z3() const { return coords[8]; }
        [[nodiscard]] inline const float& a() const { return coords[9]; }
        [[nodiscard]] inline const float& b() const { return coords[10]; }
    };

    struct TexmapState {
        std::shared_ptr<TexmapStartCommand> startCommand;
        bool fallbackSectionReached;

        void startOrNext(const std::shared_ptr<TexmapStartCommand>& command);
        void fallback();
        void end();
        [[nodiscard]] bool isActive() const;
    };

    class File {
    public:
        File() = default;
        virtual ~File();

        std::vector<std::shared_ptr<FileElement>> elements;
        uoset_t<std::shared_ptr<File>> mpdSubFiles;
        FileMetaInfo metaInfo;
        std::vector<std::shared_ptr<connection::ldcad_snap_meta::MetaCommand>> ldcadSnapMetas;

        void printStructure(int indent = 0) const;
        [[nodiscard]] const std::string& getDescription() const;
        [[nodiscard]] const std::size_t& getHash() const;

        void addTextLine(std::string_view line);
        void addShadowContent(const std::string& shadowContent);

    private:
        constexpr static const char LDCAD_META_START[] = "!LDCAD";
        mutable std::size_t hash = 0;
        BfcState bfcState;
        TexmapState texmapState;
        void parseLdcadMeta(const std::string_view& metaContent);
    };

    namespace {
        const char* getFileTypeStr(FileType type);
    }
}

namespace robin_hood {
    template <>
    struct hash<bricksim::ldr::TexmapStartCommand> {
        size_t operator()(bricksim::ldr::TexmapStartCommand const& value) const noexcept {
            return hash<std::string>()(value.getLdrLine());//todo make this faster while still being correct
        }
    };
}
