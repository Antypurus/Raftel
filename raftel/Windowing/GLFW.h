#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "../handle.h"

DEFINE_HANDLE(GLFWwindow)

#if 1
#include <utility>
namespace std {
template <>
struct hash<GLFWwindowHandle> {
    size_t operator()(const GLFWwindowHandle handle) const
    {
        return (size_t)handle;
    }
};
}
#endif
