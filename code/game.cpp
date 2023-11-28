/*
  Rules for the renderer:
  - Textures and bitmaps are srgb premultiplied-alpha
  - Colors are in linear range, alpha=1 (so pma doesn't matter)
  - Packed colors are rgba in memory order (i.e abgr in u32 register order)
  pma = pre-multiplied alpha

  todo:
  - make the tree structure
 */

// #include "kv_math.h"
#include "kv_utils.h"
#include "platform.h"

struct DocumentTree {
  String        root;
  DocumentTree *children;
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
pushRect(RenderGroup &rgroup, Rect2 rect, TextureId texture, V3 color)
{
  RenderEntryRectangle &entry = *pushRenderEntry(rgroup, Rectangle);
  entry.rect    = rect;
  entry.texture = texture;
  entry.color   = color;
}

inline void
pushRect(RenderGroup &rgroup, Rect2 rect, V3 color)
{
  pushRect(rgroup, rect, TextureIdWhite, color);
}

inline void
pushRect(RenderGroup &rgroup, Rect2 rect, TextureId texture)
{
  pushRect(rgroup, rect, texture, V3{1,1,1});
}

internal void
pushRectOutline(RenderGroup &rgroup, Rect2 outline, i32 thickness_px, V3 color)
{
  V2 min = outline.min;
  V2 max = outline.max;
  V2 t = 0.5f * V2{pixel_to_clip_x * (f32)thickness_px,
                   pixel_to_clip_y * (f32)thickness_px};
  V2 dimx = {max.x - min.x, 0};
  V2 dimy = {0, max.y - min.y};
  // left
  pushRect(rgroup, Rect2{min-t, min+dimy+t}, color);
  // right
  pushRect(rgroup, Rect2{min+dimx-t, max+t}, color);
  // bottom
  pushRect(rgroup, Rect2{min-t, min+dimx+t}, color);
  // top
  pushRect(rgroup, Rect2{min+dimy-t, max+t}, color);
}

struct DebugDrawer {
  Arena       &arena;
  RenderGroup &rgroup;
  f32          y = -1.f;
};

inline f32
pushLetter(RenderGroup &rgroup, V2 min, char character)
{
  assert(33 <= character && character <= 126);
  auto codepoint = rgroup.codepoints[(u8)character];
  auto dim = V2{pixel_to_clip_x * (f32)codepoint.width,
                pixel_to_clip_y * (f32)codepoint.height};
  pushRect(rgroup, rectMinDim(min, dim), codepointTexture(character));
  return dim.x;
}

internal void
pushText(RenderGroup &rgroup, V2 min, String string)
{
  auto x = min.x;
  for (i32 i=0; i < string.length; i++) {
    char c = string.chars[i];
    if (c == ' ') {
      x += pixel_to_clip_x * 12.f;
    } else {
      x += pushLetter(rgroup, {x, min.y}, c);
    }
  }
}

inline void
pushTextFormat(Arena &arena, RenderGroup &rgroup, V2 min, char *format, ...)
{
  va_list args;
  va_start(args, format);
  auto string = printVA(arena, format, args);
  pushText(rgroup, min, string);
  va_end(args);
}

inline void
pushDebugText(DebugDrawer &drawer, char *format, ...)
{
  va_list args;
  va_start(args, format);
  auto string = printVA(drawer.arena, format, args);
  pushText(drawer.rgroup, {-1.f, drawer.y}, string);
  drawer.y += pixel_to_clip_y * debug_font_height;
  va_end(args);
}

extern "C" GAME_UPDATE_AND_RENDER(gameUpdateAndRender)
{
  Arena &temp_arena = memory.arena;
  auto temp_marker = beginTemporaryMemory(temp_arena);
  if (!memory.initialized) {
    // ... setting startup parameters here
    memory.initialized = true;
  }
  auto &rgroup = memory.rgroup;
  rgroup.arena       = subArena(temp_arena, megaBytes(128));
  rgroup.codepoints  = memory.codepoints;

  // Game logic //////////////////////////////////////
  auto &tile_offset    = memory.tile_offset;
  auto &velocity       = memory.velocity;
  auto &absolute_coord = memory.absolute_coord;

  auto &dt = target_frame_time_sec;
  b32 moving_right = memory.action_states[GameActionMoveRight].is_down;
  b32 moving_left  = memory.action_states[GameActionMoveLeft].is_down;
  if (moving_right || moving_left) {
    if (memory.new_direction_key_press) {
      memory.velocity = 0;
      memory.absolute_coord += moving_right ? 1 : -1;
    } else {
      // unit of movement: object
      const f32 da = 4.f * (f32)screen_width_in_tiles;
      f32 acceleration = moving_right ? da : -da;
      tile_offset += velocity * dt + 0.5f * acceleration * dt * dt;
      velocity    += acceleration * dt;

      i32 tile_offset_rounded = (tile_offset == -0.5f) ? 0 : roundF32ToI32(tile_offset);
      absolute_coord += tile_offset_rounded;
      tile_offset -= (f32)tile_offset_rounded;
    }
  } else {
    velocity    = 0.f;
    tile_offset = 0.f;
  }

  DocumentTree uncle  = {.root=toString("uncle"), .children=0};
  DocumentTree myself = {.root=toString("myself")};
  DocumentTree mom    = {.root=toString("mom"), .children=&myself};
  DocumentTree grandma_children[] = {mom, uncle};
  DocumentTree tree = {
    .root = toString("grandma"),
    .children = grandma_children,
  };

  // clamp cursor coordinate
  if (absolute_coord <= 0) {
    absolute_coord = 0;
    if (tile_offset < 0) {
      tile_offset = 0;
    }
  }
  ///////////////////////////////////////////////////////

  // Draw stuff ////////////////////////////////////

  // draw backdrop
  pushRect(rgroup, Rect2{-1.f, -1.f, 1.f, 1.f}, V3{0,0,0});

  {// draw cursor
    i32 cursor_x = absolute_coord % screen_width_in_tiles;
    i32 cursor_y = absolute_coord / screen_width_in_tiles;
    f32 tile_width = 2.f / (f32)screen_width_in_tiles;
    V2 tile_dim = {tile_width, tile_width};
    V2 min = {-1.f + ((f32)cursor_x + tile_offset) * tile_dim.x,
              +1.f - (f32)(cursor_y+1) * tile_dim.y};
    auto rect = rectMinDim(min, tile_dim);
    pushRect(rgroup, rect, V3{1,0,0});
  }
  
  // Draw the tree
  pushRectOutline(rgroup, Rect2{-0.5f, -0.5f, 0.5f, 0.5f}, 4, V3{1,1,1});

  // draw debug text
  DebugDrawer debug_drawer = {.arena=temp_arena, .rgroup=rgroup};
  pushDebugText(debug_drawer, "frame time: %.3f ms", memory.last_frame_time_sec * 1000);
  pushDebugText(debug_drawer, "cursor velocity: %.3f unit/s", velocity);

  endTemporaryMemory(temp_marker);
}
