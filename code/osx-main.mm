/*
  The platform layer shouldn't know about the game, except for platform
 */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

// NOTE: Apple libs
#import <Metal/Metal.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#import <mach/mach_time.h>
#import <mach-o/dyld.h>

// NOTE: Other libraries
#import <stdio.h>
#import <sys/stat.h>
#import <dlfcn.h>
#import <pthread.h>
//
#define STBTT_STATIC  // Don't leak stb function names
#define STB_TRUETYPE_IMPLEMENTATION
#include "imstb_truetype.h"

#pragma clang diagnostic pop

// NOTE: My libraries
#import "kv-utils.h"
#import "shader-interface.h"
#import "platform.h"
#import "kv-bitmap.h"

// Application / Window Delegate (just to relay events right back to the main loop, haizz)
//

@interface OSX_MainDelegate: NSObject<NSApplicationDelegate, NSWindowDelegate>
{
  @public bool is_running;
}
@end

@implementation OSX_MainDelegate
- (NSSize)windowWillResize:(NSWindow *)window toSize:(NSSize)frame_size
{
  return frame_size;
}

- (void)windowWillClose:(id)sender 
{ 
  is_running = false; 
}
@end

///////////////////////////////////////////////////////////////////////

u8 *
osxVirtualAlloc(size_t size)
{
  u8 *data = 0;
 
  // NOTE: vm_allocate gives zerod memory.
  // https://developer.apple.com/library/archive/documentation/Performance/Conceptual/ManagingMemory/Articles/MemoryAlloc.html
  kern_return_t err = vm_allocate((vm_map_t) mach_task_self(),
                                  (vm_address_t*) &data,
                                  size,
                                  VM_FLAGS_ANYWHERE);
  assert(err == KERN_SUCCESS);
 
  return data;
}

f32
osxGetSecondsElapsed(u64 then, u64 now)
{
  local_persist mach_timebase_info_data_t timebase;
  if (timebase.denom == 0) {
    mach_timebase_info(&timebase);
  }

  u64 elapsed = now - then;
  u64 nano_secs = elapsed * timebase.numer / timebase.denom;
  return (double)nano_secs * 1.0E-9;
}

inline f32
osxGetSecondsElapsed(u64 then)
{
  return osxGetSecondsElapsed(then, mach_absolute_time());
}

void osxFreeFileMemory(u8 *memory) {
  if (memory) {
    todoIncomplete;  // #test
    vm_deallocate((vm_map_t) mach_task_self(), (vm_address_t) memory, 0);
  }
}

PLATFORM_READ_ENTIRE_FILE(osxReadEntireFile)
{
  ReadFileResult out = {};

  int fd = open(filename, O_RDONLY);
  if (fd == -1) {
    printf("platformReadEntireFile %s: open error: %d: %s\n",
           filename, errno, strerror(errno));
    return {};
  }
  defer(close(fd));

  struct stat file_stat;
  if (fstat(fd, &file_stat) != 0) {
    printf("platformReadEntireFile %s: fstat error: %d: %s\n",
           filename, errno, strerror(errno));
    return {};
  }
  i64 file_size = file_stat.st_size;
  out.content = (u8 *)pushSize(arena, file_size);
  if (!out.content) {
    printf("platformReadEntireFile %s:  vm_allocate error: %d: %s\n",
           filename, errno, strerror(errno));
    return {};
  }

  ssize_t bytes_read = read(fd, out.content, file_size);
  if (bytes_read != file_size) {
    out.content = 0;
    printf("platformReadEntireFile %s:  couldn't read file: %d: %s\n",
           filename, errno, strerror(errno));
    return {};
  }

  out.size = file_size;
  return out;
}

inline String
getParentDirName(Arena arena, String path)
{
  i32 last_slash_index = -1;
  for (i32 index=path.length-1; index >= 0; index--) {
    if (path.chars[index] == '/') {
      last_slash_index = index;
      break;
    }
  }

  String out = {.length=last_slash_index + 1};
  assert(out.length <= path.length);
  if (out.length > 0) {
    out.chars = (char *)pushCopySize(arena, path.chars, out.length+1);
  }
  out.chars[out.length] = 0;  // nil termination!
  return out;
}

