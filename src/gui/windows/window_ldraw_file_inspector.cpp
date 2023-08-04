#include "window_ldraw_file_inspector.h"
#include "../../connection/connector_data_provider.h"
#include "../../controller.h"
#include "../../element_tree.h"
#include "../../graphics/connection_visualization.h"
#include "../../ldr/file_repo.h"
#include "../../ldr/file_writer.h"
#include "../../ldr/shadow_file_repo.h"
#include "../../lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include "../gui.h"
#include "../gui_internal.h"
#include "TextEditor.h"
#include "glm/gtx/string_cast.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "spdlog/fmt/bundled/format.h"
#include "spdlog/spdlog.h"
#include <sstream>

namespace bricksim::gui::windows::ldraw_file_inspector {
    namespace {
        using namespace std::literals;
        std::stack<std::shared_ptr<ldr::File>> backwardHistory;
        std::stack<std::shared_ptr<ldr::File>> forwardHistory;
        std::shared_ptr<ldr::File> currentFile = nullptr;
        std::string content;
        std::string shadowContent;

        void currentFileChanged() {
            if (currentFile != nullptr) {
                std::stringstream sstr;
                ldr::writeFile(currentFile, sstr, currentFile->metaInfo.name);
                content = sstr.str();
                try {
                    const auto relPath = bricksim::ldr::file_repo::FileRepo::getPathRelativeToBase(currentFile->metaInfo.type, currentFile->metaInfo.name);
                    shadowContent = ldr::file_repo::getShadowFileRepo().getContent(relPath).value_or("");
                } catch (std::invalid_argument) {
                    shadowContent = "";
                }
            } else {
                content = "";
            }
        }

        int partNameInputCallback(ImGuiInputTextCallbackData* data) {
            const auto ns = controller::getActiveEditor()->getFileNamespace();
            auto& fileRepo = ldr::file_repo::get();
            if (fileRepo.hasFileCached(ns, data->Buf)) {
                setCurrentFile(fileRepo.getFile(ns, data->Buf));
            } else {
                const auto extendedName = std::string(data->Buf) + ".dat";
                if (fileRepo.hasFileCached(ns, extendedName)) {
                    const auto lengthBefore = data->BufTextLen;
                    data->InsertChars(lengthBefore, ".dat");
                    data->CursorPos = lengthBefore;
                    setCurrentFile(fileRepo.getFile(ns, extendedName));
                } else {
                    setCurrentFile(nullptr);
                }
            }
            return 0;
        }

        void showSnapLineNodes(const std::shared_ptr<ldr::File>& file,
                               std::weak_ptr<connection::ldcad_meta::MetaCommand>& currentlySelected) {
            for (const auto& item: file->ldcadMetas) {
                auto flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                if (currentlySelected.lock() == item) {
                    flags |= ImGuiTreeNodeFlags_Selected;
                }
                ImGui::TreeNodeEx(reinterpret_cast<const void*>(item.get()), flags, "%s", item->to_string().c_str());
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                    currentlySelected = item;
                }
            }
            for (const auto& item: file->elements) {
                if (item->getType() == 1) {
                    const auto subfileRef = std::dynamic_pointer_cast<ldr::SubfileReference>(item);
                    const auto subfile = subfileRef->getFile(file->nameSpace);
                    if (!subfile->ldcadMetas.empty()) {
                        if (ImGui::TreeNode(subfileRef.get(), "%s", subfileRef->filename.c_str())) {
                            showSnapLineNodes(subfile, currentlySelected);
                            ImGui::TreePop();
                        }
                    }
                }
            }
        }

