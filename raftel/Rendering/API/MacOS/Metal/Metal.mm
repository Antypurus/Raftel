#include "Metal.h"
#import <Metal/Metal.h>

namespace raftel {

void init_metal()
{
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    return;
}

}
