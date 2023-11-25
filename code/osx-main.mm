#import <stdio.h>
#import <Metal/Metal.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#import <mach/mach_time.h>
#include <sys/stat.h>

#import "kv_utils.h"
#import "kv_math.h"
#import "mac-keycodes.h"
#import "shader-interface.h"
#import "platform.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#define STB_TRUETYPE_IMPLEMENTATION
#include "imstb_truetype.h"
#pragma clang diagnostic pop

global_variable f32 global_rendering_width = 1920;
global_variable f32 global_rendering_height = 1080;
global_variable f32 pixel_to_clip_x = 2.f / global_rendering_width;
global_variable f32 pixel_to_clip_y = 2.f / global_rendering_height;
global_variable mach_timebase_info_data_t timebase;

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

u8 *virtualAlloc(size_t size)
{
    u8 *data = 0;
 
    assert(size != 0);
    size_t new_size = 4096 * ((size / 4096) + 1);
    assert(new_size >= size);
    size = new_size;
 
    // Allocate directly from VM
    kern_return_t err = vm_allocate((vm_map_t) mach_task_self(),
                                    (vm_address_t*) &data,
                                    size,
                                    VM_FLAGS_ANYWHERE);
    assert(err == KERN_SUCCESS);
 
    return data;
}

struct Renderer {
  Arena arena;
  i32 vertex_count;
};

internal void
pushRect(Renderer &renderer, f32 min_x, f32 min_y, f32 width, f32 height, i32 type)
{
  auto rect = (VertexInput *) pushSize(&renderer.arena, (6)*sizeof(VertexInput));
  f32 max_x = min_x + width;
  f32 max_y = min_y + height;
  rect[0] = {{min_x, max_y}, {0.f, 0.f}, type};
  rect[1] = {{min_x, min_y}, {0.f, 1.f}, type};
  rect[2] = {{max_x, min_y}, {1.f, 1.f}, type};
  rect[3] = {{min_x, max_y}, {0.f, 0.f}, type};
  rect[4] = {{max_x, min_y}, {1.f, 1.f}, type};
  rect[5] = {{max_x, max_y}, {1.f, 0.f}, type};
  renderer.vertex_count += 6;
}

double osxGetCurrentTimeInSeconds()
{
  u64 nano_secs = mach_absolute_time() * timebase.numer / timebase.denom;
  return (double)nano_secs * 1.0E-9;
}

struct ActionState {
  b32 is_down;
};

enum GameAction {
  GameActionMoveRight,
  GameActionMoveLeft,
  GameActionMoveUp,
  GameActionMoveDown,
  GameActionCount,
};

void platformFreeFileMemory(u8 *memory) {
  if (memory) {
    todoIncomplete;  // #test
    vm_deallocate((vm_map_t) mach_task_self(), (vm_address_t) memory, 0);
  }
}

ReadFileResult platformReadEntireFile(const char *file_name)
{
  ReadFileResult out = {};

  int fd = open(file_name, O_RDONLY);
  if (fd == -1) {
    printf("platformReadEntireFile %s: open error: %d: %s\n",
           file_name, errno, strerror(errno));
  } else {
    struct stat file_stat;
    if (fstat(fd, &file_stat) != 0) {
      printf("platformReadEntireFile %s: fstat error: %d: %s\n",
             file_name, errno, strerror(errno));
    } else {
      i64 file_size = file_stat.st_size;
      out.content = virtualAlloc(file_size);
      if (!out.content) {
        printf("platformReadEntireFile %s:  vm_allocate error: %d: %s\n",
               file_name, errno, strerror(errno));
      }
      ssize_t bytes_read;
      bytes_read = read(fd, out.content, file_size);
      if (bytes_read == file_size) {
        out.content_size = file_size;
      } else {
        platformFreeFileMemory(out.content);
        out.content = 0;
        printf("platformReadEntireFile %s:  couldn't read file: %d: %s\n",
               file_name, errno, strerror(errno));
      }
    }
  }

  close(fd);
  return out;
}

