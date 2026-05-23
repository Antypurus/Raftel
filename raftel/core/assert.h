#pragma once

#include <cassert>

#undef ENABLE_ASSERT
#undef ENABLE_SOFT_ASSERT

#define ENABLE_SOFT_ASSERT 1

#ifdef ENABLE_ASSERT
    #define ASSERT(X) assert(X)
#elifdef ENABLE_SOFT_ASSERT
    #include <core/logger.h>
    #define ASSERT(X)                                \
        if (!X) {                                    \
            LOG_WARNING("Condition Failed: {}", #X); \
        }
#else
    #define ASSERT(X)
#endif
