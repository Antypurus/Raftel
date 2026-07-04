#pragma once

#include <core/logger.h>

#include <chrono>

#define SIMPLE_MEASURE_WITH_UNIT(X, UNIT)                                                             \
    {                                                                                                 \
        const auto start__ = std::chrono::steady_clock::now();                                        \
        X;                                                                                            \
        const auto end__ = std::chrono::steady_clock::now();                                          \
        LOG_INFO("Measure Time for {} -> {}", #X, std::chrono::duration_cast<UNIT>(end__ - start__)); \
    }

#define SIMPLE_MEASURE(X) SIMPLE_MEASURE_WITH_UNIT(X, std::chrono::milliseconds)