struct Nothings {
  id<MTLTexture> texture;
  i32 width, height, xoff, yoff;
};

#define STB_IMAGE_IMPLEMENTATION
#import "stb_image.h"

internal Nothings
makeNothingsTexture(Arena *arena, id<MTLDevice> mtl_device)
{
  Nothings nothings = {};
  auto read_file = platformReadEntireFile("../resources/fonts/LiberationSans-Regular.ttf");
  u8 *ttf_buffer = read_file.content;
  if (!ttf_buffer) {
    todoErrorReport;
  } else {
    stbtt_fontinfo font;
    stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));
    f32 pixel_height = stbtt_ScaleForPixelHeight(&font, 128.f);
    u8 *mono_bitmap = stbtt_GetCodepointBitmap(&font, 0,pixel_height, 'N',
                                               &nothings.width, &nothings.height,
                                               &nothings.xoff, &nothings.yoff);
    
    auto width = nothings.width;
    auto height = nothings.height;

    u8 *bitmap = (u8 *)pushSize(arena, 4 * nothings.width * nothings.height);
    // Blow it out to rgba bitmap
    u32 *dst = (u32 *)bitmap;
    u8 *src  = mono_bitmap;
    for (i32 y=0; y < height; y++) {
      for (i32 x=0; x < width; x++) {
        u32 a = *src++;
        *dst++ = (a << 24) | (a << 16) | (a << 8) | a;
      }
    }
    stbtt_FreeBitmap(mono_bitmap, 0);

    // Create Texture (NOTE: alpha is in linear space, so this whole texture is in linear)
    auto texture_desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                         width:width height:height mipmapped:NO];
    nothings.texture = [mtl_device newTextureWithDescriptor:texture_desc];
    [texture_desc release];
    // Copy loaded image into MTLTextureObject
    [nothings.texture replaceRegion:MTLRegionMake2D(0,0,width,height)
     mipmapLevel:0
     withBytes:bitmap
     bytesPerRow:4*width];

  }
  return nothings;
}

internal Nothings
makeTestImageTexture(Arena *arena, id<MTLDevice> mtl_device)
{
  Nothings nothings = {};
  int num_channel;
  unsigned char* bitmap = stbi_load("../resources/blue.png",
                                    &nothings.width, &nothings.height, &num_channel, 4);
  assert(num_channel == 4);
  auto width = nothings.width;
  auto height = nothings.height;

  // Create Texture
  auto texture_desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm_sRGB
                       width:width height:height mipmapped:NO];
  nothings.texture = [mtl_device newTextureWithDescriptor:texture_desc];
  [texture_desc release];
  // Copy loaded image into MTLTextureObject
  [nothings.texture replaceRegion:MTLRegionMake2D(0,0,width,height)
   mipmapLevel:0
   withBytes:bitmap
   bytesPerRow:4*width];
  return nothings;
}

