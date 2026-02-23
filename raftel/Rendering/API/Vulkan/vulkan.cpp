#include <Rendering/API/Vulkan/vulkan.h>

#include <logger.h>

#include <cassert>
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
    // ============== Instance Creation ======================
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
    VkInstance instance = nullptr;
    VK_CALL(vkCreateInstance(&instance_desc, nullptr, &instance), "Failed to create vulkan instance");
    LOG_SUCCESS("Vulkan Instance created");

    uint32_t device_count = 0;
    VK_CALL(vkEnumeratePhysicalDevices(instance, &device_count, nullptr), "Failed to obtain physical device count");

    std::vector<VkPhysicalDevice> physical_devices(device_count);
    VK_CALL(vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()), "Failed to get physical devices");
    assert(device_count > 0);

    // ================= Device Selection ==================
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
    VkPhysicalDeviceFeatures selected_device_features;
    vkGetPhysicalDeviceProperties(selected_device, &selected_device_properties);
    vkGetPhysicalDeviceFeatures(selected_device, &selected_device_features);
    LOG_INFO("Selected Device: {}", selected_device_properties.deviceName);

    // ================ Gather Queue Info =======================
    std::uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(selected_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_properties(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(selected_device, &queue_family_count, queue_properties.data());

    std::int32_t graphics_queue_index = -1;
    std::int32_t compute_queue_index = -1;
    std::int32_t transfer_queue_index = -1;
    std::int32_t video_encode_queue_index = -1;
    std::int32_t video_decode_queue_index = -1;
    std::int32_t optical_flow_queue_index = -1;
    std::vector<VkDeviceQueueCreateInfo> device_queue_info;
    for (size_t i = 0; i < queue_properties.size(); ++i) {
        auto& queue_info = queue_properties[i];
        const float priorities[1] = { 0.0f };
        device_queue_info.push_back({
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = (std::uint32_t)i,
            .queueCount = queue_info.queueCount,
            .pQueuePriorities = priorities,
        });
        if (queue_info.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            LOG_INFO("Have {} Graphics Queues", queue_info.queueCount);
            // graphics_queue_index = (std::int32_t)i;
        } else if (queue_info.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            LOG_INFO("Have {} Compute Queues", queue_info.queueCount);
            // compute_queue_index = (std::int32_t)i;
        } else if (queue_info.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) {
            LOG_INFO("Have {}, Video Decode Queues", queue_info.queueCount);
            // video_decode_queue_index = (std::int32_t)i;
        } else if (queue_info.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) {
            LOG_INFO("Have {}, Video Encode Queues", queue_info.queueCount);
            // video_encode_queue_index = (std::int32_t)i;
        } else if (queue_info.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV) {
            LOG_INFO("Have {}, Optical Flow Queues", queue_info.queueCount);
            // optical_flow_queue_index = (std::int32_t)i;
        } else if (queue_info.queueFlags & VK_QUEUE_TRANSFER_BIT) {
            LOG_INFO("Have {}, Transfer Queues", queue_info.queueCount);
            // transfer_queue_index = (std::int32_t)i;
        }
    }

    // ================= Device Creation =========================
    const VkDeviceCreateInfo device_desc = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = (std::uint32_t)device_queue_info.size(),
        .pQueueCreateInfos = device_queue_info.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = nullptr,
        .pEnabledFeatures = &selected_device_features,
    };
    VkDevice logical_device = nullptr;
    VK_CALL(vkCreateDevice(selected_device, &device_desc, nullptr, &logical_device), "Failed to create logical device");
    LOG_SUCCESS("Vulkan Logical Device Created");

    // =============== Teardown ================================
    vkDestroyDevice(logical_device, nullptr);
    vkDestroyInstance(instance, nullptr);
}

}
