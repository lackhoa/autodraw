/*
  todo:
  - Draw some *sick* ray-tracing to the left
  - Why are we using 100% CPU?
  - Try my luck with the new llvm, see if I can fix the stupid macro bugs (or maybe that's)

  Rules for the renderer:
  - Textures and bitmaps are srgb premultiplied-alpha
  - Colors are in linear range, alpha=1 (so pma doesn't matter)
  - Packed colors are rgba in memory order (i.e abgr in u32 register order)
  pma = pre-multiplied alpha

 */

#include "kv-utils.h"
#include "platform.h"
#include "shader-interface.h"
#include "kv-bitmap.h"
#include "ray.h"

typedef u32 TreeID;
struct EditorTree {
  TreeID id;
  String data;

  EditorTree *children;
  EditorTree *parent;
  EditorTree *next_sibling;
  EditorTree *prev_sibling;
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
pushRect(RenderGroup &rgroup, rect2 rect, TextureId texture, v4 color)
{
  RenderEntryRectangle &entry = *pushRenderEntry(rgroup, Rectangle);
  entry.rect    = rect;
  entry.texture = texture;
  entry.color   = color;
}

inline void
pushRect(RenderGroup &rgroup, rect2 rect, v4 color)
{
  pushRect(rgroup, rect, TextureIdWhite, color);
}

inline void
pushRect(RenderGroup &rgroup, rect2 rect, TextureId texture)
{
  pushRect(rgroup, rect, texture, v4{1,1,1,1});
}

internal void
pushRectOutline(RenderGroup &rgroup, rect2 outline, i32 thickness_px, v4 color)
{
  assert(thickness_px >= 2);  // our outlines are divided in half
  v2 min = outline.min;
  v2 max = outline.max;
  v2 t = 0.5f * v2{pixel_to_clip_x * (f32)thickness_px,
                   pixel_to_clip_y * (f32)thickness_px};
  v2 dimx = {max.x - min.x, 0};
  v2 dimy = {0, max.y - min.y};
  // left
  pushRect(rgroup, rect2{min-t, min+dimy+t}, color);
  // right
  pushRect(rgroup, rect2{min+dimx-t, max+t}, color);
  // bottom
  pushRect(rgroup, rect2{min-t, min+dimx+t}, color);
  // top
  pushRect(rgroup, rect2{min+dimy-t, max+t}, color);
}

struct DebugDrawer {
  Arena       &arena;
  RenderGroup &rgroup;
  f32          y = -1.f;
};

inline f32
pushLetter(RenderGroup &rgroup, v2 min, char character)
{
  assert(33 <= character && character <= 126);
  auto codepoint = rgroup.codepoints[(u8)character];
  auto dim = v2{rgroup.monospaced_width,
                pixel_to_clip_y * (f32)codepoint.height};
  pushRect(rgroup, rectMinDim(min, dim), codepointTexture(character));
  return dim.y;
}

internal f32
pushText(RenderGroup &rgroup, v2 min, String string)
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
pushTextFormat(RenderGroup &rgroup, v2 min, char *format, ...)
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

inline void *
pushGPUCommand_(GPUCommands &gcommands, u32 size, GPUCommandType type)
{
  GPUCommandHeader *header = pushStruct(gcommands.commands, GPUCommandHeader);
  header->type = type;
  void *command = pushSize(gcommands.commands, size);
  return command;
}

#define pushGPUCommand(commands, type) (GPUCommand##type *) pushGPUCommand_(commands, sizeof(GPUCommand##type), GPUCommandType##type)

struct GameState {
  Arena perm_arena;
  Arena temp_arena;
  Codepoint *codepoints;

  b32 focused_on_slider;

  i32 cursor_coord;
  EditorTree  editor_root;
  EditorTree *hot_item;

  f32 direction_key_held_down_time;
  f32 speed;
  v2  cursor_tile_offset;
  v2  tree_tile_offset;

