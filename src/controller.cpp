#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "controller.h"
#include "config.h"
#include "db.h"
#include "graphics/opengl_native_or_replacement.h"
#include "graphics/orientation_cube.h"
#include "graphics/shaders.h"
#include "gui/gui.h"
#include "gui/modals.h"
#include "helpers/util.h"
#include "info_providers/bricklink_constants_provider.h"
#include "keyboard_shortcut_manager.h"
#include "ldr/file_repo.h"
#include "ldr/file_writer.h"
#include "ldr/shadow_file_repo.h"
#include "logging/latest_log_messages_tank.h"
#include "logging/logger.h"
#include "metrics.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include "user_actions.h"
#include "graphics/connection_visualization.h"
#include <glad/glad.h>
#include <palanteer.h>
#include <spdlog/spdlog.h>

#ifdef BRICKSIM_USE_RENDERDOC
    #include <link.h>
#endif

namespace bricksim::controller {
    namespace {
        GLFWwindow* window;
        bool selectionChanged = false;
        std::shared_ptr<graphics::ThumbnailGenerator> thumbnailGenerator;
        std::list<std::shared_ptr<Editor>> editors;
        std::shared_ptr<Editor> activeEditor = nullptr;

        unsigned int windowWidth;
        unsigned int windowHeight;

        bool openGlInitialized = false;

        bool userWantsToExit = false;

        transform_gizmo::RotationState transformGizmoRotationState = transform_gizmo::RotationState::WORLD;

        uomap_t<unsigned int, Task> backgroundTasks;
        std::queue<Task> foregroundTasks;
        std::shared_ptr<gui::modals::Modal> foregroundTaskWaitModal = nullptr;

        std::chrono::milliseconds idle_sleep(25);

        constexpr unsigned short lastFrameTimesSize = 256;
        float lastFrameTimes[lastFrameTimesSize] = {0};//in ms
        unsigned short lastFrameTimesStartIdx = 0;

        std::shared_ptr<efsw::FileWatcher> fileWatcher;

#ifdef BRICKSIM_USE_RENDERDOC
        RENDERDOC_API_1_1_2* rdoc_api = nullptr;
#endif

        void APIENTRY openGlDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
            if (id == 131185 || id == 131169) {
                //Buffer detailed info: Buffer object 2 (bound to GL_ELEMENT_ARRAY_BUFFER_ARB, usage hint is GL_STREAM_DRAW) will use VIDEO memory as the source for buffer object operations.
                //Framebuffer detailed info: The driver allocated storage for renderbuffer 1.
                return;
            }
            spdlog::level::level_enum level;
            switch (severity) {
                case GL_DEBUG_SEVERITY_NOTIFICATION: level = spdlog::level::info; break;
                case GL_DEBUG_SEVERITY_LOW: level = spdlog::level::debug; break;
                case GL_DEBUG_SEVERITY_MEDIUM: level = spdlog::level::warn; break;
                case GL_DEBUG_SEVERITY_HIGH: level = spdlog::level::err; break;
                default: level = spdlog::level::info;
            }

            const char* sourceStr;
            switch (source) {
                case GL_DEBUG_SOURCE_API: sourceStr = "API"; break;
                case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceStr = "WINDOW_SYSTEM"; break;
                case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "SHADER_COMPILER"; break;
                case GL_DEBUG_SOURCE_THIRD_PARTY: sourceStr = "THIRD_PARTY"; break;
                case GL_DEBUG_SOURCE_APPLICATION: sourceStr = "APPLICATION"; break;
                case GL_DEBUG_SOURCE_OTHER:
                default: sourceStr = "OTHER"; break;
            }

            const char* typeStr;
            switch (type) {
                case GL_DEBUG_TYPE_ERROR: typeStr = "ERROR"; break;
                case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "DEPRECATED_BEHAVIOR"; break;
                case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeStr = "UNDEFINED_BEHAVIOR"; break;
                case GL_DEBUG_TYPE_PORTABILITY: typeStr = "PORTABILITY"; break;
                case GL_DEBUG_TYPE_PERFORMANCE: typeStr = "PERFORMANCE"; break;
                case GL_DEBUG_TYPE_MARKER: typeStr = "MARKER"; break;
                case GL_DEBUG_TYPE_PUSH_GROUP: typeStr = "PUSH_GROUP"; break;
                case GL_DEBUG_TYPE_POP_GROUP: typeStr = "POP_GROUP"; break;
                case GL_DEBUG_TYPE_OTHER:
                default: typeStr = "OTHER"; break;
            }

