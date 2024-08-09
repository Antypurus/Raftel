cmake_minimum_required(VERSION 3.28)
project(raftel LANGUAGES C CXX)

# C++ Basic Configuration
set(CMAKE_CXX_STANDARD 20)
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
add_subdirectory(ThirdParty/GLFW)
add_subdirectory(ThirdParty/GLM)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

#find sources
file(GLOB_RECURSE SOURCES
        raftel/*.cxx
)
add_library(modules ThirdParty/GLAD/src/glad.c)
target_sources(modules
        PUBLIC
        FILE_SET CXX_MODULES FILES
        ${SOURCES}
)
target_include_directories(modules PRIVATE
        ThirdParty/GLAD/include
        ThirdParty
)
target_link_libraries(modules glfw glm)

add_executable(${CMAKE_PROJECT_NAME} raftel/main.cpp)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ThirdParty)
target_link_libraries(${CMAKE_PROJECT_NAME} modules glfw glm)