        void showBrickSimSnapConnectorTree() {
            if (ImGui::BeginChild("##snapConnectorTree")) {
                const auto connectors = connection::getConnectorsOfLdrFile(currentFile->nameSpace, currentFile->metaInfo.name);
                ImGui::Text("%zu Connectors:", connectors->size());
                char* nodeId = 0;
                for (const auto& item: *connectors) {
                    const auto clipConn = std::dynamic_pointer_cast<connection::ClipConnector>(item);
                    const auto cylConn = std::dynamic_pointer_cast<connection::CylindricalConnector>(item);
                    const auto fgrConn = std::dynamic_pointer_cast<connection::FingerConnector>(item);
                    const auto genConn = std::dynamic_pointer_cast<connection::GenericConnector>(item);
                    std::string name;
                    if (clipConn != nullptr) {
                        name = "Clip";
                    } else if (cylConn != nullptr) {
                        name = fmt::format("Cylinder {:g} {:g} {:g} {}", cylConn->start.x, cylConn->start.y, cylConn->start.z, magic_enum::enum_name(cylConn->gender));
                    } else if (fgrConn != nullptr) {
                        name = "Finger";
                    } else if (genConn != nullptr) {
                        name = "Generic";
                    }
                    if (ImGui::TreeNode((void*)(nodeId++), "%s", name.c_str())) {
                        ImGui::BulletText("start=%s", stringutil::formatGLM(item->start).c_str());
                        ImGui::BulletText("sourceTrace=%s", item->sourceTrace.c_str());
                        if (clipConn != nullptr) {
                            ImGui::BulletText("direction=%s", stringutil::formatGLM(clipConn->direction).c_str());
                            ImGui::BulletText("radius=%f", clipConn->radius);
                            ImGui::BulletText("width=%f", clipConn->width);
                            ImGui::BulletText("slide=%s", std::to_string(clipConn->slide).c_str());
                        } else if (cylConn != nullptr) {
                            ImGui::BulletText("direction=%s", stringutil::formatGLM(cylConn->direction).c_str());
                            ImGui::BulletText("gender=%s", magic_enum::enum_name(cylConn->gender).data());
                            if (ImGui::TreeNode("Parts")) {
                                for (const auto& part: cylConn->parts) {
                                    ImGui::BulletText("type=%s flexibleRadius=%s radius=%f length=%f",
                                                      magic_enum::enum_name(part.type).data(),
                                                      std::to_string(part.flexibleRadius).c_str(),
                                                      part.radius,
                                                      part.length);
                                }
                                ImGui::TreePop();
                            }
                            ImGui::BulletText("openStart=%s", std::to_string(cylConn->openStart).c_str());
                            ImGui::BulletText("openEnd=%s", std::to_string(cylConn->openEnd).c_str());
                            ImGui::BulletText("slide=%s", std::to_string(cylConn->slide).c_str());
                        } else if (fgrConn != nullptr) {
                            ImGui::BulletText("direction=%s", stringutil::formatGLM(fgrConn->direction).c_str());
                            ImGui::BulletText("gender=%s", magic_enum::enum_name(fgrConn->firstFingerGender).data());
                            ImGui::BulletText("radius=%f", fgrConn->radius);
                            if (ImGui::TreeNode("Finger Widths")) {
                                for (const auto& width: fgrConn->fingerWidths) {
                                    ImGui::BulletText("%f", width);
                                }
                                ImGui::TreePop();
                            }
                        } else if (genConn != nullptr) {
                            ImGui::BulletText("direction=%s", stringutil::formatGLM(genConn->direction).c_str());
                            ImGui::BulletText("gender=%s", magic_enum::enum_name(genConn->gender).data());
                            if (std::holds_alternative<connection::BoundingPnt>(genConn->bounding)) {
                                const auto& bounding = std::get<connection::BoundingPnt>(genConn->bounding);
                                ImGui::BulletText("bounding=point");
                            } else if (std::holds_alternative<connection::BoundingBox>(genConn->bounding)) {
                                const auto& bounding = std::get<connection::BoundingBox>(genConn->bounding);
                                ImGui::BulletText("bounding=box");
                                ImGui::BulletText("radius=%s", stringutil::formatGLM(bounding.radius).c_str());
                            } else if (std::holds_alternative<connection::BoundingCube>(genConn->bounding)) {
                                const auto& bounding = std::get<connection::BoundingCube>(genConn->bounding);
                                ImGui::BulletText("bounding=cube");
                                ImGui::BulletText("radius=%f", bounding.radius);
                            } else if (std::holds_alternative<connection::BoundingCyl>(genConn->bounding)) {
                                const auto& bounding = std::get<connection::BoundingCyl>(genConn->bounding);
                                ImGui::BulletText("bounding=cyl");
                                ImGui::BulletText("radius=%f", bounding.radius);
                                ImGui::BulletText("length=%f", bounding.length);
                            } else if (std::holds_alternative<connection::BoundingSph>(genConn->bounding)) {
                                const auto& bounding = std::get<connection::BoundingSph>(genConn->bounding);
                                ImGui::BulletText("bounding=sph");
                                ImGui::BulletText("radius=%f", bounding.radius);
                            }
                        }
                        ImGui::TreePop();
                    }
                }
            }
            ImGui::EndChild();
        }

