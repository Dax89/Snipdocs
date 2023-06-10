#pragma once

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl2.h>
#include <spdlog/spdlog.h>
#include "error.h"

namespace imguicpp {

template<typename Derived>
class Application
{
public:
    ~Application() {
        if(this->m_window) {
            glfwDestroyWindow(this->m_window);
            this->m_window = nullptr;
        }

        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwTerminate();
    }

    int run() {
        if(ImGui::GetCurrentContext()) return 1;

        glfwSetErrorCallback(&Application<Derived>::glfw_error);

        if(glfwInit()) {
            this->m_window = glfwCreateWindow(this->width, this->height, this->m_title.c_str(), nullptr, nullptr);
            if(!this->m_window) except("Error creating GLFWwindow");
            glfwMakeContextCurrent(this->m_window);
            glfwSwapInterval(1); // Enable vsync
        }

        else except("glfwInit() failed");

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io{ImGui::GetIO()};
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

#if defined(IMGUI_HAS_DOCK)
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable Multi-Viewport / Platform Windows
#endif

#if defined(IMGUI_HAS_VIEWPORT)
        ImGuiStyle& style = ImGui::GetStyle();
        if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
#endif

        this->setup();

        ImGui_ImplGlfw_InitForOpenGL(this->m_window, true);
        ImGui_ImplOpenGL2_Init();

        while(!glfwWindowShouldClose(this->m_window)) {
            glfwPollEvents();

            ImGui_ImplOpenGL2_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            this->update();
            ImGui::Render();

            int w, h;
            glfwGetFramebufferSize(this->m_window, &w, &h);
            glViewport(0, 0, w, h);
            glClearColor(0, 0, 0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

#if defined(IMGUI_HAS_VIEWPORT)
            if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                GLFWwindow* ctxcurrent = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(ctxcurrent);
            }
#endif

            glfwMakeContextCurrent(this->m_window);
            glfwSwapBuffers(this->m_window);
        }

        return 0;
    }

    void setup() { static_cast<Derived*>(this)->setup(); }
    void update() { static_cast<Derived*>(this)->update(); }
    const std::string& title() { return this->m_title; }

    void set_title(const std::string& title) {
        this->m_title = title;
        if(this->m_window) glfwSetWindowTitle(this->m_window, this->m_title.c_str());
    }

private:
    static void glfw_error(int error, const char* description) {
        spdlog::critical("GLFW Error {}: {}", error, description);
    }

public:
    int width{1280};
    int height{720};

private:
    GLFWwindow* m_window{nullptr};
    std::string m_title;
};

} // namespace imguicpp