            spdlog::log(level, "OpenGL debug message: source={}, type={}, id={}: {}", sourceStr, typeStr, id, message);
        }

        bool initializeGL() {
            if (openGlInitialized) {
                throw std::invalid_argument("attempting to initialize OpenGL twice");
            }
            const auto enableDebugOutput = config::get(config::ENABLE_GL_DEBUG_OUTPUT);
            glfwSetErrorCallback(glfwErrorCallback);
            glfwInit();
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_SAMPLES, (int)(config::get(config::MSAA_SAMPLES)));
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            if (enableDebugOutput) {
                spdlog::debug("OpenGL debug context is enabled");
                glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
            }

#ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

            windowWidth = std::max(25, config::get(config::SCREEN_WIDTH));
            windowHeight = std::max(25, config::get(config::SCREEN_HEIGHT));

            window = glfwCreateWindow(windowWidth, windowHeight, "BrickSim", nullptr, nullptr);
            if (window == nullptr) {
                spdlog::critical("Failed to create GLFW window");
                glfwTerminate();
                return false;
            }
            glfwMakeContextCurrent(window);
            if (!gladLoadGL()) {
                spdlog::critical("gladLoadGL() failed");
                glfwTerminate();
                return false;
            }
            if (!config::get(config::ENABLE_VSYNC)) {
                glfwSwapInterval(0);
            }
            glfwSetFramebufferSizeCallback(window, window_size_callback);
            glfwSetScrollCallback(window, scroll_callback);
            glfwSetKeyCallback(window, keyCallback);

            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
                spdlog::error("Failed to initialize GLAD");
                return false;
            }

            glEnable(GL_DEPTH_TEST);

            if (config::get(config::FACE_CULLING_ENABLED)) {
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
            }
            glFrontFace(GL_CCW);

            GLint flags;
            glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

            if (flags & GL_CONTEXT_FLAG_DEBUG_BIT && glDebugMessageCallback) {
                glEnable(GL_DEBUG_OUTPUT);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                glDebugMessageCallback(static_cast<GLDEBUGPROC>(openGlDebugMessageCallback), nullptr);
                glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
            }

#ifdef BRICKSIM_USE_RENDERDOC
            if (void* mod = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD)) {
                auto RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
                int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&rdoc_api);
                assert(ret == 1);
            }
