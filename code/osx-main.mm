#import <stdio.h>

#import <Metal/Metal.h>
#import <Cocoa/Cocoa.h>
#include <QuartzCore/CAMetalLayer.h>

#import "utils.h"
#import "mac-keycodes.h"

global_variable r32 global_rendering_width = 1024;
global_variable r32 global_rendering_height = 768;

///////////////////////////////////////////////////////////////////////
// Application / Window Delegate (just to relay events right back to the main loop, haizz)

@interface OSX_MainDelegate: NSObject<NSApplicationDelegate, NSWindowDelegate>
{
  @public bool is_running;
  @public bool window_was_resized;
}
@end

@implementation OSX_MainDelegate
// NSApplicationDelegate methods
// NSWindowDelegate methods
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

  NSRect screen_rect = [NSScreen mainScreen].frame;
  NSRect content_rect = NSMakeRect((screen_rect.size.width - global_rendering_width) * .5,
                                   (screen_rect.size.height - global_rendering_height) * .5,
                                   global_rendering_width,
                                   global_rendering_height);

  NSWindow *main_window = [[NSWindow alloc]
                     initWithContentRect: content_rect
                     styleMask: (NSWindowStyleMaskTitled |
                                 NSWindowStyleMaskClosable |
                                 NSWindowStyleMaskMiniaturizable |
                                 NSWindowStyleMaskResizable) 
                     backing: NSBackingStoreBuffered
                     defer: NO];
  main_window.backgroundColor = NSColor.purpleColor;
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

  id<MTLCommandQueue> mtlCommandQueue = [mtl_device newCommandQueue];

  osx_main_delegate->is_running = true;
  while (osx_main_delegate->is_running)
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

      if (osx_main_delegate->window_was_resized)
      {
        // todo study
        ca_metal_layer.frame = main_window.contentView.frame;
        ca_metal_layer.drawableSize = ca_metal_layer.frame.size;
        osx_main_delegate->window_was_resized = false;
      }

      if (id<CAMetalDrawable> ca_metal_drawable = [ca_metal_layer nextDrawable])
      {
        // todo study render pass descriptor
        MTLRenderPassDescriptor *render_pass_descriptor = [MTLRenderPassDescriptor new];
        render_pass_descriptor.colorAttachments[0].texture     = ca_metal_drawable.texture;
        render_pass_descriptor.colorAttachments[0].loadAction  = MTLLoadActionClear;
        render_pass_descriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
        render_pass_descriptor.colorAttachments[0].clearColor  = MTLClearColorMake(.1, .2, .6, 1.f);

        id<MTLCommandBuffer> mtl_command_buffer = [mtlCommandQueue commandBuffer];

        id<MTLRenderCommandEncoder> render_command_encoder =
          [mtl_command_buffer renderCommandEncoderWithDescriptor:render_pass_descriptor];
        [render_pass_descriptor release];

        [render_command_encoder endEncoding];

        [mtl_command_buffer presentDrawable:ca_metal_drawable];
        [mtl_command_buffer commit];
      }
    }
  }

  printf("objective-c autodraw finished!\n");
  return 0;
}
