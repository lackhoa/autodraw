/*
  Rules for the renderer:
  - Textures and bitmaps are srgb premultiplied-alpha
  - Colors are in linear range, alpha=1 (so pma doesn't matter)
  - Packed colors are rgba in memory order (i.e abgr in u32 register order)
  pma = pre-multiplied alpha

  todo:
  - navigate the tree vertically
  - hot code reload
  - try my luck with the new llvm, see if I can fix the stupid macro bug
 */

// #include "kv_math.h"
#include "kv_utils.h"
#include "platform.h"

struct UITree {
  String  data;

  UITree *children;  // todo this should be double star
  UITree *parent;
  UITree *next_sibling;
  UITree *prev_sibling;
};

inline void *
pushRenderEntry_(RenderGroup &rgroup, u32 size, RenderEntryType type)
{
  RenderEntryHeader *header = pushStruct(rgroup.commands, RenderEntryHeader);
  header->type = type;
  void *entry = pushSize(rgroup.commands, size);
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
  auto dim = V2{rgroup.monospaced_width,
                pixel_to_clip_y * (f32)codepoint.height};
  pushRect(rgroup, rectMinDim(min, dim), codepointTexture(character));
  return dim.y;
}

internal f32
pushText(RenderGroup &rgroup, V2 min, String string)
{
  f32 max_dim_y = 0;
  auto x = min.x;
  for (i32 i=0; i < string.length; i++) {
    char c = string.chars[i];
    if (c != ' ') {
      f32 dim_y = pushLetter(rgroup, {x, min.y}, c);
      max_dim_y = maximum(max_dim_y, dim_y);
    }
    x += rgroup.monospaced_width;
  }
  return min.y + max_dim_y;
}

