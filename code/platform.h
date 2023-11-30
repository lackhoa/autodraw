/*
  Header file imported by the game and all platforms.
  Define protocol of communication between the game and the platform.
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

// NOTE: currently we deal with monospaced fonts only
struct Codepoint {
  i32 width, height;
  f32 width_over_height;
  i32 xoff, yoff;
};

struct RenderGroup {
  Arena      commands;
  Arena     *temp;
  Codepoint *codepoints;
  f32        monospaced_width;
};

enum TextureId {
  // 0-127 are reserved for ascii glyphs
  TextureIdWhite = 128,
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
  v3        color;
};

struct ActionState {
  b32 is_down;
};

struct GameMemory {
  // From platform
  Arena       arena;            // Passing arena to show also how much space there are.
  ActionState key_states[kVK_Count];
  b32         new_key_press;    // todo: this logic is very hacky
  f32         last_frame_time_sec;

  // From game 
  RenderGroup rgroup;
};

#define GAME_UPDATE_AND_RENDER(name) void name(GameMemory &memory)
typedef GAME_UPDATE_AND_RENDER(GameUpdateAndRender);

#define GAME_INITIALIZE_MEMORY(name) void name(GameMemory &memory, Codepoint *codepoints)
typedef GAME_INITIALIZE_MEMORY(GameInitializeMemory);
