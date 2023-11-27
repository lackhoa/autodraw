/*
  Rules for the renderer:
  - Textures and bitmaps are srgb premultiplied-alpha
  - Colors are in linear range, alpha=1 (so pma doesn't matter)
  - Packed colors are rgba in memory order (i.e abgr in u32 register order)
  pma = pre-multiplied alpha

  TODO:
  - draw the debug info
 */

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

global_variable id<MTLDevice> mtl_device;
global_variable mach_timebase_info_data_t timebase;
internal f32 global_rendering_width = 1920;
internal f32 global_rendering_height = 1080;
internal f32 pixel_to_clip_x = 2.f / global_rendering_width;
internal f32 pixel_to_clip_y = 2.f / global_rendering_height;

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

enum RenderEntryType {
  // todo: Is there a fast way to clear the buffer in metal?
  RenderEntryTypeRectangle,
};

struct RenderEntryHeader {
    RenderEntryType type;
};

struct RenderGroup {
  Arena arena;
};

struct RenderEntryRectangle {
  Rect2          rect;
  id<MTLTexture> texture;
};

inline void *
pushRenderEntry_(RenderGroup &rgroup, u32 size, RenderEntryType type)
{
  RenderEntryHeader *header = pushStruct(rgroup.arena, RenderEntryHeader);
  header->type = type;
  void *entry = pushSize(rgroup.arena, size);
  return entry;
}

#define pushRenderEntry(rgroup, type) (RenderEntry##type *) pushRenderEntry_(rgroup, sizeof(RenderEntry##type), RenderEntryType##type)

internal void
pushRect(RenderGroup &rgroup, f32 min_x, f32 min_y, f32 width, f32 height, id<MTLTexture>texture)
{
  RenderEntryRectangle &entry = *pushRenderEntry(rgroup, Rectangle);
  f32 max_x = min_x + width;
  f32 max_y = min_y + height;
  entry.rect    = {min_x, min_y, max_x, max_y};
  entry.texture = texture;
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

struct Codepoint {
  id<MTLTexture> texture;
  i32 width, height;
  f32 width_over_height;
  i32 xoff, yoff;
};

#define STB_IMAGE_IMPLEMENTATION
#import "stb_image.h"

inline V4
linearToSrgb(V4 linear)
{
    V4 result;
    result.r = squareRoot(linear.r);
    result.g = squareRoot(linear.g);
    result.b = squareRoot(linear.b);
    result.a = linear.a;
    return result;
}

inline u32
pack8x4(V4 color)
{
  u32 result = ((u32)(color.a*255.0f + 0.5f) << 24
                | (u32)(color.b*255.0f + 0.5f) << 16
                | (u32)(color.g*255.0f + 0.5f) << 8
                | (u32)(color.r*255.0f + 0.5f));
  return result;
}

internal id<MTLTexture>
sRGBATexture(void *bitmap, i32 width, i32 height)
{
  auto texture_desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm_sRGB
                       width:width height:height mipmapped:NO];
  auto texture = [mtl_device newTextureWithDescriptor:texture_desc];
  [texture_desc release];
  [texture replaceRegion:MTLRegionMake2D(0,0,width,height)
   mipmapLevel:0
   withBytes:bitmap
   bytesPerRow:4*width];
  return texture;
}

inline id<MTLTexture>
makeColorTexture(V4 color)
{
  V4 srgb = linearToSrgb(color);
  u32 packed = pack8x4(srgb);
  return sRGBATexture(&packed, 1, 1);
}

internal Codepoint codepoints[128];

