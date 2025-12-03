cmake_minimum_required(VERSION 3.28)
project(raftel LANGUAGES C CXX)

# C++ Basic Configuration
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR})

# third party dependencies
find_package(OpenGL REQUIRED)
set(GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_INSTALL OFF CACHE BOOL  "GLFW lib only" )
include_directories("${GLFW_SOURCE_DIR}/deps")
set(GLAD_GL "${GLFW_SOURCE_DIR}/deps/glad/gl.h" )
add_subdirectory(ThirdParty/GLFW)
add_subdirectory(ThirdParty/GLM)

#find sources
file(GLOB_RECURSE SOURCES
        raftel/*.cpp
        raftel/*.h
)
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ThirdParty/glad/src/glad.c)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ThirdParty)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ThirdParty/glad/include
        ThirdParty
)
target_link_libraries(${CMAKE_PROJECT_NAME} ${OPENGL_LIBRARIES} glfw glm)