PLATFORM_WRITE_ENTIRE_FILE(osxWriteEntireFile)
{
  // Create the directory along with missing parent directories
  u8 buffer[256] = {};
  Arena arena = newArena(buffer, 256);
  String dir_name = getParentDirName(arena, toString(filename));
  //
  i32 mkdir_output = mkdir(dir_name.chars, S_IRWXU | S_IRWXG | S_IROTH);
  if (mkdir_output == 0) {
    printf("Directory created successfully.\n");
  } else if (errno != EEXIST) {
    printf("failed to create directory %s: code %d, reason %s\n",
           dir_name.chars, errno, strerror(errno));
    return 0;  // Return an error code
  }

  i32 fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd == -1) {
    printf("failed to open file handle of file %s: code %d, reason %s\n",
           filename, errno, strerror(errno));
    return 0;
  }
  defer(close(fd));
  ssize_t bytes_written = write(fd, content, content_size);
  if (bytes_written != content_size) {
    printf("failed to write to file %s\n", filename);
    return 0;
  }
  return 1;
}

id<MTLTexture>
metal_sRGBATexture(id<MTLDevice> mtl_device, void *bitmap, i32 dimx, i32 dimy)
{
  id<MTLTexture> texture;
  @autoreleasepool
  {
    auto texture_desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm_sRGB
                         width:dimx height:dimy mipmapped:NO];
    texture = [mtl_device newTextureWithDescriptor:texture_desc];
    [texture replaceRegion:MTLRegionMake2D(0,0,dimx,dimy)
     mipmapLevel:0
     withBytes:bitmap
     bytesPerRow:4*dimx];
  }
  return texture;
}

inline id<MTLTexture>
makeColorTexture(id<MTLDevice> mtl_device, v4 color)
{
  u32 packed = pack_sRGBA(color);
  return metal_sRGBATexture(mtl_device, &packed, 1, 1);
}

id<MTLTexture> metal_textures[TextureIdCount];
Codepoint codepoints[128];

// todo: #startup #speed Maybe store the bitmaps in the asset system, but idk if it's even faster.
void makeCodepointTextures(Arena &arena, id<MTLDevice> mtl_device, char *font_file_path) {
  auto temp = beginTemporaryMemory(arena);
  defer(endTemporaryMemory(temp));

  auto read_file = osxReadEntireFile(arena, font_file_path);
  u8 *ttf_buffer = read_file.content;
  if (!ttf_buffer) {
    todoErrorReport;
    return;
  }
  stbtt_fontinfo font;
  stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));
  f32 pixel_height = stbtt_ScaleForPixelHeight(&font, font_height_px);

  for (i32 ascii_char=33; ascii_char <= 126; ascii_char++) {
    i32 width, height, xoff, yoff;
    u8 *mono_bitmap = stbtt_GetCodepointBitmap(&font, 0,pixel_height, ascii_char,
                                               &width, &height, &xoff, &yoff);
    assert(width != 0 && height != 0);
    u8 *bitmap = (u8 *)pushSize(arena, 4 * width * height);
    // Blow it out to rgba bitmap
    u32 *dst = (u32 *)bitmap;
    i32 pitch = width;
    u8 *src_row  = mono_bitmap + (height-1)*pitch;  // read backward in y
    for (i32 y=0; y < height; y++) {
      u8 *src = src_row;
      for (i32 x=0; x < width; x++) {
        u32 au = *src++;
        // assert(au < 256);
        f32 c = (f32)au / 255.f;
        // pre-multiplied alpha (NOTE: we assume color is white)
        c = square(c);
        u32 cu = (u32)(255.f*c + 0.5f) ;
        *dst++ = (au << 24) | (cu << 16) | (cu << 8) | (cu << 0);
      }
      src_row -= pitch;
    }

    stbtt_FreeBitmap(mono_bitmap, 0);
    // Note: The color is white so srgb doesn't matter
    metal_textures[ascii_char] = metal_sRGBATexture(mtl_device, bitmap, width, height);
    codepoints[ascii_char]     = {width, height, (r32)width/(r32)height, xoff, yoff};
  }
}

