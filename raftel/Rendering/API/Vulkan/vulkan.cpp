#include <Rendering/API/Vulkan/vulkan.h>

#include <core/logger.h>

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

void InitVulkan()
{
    // ============== Instance Creation ======================
    const VkApplicationInfo appDesc = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Raftel",
        .applicationVersion = 1,
        .pEngineName = "Raftel Engine",
        .engineVersion = 1,
        .apiVersion = VK_API_VERSION_1_3,
    };

    std::vector<const char*> instanceExtensions = {
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME
#ifdef __linux__
        ,
        VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif
    };

    const VkInstanceCreateInfo instanceDesc = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        .pApplicationInfo = &appDesc,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = (std::uint32_t)instanceExtensions.size(),
        .ppEnabledExtensionNames = instanceExtensions.data(),
    };
    VkInstance instance = nullptr;
    VK_CALL(vkCreateInstance(&instanceDesc, nullptr, &instance), "Failed to create vulkan instance");
    LOG_SUCCESS("Vulkan Instance created");

    uint32_t deviceCount = 0;
    VK_CALL(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr), "Failed to obtain physical device count");

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    VK_CALL(vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data()), "Failed to get physical devices");
    assert(deviceCount > 0);

    // ================= Device Selection ==================
    VkPhysicalDevice selectedDevice = nullptr;
    if (deviceCount == 1) {
        selectedDevice = physicalDevices[0];
    } else {
        // find first discrete gpu
        for (size_t i = 0; i < deviceCount; ++i) {
            VkPhysicalDeviceProperties2 deviceProperties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
                .pNext = nullptr,
                .properties = { },
            };
            vkGetPhysicalDeviceProperties2(physicalDevices[i], &deviceProperties);
            if (deviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                selectedDevice = physicalDevices[i];
                break;
            }
        }
        // fallback onto first gpu in case a discrete gpu is not found
        if (selectedDevice == nullptr) {
            selectedDevice = physicalDevices[0];
        }
    }

    VkPhysicalDeviceProperties selectedDeviceProperties;
    VkPhysicalDeviceFeatures selectedDeviceFeatures;
    vkGetPhysicalDeviceProperties(selectedDevice, &selectedDeviceProperties);
    vkGetPhysicalDeviceFeatures(selectedDevice, &selectedDeviceFeatures);
    LOG_INFO("Selected Device: {}", selectedDeviceProperties.deviceName);

    // ================ Gather Queue Info =======================
    std::uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(selectedDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(selectedDevice, &queueFamilyCount, queueProperties.data());

    std::int32_t graphicsQueueIndex = -1;
    std::int32_t computeQueueIndex = -1;
    std::int32_t transferQueueIndex = -1;
    std::int32_t videoEncodeQueueIndex = -1;
    std::int32_t videoDecodeQueueIndex = -1;
    std::int32_t opticalFlowQueueIndex = -1;
    std::vector<VkDeviceQueueCreateInfo> deviceQueueInfo;
    for (size_t i = 0; i < queueProperties.size(); ++i) {
        auto& queueInfo = queueProperties[i];
        const float priorities[1] = { 0.0f };
        deviceQueueInfo.push_back({
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = (std::uint32_t)i,
            .queueCount = queueInfo.queueCount,
            .pQueuePriorities = priorities,
        });
        if (queueInfo.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            LOG_INFO("Have {} Graphics Queues", queueInfo.queueCount);
            // graphics_queue_index = (std::int32_t)i;
        } else if (queueInfo.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            LOG_INFO("Have {} Compute Queues", queueInfo.queueCount);
            // compute_queue_index = (std::int32_t)i;
        } else if (queueInfo.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) {
            LOG_INFO("Have {}, Video Decode Queues", queueInfo.queueCount);
            // video_decode_queue_index = (std::int32_t)i;
        } else if (queueInfo.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) {
            LOG_INFO("Have {}, Video Encode Queues", queueInfo.queueCount);
            // video_encode_queue_index = (std::int32_t)i;
        } else if (queueInfo.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV) {
            LOG_INFO("Have {}, Optical Flow Queues", queueInfo.queueCount);
            // optical_flow_queue_index = (std::int32_t)i;
        } else if (queueInfo.queueFlags & VK_QUEUE_TRANSFER_BIT) {
            LOG_INFO("Have {}, Transfer Queues", queueInfo.queueCount);
            // transfer_queue_index = (std::int32_t)i;
        }
    }

    // ================= Device Creation =========================
    const VkDeviceCreateInfo deviceDesc = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = (std::uint32_t)deviceQueueInfo.size(),
        .pQueueCreateInfos = deviceQueueInfo.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = nullptr,
        .pEnabledFeatures = &selectedDeviceFeatures,
    };
    VkDevice logicalDevice = nullptr;
    VK_CALL(vkCreateDevice(selectedDevice, &deviceDesc, nullptr, &logicalDevice), "Failed to create logical device");
    LOG_SUCCESS("Vulkan Logical Device Created");

    // =============== Teardown ================================
    vkDestroyDevice(logicalDevice, nullptr);
    vkDestroyInstance(instance, nullptr);
}

}
