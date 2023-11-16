#import <stdio.h>
#import <Metal/Metal.h>
#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>
#import <MetalKit/MetalKit.h>
#import "utils.h"

global_variable r32 global_rendering_width = 1024;
global_variable r32 global_rendering_height = 768;
global_variable b32 global_running = true;

@interface AutoRenderer : NSObject<MTKViewDelegate>

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)mtkView;

@end

@implementation AutoRenderer
{
  id<MTLDevice> _device;
  id<MTLCommandQueue> _commandQueue;
}

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)mtkView
{
  self = [super init];
  if (self)
  {
    _device = mtkView.device;
    _commandQueue = [_device newCommandQueue];
  }
  return self;
}

- (void)drawInMTKView:(nonnull MTKView *)view
{
  MTLRenderPassDescriptor *renderPassDescriptor = view.currentRenderPassDescriptor;
  if (renderPassDescriptor)
  {
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
    [commandEncoder endEncoding];
    [commandBuffer presentDrawable:view.currentDrawable];
    [commandBuffer commit];
  }
}

- (void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size
{
}

@end

@interface AutoViewController : NSViewController
@end
@implementation AutoViewController
{
  MTKView *_view;
  AutoRenderer *_renderer;
}

- (void)viewDidLoad
{
  NSRect screen_rect = NSMakeRect(0,0,global_rendering_width,global_rendering_height);
  id<MTLDevice> device = MTLCreateSystemDefaultDevice();
  _view = [[MTKView alloc] initWithFrame:screen_rect device:device];
  self.view = _view;

  _view.enableSetNeedsDisplay = YES;
  _view.clearColor = MTLClearColorMake(0.0, .5, 1.0, 1.0);
  _renderer = [[AutoRenderer alloc] initWithMetalKitView:_view];
  if (_renderer)
  {
    [_renderer mtkView:_view drawableSizeWillChange:_view.drawableSize];
    _view.delegate = _renderer;
  }
  else
  {
    NSLog(@"Renderer initialization failed");
    return;
  }
}
@end

@interface WindowDelegate: NSObject<NSWindowDelegate>
@end

@implementation WindowDelegate
- (void)windowWillClose: (NSNotification *)notification
{
  global_running = false;
}
@end

int main(int argc, const char *argv[])
{
  NSRect screen_rect = [[NSScreen mainScreen] frame];
  NSRect content_rect = NSMakeRect((screen_rect.size.width - global_rendering_width) * .5,
                                   (screen_rect.size.height - global_rendering_height) * .5,
                                   global_rendering_width,
                                   global_rendering_height);

  NSWindow *window = [[NSWindow alloc]
                     initWithContentRect: content_rect
                     styleMask: (NSWindowStyleMaskTitled |
                                 NSWindowStyleMaskClosable |
                                 NSWindowStyleMaskMiniaturizable |
                                 NSWindowStyleMaskResizable) 
                     backing: NSBackingStoreBuffered
                     defer: NO];
  [window setTitle: @"AutoDraw"];
  [window makeKeyAndOrderFront: nil];
  [window setDelegate:[[WindowDelegate alloc] init]];
  window.contentViewController = [[AutoViewController alloc] init];

  while (global_running)
  {
    NSEvent *event = [NSApp nextEventMatchingMask: NSEventMaskAny
                      untilDate: nil
                      inMode: NSDefaultRunLoopMode
                      dequeue: YES];
    switch ([event type])
    {
      default:
      {
        [NSApp sendEvent: event];
      }
    }
  }

  printf("objective-c autodraw finished!\n");
}
