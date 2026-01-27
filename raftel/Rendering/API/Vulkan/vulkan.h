#pragma once

#include <vulkan/vulkan.h>

namespace raftel {

void init_vulkan();

#define VK_CALL(X, error_message, ...)                                             \
    {                                                                              \
        VkResult result##__LINE__ = X;                                             \
        if (result##__LINE__ != VK_SUCCESS) {                                      \
            LOG_ERROR("[Vulkan Error Code = {}]", (unsigned int)result##__LINE__); \
            LOG_ERROR(error_message, __VA_ARGS__);                                 \
            exit(-1);                                                              \
        }                                                                          \
    }

}