internal void
makeCodepointTextures(Arena &arena, id<MTLDevice> mtl_device) {
  auto temp = beginTemporaryMemory(arena);
  auto read_file = platformReadEntireFile("../resources/fonts/LiberationSans-Regular.ttf");
  u8 *ttf_buffer = read_file.content;
  if (!ttf_buffer) {
    todoErrorReport;
  } else {
    stbtt_fontinfo font;
    stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));
    f32 pixel_height = stbtt_ScaleForPixelHeight(&font, 128.f);

    for (i32 ascii_char=33; ascii_char < 127; ascii_char++) {
      i32 width, height, xoff, yoff;
      u8 *mono_bitmap = stbtt_GetCodepointBitmap(&font, 0,pixel_height, ascii_char,
                                                 &width, &height, &xoff, &yoff);
      assert(width != 0 && height != 0);
      u8 *bitmap = (u8 *)pushSize(arena, 4 * width * height);
      // Blow it out to rgba bitmap
      u32 *dst = (u32 *)bitmap;
      u8 *src  = mono_bitmap;
      for (i32 y=0; y < height; y++) {
        for (i32 x=0; x < width; x++) {
          u32 au = *src++;
          // assert(au < 256);
          f32 c = (f32)au / 255.f;
          // pre-multiplied alpha (NOTE: we assume color is white)
          c = square(c);
          u32 cu = (u32)(255.f*c + 0.5f) ;
          *dst++ = (au << 24) | (cu << 16) | (cu << 8) | (cu << 0);
        }
      }

      stbtt_FreeBitmap(mono_bitmap, 0);
      // Note: The color is white so srgb doesn't matter
      auto texture = sRGBATexture(bitmap, width, height);
      codepoints[ascii_char] = {texture, width, height, (r32)width/(r32)height, xoff, yoff};}
  }
  endTemporaryMemory(temp);
}

internal id<MTLTexture>
makeTestImageTexture(id<MTLDevice> mtl_device)
{
  i32 width, height;
  int num_channel;
  unsigned char* bitmap = stbi_load("../resources/testTexture.png",
                                    &width, &height, &num_channel, 4);
  assert(bitmap && num_channel == 4);

  return sRGBATexture(bitmap, width, height);
}

inline f32
pushLetter(RenderGroup &rgroup, f32 min_x, f32 min_y, char character)
{
  auto codepoint = codepoints[(u8)character];
  auto width  = pixel_to_clip_x * (f32)codepoint.width;
  auto height = pixel_to_clip_y * (f32)codepoint.height;
  pushRect(rgroup, min_x, min_y, width, height, codepoint.texture);
  return width;
}

