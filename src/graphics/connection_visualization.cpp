#include "connection_visualization.h"
#include "../connection/visualization/connector_data_visualizer.h"
#include "scene.h"

namespace bricksim::graphics::connection_visualization {
    namespace {
        BRICKSIM_GLM_USVEC_CONST glm::usvec2 IMAGE_SIZE{static_cast<unsigned short>(512), static_cast<unsigned short>(512)};
        std::shared_ptr<Scene> scene;
        std::shared_ptr<CadCamera> camera;
        std::string visualizedPart;
    }

    void initializeIfNeeded() {
        static bool initialized = false;
        if (initialized) {
            return;
        }
        camera = std::make_shared<CadCamera>();
        camera->setDistance(1.f);
        scene = scenes::create(scenes::CONNECTION_VISUALIZATION_SCENE_ID);
        scene->setCamera(camera);
        scene->setImageSize(IMAGE_SIZE);

        initialized = true;
    }

    void setVisualizedPart(const std::shared_ptr<ldr::FileNamespace>& nameSpace, const std::string& partName) {
        if (visualizedPart != partName) {
            visualizedPart = partName;
            connection::visualization::VisualizationGenerator generator(std::make_shared<etree::RootNode>(), nameSpace, partName);
            generator.addFileNode();
            generator.addXYZLineNode();
            generator.addConnectorNodes();
            scene->setRootNode(generator.getContainer());
        }
    }

    unsigned int getImage() {
        scene->updateImage();
        return scene->getImage().getTexBO();
    }

    void cleanupIfNeeded() {
        scene = nullptr;
    }

    const std::shared_ptr<CadCamera>& getCamera() {
        return camera;
    }
}
