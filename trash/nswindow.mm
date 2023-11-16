#include <stdio.h>

#include <AppKit/AppKit.h>

#include "utils.h"

global_variable r32 global_rendering_width = 1024;
global_variable r32 global_rendering_height = 768;
global_variable b32 global_running = true;

@interface WindowDelegate: NSObject<NSWindowDelegate>
@end

@implementation WindowDelegate
- (void)windowWillClose: (NSNotification *)notification
{
  global_running = false;
}
@end

int main(int argc, const char *argv[]) {
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
  [window setBackgroundColor: NSColor.redColor];
  [window setTitle: @"AutoDraw"];
  [window makeKeyAndOrderFront: nil];

  WindowDelegate *windowDelegate = [[WindowDelegate alloc] init];
  [window setDelegate:windowDelegate];

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

  printf("autodraw finished!");
}