inline void
pushString(RenderGroup &rgroup, f32 min_x, f32 min_y, char *cstring)
{
  auto x = min_x;
  while (char c = *cstring++) {
    x += pushLetter(rgroup, x, min_y, c);
  }
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

  mtl_device = MTLCreateSystemDefaultDevice();
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
  Arena temp_arena = newArena(temp_size, memory);
  Arena perm_arena = newArena(memory_cap-temp_size, memory+temp_size);

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
    // layout
    v.layouts[0].stride       = sizeof(VertexInput);
    v.layouts[0].stepRate     = 1;
    v.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

    assert(offset == sizeof(VertexInput));
  }

  makeCodepointTextures(temp_arena, mtl_device);
  internal auto red_texture = makeColorTexture(v4(1,0,0,1));
  // internal auto bg_texture  = makeColorTexture(v4(0,0.25f,0.25f,1));
  internal auto bg_texture  = makeColorTexture(v4(0,0.f,0.f,1));

  auto render_pipeline_desc = [MTLRenderPipelineDescriptor new];
  render_pipeline_desc.vertexFunction   = vert_func;
  render_pipeline_desc.fragmentFunction = frag_func;
  render_pipeline_desc.vertexDescriptor = vertex_descriptor;
  auto attachment = render_pipeline_desc.colorAttachments[0];
  attachment.pixelFormat                 = ca_metal_layer.pixelFormat;
  attachment.blendingEnabled             = YES;
  // pre-multiplied alpha blending 
  attachment.rgbBlendOperation           = MTLBlendOperationAdd;
  attachment.alphaBlendOperation         = MTLBlendOperationAdd;
  attachment.sourceRGBBlendFactor        = MTLBlendFactorOne;
  attachment.sourceAlphaBlendFactor      = MTLBlendFactorOne;
  attachment.destinationRGBBlendFactor   = MTLBlendFactorOneMinusSourceAlpha;
  attachment.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;  // NOTE: This works when dst alpha=1, but idk about compositing with a translucent destination
  auto render_pipeline_state = [mtl_device newRenderPipelineStateWithDescriptor:render_pipeline_desc error:&error];
  if (!render_pipeline_state)
  {
    printf("failed to create render pipeline state: %s\n", error.localizedDescription.UTF8String);
    return 1;
  }

  [vert_func release];
  [frag_func release];
  [vertex_descriptor release];
  [render_pipeline_desc release];

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
          if (velocity > 100) {debugbreak;}  // TODO: why isn't this getting hit after 1s?

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

      // Draw stuff ////////////////////////////////////
      RenderGroup rgroup = {};
      rgroup.arena = subArena(temp_arena, megaBytes(128));

      // draw backdrop
      pushRect(rgroup, -1.f, -1.f, +2.f, +2.f, bg_texture);

      // draw nothings text
      pushString(rgroup, -1.f, -1.f, "nothings");

      // draw cursor
      i32 cursor_x = absolute_coord % screen_width_in_tiles;
      i32 cursor_y = absolute_coord / screen_width_in_tiles;
      f32 tile_width  = 2.f / (f32)screen_width_in_tiles;
      f32 tile_height = tile_width;
      pushRect(rgroup,
               -1.f + ((f32)cursor_x + tile_offset) * tile_width,
               +1.f - (f32)(cursor_y+1) * tile_height,
               tile_width, tile_height, red_texture);

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
        // "Private" mode crashes the machine


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
        auto command_encoder = [command_buffer renderCommandEncoderWithDescriptor:render_pass_descriptor];
        [command_encoder setViewport:(MTLViewport){0,0, ca_metal_layer.drawableSize.width, ca_metal_layer.drawableSize.height, 0,1}];
        [command_encoder setRenderPipelineState:render_pipeline_state];
        [command_encoder setFragmentSamplerState:sampler_state atIndex:0];

        // todo Tell the render group to keep track of frame buffer size, but that's platform-specific.
        auto vertex_arena = subArena(temp_arena, megaBytes(64));
        {// render group processing 1.: build vertex buffer
          u8 *next = rgroup.arena.base;
          u8 *end  = rgroup.arena.base + rgroup.arena.used;
          while (next != end) {
            auto header = (RenderEntryHeader *)next;
            next += sizeof(RenderEntryHeader);

            switch (header->type) {
              case RenderEntryTypeRectangle: {
                auto entry = (RenderEntryRectangle *)next;
                next += sizeof(RenderEntryRectangle);

                auto min_x = entry->rect.min.x;
                auto max_x = entry->rect.max.x;
                auto min_y = entry->rect.min.y;
                auto max_y = entry->rect.max.y;

                VertexInput *verts = pushArray(vertex_arena, (6), VertexInput);
                verts[0] = {{min_x, max_y}, {0.f, 0.f}};
                verts[1] = {{min_x, min_y}, {0.f, 1.f}};
                verts[2] = {{max_x, min_y}, {1.f, 1.f}};
                verts[3] = {{min_x, max_y}, {0.f, 0.f}};
                verts[4] = {{max_x, min_y}, {1.f, 1.f}};
                verts[5] = {{max_x, max_y}, {1.f, 0.f}};

                break;
              }

                invalidDefaultCase;
            }
          }
        }
        auto vertex_buffer = [[mtl_device newBufferWithBytes:vertex_arena.base
                               length:vertex_arena.used
                               options:MTLResourceStorageModeShared] autorelease];

        [command_encoder setVertexBuffer:vertex_buffer offset:0 atIndex:0];

        {// render group processing 2.: send commands (todo: because apparently we can't do this before setVertexBuffer?)
          i32 vstart = 0;
          u8 *next = rgroup.arena.base;
          u8 *end  = rgroup.arena.base + rgroup.arena.used;
          while (next != end) {
            auto header = (RenderEntryHeader *)next;
            next += sizeof(RenderEntryHeader);

            switch (header->type) {
              case RenderEntryTypeRectangle: {
                auto entry = (RenderEntryRectangle *)next;
                next += sizeof(RenderEntryRectangle);

                [command_encoder setFragmentTexture:entry->texture atIndex:0];
                [command_encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:vstart vertexCount:6];
                vstart += 6;

                break;
              }

                invalidDefaultCase;
            }
          }
        }

        [command_encoder endEncoding];

        [command_buffer presentDrawable:ca_metal_drawable];
        [command_buffer commit];
      }
    }
    endTemporaryMemory(temp);
  }

  printf("objective-c autodraw finished!\n");
  return 0;
}
