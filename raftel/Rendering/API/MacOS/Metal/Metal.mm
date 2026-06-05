#include "Metal.h"

#include <Windowing/Window.h>
#include <core/logger.h>

#import <Cocoa/Cocoa.h>
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

void InitMetal(WindowHandle p_Window)
{
    NSWindow* handle = (__bridge NSWindow*)WindowingSystem::GetInstance().GetNativeWindowHandle(p_Window);
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
    id<MTLLibrary> shaderLibrary = [device newLibraryWithSource:shaderSource options:nil error:&err];
    id<MTLFunction> vertexShader = [shaderLibrary newFunctionWithName:@"vs_main"];
    id<MTLFunction> pixelShader = [shaderLibrary newFunctionWithName:@"fs_main"];

    MTLRenderPipelineDescriptor* pipelineDesc = [MTLRenderPipelineDescriptor new];
    pipelineDesc.vertexFunction = vertexShader;
    pipelineDesc.fragmentFunction = pixelShader;
    pipelineDesc.colorAttachments[0].pixelFormat = layer.pixelFormat;

    id<MTLRenderPipelineState> pso = [device newRenderPipelineStateWithDescriptor:pipelineDesc error:&err];
    if (err != nil) {
        LOG_ERROR("Failed to create PSO");
    }
    LOG_SUCCESS("Metal PSO Created");

    auto& windowingSystem = WindowingSystem::GetInstance();
    windowingSystem.Update();
    while (windowingSystem.HasOpenWindows()) {
        Resolution framebufferRes = windowingSystem.GetWindowResolution(p_Window);
        layer.drawableSize = CGSizeMake(framebufferRes.Width, framebufferRes.Height);

        id<CAMetalDrawable> drawable = [layer nextDrawable];
        if (!drawable)
            continue;

        MTLRenderPassDescriptor* renderpassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
        renderpassDesc.colorAttachments[0].texture = drawable.texture;
        renderpassDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
        renderpassDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
        renderpassDesc.colorAttachments[0].clearColor = MTLClearColorMake(0.1, 0.1, 0.12, 1.0);

        id<MTLCommandBuffer> commandBuffer = [queue commandBuffer];
        id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderpassDesc];

        [commandEncoder setRenderPipelineState:pso];
        [commandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
        [commandEncoder endEncoding];

        [commandBuffer presentDrawable:drawable];
        [commandBuffer commit];

        windowingSystem.Update();
    }

    return;
}

}
