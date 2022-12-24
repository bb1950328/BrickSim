#include "connection_visualization.h"
#include "scene.h"
#include "../connection/connector_data_visualizer.h"

namespace bricksim::graphics::connection_visualization {
    namespace {
        constexpr glm::usvec2 IMAGE_SIZE{512, 512};
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
        scene = scenes::create(scenes::CONNECTION_VISUALIZATION_SCENE_ID);
        scene->setCamera(camera);
        scene->setImageSize(IMAGE_SIZE);

        initialized = true;
    }

    void setVisualizedPart(const std::string &partName) {
        if (visualizedPart != partName) {
            visualizedPart = partName;
            const auto rootNode = connection::visualization::generateVisualization(partName);
            scene->setRootNode(rootNode);
        }
    }

    unsigned int getImage() {
        scene->updateImage();
        return scene->getImage().getTexBO();
    }

    void cleanupIfNeeded() {
        scene = nullptr;
    }

    const std::shared_ptr<CadCamera> &getCamera() {
        return camera;
    }
}
