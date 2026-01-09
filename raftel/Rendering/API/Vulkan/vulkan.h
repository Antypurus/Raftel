#pragma once

#include <logger.h>
#include <vulkan/vulkan.h>

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
    const VkInstanceCreateInfo instance_desc = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .pApplicationInfo = &app_desc,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = nullptr,
    };
    VkResult result = vkCreateInstance(&instance_desc, nullptr, &instance);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan Instance");
        exit(-1);
    }
    LOG_SUCCESS("Vulkan Instance created");
}

}
