#include "GLFWInstance.h"

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
