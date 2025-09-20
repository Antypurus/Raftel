module;

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifdef __APPLE__
#include <GLFW/deps/glad/gl.h>
#else
#include <glad/glad.h>
#endif

#include <iostream>

export module GLFWInstance;
namespace raftel {

export struct GLFWInstance {
private:
    static GLFWInstance instance;

    GLFWInstance();
    ~GLFWInstance();

public:
    static GLFWInstance& Instance();
};

}

namespace raftel {

GLFWInstance GLFWInstance::instance;

GLFWInstance& GLFWInstance::Instance()
{
    return instance;
}

GLFWInstance::GLFWInstance()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

GLFWInstance::~GLFWInstance()
{
    glfwTerminate();
}

}
