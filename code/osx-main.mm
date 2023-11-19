#import <stdio.h>
#import <Metal/Metal.h>
#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>
#import <MetalKit/MetalKit.h>
#import "utils.h"
#import "mac-keycodes.h"

global_variable r32 global_rendering_width = 1024;
global_variable r32 global_rendering_height = 768;

///////////////////////////////////////////////////////////////////////
// Application / Window Delegate

@interface OSX_MainDelegate: NSObject<NSApplicationDelegate, NSWindowDelegate>
{
  @public bool isRunning;
}
@end

@implementation OSX_MainDelegate
// NSApplicationDelegate methods
// NSWindowDelegate methods
- (void)windowWillClose:(id)sender 
{ 
  isRunning = false; 
}
@end

int main(int argc, const char *argv[])
{
  NSApplication *app = [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  [NSApp activateIgnoringOtherApps:YES];

  OSX_MainDelegate *osx_main_delegate = [OSX_MainDelegate new];
  app.delegate = osx_main_delegate;

  NSRect screen_rect = [NSScreen mainScreen].frame;
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
  window.backgroundColor = NSColor.purpleColor;
  window.title = @"AutoDraw";
  window.delegate = osx_main_delegate;
  window.contentView.wantsLayer = YES; // todo what
  [window makeKeyAndOrderFront: nil];

  [NSApp finishLaunching];

  osx_main_delegate->isRunning = true;
  while (osx_main_delegate->isRunning)
  {
    NSEvent *event = [NSApp nextEventMatchingMask: NSEventMaskAny
                      untilDate: nil
                      inMode: NSDefaultRunLoopMode
                      dequeue: YES];
    switch ([event type])
    {
      case NSEventTypeKeyDown:
      {
        if (event.keyCode == KEY_Q) osx_main_delegate->isRunning = false;
      } break;

      default:
      {
        [NSApp sendEvent: event];
      }
    }
  }

  printf("objective-c autodraw finished!\n");
}
