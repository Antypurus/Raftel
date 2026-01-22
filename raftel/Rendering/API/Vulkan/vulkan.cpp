#include <Rendering/API/Vulkan/vulkan.h>

#include <logger.h>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#ifdef __linux__
    #include <X11/Xlib.h>
    #include <vulkan/vulkan_wayland.h>
    #include <vulkan/vulkan_xlib.h>
#endif

namespace raftel {

void init_vulkan()
{
    VkInstance instance = nullptr;
    const VkApplicationInfo app_desc = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Raftel",
        .applicationVersion = 1,
        .engineVersion = 1,
        .apiVersion = VK_API_VERSION_1_3,
    };

    std::vector<const char*> instance_extensions = {
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME
#ifdef __linux__
        ,
        VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif
    };

    const VkInstanceCreateInfo instance_desc = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        .pApplicationInfo = &app_desc,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = (std::uint32_t)instance_extensions.size(),
        .ppEnabledExtensionNames = instance_extensions.data(),
    };
    VkResult result = vkCreateInstance(&instance_desc, nullptr, &instance);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan Instance, error code = {}", (std::uint32_t)result);
        exit(-1);
    }
    LOG_SUCCESS("Vulkan Instance created");
}

}
