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
        if(m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }

        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwTerminate();
    }

    int run() {
        if(ImGui::GetCurrentContext()) return 1;

        this->initialize();
        glfwSetErrorCallback(&Application<Derived>::glfw_error);

        if(glfwInit()) {
            m_window = glfwCreateWindow(this->width, this->height, this->m_title.c_str(), nullptr, nullptr);
            if(!m_window) except("Error creating GLFWwindow");
            glfwMakeContextCurrent(m_window);
            glfwSwapInterval(1); // Enable vsync
        }

        else except("glfwInit() failed");

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO io{ImGui::GetIO()};
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL2_Init();

        while(!glfwWindowShouldClose(m_window)) {
            glfwPollEvents();

            ImGui_ImplOpenGL2_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            this->update();
            ImGui::Render();

            int w, h;
            glfwGetFramebufferSize(m_window, &w, &h);
            glViewport(0, 0, w, h);
            glClearColor(0, 0, 0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

            glfwMakeContextCurrent(m_window);
            glfwSwapBuffers(m_window);
        }

        return 0;
    }

    void initialize() { static_cast<Derived*>(this)->initialize(); }
    void update() { static_cast<Derived*>(this)->update(); }
    const std::string& title() { return m_title; }

    void set_title(const std::string& title) {
        m_title = title;
        if(m_window) glfwSetWindowTitle(m_window, m_title.c_str());
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