// #define STB_IMAGE_IMPLEMENTATION
// #import "stb_image.h"
// id<MTLTexture>
// makeTestImageTexture()
// {
//   i32 width, height;
//   int num_channel;
//   unsigned char* bitmap = stbi_load("../resources/testTexture.png",
//                                     &width, &height, &num_channel, 4);
//   assert(bitmap && num_channel == 4);
//   //
//   return metalsRGBATexture(bitmap, width, height);
// }

struct GameCode {
  String dylib_path;
  void *dl;
  GameUpdateAndRender *updateAndRender;
  GameInitialize      *initialize;
  time_t mtime;
};

time_t
osxGetMtime(char* filename)
{
	time_t mtime = 0;
	struct stat filestat;
	if (stat(filename, &filestat) == 0)
	{
		mtime = filestat.st_mtimespec.tv_sec;
	}
	return mtime;
}

b32
osxLoadOrReloadGameCode(GameCode &game) {
  time_t mtime = osxGetMtime(game.dylib_path.chars);
  if (mtime != game.mtime) {
    game.mtime = mtime;
    auto &dl = game.dl;
    dlclose(dl);

    dl = dlopen(game.dylib_path.chars, RTLD_LAZY|RTLD_GLOBAL);
    if (!dl) {
      printf("error: can't open game dylib: %s\n", game.dylib_path.chars);
      return false;
    }

    if (!game.initialize){
      game.initialize = (GameInitialize *)dlsym(dl, "gameInitialize");
      assert(game.initialize);
    }

    auto updateAndRender = (GameUpdateAndRender *)dlsym(dl, "gameUpdateAndRender");
    if (!updateAndRender) {
      assert(game.updateAndRender);  // We'll fail if the game function doesn't exist
      printf("error: can't load gameUpdateAndRender from %s\n", game.dylib_path.chars);
      return false;
    }
    game.updateAndRender = updateAndRender;

    printf("Hot loaded: %s, mtime: %ld\n", game.dylib_path.chars, mtime);
    return true;
  }
  return false;
}

struct ADMainInput {
  String autodraw_path;
  b32 is_fcoder_custom;
  NSWindow *main_window;
};

// todo: name
NSWindow *adMainFunctionBodyInMainThread(b32 is_fcoder_custom)
{
  NSArray *screens = [NSScreen screens];
  NSScreen *screen = [screens objectAtIndex:0];
  if (is_fcoder_custom) {
    if ([screens count] >= 2) {
      // todo: #hack to make our window appear on the other screen
      if (screen == [NSScreen mainScreen]) {
        screen = [screens objectAtIndex:1];
      }
    }
  }

  i32 init_width  = 1280;
  i32 init_height = 720;
  i32 init_origin_x = screen.frame.origin.x + (screen.frame.size.width - init_width)   * .5;
  i32 init_origin_y = screen.frame.origin.y + (screen.frame.size.height - init_height) * .5;
  NSRect ns_initial_frame = NSMakeRect(init_origin_x,
                                       init_origin_y,
                                       init_width,
                                       init_height);

  NSWindow *main_window = [[NSWindow alloc]
                           // initWithContentRect: ns_initial_frame
                           initWithContentRect: screen.frame
                           styleMask: (NSWindowStyleMaskTitled |
                                       NSWindowStyleMaskClosable |
                                       NSWindowStyleMaskMiniaturizable |
                                       NSWindowStyleMaskResizable) 
                           backing: NSBackingStoreBuffered
                           defer: NO];
  // I don't know why I have to do this to get the window to the other screen
  [main_window setFrame:ns_initial_frame display:YES];
  //
  main_window.backgroundColor = NSColor.purpleColor;
  main_window.title = @"AutoDraw";
  main_window.contentView.wantsLayer = YES;
  main_window.contentAspectRatio = NSMakeSize(16,9);
  [main_window makeKeyAndOrderFront: nil];


  return main_window;
}