inline f32
pushTextFormat(RenderGroup &rgroup, V2 min, char *format, ...)
{
  va_list args;
  va_start(args, format);
  auto string = printVA(*rgroup.temp, format, args);
  va_end(args);
  return pushText(rgroup, min, string);
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

struct GameState {
  Arena perm_arena;

  f32 velocity;
  f32 cursor_tile_offset;
  f32 tree_tile_offset;
  i32 cursor_coord;

  UITree  grandma;
  UITree *hot_item;
  b32     cursor_mode;
};

internal V2
drawTree(GameState &state, RenderGroup &rgroup, UITree &tree, V2 min)
{
  f32 margin  = pixel_to_clip_x * 5;  // todo cleanup
  f32 spacing = pixel_to_clip_x * 20;
  f32 data_dim_x = rgroup.monospaced_width * (f32)tree.data.length;
  f32 max_y = pushText(rgroup, min, tree.data);
  f32 max_x = min.x + data_dim_x;

  UITree *childp = tree.children;
  while (childp) {
    auto &child = *childp;
    V2 child_min = {max_x + spacing, min.y + margin};
    V2 child_max = drawTree(state, rgroup, child, child_min);
    max_y = maximum(max_y, child_max.y);
    max_x = child_max.x + margin;
    childp = child.next_sibling;
  }

  if (tree.children) {
    max_y += margin;
  }

  f32 outline_thickness = 1;
  if (&tree == state.hot_item) {
    outline_thickness = 4;
  }
  pushRectOutline(rgroup, Rect2{min, V2{max_x, max_y}}, outline_thickness, V3{1,1,1});

  return V2{max_x, max_y};
}

internal void
initGameState(GameMemory &memory, GameState &state) {
  state.perm_arena = subArena(memory.arena, megaBytes(512));
  auto &arena = state.perm_arena;

  auto grandma = &state.grandma;
  auto myself  = pushStruct(arena, UITree);
  auto uncle   = pushStruct(arena, UITree);
  auto mom     = pushStruct(arena, UITree);

  state.grandma = {.data=toString("grandma"), .children=uncle};
  *uncle        = {.data=toString("uncle"), .parent=grandma, .next_sibling=mom};
  *mom          = {.data=toString("mom"), .children=myself, .parent=grandma, .prev_sibling=uncle};
  *myself       = {.data=toString("myself"), .parent=mom};

  state.hot_item = uncle;
}

extern "C" GAME_INITIALIZE_MEMORY(gameInitializeMemory)
{
  memory.rgroup.codepoints       = codepoints;
  memory.rgroup.monospaced_width = pixel_to_clip_x * (f32)codepoints[(u8)'a'].width;
  auto &state = *pushStruct(memory.arena, GameState);
  initGameState(memory, state);
}

inline void
moveTreeHorizontally(GameState &state, i32 dx)
{
  auto hi = state.hot_item;

  if (dx > 0) {
    for (i32 i=0; i < dx; i++) {
      if (hi->next_sibling) {
        hi = hi->next_sibling;
      } else {
        break;
      }
    }
  } else {
    for (i32 i=0; i < -dx; i++) {
      if (hi->prev_sibling) {
        hi = hi->prev_sibling;
      } else {
        break;
      }
    }
  }
  
  state.hot_item = hi;
}

inline void
moveCursorHorizontally(GameState &state, i32 dx)
{
  i32 &coord = state.cursor_coord;
  coord += dx;
  if (coord <= 0) {
    // clamp cursor to left side of the screen
    coord = 0;
    state.cursor_tile_offset = maximum(state.cursor_tile_offset, 0);
  }
}

extern "C" GAME_UPDATE_AND_RENDER(gameUpdateAndRender)
{
  GameState &state = *(GameState *)memory.arena.base;
  Arena &temp_arena  = memory.arena;
  auto   temp_marker = beginTemporaryMemory(temp_arena);

  auto &rgroup    = memory.rgroup;
  rgroup.commands = subArena(temp_arena, megaBytes(8));
  rgroup.temp     = &temp_arena;

  // Game logic //////////////////////////////////////
  f32 &velocity       = state.velocity;
  i32 &absolute_coord = state.cursor_coord;
  f32 &dt             = target_frame_time_sec;

  b32 &cursor_mode = state.cursor_mode;
  if (memory.key_states[kVK_Tab].is_down && memory.new_key_press) {
    cursor_mode = !cursor_mode;
  }

  f32 direction_x = 0;
  if (memory.key_states[kVK_ANSI_L].is_down) {
    direction_x = 1;
  } else if (memory.key_states[kVK_ANSI_H].is_down) {
    direction_x = -1;
  }

  f32 acceleration_abs = state.cursor_mode ? 320.f : 40.f;

  // abstract tiled movement update
  f32 &tile_offset = cursor_mode ? state.cursor_tile_offset : state.tree_tile_offset;
  if (direction_x != 0) {
    if (memory.new_key_press) {
      velocity = 0;
      tile_offset += direction_x;
    } else {
      // unit of movement: object
      f32 a = direction_x * acceleration_abs;
      tile_offset += velocity * dt + 0.5f * a * dt * dt;
      velocity    += a * dt;
    }
  } else {
    velocity    = 0.f;
    tile_offset = 0.f;
  }

  i32 tile_offset_rounded = (tile_offset == -0.5f) ? 0 : roundF32ToI32(tile_offset);
  tile_offset -= (f32)tile_offset_rounded;

  // update game state according to movement
  if (state.cursor_mode) {
    moveCursorHorizontally(state, tile_offset_rounded);
  } else {// tree navigation
    moveTreeHorizontally(state, tile_offset_rounded);
  }

  ///////////////////////////////////////////////////////

  // Draw stuff ////////////////////////////////////

  // draw backdrop
  pushRect(rgroup, Rect2{-1.f, -1.f, 1.f, 1.f}, V3{0,0,0});

  {// draw cursor
    i32 screen_width_in_tiles = 80;
    i32 cursor_x = absolute_coord % screen_width_in_tiles;
    i32 cursor_y = absolute_coord / screen_width_in_tiles;
    f32 tile_width = 2.f / (f32)screen_width_in_tiles;
    V2 tile_dim = {tile_width, tile_width};
    V2 min = {-1.f + ((f32)cursor_x + state.cursor_tile_offset) * tile_dim.x,
              +1.f - (f32)(cursor_y+1) * tile_dim.y};
    auto rect = rectMinDim(min, tile_dim);
    pushRect(rgroup, rect, V3{1,0,0});
  }
  
  {// Draw the tree
    V2 min = {-0.9f, -0.1f};
    V2 dim = drawTree(state, rgroup, state.grandma, min);
  }

  // draw debug text
  DebugDrawer debug_drawer = {.arena=temp_arena, .rgroup=rgroup};
  pushDebugText(debug_drawer, "frame time: %.3f ms", memory.last_frame_time_sec * 1000);
  pushDebugText(debug_drawer, "cursor velocity: %.3f tiles/s", velocity);

  endTemporaryMemory(temp_marker);
}
