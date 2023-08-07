#pragma once

#include "../../element_tree.h"
#include "../connection.h"
#include "../connector/clip.h"
#include "../connector/cylindrical.h"
#include "../connector/finger.h"
#include "../connector/generic.h"

namespace bricksim::connection::visualization {
    class VisualizationGenerator {
    protected:
        std::shared_ptr<etree::Node> root;
        std::shared_ptr<ldr::FileNamespace> nameSpace;
        std::string fileName;

        void generateCylindrical(std::shared_ptr<CylindricalConnector> connector);
        void generateClip(std::shared_ptr<ClipConnector> connector);
        void generateFinger(std::shared_ptr<FingerConnector> connector);
        void generateGeneric(std::shared_ptr<GenericConnector> connector);

    public:
        VisualizationGenerator(const std::shared_ptr<ldr::FileNamespace>& nameSpace, const std::string& fileName);
        VisualizationGenerator(const std::shared_ptr<etree::Node>& container, const std::shared_ptr<ldr::FileNamespace>& nameSpace, const std::string& fileName);
        const std::shared_ptr<etree::Node>& getContainer() const;
        void addConnectorNodes();
        void addFileNode();
        void addXYZLineNode();
    };

}