int main(int argc, const char *argv[])
{
  mach_timebase_info(&timebase);
  f32 game_update_hz = 60;
  const f32 target_seconds_per_frame = 1.0f / game_update_hz;

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
  // main_window.contentAspectRatio = NSMakeSize(4,3);
  main_window.title = @"AutoDraw";
  main_window.delegate = osx_main_delegate;
  main_window.contentView.wantsLayer = YES;
  [main_window makeKeyAndOrderFront: nil];

  [NSApp finishLaunching];

  id<MTLDevice> mtl_device = MTLCreateSystemDefaultDevice();
  printf("System default GPU: %s\n", mtl_device.name.UTF8String);

  CAMetalLayer *ca_metal_layer = [CAMetalLayer new];
  ca_metal_layer.frame       = main_window.contentView.frame;
  ca_metal_layer.device      = mtl_device;
  ca_metal_layer.pixelFormat = MTLPixelFormatRGBA8Unorm_sRGB;
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

  // Create a Sampler State
  auto sampler_desc = [MTLSamplerDescriptor new];
  sampler_desc.minFilter = MTLSamplerMinMagFilterLinear;
  sampler_desc.magFilter = MTLSamplerMinMagFilterLinear;
  auto sampler_state = [mtl_device newSamplerStateWithDescriptor:sampler_desc];
  [sampler_desc release];

  // Allocate a big block of memory
  size_t memory_cap = gigaBytes(1);
  u8 *memory = virtualAlloc(memory_cap);
  size_t temp_size = megaBytes(512);
  Arena temp_arena_ = newArena(temp_size, memory);
  Arena *temp_arena = &temp_arena_;
  Arena perm_arena_ = newArena(memory_cap-temp_size, memory+temp_size);;
  Arena *perm_arena = &perm_arena_;

  i32 screen_width_in_tiles = 80;

  auto vertex_descriptor = [MTLVertexDescriptor new];
  {
    auto v = vertex_descriptor;
    u64 offset = 0;
    i32 i = 0;
    // position
    v.attributes[i].format      = MTLVertexFormatFloat2;
    v.attributes[i].offset      = offset;
    v.attributes[i].bufferIndex = 0;
    offset += sizeof(simd_float2);
    i++;
    // uv
    v.attributes[i].format      = MTLVertexFormatFloat2;
    v.attributes[i].offset      = offset;
    v.attributes[i].bufferIndex = 0;
    offset += sizeof(simd_float2);
    i++;
    // type
    v.attributes[i].format      = MTLVertexFormatInt;
    v.attributes[i].offset      = offset;
    v.attributes[i].bufferIndex = 0;
    offset += sizeof(int);
    i++;
    // layout
    v.layouts[0].stride       = sizeof(VertexInput);
    v.layouts[0].stepRate     = 1;
    v.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
  }

  auto nothings = makeNothingsTexture(perm_arena, mtl_device);
  auto texture = nothings.texture;

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

  ActionState action_state[GameActionCount] = {};
  b32 new_direction_key_press = false;
  f32 velocity = {};
  f32 tile_offset = {};
  i32 absolute_coord = {};

  // Main loop
  f32 frame_start_sec = osxGetCurrentTimeInSeconds();
  osx_main_delegate->is_running = true;
  while (osx_main_delegate->is_running)
  {
    auto temp = beginTemporaryMemory(temp_arena);
    @autoreleasepool
    {
      // Process events
      new_direction_key_press = false;
      while (NSEvent *event = [NSApp nextEventMatchingMask: NSEventMaskAny
                               untilDate: nil
                               inMode: NSDefaultRunLoopMode
                               dequeue: YES])
      {
        switch (event.type) {
          case NSEventTypeKeyUp:
          case NSEventTypeKeyDown: {
            bool is_down = (event.type == NSEventTypeKeyDown);
            if ((event.keyCode == kVK_ANSI_Q) && is_down) {
              // todo: this should be Cmd+Q
              osx_main_delegate->is_running = false;
            } else {
              switch (event.keyCode) {
                case kVK_ANSI_H: {
                  action_state[GameActionMoveLeft].is_down = is_down;
                  new_direction_key_press = true;
                  break;
                }
                case kVK_ANSI_L: {
                  action_state[GameActionMoveRight].is_down = is_down;
                  new_direction_key_press = true;
                  break;
                }
              }
            }
          } break;

          default: {
            [NSApp sendEvent: event];
          }
        }
      }

      if (osx_main_delegate->window_was_resized) {
        ca_metal_layer.frame = main_window.contentView.frame;
        ca_metal_layer.drawableSize = ca_metal_layer.frame.size;
        osx_main_delegate->window_was_resized = false;
      }

      // Game logic
      auto &dt = target_seconds_per_frame;
      b32 moving_right = action_state[GameActionMoveRight].is_down;
      b32 moving_left  = action_state[GameActionMoveLeft].is_down;
      if (moving_right || moving_left) {
        if (new_direction_key_press) {
          absolute_coord += moving_right ? 1 : -1;
        } else {
          // unit of movement: object
          const f32 da = 100.f;
          f32 acceleration = moving_right ? da : -da;
          tile_offset += velocity * dt + 0.5f * acceleration * dt * dt;
          velocity    += acceleration * dt;
          if (velocity > 100) {debugbreak;}  // todo: why isn't this getting hit after 1s?

          i32 tile_offset_rounded = (tile_offset == -0.5f) ? 0 : roundF32ToI32(tile_offset);
          absolute_coord += tile_offset_rounded;
          tile_offset -= (f32)tile_offset_rounded;
        }
      } else {
        velocity    = 0.f;
        tile_offset = 0.f;
      }

      // clamp
      if (absolute_coord <= 0) {
        absolute_coord = 0;
        if (tile_offset < 0) {
          tile_offset = 0;
        }
      }

      // Render ////////////////////////////////////
      Renderer renderer = {};
      renderer.arena = subArena(temp_arena, megaBytes(128));

      // draw backdrop
      pushRect(renderer, -1.f, -1.f, +2.f, +2.f, 0);
      // draw nothings text
      pushRect(renderer, -0.f, -0.f, pixel_to_clip_x * (f32)nothings.width, pixel_to_clip_y * (f32)nothings.height, 2);

      i32 cursor_x = absolute_coord % screen_width_in_tiles;
      i32 cursor_y = absolute_coord / screen_width_in_tiles;
      f32 tile_width  = 2.f / (f32)screen_width_in_tiles;
      f32 tile_height = tile_width;
      // draw cursor
      pushRect(renderer,
               -1.f + ((f32)cursor_x + tile_offset) * tile_width,
               +1.f - (f32)(cursor_y+1) * tile_height,
               tile_width, tile_height, 1);

      // "Private" mode crashes the machine
      id<MTLBuffer> vertex_buffer = [[mtl_device newBufferWithBytes:renderer.arena.base
                                      length:renderer.arena.used
                                      options:MTLResourceStorageModeShared] autorelease];

      /////////////////////////////////////

      // sleep
      r32 sleep_sec;
      r32 seconds_elapsed_for_frame = osxGetCurrentTimeInSeconds() - frame_start_sec;
      if (seconds_elapsed_for_frame < target_seconds_per_frame) {
        sleep_sec = .8f * (target_seconds_per_frame - seconds_elapsed_for_frame);
        if (sleep_sec > 0) {
          sleep(sleep_sec);
        }
        seconds_elapsed_for_frame += sleep_sec;

        // busy wait
        while (seconds_elapsed_for_frame < target_seconds_per_frame) {
          seconds_elapsed_for_frame = osxGetCurrentTimeInSeconds() - frame_start_sec;
        }
      }

      frame_start_sec = osxGetCurrentTimeInSeconds();

      {// drawing
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
        render_pass_descriptor.colorAttachments[0].clearColor  = MTLClearColorMake(.6f, .0f, .6f, 1.f);

        auto command_buffer = [command_queue commandBuffer];

        auto render_command_encoder = [command_buffer renderCommandEncoderWithDescriptor:render_pass_descriptor];
        [render_command_encoder setViewport:(MTLViewport){0,0, ca_metal_layer.drawableSize.width, ca_metal_layer.drawableSize.height, 0,1}];
        [render_command_encoder setRenderPipelineState:render_pipeline_state];
        [render_command_encoder setVertexBuffer:vertex_buffer offset:0 atIndex:0];
        [render_command_encoder setFragmentTexture:texture atIndex:0];
        [render_command_encoder setFragmentSamplerState:sampler_state atIndex:0];
        [render_command_encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:renderer.vertex_count];
        [render_command_encoder endEncoding];

        [command_buffer presentDrawable:ca_metal_drawable];
        [command_buffer commit];
      }
    }
    endTemporaryMemory(temp);
  }

  printf("objective-c autodraw finished!\n");
  return 0;
}
