/*
  Header file imported by the game and all platforms.
  Define protocol of communication between the game and the platform.
  IMPORTANT: Minimize burden for the platform! Do not make them do work that the game can do itself.
 */

#pragma once

#include "kv-utils.h"
#include "kv-math.h"
#include "keycodes.h"

f32 debug_font_height = 128.f;
f32 global_rendering_width  = 1920;
f32 global_rendering_height = 1080;
f32 pixel_to_clip_x = 2.f / global_rendering_width;
f32 pixel_to_clip_y = 2.f / global_rendering_height;
f32 game_update_hz = 60.f;
f32 target_frame_time_sec = 1.f / game_update_hz;

#define PLATFORM_UPLOAD_RAY_TRACING_BITMAP(NAME) void NAME(u32 *bitmap, i32 dimx, i32 dimy)
typedef PLATFORM_UPLOAD_RAY_TRACING_BITMAP(PlatformUploadRayTracingBitmap);

struct PlatformCode {
  PlatformUploadRayTracingBitmap *uploadRayTracingBitmap;
};

// NOTE: currently we deal with monospaced fonts only
struct Codepoint {
  i32 width, height;
  f32 width_over_height;
  i32 xoff, yoff;
};

// todo: Does the platform really need to understand everything in the render group? I doubt it.
// But we need to keep things flexible since right now I don't understand how GPU works very well.
struct RenderGroup {
  Arena      commands;
  Arena      gpu_commands;
  Arena     *temp;
  Codepoint *codepoints;
  f32        monospaced_width;
};

enum TextureId {
  // 0-127 are reserved for ascii glyphs
  TextureIdWhite    = 128,
  TextureIdRayTrace = 129,
  TextureIdCount,
};

inline TextureId codepointTexture(char character) {
  return (TextureId)character;
}

enum RenderEntryType {
  // todo: Is there a fast way to clear the buffer in metal?
  RenderEntryTypeRectangle,
};

struct RenderEntryHeader {
    RenderEntryType type;
};

struct RenderEntryRectangle {
  rect2     rect;
  TextureId texture;
  v4        color;
};

enum GPUCommandType {
  GPUCommandTypeTriangle,
};

struct GPUCommandHeader {
  GPUCommandType type;
};

struct GPUCommands {
  Arena commands;
  Arena vertex_buffer;
  i32   vertex_start;
};

struct GPUCommandTriangle {
  i32 vertex_start;
  i32 vertex_count;
  TextureId texture;
};

struct ActionState {
  b32 is_down;
};

struct GameOutput {
  GPUCommands gcommands;
};

// NOTE: GameMemory is the communication interface between the game and platform.
struct GameInput {
  Arena arena;                  // All memory allocated for the game by the platform

  ActionState key_states[kVK_Count];
  f32         last_frame_time_sec;
  b32         hot_reloaded;
};

#define GAME_UPDATE_AND_RENDER(NAME) GameOutput NAME(GameInput &input)
typedef GAME_UPDATE_AND_RENDER(GameUpdateAndRender);

#define GAME_INITIALIZE(name) void name(Codepoint *codepoints, Arena &arena, PlatformCode *code)
typedef GAME_INITIALIZE(GameInitialize);