#endif

            graphics::opengl_native_or_replacement::initialize();

            spdlog::info("OpenGL initialized");
            openGlInitialized = true;
            return true;
        }

        void window_size_callback(GLFWwindow* _, int width, int height) {
            if (windowWidth != width || windowHeight != height) {
                windowWidth = width;
                windowHeight = height;
                controller::executeOpenGL([&]() {
                    glViewport(0, 0, width, height);
                });
            }
        }

        void keyCallback(GLFWwindow* _, int key, int scancode, int action, int mods) {
            keyboard_shortcut_manager::shortcutPressed(key, action, mods, gui::areKeysCaptured());
        }

        void scroll_callback(GLFWwindow* _, double xoffset, double yoffset) {
            //todo use xoffset to do something, maybe pan?
            gui::setLastScrollDeltaY(yoffset);
            if (ImGui::GetIO().WantCaptureMouse) {
                return;
            }
            //todo find out on which window the mouse is
            //camera->moveForwardBackward((float)yoffset);
        }

        void checkForFinishedBackgroundTasks() {
            static double lastCheck = 0;
            auto now = glfwGetTime();
            if (now - lastCheck > 0.5) {
                for (auto iter = backgroundTasks.begin(); iter != backgroundTasks.end();) {
                    if (iter->second.isDone()) {
                        iter->second.joinThread();
                        iter = backgroundTasks.erase(iter);
                    } else {
                        ++iter;
                    }
                }
                lastCheck = now;
            }
        }

        bool loopPartsLibrarySetupPrompt() {
            auto status = gui::PartsLibrarySetupResponse::RUNNING;
            while (status == gui::PartsLibrarySetupResponse::RUNNING
                   && !glfwWindowShouldClose(window)) {
                gui::beginFrame();
                status = gui::drawPartsLibrarySetupScreen();
                gui::endFrame();
                executeOpenGL([]() {
                    glfwSwapBuffers(window);
                    glfwPollEvents();
                });
            }
            if (status == gui::PartsLibrarySetupResponse::REQUEST_EXIT || glfwWindowShouldClose(window)) {
                return false;
            }
            return true;
        }

        bool initialize() {
            plInitAndStart("BrickSim");
            plDeclareThread("Main Thread");
            plFunction();
            logging::initialize();

            spdlog::info("current working directory is {}", std::filesystem::absolute(std::filesystem::current_path()).string());

            db::initialize();

            if (!initializeGL()) {
                spdlog::critical("failed to initialize OpenGL / glfw, exiting");
                return false;
            }

            util::setStbiFlipVertically(true);

            graphics::shaders::initialize();

            gui::setWindow(window);
            gui::initialize();

            while (!ldr::file_repo::checkLdrawLibraryLocation()) {
                if (!loopPartsLibrarySetupPrompt()) {
                    spdlog::info("user closed window while in parts library setup prompt, exiting application");
                    cleanup();
                    return false;
                }
            }

            Task initSteps[]{
                    {"load color definitions", ldr::color_repo::initialize},
                    {"initialize shadow file repo", ldr::file_repo::initializeShadowFileRepo},
                    {"initialize file list", [](float* progress) { ldr::file_repo::get().initialize(progress); spdlog::info("File Repo base path is {}", ldr::file_repo::get().getBasePath().string()); }},
                    {"initialize price guide provider", info_providers::price_guide::initialize},
                    {"initialize thumbnail generator", []() { thumbnailGenerator = std::make_shared<graphics::ThumbnailGenerator>(); }},
                    {"initialize BrickLink constants", info_providers::bricklink_constants::initialize},
                    {"initialize keyboard shortcuts", keyboard_shortcut_manager::initialize},
                    {"initialize orientation cube generator", graphics::orientation_cube::initialize},
            };

            const auto drawWaitMessageInFrame = [](const std::string& message, float progress) {
                gui::beginFrame();
                gui::drawWaitMessage(message, progress);
                gui::endFrame();

                executeOpenGL([]() {
                    glfwSwapBuffers(window);
                    glfwPollEvents();
                });
            };

            constexpr float progressStep = 1.0f / std::size(initSteps);
            for (int i = 0; i < std::size(initSteps); ++i) {
                auto& currentStep = initSteps[i];
                currentStep.startThread();
                while (!currentStep.isDone()) {
                    if (gui::isSetupDone()) {
                        drawWaitMessageInFrame(currentStep.getName(), progressStep * (i + currentStep.getProgress()));
                    } else {
                        std::chrono::milliseconds sleepTime(16);
                        std::this_thread::sleep_for(sleepTime);
                    }
                }
                currentStep.joinThread();
            }

            drawWaitMessageInFrame("initialisation finished", 1.f);

            fileWatcher = std::make_shared<efsw::FileWatcher>();
            fileWatcher->watch();
            return true;
        }

        void cleanup() {
            plFunction();
            if (ldr::file_repo::isInitialized()) {
                ldr::file_repo::get().cleanup();
            }
            while (!foregroundTasks.empty()) {
                handleForegroundTasks();
            }
            auto& bgTasks = getBackgroundTasks();
            spdlog::info("waiting for {} background threads to finish...", bgTasks.size());
            for (auto & bgTask : bgTasks) {
                bgTask.second.joinThread();
            }
            spdlog::info("all background tasks finished, exiting now");
            gui::cleanup();
            graphics::orientation_cube::cleanup();
            graphics::connection_visualization::cleanupIfNeeded();
            graphics::Texture::deleteCached();
            graphics::shaders::cleanup();
            activeEditor = nullptr;
            for (const auto &item : editors) {
                if (item.use_count() > 1) {
                    spdlog::warn("somebody else still has a shared_ptr to editor \"{}\". use_count={}", item->getFilename(), item.use_count());
                }
            }
            editors.clear();
            mesh::SceneMeshCollection::deleteAllMeshes();
            graphics::scenes::deleteAll();
            thumbnailGenerator = nullptr;
            glfwTerminate();
            openGlInitialized = false;
            spdlog::info("GLFW terminated.");
            logging::cleanup();
            plStopAndUninit();
        }

        void handleForegroundTasks() {
            plFunction();
            while (!foregroundTasks.empty()) {
                Task& frontTask = foregroundTasks.front();
                if (!frontTask.isStarted()) {
                    frontTask.startThread();
                    foregroundTaskWaitModal = std::make_shared<gui::modals::WaitModal>(frontTask.getName(), frontTask.getProgressPtr());
                    gui::modals::addToQueue(foregroundTaskWaitModal);
                }
                if (frontTask.isDone()) {
                    frontTask.joinThread();
                    foregroundTasks.pop();
                    foregroundTaskWaitModal->close();
                    foregroundTaskWaitModal = nullptr;
                } else {
                    return;
                }
            }
        }

        void glfwErrorCallback(int code, const char* message) {
            spdlog::error("GLFW Error: {} {}", code, message);
        }
    }

    int run() {
        spdlog::info("BrickSim started.");
        const auto startupTime = std::chrono::high_resolution_clock::now();
        if (initialize()) {
            spdlog::info("Initialisation finished in {}s", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startupTime).count() / 1000.0f);
        } else {
            return 1;
        }

        //openFile("test_files/bricks_test.ldr");
        //openFile("test_files/triangle_test.ldr");
        //openFile("test_files/mpd_test.mpd");
        //openFile("test_files/texmap_planar.ldr");
        //openFile("test_files/texmap_planar3.ldr");
        //openFile("test_files/texmap_planar_perpendicular.ldr");
        //openFile("test_files/omr/arocs.mpd");
        //openFile("test_files/omr/chiron.mpd");
        //openFile("test_files/connection_info_problem_parts.ldr");
        //openFile("3001.dat");
        //openFile("car.ldr");
        //openFile("3626b.dat");
        openFile("~/Downloads/datsville.ldr");
        //openFile("~/Downloads/42131_Bulldozer.mpd");

        std::vector<uint64_t> lastEditorRootNodeVersions;

        while (!glfwWindowShouldClose(window) && !userWantsToExit) {
            //todo refactor this into method
            bool atLeastOneEditorChanged = editors.size() != lastEditorRootNodeVersions.size();
            std::vector<uint64_t> currentEditorRootNodeVersions;
            auto lastIt = lastEditorRootNodeVersions.begin();
            for (const auto& ed : editors) {
                const uint64_t version = ed->getRootNode()->getVersion();
                if (lastIt != lastEditorRootNodeVersions.end()) {
                    atLeastOneEditorChanged |= (*lastIt != version);
                }
                currentEditorRootNodeVersions.push_back(version);
            }
            lastEditorRootNodeVersions = currentEditorRootNodeVersions;

            if (foregroundTasks.empty() && backgroundTasks.empty() && thumbnailGenerator->renderQueueEmpty() && glfwGetWindowAttrib(window, GLFW_FOCUSED) == 0 && !atLeastOneEditorChanged) {
                plBegin("idle sleep");
                std::this_thread::sleep_for(idle_sleep);
                plEnd("idle sleep");

                plBegin("glfwPollEvents");
                glfwPollEvents();
                plEnd("glfwPollEvents");
                continue;
            }

            const auto loopStart = glfwGetTime();
            auto before = std::chrono::high_resolution_clock::now();

            plBegin("update transform gizmos");
            for (auto& item: editors) {
                item->update();
            }
            plEnd("update transform gizmos");

            plBegin("update editor images");
            for (auto& item: editors) {
                //todo only update image which is visible
                item->getScene()->updateImage();
            }
            plEnd("update editor images");

            gui::beginFrame();
            gui::drawMainWindows();
            gui::modals::handle();

            handleForegroundTasks();

            gui::endFrame();

            thumbnailGenerator->discardOldestImages(0);
            bool moreWork;
            do {
                moreWork = thumbnailGenerator->workOnRenderQueue();
            } while (glfwGetTime() - loopStart < 1.0 / 60 && moreWork);

            auto after = std::chrono::high_resolution_clock::now();
            lastFrameTimes[lastFrameTimesStartIdx] = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count() / 1000.0f;
            lastFrameTimesStartIdx = (lastFrameTimesStartIdx + 1) % lastFrameTimesSize;

            executeOpenGL([]() {
                plBegin("glFinish");
                glFinish();
                plEnd("glFinish");

                plBegin("glfwSwapBuffers");
                glfwSwapBuffers(window);
                plEnd("glfwSwapBuffers");

                plBegin("glfwPollEvents");
                glfwPollEvents();
                plEnd("glfwPollEvents");
            });
        }
        config::set(config::SCREEN_WIDTH, windowWidth);
        config::set(config::SCREEN_HEIGHT, windowHeight);
        cleanup();
        return 0;
    }

    bool doesUserWantToExit() {
        return glfwWindowShouldClose(window) || userWantsToExit;
    }

    void openFile(const std::string& path) {
        foregroundTasks.push(Task(std::string("Open ") + path, [path]() {
            editors.emplace_back(Editor::openFile(path));
        }));
    }

    void createNewFile() {
        editors.emplace_back(Editor::createNew());
    }

    void undoLastAction() {
        //todo implement
    }

    void redoLastAction() {
        //todo implement
    }

    void cutSelectedObject() {
        //todo implement
    }

    void copySelectedObject() {
        //todo implement
    }

    void pasteObject() {
        //todo implement
    }

    void setUserWantsToExit(bool val) {
        userWantsToExit = val;
    }

    std::shared_ptr<graphics::ThumbnailGenerator> getThumbnailGenerator() {
        return thumbnailGenerator;
    }

    uomap_t<unsigned int, Task>& getBackgroundTasks() {
        checkForFinishedBackgroundTasks();
        return backgroundTasks;
    }

    void addBackgroundTask(std::string name, const std::function<void()>& function) {
        static unsigned int sId = 0;
        backgroundTasks.emplace(sId++, Task(std::move(name), function)).first->second.startThread();
    }

    std::queue<Task>& getForegroundTasks() {
        return foregroundTasks;
    }

    std::tuple<unsigned short, float*, unsigned short> getLastFrameTimes() {
        return std::make_tuple(lastFrameTimesSize, lastFrameTimes, lastFrameTimesStartIdx);
    }

    void executeOpenGL(std::function<void()> const& functor) {
        if (!openGlInitialized) {
            throw std::invalid_argument("attempting to use OpenGL, but OpenGL isn't initialized");
        }
        static std::recursive_mutex openGlMutex;
        plLockWait("OpenGL");
        std::lock_guard<std::recursive_mutex> lg(openGlMutex);
        plLockScopeState("OpenGL", true);
        glfwMakeContextCurrent(window);
        functor();
        glfwMakeContextCurrent(nullptr);
    }

    transform_gizmo::RotationState getTransformGizmoRotationState() {
        return transformGizmoRotationState;
    }

    void toggleTransformGizmoRotationState() {
        transformGizmoRotationState =
                transformGizmoRotationState == transform_gizmo::RotationState::WORLD
                        ? transform_gizmo::RotationState::SELECTED_ELEMENT
                        : transform_gizmo::RotationState::WORLD;
    }

    std::list<std::shared_ptr<Editor>>& getEditors() {
        return editors;
    }

    std::shared_ptr<Editor>& getActiveEditor() {
        return activeEditor;
    }

    void setActiveEditor(std::shared_ptr<Editor>& editor) {
        activeEditor = editor;
    }

    std::optional<std::shared_ptr<Editor>> getEditorOfScene(scene_id_t sceneId) {
        auto& scene = graphics::scenes::get(sceneId);
        for (auto& item: editors) {
            if (item->getScene() == scene) {
                return item;
            }
        }
        return {};
    }
    std::shared_ptr<efsw::FileWatcher> getFileWatcher() {
        return fileWatcher;
    }

#ifdef BRICKSIM_USE_RENDERDOC
    RENDERDOC_API_1_1_2* getRenderdocAPI() {
        return rdoc_api;
    }
#endif
}