b32 adMainFunctionBody(String autodraw_path, b32 is_fcoder_custom, NSWindow *main_window)
{
  size_t game_memory_cap     = gigaBytes(1);
  size_t platform_memory_cap = gigaBytes(1);
  //
  u8 *memory_base = osxVirtualAlloc(game_memory_cap + platform_memory_cap);
  Arena arena = newArena(memory_base, platform_memory_cap);
  memory_base += platform_memory_cap;
  //
  size_t frame_memory_cap = megaBytes(64);
  Arena frame_arena = subArena(arena, frame_memory_cap);

  GameCode game = {};
  game.dylib_path = concatenate(arena, autodraw_path, toString("/libgame.dylib"));

  [[NSFileManager defaultManager] changeCurrentDirectoryPath:[NSBundle mainBundle].bundlePath];

  id<MTLDevice> mtl_device = MTLCreateSystemDefaultDevice();
  printf("System default GPU: %s\n", mtl_device.name.UTF8String);

  {// NOTE: codepoint
    // u64 make_codepoint_start = mach_absolute_time();
    String font_file_path = concatenate(arena, autodraw_path, toString("/../resources/fonts/LiberationMono-Regular.ttf"));
    makeCodepointTextures(arena, mtl_device, font_file_path.chars);
    // f32 elapsed = osxGetSecondsElapsed(make_codepoint_start);
    // printf("make codepoint textures time: %f\n", elapsed);
  }
  
  metal_textures[TextureIdWhite] = makeColorTexture(mtl_device, v4{1,1,1,1});

  CAMetalLayer *ca_metal_layer = [CAMetalLayer new];
  ca_metal_layer.frame       = main_window.contentView.frame;
  ca_metal_layer.device      = mtl_device;
  ca_metal_layer.pixelFormat = MTLPixelFormatRGBA8Unorm_sRGB;
  [main_window.contentView.layer addSublayer:ca_metal_layer];

  // Load shaders
  NSError *error = nil;
  auto render_pipeline_desc = [MTLRenderPipelineDescriptor new];
  @autoreleasepool {
    // Thank you NSString!
    String shaders_metallib_ = concatenate(arena, autodraw_path, toString("/shaders.metallib"));
    NSString *shaders_metallib = [NSString stringWithCString:shaders_metallib_.chars encoding:NSUTF8StringEncoding];
    id<MTLLibrary> mtl_library = [mtl_device newLibraryWithFile: shaders_metallib error:&error];
    if (!mtl_library) {
      printf("Failed to load shader library: %s\n", error.localizedDescription.UTF8String);
      return false;
    }
    id<MTLFunction> vert_func = [mtl_library newFunctionWithName:@"vert"];
    id<MTLFunction> frag_func = [mtl_library newFunctionWithName:@"frag"];

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
      // color
      v.attributes[i].format      = MTLVertexFormatFloat4;
      v.attributes[i].offset      = offset;
      v.attributes[i].bufferIndex = 0;
      offset += sizeof(simd_float4);
      i++;
      // layout
      v.layouts[0].stride       = sizeof(VertexInput);
      v.layouts[0].stepRate     = 1;
      v.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
      assert(offset == sizeof(VertexInput));
    }

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
  }

  // Create a Sampler State
  auto sampler_desc = [MTLSamplerDescriptor new];
  sampler_desc.minFilter = MTLSamplerMinMagFilterLinear;
  sampler_desc.magFilter = MTLSamplerMinMagFilterLinear;
  auto sampler_state = [mtl_device newSamplerStateWithDescriptor:sampler_desc];
  [sampler_desc release];

  auto render_pipeline_state = [mtl_device newRenderPipelineStateWithDescriptor:render_pipeline_desc error:&error];
  if (!render_pipeline_state) {
    printf("failed to create render pipeline state: %s\n", error.localizedDescription.UTF8String);
    return false;
  }
  [render_pipeline_desc release];

  auto command_queue = [mtl_device newCommandQueue];

  GameInput game_input = {};
  game_input.arena = newArena(memory_base, game_memory_cap);
  PlatformCode platform_code = {.readEntireFile = osxReadEntireFile,
                                .writeEntireFile = osxWriteEntireFile};
  osxLoadOrReloadGameCode(game);
  game.initialize(game_input.arena, platform_code, autodraw_path, codepoints);

  OSX_MainDelegate *osx_main_delegate = [OSX_MainDelegate new];
  main_window.delegate = osx_main_delegate;

  if (!is_fcoder_custom) {
    NSApplication *app = [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [app activate];  // Switch to this window, handy for development
    [NSApp finishLaunching];
  }

  // NOTE: Main game loop
  u64 frame_start_tick = mach_absolute_time();
  f32 frame_time_sec  = 0;
  osx_main_delegate->is_running = true;
  b32 initial_frame = true;
  while (osx_main_delegate->is_running)
  {
    @autoreleasepool
    {
      auto temp_marker = beginTemporaryMemory(frame_arena);
      defer(endTemporaryMemory(temp_marker));

      game_input.hot_reloaded = initial_frame || osxLoadOrReloadGameCode(game);

      if (!is_fcoder_custom)
      {// Process events
        while (NSEvent *event = [NSApp nextEventMatchingMask: NSEventMaskAny
                                 untilDate: nil
                                 inMode: NSDefaultRunLoopMode
                                 dequeue: YES])
        {
          switch (event.type) {
            case NSEventTypeKeyUp:
            case NSEventTypeKeyDown: {
              u32 modifier_flags = [event modifierFlags];
              b32 cmd_held       = (modifier_flags & NSCommandKeyMask) > 0;
              b32 ctrl_held      = (modifier_flags & NSControlKeyMask) > 0;
              unused_var b32 alt_held       = (modifier_flags & NSAlternateKeyMask) > 0;
              unused_var b32 shift_held     = (modifier_flags & NSShiftKeyMask) > 0;
            
              // We'd like to handle repeat ourselves
              unused_var b32 is_repeat = [event isARepeat];

              bool is_down = (event.type == NSEventTypeKeyDown);
              if ((event.keyCode == kVK_ANSI_Q) && is_down && cmd_held) {
                // Cmd-Q
                osx_main_delegate->is_running = false;
              } else if ((event.keyCode == kVK_ANSI_F) && is_down && ctrl_held && cmd_held) {
                // Ctrl-Cmd-F
                [main_window toggleFullScreen:nil];
              } else {
                switch (event.keyCode) {
                  default: {
                    game_input.key_states[event.keyCode].is_down = is_down;
                  }
                }
              }
            } break;

            default: {
              [NSApp sendEvent: event];
            }
          }
        }
      }

      {// Frame size
        NSRect frame = main_window.contentView.frame;
        ca_metal_layer.frame        = frame;
        ca_metal_layer.drawableSize = frame.size;
        game_input.screen_dim = v2{(f32)frame.size.width,
                                   (f32)frame.size.height};
      }

      GameOutput game_output = game.updateAndRender(game_input);

      // sleep
      frame_time_sec = osxGetSecondsElapsed(frame_start_tick);
      f32 diff = (target_frame_time_sec - frame_time_sec);
      if (diff > 0) {
        f32 sleep_sec = diff;  // oh man, osx sleep is awesome!
        sleep(sleep_sec);

        // busy wait
        frame_time_sec = osxGetSecondsElapsed(frame_start_tick);
        while (frame_time_sec < target_frame_time_sec) {
          frame_time_sec = osxGetSecondsElapsed(frame_start_tick);
        }
      }

      game_input.last_frame_time_sec = frame_time_sec;
      frame_start_tick = mach_absolute_time();

      {// drawing to the screen
        auto ca_metal_drawable = [ca_metal_layer nextDrawable];
        if (!ca_metal_drawable) {
          printf("ERROR: nextDrawable timed out!\n");
          continue;
        }

        auto render_pass_descriptor = [[MTLRenderPassDescriptor new] autorelease];
        render_pass_descriptor.colorAttachments[0].texture     = ca_metal_drawable.texture;
        render_pass_descriptor.colorAttachments[0].loadAction  = MTLLoadActionClear;
        render_pass_descriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
        render_pass_descriptor.colorAttachments[0].clearColor  = MTLClearColorMake(.6f, .0f, .6f, 1.f);

        auto command_buffer  = [command_queue commandBuffer];

        {// blitting
          Bitmap bitmap = game_output.raytracing_bitmap;
          id<MTLTexture> *texture_ptr = &metal_textures[TextureIdRayTrace];
          id<MTLTexture> texture = *texture_ptr;
          if (texture) {
            assert((i32)bitmap.dim.x == (i32)texture.width &&
                   (i32)bitmap.dim.y == (i32)texture.height);  // todo: support changing dimension
            [texture replaceRegion:MTLRegionMake2D(0,0,bitmap.dim.x,bitmap.dim.y)
             mipmapLevel:0
             withBytes:bitmap.memory
             bytesPerRow:bitmap.pitch];
          } else {
            *texture_ptr = metal_sRGBATexture(mtl_device, bitmap.memory, bitmap.dim.x, bitmap.dim.y);
            texture = *texture_ptr;
          }

          auto blit_encoder = [command_buffer blitCommandEncoder];
          [blit_encoder synchronizeResource:texture];
          [blit_encoder endEncoding];
        }

        auto command_encoder = [command_buffer renderCommandEncoderWithDescriptor:render_pass_descriptor];
        [command_encoder setViewport:(MTLViewport){0,0, ca_metal_layer.drawableSize.width, ca_metal_layer.drawableSize.height, 0,1}];
        [command_encoder setRenderPipelineState:render_pipeline_state];
        [command_encoder setFragmentSamplerState:sampler_state atIndex:0];

        auto &gcommands = game_output.gcommands;
        // "MTLResourceStorageModePrivate" mode crashes my machine
        auto vertex_buffer = [[mtl_device newBufferWithBytes:gcommands.vertex_buffer.base
                               length:gcommands.vertex_buffer.used
                               options:MTLResourceStorageModeShared] autorelease];

        [command_encoder setVertexBuffer:vertex_buffer offset:0 atIndex:0];

        {// executing gpu commands
          auto &commands = gcommands.commands;
          u8 *next = commands.base;
          u8 *end  = commands.base + commands.used;
          while (next != end) {
            auto &header = *EAT_TYPE(next, GPUCommandHeader);

            switch (header.type) {
              case GPUCommandTypeTriangle: {
                auto &command = *EAT_TYPE(next, GPUCommandTriangle);

                [command_encoder setFragmentTexture:metal_textures[command.texture] atIndex:0];
                [command_encoder drawPrimitives:MTLPrimitiveTypeTriangle
                 vertexStart:command.vertex_start
                 vertexCount:command.vertex_count];

                break;
              }

                // todo: cutnpaste
              case GPUCommandTypeTriangleStrip: {
                auto &command = *EAT_TYPE(next, GPUCommandTriangleStrip);

                [command_encoder setFragmentTexture:metal_textures[command.texture] atIndex:0];
                [command_encoder drawPrimitives:MTLPrimitiveTypeTriangleStrip
                 vertexStart:command.vertex_start
                 vertexCount:command.vertex_count];

                break;
              }

                invalidDefaultCase;
            }
          }
        }

        [command_encoder endEncoding];
        [command_buffer presentDrawable:ca_metal_drawable];
        [command_buffer commit];
        [command_buffer waitUntilCompleted];
      }
    }
    initial_frame = false;
  }
  return true;
}

