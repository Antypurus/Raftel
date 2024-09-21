module;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

import GLFWInstance;

export module Window;
namespace raftel {

    export class Window {
    private:
        GLFWwindow *m_window_handle = nullptr;

    public:
        Window(const char *title, uint32_t width, uint32_t height) {
            raftel::GLFWInstance& instance = GLFWInstance::Instance();

            this->m_window_handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
            glfwMakeContextCurrent(this->m_window_handle);
        }

        ~Window() {
            glfwDestroyWindow(this->m_window_handle);
        }

        void Update() {
            glViewport(0, 0, 1920, 1080);
            glfwSwapBuffers(this->m_window_handle);
            glfwPollEvents();
        }

        bool IsOpen() const {
            return !glfwWindowShouldClose(this->m_window_handle);
        }
    };

}
