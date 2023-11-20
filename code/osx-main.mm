#import <stdio.h>

#import <Metal/Metal.h>
#import <Cocoa/Cocoa.h>
#include <QuartzCore/CAMetalLayer.h>

#import "utils.h"
#import "mac-keycodes.h"

global_variable r32 global_rendering_width = 1024;
global_variable r32 global_rendering_height = 768;

//
// Application / Window Delegate (just to relay events right back to the main loop, haizz)
//

@interface OSX_MainDelegate: NSObject<NSApplicationDelegate, NSWindowDelegate>
{
  @public bool is_running;
  @public bool window_was_resized;
}
@end

@implementation OSX_MainDelegate
- (NSSize)windowWillResize:(NSWindow *)window toSize:(NSSize)frame_size
{
  window_was_resized = true;
  return frame_size;
}

- (void)windowWillClose:(id)sender 
{ 
  is_running = false; 
}
@end

///////////////////////////////////////////////////////////////////////

int main(int argc, const char *argv[])
{
  NSApplication *app = [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

  OSX_MainDelegate *osx_main_delegate = [OSX_MainDelegate new];
  app.delegate = osx_main_delegate;

  [[NSFileManager defaultManager] changeCurrentDirectoryPath:[NSBundle mainBundle].bundlePath];

  NSRect screen_rect = [NSScreen mainScreen].frame;
  NSRect initial_frame = NSMakeRect((screen_rect.size.width - global_rendering_width) * .5,
                                    (screen_rect.size.height - global_rendering_height) * .5,
                                    global_rendering_width,
                                    global_rendering_height);

  NSWindow *main_window = [[NSWindow alloc]
                           initWithContentRect: initial_frame
                           styleMask: (NSWindowStyleMaskTitled |
                                       NSWindowStyleMaskClosable |
                                       NSWindowStyleMaskMiniaturizable |
                                       NSWindowStyleMaskResizable) 
                           backing: NSBackingStoreBuffered
                           defer: NO];
  main_window.backgroundColor = NSColor.purpleColor;
  main_window.contentAspectRatio = NSMakeSize(4,3);
  main_window.title = @"AutoDraw";
  main_window.delegate = osx_main_delegate;
  main_window.contentView.wantsLayer = YES;
  [main_window makeKeyAndOrderFront: nil];

  [NSApp finishLaunching];

  id<MTLDevice> mtl_device = MTLCreateSystemDefaultDevice();
  printf("System default GPU: %s\n", mtl_device.name.UTF8String);

  CAMetalLayer *ca_metal_layer = [CAMetalLayer new];
  ca_metal_layer.frame = main_window.contentView.frame;
  ca_metal_layer.device = mtl_device;
  ca_metal_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  [main_window.contentView.layer addSublayer:ca_metal_layer];

  // Load shaders
  NSError *error = nil;
  id<MTLLibrary> mtl_library = [mtl_device newLibraryWithFile: @"shaders.metallib" error:&error];
  if (!mtl_library)
  {
    printf("Failed to load library: %s\n", error.localizedDescription.UTF8String);
    return 1;
  }
  id<MTLFunction> vert_func = [mtl_library newFunctionWithName:@"vert"];
  id<MTLFunction> frag_func = [mtl_library newFunctionWithName:@"frag"];
  [mtl_library release];

  float vertex_data[] = {     // x, y, r, g, b, a
    0.5f, -0.5f, 1.f, 0.f, 0.f, 1.f,
    0.f, 0.5f, 0.f, 1.f, 0.f, 1.f,
    -0.5f, -0.5f, 0.f, 0.f, 1.f, 1.f,
  };

  // "Private" crashes the machine
  id<MTLBuffer> vertex_buffer = [mtl_device newBufferWithBytes:vertex_data length:sizeof(vertex_data)
                                 options:MTLResourceStorageModeShared];

  MTLVertexDescriptor *vertex_descriptor = [MTLVertexDescriptor new];
  {
    auto v = vertex_descriptor;
    v.attributes[0].format      = MTLVertexFormatFloat2;
    v.attributes[0].offset      = 0;
    v.attributes[0].bufferIndex = 0;

    v.attributes[1].format      = MTLVertexFormatFloat4;
    v.attributes[1].offset      = 2 * sizeof(float);
    v.attributes[1].bufferIndex = 0;

    v.layouts[0].stride       = 6 * sizeof(float);
    v.layouts[0].stepRate     = 1;
    v.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
  }

  auto render_pipeline_descriptor = [MTLRenderPipelineDescriptor new];
  render_pipeline_descriptor.vertexFunction   = vert_func;
  render_pipeline_descriptor.fragmentFunction = frag_func;
  render_pipeline_descriptor.vertexDescriptor = vertex_descriptor;
  render_pipeline_descriptor.colorAttachments[0].pixelFormat = ca_metal_layer.pixelFormat;
  auto render_pipeline_state = [mtl_device newRenderPipelineStateWithDescriptor:render_pipeline_descriptor error:&error];
  if (!render_pipeline_state)
  {
    printf("failed to create render pipeline state: %s\n", error.localizedDescription.UTF8String);
    return 1;
  }

  [vert_func release];
  [frag_func release];
  [vertex_descriptor release];
  [render_pipeline_descriptor release];

  auto command_queue = [mtl_device newCommandQueue];

  osx_main_delegate->is_running = true;
  while (osx_main_delegate->is_running)
  {
    @autoreleasepool
    {
      while (NSEvent *event = [NSApp nextEventMatchingMask: NSEventMaskAny
                               untilDate: nil
                               inMode: NSDefaultRunLoopMode
                               dequeue: YES])
      {
        switch ([event type])
        {
          case NSEventTypeKeyDown:
          {
            if (event.keyCode == KEY_Q) osx_main_delegate->is_running = false;
          } break;

          default:
          {
            [NSApp sendEvent: event];
          }
        }
      }

      if (osx_main_delegate->window_was_resized)
      {
        ca_metal_layer.frame = main_window.contentView.frame;
        ca_metal_layer.drawableSize = ca_metal_layer.frame.size;
        osx_main_delegate->window_was_resized = false;
      }

      auto ca_metal_drawable = [ca_metal_layer nextDrawable];
      if (!ca_metal_drawable)
      {
        printf("ERROR: nextDrawable timed out!\n");
        continue;
      }

      auto render_pass_descriptor = [[MTLRenderPassDescriptor new] autorelease];
      render_pass_descriptor.colorAttachments[0].texture     = ca_metal_drawable.texture;
      render_pass_descriptor.colorAttachments[0].loadAction  = MTLLoadActionClear;
      render_pass_descriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
      render_pass_descriptor.colorAttachments[0].clearColor  = MTLClearColorMake(.1, .2, .6, 1.f);

      auto command_buffer = [command_queue commandBuffer];

      auto render_command_encoder = [command_buffer renderCommandEncoderWithDescriptor:render_pass_descriptor];
      [render_command_encoder setViewport:(MTLViewport){0,0, ca_metal_layer.drawableSize.width, ca_metal_layer.drawableSize.height, 0,1}];
      [render_command_encoder setRenderPipelineState:render_pipeline_state];
      [render_command_encoder setVertexBuffer:vertex_buffer offset:0 atIndex:0];
      [render_command_encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
      [render_command_encoder endEncoding];

      [command_buffer presentDrawable:ca_metal_drawable];
      [command_buffer commit];
    }
  }

  printf("objective-c autodraw finished!\n");
  return 0;
}
