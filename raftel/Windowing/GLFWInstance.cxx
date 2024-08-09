module;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

export module GLFWInstance;
namespace raftel {

    export struct GLFWInstance {
        static GLFWInstance instance;

        GLFWInstance() {
            glfwInit();
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
        }

        static GLFWInstance& Instance() {
            return instance;
        }

        ~GLFWInstance() {
            glfwTerminate();
        }
    };

    GLFWInstance GLFWInstance::instance;

}