  PlatformCode platform;
};

internal rect2
drawTree(GameState &state, RenderGroup &rgroup, EditorTree &tree, v2 min)
{
  // TODO: try switching to pixel-based coordinates computation
  f32 margin      = pixel_to_clip_x * 5;
  f32 spacing     = pixel_to_clip_x * 20;
  f32 indentation = pixel_to_clip_x * 40;
  f32 data_dim_x  = rgroup.monospaced_width * (f32)tree.data.length;
  f32 line_height = pixel_to_clip_x * 128.f;  // todo: not a real height
  v2 max = {};
  max.y = pushText(rgroup, min, tree.data);
  max.x = min.x + data_dim_x;

  EditorTree *childp = tree.children;
  while (childp) {
    auto &child = *childp;
    v2 child_min = {min.x + indentation, min.y - line_height};  // this is just the input, the actual min is different
    auto crect = drawTree(state, rgroup, child, child_min);

    // TODO use rect union
    if (crect.max.x > max.x) max.x = crect.max.x;
    min.y = crect.min.y - margin;

    childp = child.next_sibling;
  }

  max.y += margin;

  if (tree.id == state.hot_item->id) {
    i32 outline_thickness = 2;
    v4 color = {1,0,0,1};
    pushRectOutline(rgroup, rect2{min, max}, outline_thickness, color);
  }

  return rect2{min, max};
}

inline void
moveTree(GameState &state, i32 dx, i32 dy)
{
  auto hi = state.hot_item;
  assert(dx == 0 || dy == 0);
  i32 iterations = maximum(abs(dx), abs(dy));
  if (dy != 0) {
    breakhere;
  }

  for (i32 i=0; i < iterations; i++) {
    EditorTree *next = 0;
    if (dx > 0) next = hi->next_sibling;
    else if (dx < 0) next = hi->prev_sibling;
    else if (dy > 0) next = hi->parent;
    else if (dy < 0) next = hi->children;

    if (next) hi = next;
    else break;
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
    if (state.cursor_tile_offset.x < 0) state.cursor_tile_offset.x = 0;
  }
}

struct PushVerticesOutput {
  VertexInput *vertices;
  i32          vertex_start;
};

inline PushVerticesOutput
pushVertices(GPUCommands &gcommands, i32 count)
{
  VertexInput *verts = pushArray(gcommands.vertex_buffer, count, VertexInput);
  i32 vstart = gcommands.vertex_start;
  gcommands.vertex_start += count;
  return {verts, vstart};
}

extern "C" GAME_INITIALIZE(gameInitialize)
{
  auto &state = *pushStruct(arena, GameState);
  state.perm_arena = subArena(arena, megaBytes(512));
  state.temp_arena = subArenaWithRemainingMemory(arena);
  state.codepoints = codepoints;
  state.platform   = *code;
}

extern "C" GAME_UPDATE_AND_RENDER(gameUpdateAndRender)
{
  b32 hot_reloaded = input.hot_reloaded;
  GameState &state = *(GameState *)input.arena.base;

  Arena &temp_arena  = state.temp_arena;
  auto   temp_marker = beginTemporaryMemory(temp_arena);
  defer(endTemporaryMemory(temp_marker));

  auto grandma = &state.editor_root;
  auto slider  = pushStruct(temp_arena, EditorTree);
  auto uncle   = pushStruct(temp_arena, EditorTree);
  auto mom     = pushStruct(temp_arena, EditorTree);

  TreeID SLIDER_ID=3;
  *grandma = {.id=0, .data=toString(temp_arena, "grandma"), .children=uncle};
  *uncle   = {.id=1, .data=toString(temp_arena, "uncle"), .parent=grandma, .next_sibling=mom};
  *mom     = {.id=2, .data=toString(temp_arena, "mom"), .children=slider, .parent=grandma, .prev_sibling=uncle};
  *slider  = {.id=SLIDER_ID, .data=toString(temp_arena, "slider"), .parent=mom};

  if (!state.hot_item) state.hot_item = uncle;

  RenderGroup rgroup = {};
  rgroup.codepoints       = state.codepoints;
  rgroup.monospaced_width = pixel_to_clip_x * (f32)state.codepoints[(u8)'a'].width;
  rgroup.commands         = subArena(temp_arena, megaBytes(8));
  rgroup.temp             = &temp_arena;

  // Game logic //////////////////////////////////////
  f32 &speed          = state.speed;
  i32 &absolute_coord = state.cursor_coord;
  f32 &dt             = target_frame_time_sec;

  b32 &focused_on_slider = state.focused_on_slider;
  if (input.key_states[kVK_ANSI_E].is_down) {
    if (state.hot_item->id == SLIDER_ID) {
      focused_on_slider = true;
    }
  }

  if (input.key_states[kVK_Escape].is_down) {
    focused_on_slider = false;
  }

  v2 direction = {};
  if (input.key_states[kVK_ANSI_L].is_down) {
    direction.x = 1;
  } else if (input.key_states[kVK_ANSI_H].is_down) {
    direction.x = -1;
  } else if (input.key_states[kVK_ANSI_K].is_down) {
    direction.y = 1;
  } else if (input.key_states[kVK_ANSI_J].is_down) {
    direction.y = -1;
  }

  f32 acceleration_magnitude = state.focused_on_slider ? 80.f : 40.f;

  // abstract tiled movement update
  v2 &tile_offset = focused_on_slider ? state.cursor_tile_offset : state.tree_tile_offset;
  f32 &held_time = state.direction_key_held_down_time;
  if (direction.x != 0 || direction.y != 0) {
    if (held_time == 0.f) {
      tile_offset += direction;
    } else if (held_time < 0.25f) {
      // hold right there!
    } else {
      auto a = acceleration_magnitude;
      tile_offset += direction * (speed * dt + 0.5f * a * dt * dt);
      speed       += a * dt;
    }
    held_time += dt;
  } else {
    speed       = 0.f;
    tile_offset = {};
    held_time   = 0.f;
  }

  i32 tile_offset_int_x = (tile_offset.x == -0.5f) ? 0 : (i32)roundF32(tile_offset.x);
  i32 tile_offset_int_y = (tile_offset.y == -0.5f) ? 0 : (i32)roundF32(tile_offset.y);
  tile_offset.x -= (f32)tile_offset_int_x;
  tile_offset.y -= (f32)tile_offset_int_y;

  // update game state according to movement
  if (state.focused_on_slider) {
    moveCursorHorizontally(state, tile_offset_int_x);
  } else {// tree navigation
    moveTree(state, tile_offset_int_x, tile_offset_int_y);
  }

  if (hot_reloaded)
  {// Ray tracing
    World world = {};
    world.plane_count    = 1;
    world.material_count = 2;

    Material materials[2];
    materials[0] = {.color=v3{0,0,0}};
    materials[1] = {.color=v3{1,1,1}};
    world.materials = materials;

    Plane planes[1] = {{.N=v3{1,1,1}, .d=1, .mat_index=1}};
    world.planes = planes;

    // let's fill a 512x512 rectangle with gray
    // rgba in memory order
    i32 dimx = 1;
    i32 dimy = 1;
    u32 *bitmap = pushArray(temp_arena, dimx*dimy, u32);
    u32 *dst = bitmap;
    for (i32 y=0; y < dimy; y++) {
      for (i32 x=0; x < dimx; x++) {
        v4 color = {.5, .5, .5, 1};
        *dst++ = pack_sRGBA(color);
      }
    }
    state.platform.uploadRayTracingBitmap(bitmap, dimx, dimy);
  }

  // Render: Build the push buffer ////////////////////////////////////

  // draw backdrop
  pushRect(rgroup, rect2{-1.f, -1.f, 1.f, 1.f}, v4{0,0,0,1});

  {// draw cursor
    i32 screen_width_in_tiles = 80;
    i32 cursor_x = absolute_coord % screen_width_in_tiles;
    i32 cursor_y = absolute_coord / screen_width_in_tiles;
    f32 tile_width = 2.f / (f32)screen_width_in_tiles;
    v2 tile_dim = {tile_width, tile_width};
    v2 min = {-1.f + ((f32)cursor_x + state.cursor_tile_offset.x) * tile_dim.x,
              +1.f - (f32)(cursor_y+1) * tile_dim.y};
    auto rect = rectMinDim(min, tile_dim);
    pushRect(rgroup, rect, v4{1,0,0});
  }
  
  {// Draw the editor tree
    v2 min = {0.5f, -0.12f};
    drawTree(state, rgroup, state.editor_root, min);
  }

  // draw debug text
  DebugDrawer debug_drawer = {.arena=temp_arena, .rgroup=rgroup};
  pushDebugText(debug_drawer, "frame time: %.3f ms", input.last_frame_time_sec * 1000);
  pushDebugText(debug_drawer, "cursor speed: %.3f tiles/s", speed);

  // Render: Create GPU commands ///////////////////////

  GPUCommands gcommands = {};
  gcommands.commands      = subArena(temp_arena, megaBytes(8));
  gcommands.vertex_buffer = subArena(temp_arena, megaBytes(8));

  u8 *next = rgroup.commands.base;
  u8 *end  = rgroup.commands.base + rgroup.commands.used;
  while (next != end) {
    auto &header = EAT_TYPE(next, RenderEntryHeader);

    switch (header.type) {
      case RenderEntryTypeRectangle: {
        auto &entry = EAT_TYPE(next, RenderEntryRectangle);

        auto out = pushVertices(gcommands, 6);
        auto c = entry.color;
        auto cs = simd_float4{c.r, c.g, c.b, c.a};
        auto &rect = entry.rect;
        auto min_x = rect.min.x;
        auto max_x = rect.max.x;
        auto min_y = rect.min.y;
        auto max_y = rect.max.y;

        out.vertices[0] = {{min_x, max_y}, {0.f, 0.f}, cs};
        out.vertices[1] = {{min_x, min_y}, {0.f, 1.f}, cs};
        out.vertices[2] = {{max_x, min_y}, {1.f, 1.f}, cs};
        out.vertices[3] = {{min_x, max_y}, {0.f, 0.f}, cs};
        out.vertices[4] = {{max_x, min_y}, {1.f, 1.f}, cs};
        out.vertices[5] = {{max_x, max_y}, {1.f, 0.f}, cs};

        auto &command = *pushGPUCommand(gcommands, Triangle);
        command.vertex_start = out.vertex_start;
        command.vertex_count = 6;
        command.texture      = entry.texture;

        break;
      }

        invalidDefaultCase;
    }
  }
  return GameOutput{gcommands};
}
