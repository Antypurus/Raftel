module;


#include <GLFW/glfw3.h>

#include <iostream>

export module GLFWInstance;
namespace raftel {

export struct GLFWInstance {
private:
  static GLFWInstance instance;

  GLFWInstance() { glfwInit(); }

  ~GLFWInstance() { glfwTerminate(); }

public:
  static GLFWInstance &Instance() { return instance; }
};

GLFWInstance GLFWInstance::instance;

} // namespace raftel