        void showBrickSimConnectionVisualization() {
            constexpr ImVec2 imgButtonSize(512, 512);
            graphics::connection_visualization::initializeIfNeeded();
            graphics::connection_visualization::setVisualizedPart(currentFile->nameSpace, currentFile->metaInfo.name);
            const auto camera = graphics::connection_visualization::getCamera();

            const ImVec2& windowPos = ImGui::GetWindowPos();
            const ImVec2& imgPos = ImGui::GetCursorPos();
            const ImVec2& mousePos = ImGui::GetMousePos();
            const ImGuiIO& imGuiIo = ImGui::GetIO();
            const glm::svec2 nowRelCursorPos = {
                    mousePos.x - windowPos.x - imgPos.x,
                    mousePos.y - windowPos.y - imgPos.y};
            static glm::svec2 lastRelCursorPos = nowRelCursorPos;
            const glm::svec2 cursorDelta = nowRelCursorPos - lastRelCursorPos;
            static bool lastLeftDown = false;

            const bool nowFocussedAndHovered = ImGui::IsWindowFocused(ImGuiFocusedFlags_None)
                                               && 0 <= nowRelCursorPos.x && nowRelCursorPos.x < imgButtonSize.x
                                               && 0 <= nowRelCursorPos.y && nowRelCursorPos.y < imgButtonSize.y;
            const bool nowLeftDown = imGuiIo.MouseDown[ImGuiMouseButton_Left];

            static bool dragging = false;
            if (!lastLeftDown && nowLeftDown && nowFocussedAndHovered) {
                dragging = true;
            } else if (lastLeftDown && !nowLeftDown) {
                dragging = false;
            }

            if (dragging && (cursorDelta.x != 0 || cursorDelta.y != 0)) {
                camera->mouseRotate(cursorDelta);
            }

            const auto lastScrollDeltaY = getLastScrollDeltaY();
            if (std::abs(lastScrollDeltaY) > 0.01 && nowFocussedAndHovered) {
                camera->moveForwardBackward((float)lastScrollDeltaY);
            }

            lastLeftDown = nowLeftDown;
            lastRelCursorPos = nowRelCursorPos;

            const auto visualizationImg = graphics::connection_visualization::getImage();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::ImageButton("connectionVisualization",
                               gui_internal::convertTextureId(visualizationImg),
                               imgButtonSize,
                               {0, 1},
                               {1, 0});
            ImGui::PopStyleVar();
        }

        void showMetaInfo() {
            const auto& metaInfo = currentFile->metaInfo;

            if (ImGui::BeginTable("##metaInfoTable", 2)) {
                constexpr auto rowStart = [](const char* const name) {
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", name);
                    ImGui::TableNextColumn();
                };

                ImGui::TableSetupColumn("Attribute", ImGuiTableColumnFlags_WidthFixed,
                                        ImGui::GetFontSize() * 6);
                ImGui::TableSetupColumn("Value");
                ImGui::TableHeadersRow();

                rowStart("Title");
                ImGui::Text("%s", metaInfo.title.c_str());

                rowStart("Name");
                ImGui::Text("%s", metaInfo.name.c_str());

                rowStart("Author");
                ImGui::Text("%s", metaInfo.author.c_str());

                rowStart("Keywords");
                for (const auto& item: metaInfo.keywords) {
                    ImGui::BulletText("%s", item.c_str());
                }

                rowStart("History");
                for (const auto& item: metaInfo.history) {
                    ImGui::BulletText("%s", item.c_str());
                }

                rowStart("License");
                ImGui::Text("%s", metaInfo.license.c_str());

                rowStart("Theme");
                ImGui::Text("%s", metaInfo.theme.c_str());

                rowStart("Category");
                ImGui::Text("%s", metaInfo.headerCategory.value_or("").c_str());

                rowStart("File Type");
                ImGui::Text("%s", std::string(magic_enum::enum_name(metaInfo.type)).c_str());

                rowStart("Source");
                if (currentFile->source.path.empty()) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-zero-length"
                    ImGui::Text("");
#pragma GCC diagnostic pop
                } else {
                    ImGui::Text("%sfile of %s", currentFile->source.isMainFile ? "main " : "sub", currentFile->source.path.c_str());
                    ImGui::SameLine();
                    if (ImGui::Button(ICON_FA_PASTE)) {
                        glfwSetClipboardString(getWindow(), currentFile->source.path.string().c_str());
                    }
                }

                ImGui::EndTable();
            }