void *adMainFunctionThread(void *ad_main_input)
{
  ADMainInput input = *(ADMainInput *)ad_main_input;
  b32 success = adMainFunctionBody(input.autodraw_path, input.is_fcoder_custom, input.main_window);
  return (void *)((i64)success);
}

// todo: We should make "autodraw_path_chars" point to the "AutoDraw" root
DLL_EXPORT
b32 adMainFcoder(char *autodraw_path_chars)
{
  pthread_attr_t  attr;
  pthread_t       posix_thread_id;
  int             result;

  result = pthread_attr_init(&attr);
  assert(!result);
  //
  ADMainInput &input = *(ADMainInput *)malloc(sizeof(ADMainInput));
  input.autodraw_path    = toString(autodraw_path_chars);
  input.is_fcoder_custom = true;
  input.main_window = adMainFunctionBodyInMainThread(true);
  //
  int thread_error = pthread_create(&posix_thread_id, &attr, &adMainFunctionThread, &input);
  if (thread_error != 0) {
    return false;
  }

  return true;
}

int main(int argc, const char *argv[])
{
  u8 buffer[2048];
  u32 buffer_size = sizeof(buffer);
  Arena arena = newArena(buffer, sizeof(buffer));
  i32 result = _NSGetExecutablePath((char *)buffer, &buffer_size);
  assert(result == 0);
  String exe_path = toString((char *)buffer);
  NSWindow *main_window = adMainFunctionBodyInMainThread(false);
  b32 success = adMainFunctionBody(getParentDirName(arena, exe_path), false, main_window);
  return success;
}
