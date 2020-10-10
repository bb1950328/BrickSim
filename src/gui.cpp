//
// Created by Bader on 09.10.2020.
//

#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include <iostream>
#include "gui.h"
#include "config.h"
#include "controller.h"

void Gui::setup() {
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();//todo get the monitor on which the window is
    float xscale, yscale;
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);
    std::cout << "xscale: " << xscale << "\tyscale: " << yscale << std::endl;
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto scaleFactor = (float)(Configuration::getInstance()->get_double(config::KEY_GUI_SCALE));
    if (xscale>1 || yscale>1) {
        scaleFactor = (xscale+yscale)/2.0f;
        ImGuiStyle &style = ImGui::GetStyle();
        style.ScaleAllSizes(scaleFactor);
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    }
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.Fonts->AddFontFromFileTTF("RobotoMono-Regular.ttf", 13.0f * scaleFactor, nullptr, nullptr);
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    // Setup Dear ImGui style
    auto guiStyle = Configuration::getInstance()->get_string(config::KEY_GUI_STYLE);
    if (guiStyle=="light") {
        ImGui::StyleColorsLight();
    } else if (guiStyle=="classic") {
        ImGui::StyleColorsClassic();
    } else if (guiStyle=="dark") {
        ImGui::StyleColorsDark();
    } else {
        std::cout << "WARNING: please set "<<config::KEY_GUI_STYLE << "to light, classic or dark" << std::endl;
    }
}

void Gui::loop() {
    static bool showElementTreeWindow = true;
    static bool showSettingsWindow = true;
    static bool showDemoWindow = true;
    static bool showDebugWindow = true;
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.6f, 1.0f);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open", "CTRL+O")) {
                //todo open file dialog
            }
            if (ImGui::MenuItem("Exit", "CTRL+W")) {
                //todo exit
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            //todo implement these functions
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Element Tree", "ALT+T", &showElementTreeWindow);
            ImGui::MenuItem("Settings", "ALT+S", &showSettingsWindow);
            ImGui::Separator();
            ImGui::MenuItem("Demo", "", &showDemoWindow);
            ImGui::MenuItem("Debug", "ALT+D", &showDebugWindow);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (showElementTreeWindow) {
        ImGui::Begin("Element Tree", &showElementTreeWindow);
        ImGui::Text("TODO");//todo implement
        ImGui::End();
    }

    if (showSettingsWindow) {
        ImGui::Begin("Settings", &showSettingsWindow);
        auto conf = Configuration::getInstance();
        static auto guiScale = (float)(conf->get_double(config::KEY_GUI_SCALE));
        static int initialWindowSize[2]{
            static_cast<int>(conf->get_long(config::KEY_SCREEN_WIDTH)),
            static_cast<int>(conf->get_long(config::KEY_SCREEN_HEIGHT))
        };
        static auto ldrawDirString = conf->get_string(config::KEY_LDRAW_PARTS_LIBRARY);
        static auto ldrawDir = ldrawDirString.c_str();
        static auto guiStyleString = conf->get_string(config::KEY_GUI_STYLE);
        static auto guiStyle = guiStyleString=="light"?0:(guiStyleString=="classic"?1:2);
        ImGui::SliderFloat("UI Scale", &guiScale, 0.25, 8, "%.2f");
        ImGui::InputInt2("Initial Window Size", initialWindowSize);
        ImGui::InputText("Ldraw path", const_cast<char *>(ldrawDir), 256);
        ImGui::Combo("Output", &guiStyle, "Light\0Classic\0Dark\0");
        if(ImGui::Button("Save")) {
            conf->set_double(config::KEY_GUI_SCALE, guiScale);
            conf->set_long(config::KEY_SCREEN_WIDTH, initialWindowSize[0]);
            conf->set_long(config::KEY_SCREEN_HEIGHT, initialWindowSize[1]);
            conf->set_string(config::KEY_LDRAW_PARTS_LIBRARY, ldrawDir);
            switch (guiStyle) {
                //todo make this work
                case 0: conf->set_string(config::KEY_GUI_STYLE, "light");
                case 1: conf->set_string(config::KEY_GUI_STYLE, "classic");
                default: conf->set_string(config::KEY_GUI_STYLE, "dark");
            }
            conf->save();//todo show message when failed
        }
        ImGui::End();
    }

    if (showDebugWindow) {
        ImGui::Begin("Debug Information", &showDebugWindow);
        long lastFrameTime = Controller::getInstance()->lastFrameTime;
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", lastFrameTime/1000.0, 1000000.0/lastFrameTime);
        ImGui::End();
    }

    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Gui::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}