            /*

            ImGui::Separator();

            static bool explicitCategory;
            if (selectedEditorLocked != lastSelectedEditor.lock()) {
                explicitCategory = metaInfo.headerCategory.has_value();
            }
            if (ImGui::Checkbox("Explicit Category", &explicitCategory)) {
                if (explicitCategory && !metaInfo.headerCategory.has_value()) {
                    metaInfo.headerCategory = metaInfo.getCategory();
                } else if (!explicitCategory && metaInfo.headerCategory.has_value()) {
                    metaInfo.headerCategory = std::nullopt;
                }
            }
            if (explicitCategory) {
                ImGui::InputText("Category", &metaInfo.headerCategory.value());
            } else {
                ImGui::BeginDisabled();
                static char zeroChar = 0;
                ImGui::InputText("Category", &zeroChar, 1);
                ImGui::EndDisabled();
            }*/
        }

        enum class LDrawFileContentType {
            REGULAR,
            SHADOW,
        };

        static const uoset_t<std::string_view> META_WORDS = {
                "STEP"sv,
                "WRITE"sv,
                "PRINT"sv,
                "CLEAR"sv,
                "PAUSE"sv,
                "SAVE"sv,
                "!LDRAW_ORG"sv,
                "LDRAW_ORG"sv,
                "!LICENSE"sv,
                "!HELP"sv,
                "BFC"sv,
                "!CATEGORY"sv,
                "!KEYWORDS"sv,
                "!HISTORY"sv,
                "FILE"sv,
                "Name:"sv,
                "Author:"sv,
        };

        static const TextEditor::PaletteIndex PI_XVALUE = TextEditor::PaletteIndex::Number;
        static const TextEditor::PaletteIndex PI_YVALUE = TextEditor::PaletteIndex::Identifier;
        static const TextEditor::PaletteIndex PI_ZVALUE = TextEditor::PaletteIndex::Punctuation;
        static const std::array<TextEditor::PaletteIndex, 3> PIS_AXISVALUE = {{PI_XVALUE, PI_YVALUE, PI_ZVALUE}};
        void showLDrawFileContent(LDrawFileContentType type, const std::string& fileContent) {
            static std::array<TextEditor, magic_enum::enum_count<LDrawFileContentType>()> allEditors;
            static std::array<std::string, magic_enum::enum_count<LDrawFileContentType>()> allLastTexts;
            auto& editor = allEditors[*magic_enum::enum_index(type)];
            auto& lastText = allLastTexts[*magic_enum::enum_index(type)];

            static bool initialized = false;
            if (!initialized) {
                TextEditor::LanguageDefinition langDef;
                langDef.mTokenize = [](const char* inBegin, const char* inEnd, const char*& outBegin, const char*& outEnd, TextEditor::PaletteIndex& paletteIndex) -> bool {
                    static bool startOfLine = true;

                    static int lineType = -1;
                    static int progress = 0;
                    if (startOfLine) {
                        lineType = -1;
                        progress = 0;
                    }

                    paletteIndex = TextEditor::PaletteIndex::Max;
                    if (std::isblank(*inBegin)) {
                        outBegin = inBegin;
                        outEnd = outBegin + 1;
                        while (outEnd < inEnd && std::isblank(*outEnd)) {
                            ++outEnd;
                        }
                        paletteIndex = TextEditor::PaletteIndex::Default;
                    } else if (inBegin == inEnd) {
                        outBegin = inEnd;
                        outEnd = inEnd;
                        paletteIndex = TextEditor::PaletteIndex::Default;
                    } else {
                        outBegin = inBegin;
                        outEnd = outBegin + 1;
                        while (!std::isspace(*outEnd) && outEnd < inEnd) {
                            ++outEnd;
                        }
                        std::string_view currentWord(outBegin, outEnd - outBegin);
                        if (startOfLine) {
                            lineType = -2;
                            std::from_chars(outBegin, outEnd, lineType);
                            paletteIndex = TextEditor::PaletteIndex::Keyword;
                        } else if (lineType == 0) {
                            if (progress == 0) {
                                if (META_WORDS.contains(currentWord)) {
                                    paletteIndex = TextEditor::PaletteIndex::PreprocIdentifier;
                                    progress = 1;
                                } else {
                                    paletteIndex = TextEditor::PaletteIndex::Comment;
                                    outEnd = inEnd;
                                }
                            } else {
                                paletteIndex = TextEditor::PaletteIndex::Preprocessor;
                                outEnd = inEnd;
                            }
                        } else {
                            if (progress == 0) {
                                paletteIndex = TextEditor::PaletteIndex::KnownIdentifier;
                                progress = 1;
                            } else if (lineType == 1 && progress == 13) {
                                paletteIndex = ldr::file_repo::get().getFileOrNull(currentFile->nameSpace, std::string(currentWord)) != nullptr
                                                       ? TextEditor::PaletteIndex::KnownIdentifier
                                                       : TextEditor::PaletteIndex::ErrorMarker;
                            } else {
                                const auto axis = (progress - 1) % 3;
                                paletteIndex = PIS_AXISVALUE[axis];
                                ++progress;
                            }
                        }
                    }

                    startOfLine = outEnd == inEnd;
                    return paletteIndex != TextEditor::PaletteIndex::Max;
                };

                langDef.mCaseSensitive = false;
                langDef.mAutoIndentation = false;
                langDef.mName = "LDraw";

                for (auto& item: allEditors) {
                    item.SetLanguageDefinition(langDef);
                    auto palette = item.GetPalette();
                    palette[static_cast<unsigned>(PI_XVALUE)] = 0xaa0000ff;
                    palette[static_cast<unsigned>(PI_YVALUE)] = 0xaa00cc00;
                    palette[static_cast<unsigned>(PI_ZVALUE)] = 0xffff2020;
                    item.SetPalette(palette);
                }
                initialized = true;
            }
            if (lastText != fileContent) {
                editor.SetText(fileContent);
                lastText = fileContent;
            }
            editor.SetReadOnly(true);
            editor.Render(magic_enum::enum_name(type).data());
            if (editor.HasSelection()) {
                const auto selectedText = editor.GetSelectedText();
                if (selectedText != currentFile->metaInfo.name) {
                    const auto selectedFile = ldr::file_repo::get().getFileOrNull(currentFile->nameSpace, selectedText);
                    if (selectedFile != nullptr) {
                        setCurrentFile(ldr::file_repo::get().getFile(currentFile->nameSpace, selectedText));
                        editor.SetSelection(TextEditor::Coordinates(0, 0), TextEditor::Coordinates(0, 0));
                    }
                }
            }
        }
    }

    void setCurrentFile(const std::shared_ptr<ldr::File>& newFile) {
        if (currentFile != newFile) {
            if (currentFile != nullptr) {
                backwardHistory.push(currentFile);
            }
            forwardHistory = {};
            currentFile = newFile;
            currentFileChanged();
        }
    }

    void historyBrowseBack() {
        if (!backwardHistory.empty()) {
            forwardHistory.push(currentFile);
            currentFile = backwardHistory.top();
            backwardHistory.pop();
            currentFileChanged();
        }
    }

    void historyBrowseForward() {
        if (!forwardHistory.empty()) {
            backwardHistory.push(currentFile);
            currentFile = forwardHistory.top();
            forwardHistory.pop();
            currentFileChanged();
        }
    }

    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible)) {
            collectWindowInfo(data.id);
            static std::array<bool, magic_enum::enum_count<ldr::FileType>()> showTypes;

            if (static bool first = true; first) {
                first = false;
                showTypes[*magic_enum::enum_index(ldr::FileType::MODEL)] = true;
                showTypes[*magic_enum::enum_index(ldr::FileType::MPD_SUBFILE)] = true;
                showTypes[*magic_enum::enum_index(ldr::FileType::PART)] = true;
            }
            char partName[256] = {0};
            ImGui::InputText("Part Name", partName, sizeof(partName), ImGuiInputTextFlags_CallbackAlways,
                             partNameInputCallback);

            if (ImGui::BeginTable("##File selector", 2, ImGuiTableFlags_None)) {
                ImGui::TableSetupColumn("##filters", ImGuiTableColumnFlags_NoReorder
                                                             | ImGuiTableColumnFlags_NoResize
                                                             | ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("##listbox", ImGuiTableColumnFlags_NoReorder
                                                             | ImGuiTableColumnFlags_NoResize);
                ImGui::TableNextColumn();
                for (size_t i = 0; i < magic_enum::enum_count<ldr::FileType>(); ++i) {
                    const auto type = static_cast<const ldr::FileType>(i);
                    const auto typeName = magic_enum::enum_name<ldr::FileType>(type);
                    ImGui::Checkbox(typeName.data(), &showTypes[i]);
                }

                ImGui::TableNextColumn();

                ImGui::PushItemWidth(-1.f);
                if (ImGui::BeginListBox("##All Files")) {
                    for (const auto& [nsKey, nsMap]: ldr::file_repo::get().getAllFilesInMemory()) {
                        if (nsKey == nullptr ? ImGui::TreeNode("Library Namespace") : ImGui::TreeNode("%s (%s)", nsKey->name.c_str(), nsKey->searchPath.c_str())) {
                            for (const auto& [nameKey, value]: nsMap) {
                                const auto& [fileType, file] = value;
                                if (showTypes[magic_enum::enum_index(file->metaInfo.type).value()]) {
                                    const auto text = fmt::format("{}: {}", nameKey, file->metaInfo.title);
                                    if (ImGui::Selectable(text.c_str(), file == currentFile)) {
                                        setCurrentFile(file);
                                    }
                                }
                            }
                            ImGui::TreePop();
                        }
                    }
                    ImGui::EndListBox();
                }
                ImGui::PopItemWidth();

                ImGui::EndTable();
            }

            if (currentFile != nullptr) {
                ImGui::Separator();

                if (backwardHistory.empty()) {
                    ImGui::BeginDisabled();
                    ImGui::Button(ICON_FA_ARROW_LEFT);
                    ImGui::EndDisabled();
                } else if (ImGui::Button(ICON_FA_ARROW_LEFT)) {
                    historyBrowseBack();
                }

                ImGui::SameLine();
                if (forwardHistory.empty()) {
                    ImGui::BeginDisabled();
                    ImGui::Button(ICON_FA_ARROW_RIGHT);
                    ImGui::EndDisabled();
                } else if (ImGui::Button(ICON_FA_ARROW_RIGHT)) {
                    historyBrowseForward();
                }

                ImGui::SameLine();

                ImGui::Text("Inspecting %s: %s", currentFile->metaInfo.name.c_str(), currentFile->metaInfo.title.c_str());

                if (ImGui::BeginTabBar("##fileInspectorTabBar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton)) {
                    if (ImGui::BeginTabItem("Raw Content")) {
                        showLDrawFileContent(LDrawFileContentType::REGULAR, content);
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Shadow Content")) {
                        showLDrawFileContent(LDrawFileContentType::SHADOW, shadowContent);
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Meta Snap Info")) {
                        static std::weak_ptr<connection::ldcad_meta::MetaCommand> currentlySelected;
                        showSnapLineNodes(currentFile, currentlySelected);
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("BrickSim Snap Info")) {
                        showBrickSimConnectionVisualization();
                        ImGui::SameLine();
                        showBrickSimSnapConnectorTree();

                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Meta Info")) {
                        showMetaInfo();
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
            } else {
                ImGui::Text("No file named \"%s\" in memory.", partName);
            }
        }
        ImGui::End();
    }
}
