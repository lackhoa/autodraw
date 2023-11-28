#pragma once

#include "kv_utils.h"
#include "kv_rect2.h"

i32 screen_width_in_tiles = 80;
f32 debug_font_height = 128.f;
f32 global_rendering_width  = 1920;  // TODO: this could change
f32 global_rendering_height = 1080;
f32 pixel_to_clip_x = 2.f / global_rendering_width;
f32 pixel_to_clip_y = 2.f / global_rendering_height;
f32 game_update_hz = 60.f;
f32 target_frame_time_sec = 1.f / game_update_hz;

struct ReadFileResult {
    u32 content_size;
    u8 *content;
};

ReadFileResult platformReadEntireFile(const char *file_name);

struct Codepoint {
  i32 width, height;
  f32 width_over_height;
  i32 xoff, yoff;
};

struct RenderGroup {
  Arena      arena;
  Codepoint *codepoints;
};

enum TextureId {
  // 0-127 are reserved for ascii glyphs
  TextureIdBackground = 128,
  TextureIdCursor     = 129,
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
  Rect2     rect;
  TextureId texture;
};

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

struct GameMemory {
  b32 initialized;

  Arena      arena;
  Codepoint *codepoints;        // TODO: can you do reference here?

  ActionState action_states[GameActionCount];
  b32         new_direction_key_press;
  f32         frame_time_sec;

  RenderGroup rgroup;
  f32         velocity;
  f32         tile_offset;
  i32         absolute_coord;
};
