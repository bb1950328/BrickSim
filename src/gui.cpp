//
// Created by Bader on 09.10.2020.
//

#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include <iostream>
#include "gui.h"
#include "config.h"

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
    io.Fonts->AddFontFromFileTTF("FreeSans.ttf", 13.0f * scaleFactor, nullptr, nullptr);
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
    static bool showDemoWindow = true;
    static bool showOtherWindow = true;
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.6f, 1.0f);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &showDemoWindow);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &showOtherWindow);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (showOtherWindow)
    {
        ImGui::Begin("Another Window", &showOtherWindow);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            showOtherWindow = false;
        ImGui::End();
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
