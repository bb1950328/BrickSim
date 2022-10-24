#pragma once
#include <string>
#include <vector>

namespace bricksim::connection {
    enum class Gender {
        M,
        F,
    };

    class Connector {
        std::string group;
    };

    enum class CylindricalShapeType {
        ROUND,
        AXLE,
        SQUARE,
    };
    class CylindricalShapePart {
    public:
        CylindricalShapeType type;
        bool flexibleRadius;
        float radius;
        float length;
    };

    class CylindricalConnector : public Connector {
    public:
        Gender gender;
        std::vector<CylindricalShapePart> parts;
        bool openStart, openEnd;
        bool slide;
    };

    class ClipConnector : public Connector {
    public:
        float radius;
        float width;
        bool slide;
    };

    class FingerConnector : public Connector {
        Gender firstFingerGender;
        float radius;
        std::vector<float> fingerWidths;
    };

    //TODO generic
}