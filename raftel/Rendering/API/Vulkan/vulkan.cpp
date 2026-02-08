#include <Rendering/API/Vulkan/vulkan.h>

#include <cassert>
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
        .pEngineName = "Raftel Engine",
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
    VK_CALL(vkCreateInstance(&instance_desc, nullptr, &instance), "Failed to create vulkan instance");
    LOG_SUCCESS("Vulkan Instance created");

    uint32_t device_count = 0;
    VK_CALL(vkEnumeratePhysicalDevices(instance, &device_count, nullptr), "Failed to obtain physical device count");

    std::vector<VkPhysicalDevice> physical_devices(device_count);
    VK_CALL(vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()), "Failed to get physical devices");
    assert(device_count > 0);

    VkPhysicalDevice selected_device = nullptr;
    if (device_count == 1) {
        selected_device = physical_devices[0];
    } else {
        // find first discrete gpu
        for (size_t i = 0; i < device_count; ++i) {
            VkPhysicalDeviceProperties2 device_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
                .pNext = nullptr,
                .properties = {},
            };
            vkGetPhysicalDeviceProperties2(physical_devices[i], &device_properties);
            if (device_properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                selected_device = physical_devices[i];
                break;
            }
        }
        // fallback onto first gpu in case a discrete gpu is not found
        if (selected_device == nullptr) {
            selected_device = physical_devices[0];
        }
    }

    VkPhysicalDeviceProperties selected_device_properties;
    vkGetPhysicalDeviceProperties(selected_device, &selected_device_properties);
    LOG_INFO("Selected Device: {}", selected_device_properties.deviceName);
}

}
