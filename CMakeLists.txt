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
set(GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_INSTALL OFF CACHE BOOL  "GLFW lib only" )
include_directories("${GLFW_SOURCE_DIR}/deps")
add_subdirectory(ThirdParty/GLFW)
add_subdirectory(ThirdParty/GLM)

find_package(Vulkan REQUIRED)
if(APPLE)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(METAL_FRAMEWORK Metal)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore)
endif()

#obj-c sources
file(GLOB_RECURSE OBJC_SOURCES
    raftel/*.mm
)
#find sources
file(GLOB_RECURSE SOURCES
        raftel/*.cpp
        raftel/*.h
)
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${OBJC_SOURCES})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ThirdParty
        raftel
        ${Vulkan_INCLUDE_DIRS}
)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE glfw glm Vulkan::Vulkan)
if(WIN32)
    target_link_libraries(${CMAKE_PROJECT_NAME} dxgi dxguid d3d12)
endif()
if(APPLE)
    set_source_files_properties(${OBJC_SOURCES} PROPERTIES COMPILE_FLAGS "-x objective-c++")
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
        ${COCOA_FRAMEWORK}
        ${METAL_FRAMEWORK}
        ${QUARTZCORE_FRAMEWORK}
    )
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE FALSE
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC NO
    )
endif()

