#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

int main() {
    glfwInit();

    auto window = glfwCreateWindow(1280, 720, "Onto Raftael", nullptr, nullptr);
    if(!window) return -1;

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
