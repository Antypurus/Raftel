#include "Metal.h"

#include <Windowing/Window.h>
#include <logger.h>

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

namespace raftel {

static const char* kShaderSrc = R"metal(
#include <metal_stdlib>
using namespace metal;

struct VSOut { float4 pos [[position]]; float3 col; };

vertex VSOut vs_main(uint vid [[vertex_id]]) {
    float2 positions[3] = { float2(0.0, 0.6), float2(-0.6, -0.6), float2(0.6, -0.6) };
    float3 colors[3]    = { float3(1,0,0),    float3(0,1,0),       float3(0,0,1) };

    VSOut o;
    o.pos = float4(positions[vid], 0.0, 1.0);
    o.col = colors[vid];
    return o;
}

fragment float4 fs_main(VSOut in [[stage_in]]) {
    return float4(in.col, 1.0);
}
)metal";

void init_metal(WindowHandle window)
{
    NSWindow* handle = (__bridge NSWindow*)WindowingSystem::get_instance().get_native_window_handle(window);
    NSView* view = [handle contentView];

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    id<MTLCommandQueue> queue = [device newCommandQueue];

    // metal layer
    CAMetalLayer* layer = [CAMetalLayer layer];
    layer.device = device;
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    layer.framebufferOnly = true;

    view.wantsLayer = true;
    view.layer = layer;

    NSError* err = nil;
    NSString* shaderSource = [NSString stringWithUTF8String:kShaderSrc];
    id<MTLLibrary> shader_library = [device newLibraryWithSource:shaderSource options:nil error:&err];
    id<MTLFunction> vertex_shader = [shader_library newFunctionWithName:@"vs_main"];
    id<MTLFunction> pixel_shader = [shader_library newFunctionWithName:@"fs_main"];

    MTLRenderPipelineDescriptor* pipeline_desc = [MTLRenderPipelineDescriptor new];
    pipeline_desc.vertexFunction = vertex_shader;
    pipeline_desc.fragmentFunction = pixel_shader;
    pipeline_desc.colorAttachments[0].pixelFormat = layer.pixelFormat;

    id<MTLRenderPipelineState> pso = [device newRenderPipelineStateWithDescriptor:pipeline_desc error:&err];
    if (err != nil) {
        LOG_ERROR("Failed to create PSO");
    }
    LOG_SUCCESS("Metal PSO Created");

    auto& windowing_system = WindowingSystem::get_instance();
    windowing_system.update();
    while (windowing_system.has_open_windows()) {
        Resolution framebuffer_res = windowing_system.get_window_resolution(window);
        layer.drawableSize = CGSizeMake(framebuffer_res.width, framebuffer_res.height);

        id<CAMetalDrawable> drawable = [layer nextDrawable];
        if (!drawable)
            continue;

        MTLRenderPassDescriptor* renderpass_desc = [MTLRenderPassDescriptor renderPassDescriptor];
        renderpass_desc.colorAttachments[0].texture = drawable.texture;
        renderpass_desc.colorAttachments[0].loadAction = MTLLoadActionClear;
        renderpass_desc.colorAttachments[0].storeAction = MTLStoreActionStore;
        renderpass_desc.colorAttachments[0].clearColor = MTLClearColorMake(0.1, 0.1, 0.12, 1.0);

        id<MTLCommandBuffer> command_buffer = [queue commandBuffer];
        id<MTLRenderCommandEncoder> command_encoder = [command_buffer renderCommandEncoderWithDescriptor:renderpass_desc];

        [command_encoder setRenderPipelineState:pso];
        [command_encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
        [command_encoder endEncoding];

        [command_buffer presentDrawable:drawable];
        [command_buffer commit];

        windowing_system.update();
    }

    return;
}

}
