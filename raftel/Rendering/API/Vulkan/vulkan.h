#pragma once

#include <vulkan/vulkan.h>

namespace raftel {

void InitVulkan();

#define VK_CALL(X, p_ErrorMessage, ...)                                            \
    {                                                                              \
        VkResult result##__LINE__ = X;                                             \
        if (result##__LINE__ != VK_SUCCESS) {                                      \
            LOG_ERROR("[Vulkan Error Code = {}]", (unsigned int)result##__LINE__); \
            LOG_ERROR(p_ErrorMessage, __VA_ARGS__);                                \
            exit(-1);                                                              \
        }                                                                          \
    }

}
