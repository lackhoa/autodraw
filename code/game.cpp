/*
  todo:
  - Clean up our coordinate system: our dimensions are super effed
  - Tell lldb to please not block the stdin when running the game
  - Smaller font please!
  - Controls for the camera
  - lldb print out the v2,v3, etc.
  - Resolution independence
  - change EAT_TYPE to return a pointer
  - Camera control
  - Sorting
  - Draw some *sick* ray-tracing to the left
  - Z-sorting for UI elements
  - Try my luck with the new llvm, see if I can fix the stupid macro bugs (or maybe that's)

  Rules for the renderer:
  - When you push render entries, dimensions are specified in "meter"
  - Textures and bitmaps are srgb premultiplied-alpha
  - v4 colors are in linear space, alpha=1 (so pma doesn't matter)
  - Packed colors are rgba in memory order (i.e abgr in u32 register order)
  pma = pre-multiplied alpha

 */

#include "kv-utils.h"
#include "platform.h"
#include "shader-interface.h"
#include "kv-bitmap.h"
#include "ray.h"
#include <math.h>  // sin

inline simd_float2
toFloat2(v2 v)
{
  return {v.x, v.y};
}

typedef u32 TreeID;
struct EditorTree {
  TreeID id;
  String name;

  EditorTree *children;
  EditorTree *parent;
  EditorTree *next_sibling;
  EditorTree *prev_sibling;
};

struct RenderGroup {
  Arena      commands;
  Arena     *temp;  // TODO: have its own arena
  Codepoint *codepoints;
  i32        monospaced_width;
  // We won't specific meter_to_pixel here, it is up to the thing that does the rendering
};

enum RenderEntryType {
  // todo: Is there a fast way to clear the buffer in metal?
  RenderEntryTypeRectangle,
  RenderEntryTypeQuad,
};

struct RenderEntryHeader {
    RenderEntryType type;
};

struct RenderEntryRectangle {
  rect2     rect;
  TextureId texture;
  v4        color;
};

struct RenderEntryQuad {
  v2        x0y0;
  v2        x0y1;
  v2        x1y0;
  v2        x1y1;
  TextureId texture;
  v4        color;
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
pushRectOutline(RenderGroup &rgroup, rect2 outline, f32 thickness, v4 color)
{
  assert(thickness_px >= 2);  // our outlines are divided in half
  v2 min = outline.min;
  v2 max = outline.max;
  v2 t = 0.5f * v2{thickness, thickness};
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

internal void
pushLine(RenderGroup &rgroup, v2 p0, v2 p1, f32 thickness, v4 color)
{
  // TODO: thickness doesn't work with our current coordinate system ya'll
  RenderEntryQuad &entry = *pushRenderEntry(rgroup, Quad);
  assert(thickness_px >= 2);
  v2 d = noz(p1 - p0);
  v2 perp = 0.5f * thickness * v2{-d.y, d.x};
  // NOTE: gotta draw a "z" here for triangle strip
  entry.x0y0 = p0 - perp;
  entry.x0y1 = p0 + perp;
  entry.x1y0 = p1 - perp;
  entry.x1y1 = p1 + perp;
  entry.texture = TextureIdWhite;
  entry.color   = color;
}

struct DebugDrawer {
  Arena        arena;
  RenderGroup *rgroup;
  v2           at;
};

global_variable DebugDrawer debug_drawer;

global_variable f32 TODO_text_scale = 0.2f;  // this should make the text tiny, but it isn't

inline f32
pushLetter(RenderGroup &rgroup, v2 min, char character, v4 color)
{
  assert(33 <= character && character <= 126);
  auto codepoint = rgroup.codepoints[(u8)character];
  auto dim = TODO_text_scale * v2{(f32)codepoint.width, (f32)codepoint.height};
  pushRect(rgroup, rectMinDim(min, dim), codepointTexture(character), color);
  return dim.y;
}

internal f32
pushText(RenderGroup &rgroup, v2 min, String string, v4 color)
{
  f32 max_dim_y = 0;
  auto x = min.x;
  for (i32 i=0; i < string.length; i++) {
    char c = string.chars[i];
    if (c != ' ') {
      f32 dim_y = pushLetter(rgroup, {x, min.y}, c, color);
      max_dim_y = maximum(max_dim_y, dim_y);
    }
    x += TODO_text_scale * (f32)rgroup.monospaced_width;
  }
  return min.y + max_dim_y;
}

inline f32
pushTextFormat(RenderGroup &rgroup, v2 min, v4 color, char *format, ...)
{
  va_list args;
  va_start(args, format);
  auto string = printVA(*rgroup.temp, format, args);
  va_end(args);
  return pushText(rgroup, min, string, color);
}

inline void
pushDebugText(char *format, ...)
{
  auto &d = debug_drawer;
  va_list args;
  va_start(args, format);
  auto string = printVA(debug_drawer.arena, format, args);
  v4 debug_text_color = {.5, .5, .5, 1};
  pushText(*d.rgroup, d.at, string, debug_text_color);
  d.at.y += TODO_text_scale * font_height_px;
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
  f32 revolution;
};

v4 warm_yellow = v4{.5, .5, .3, 1};

internal rect2
drawTree(GameState &state, RenderGroup &rgroup, EditorTree &tree, v2 min)
{
  f32 margin      = 5;
  f32 spacing     = 20;
  f32 indentation = 20;
  f32 data_dim_x  = TODO_text_scale * (f32)rgroup.monospaced_width * (f32)tree.name.length;
  v2 max = {};
  max.y = pushText(rgroup, min, tree.name, warm_yellow);
  max.x = min.x + data_dim_x;

  EditorTree *childp = tree.children;
  while (childp) {
    auto &child = *childp;
    v2 child_min = {min.x + indentation, min.y - TODO_text_scale * font_height_px};  // this is just the input, the actual min is different
    auto crect = drawTree(state, rgroup, child, child_min);

    // todo use rect union
    if (crect.max.x > max.x) max.x = crect.max.x;
    min.y = crect.min.y - margin;

    childp = child.next_sibling;
  }

  max.y += margin;

  if (tree.id == state.hot_item->id) {
    f32 outline_thickness = 2;
    pushRectOutline(rgroup, rect2{min, max}, outline_thickness, warm_yellow);
  }

  return rect2{min, max};
}

inline void
moveTree(GameState &state, i32 dx, i32 dy)
{
  auto hi = state.hot_item;
  assert(dx == 0 || dy == 0);
  i32 iterations = maximum(absoslute(dx), absoslute(dy));
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

struct RaycastOutput {
  b32 hit;
  v3  p;
};

internal RaycastOutput
raycastPlane(v3 ray_origin, v3 ray_direction, v3 plane_N, f32 plane_d)
{
  RaycastOutput out = {};
  f32 denom = dot(plane_N, ray_direction);
  if (absolute(denom) >= .0001f) {
    f32 t = -(plane_d + dot(plane_N, ray_origin)) / denom;
    if (t >= 0) {
      out.hit = true;
      out.p = ray_origin + t*ray_direction;
    }
  }
  return out;
}

internal RaycastOutput
screencast(v3 ray_origin, v3 ray_direction, v3 plane_N, f32 plane_d)
{
  RaycastOutput out = {};
  f32 denom = dot(plane_N, ray_direction);
  if (absolute(denom) >= .0001f) {
    f32 t = -(plane_d + dot(plane_N, ray_origin)) / denom;
    if (t >= 0) {
      out.p = ray_origin + t*ray_direction;
    }
  }
  return out;
}

internal v3
raycast(World &world, v3 ray_origin, v3 ray_direction)
{
  i32 mat_index = 0;
  f32 min_t = FLT_MAX;
  for (i32 plane_index=0; plane_index < world.plane_count; plane_index++) {
    Plane plane = world.planes[plane_index];
    f32 denom = dot(plane.N, ray_direction);
    if (absolute(denom) >= .0001f) {
      f32 t = -(plane.d + dot(plane.N, ray_origin)) / denom;
      if ((t >= 0) && (t < min_t)) {
        min_t = t;
        mat_index = plane.mat_index;
      }
    }
  }
  return world.materials[mat_index].color;
}

internal void
pushLine(RenderGroup rgroup, v3 p0, v3 p1, f32 thickness_px, v4 color)
{
  // v2 p0_screen = eye, p0;
  rect2 rect = {};
  pushRect(rgroup, rect, color);
}

struct Screen {
  v3 center;
  v3 x;
  v3 y;
};

inline v2
screenProject(Screen screen, v3 p)
{
  v3 d = p-screen.center;
  return v2{dot(screen.x, d),
            dot(screen.y, d)};
}

extern "C" GAME_UPDATE_AND_RENDER(gameUpdateAndRender)
{
  b32 hot_reloaded = input.hot_reloaded;
  GameState &state = *(GameState *)input.arena.base;

  Arena &temp_arena  = state.temp_arena;
  auto   temp_marker = beginTemporaryMemory(temp_arena);
  defer(endTemporaryMemory(temp_marker));

  v2 &screen_dim      = input.screen_dim;
  v2  screen_half_dim = 0.5f * screen_dim;
  v2 pixel_to_clip = {1.f / screen_half_dim.x,
                      1.f / screen_half_dim.y};

  auto grandma = &state.editor_root;
  auto slider  = pushStruct(temp_arena, EditorTree);
  auto uncle   = pushStruct(temp_arena, EditorTree);
  auto mom     = pushStruct(temp_arena, EditorTree);

  TreeID SLIDER_ID=3;
  *grandma = {.id=0, .name=toString(temp_arena, "grandma"), .children=uncle};
  *uncle   = {.id=1, .name=toString(temp_arena, "uncle"), .parent=grandma, .next_sibling=mom};
  *mom     = {.id=2, .name=toString(temp_arena, "mom"), .children=slider, .parent=grandma, .prev_sibling=uncle};
  *slider  = {.id=SLIDER_ID, .name=toString(temp_arena, "slider"), .parent=mom};

  if (!state.hot_item) state.hot_item = uncle;

  RenderGroup rgroup = {};
  rgroup.codepoints       = state.codepoints;
  rgroup.monospaced_width = (f32)state.codepoints[(u8)'a'].width;
  rgroup.commands         = subArena(temp_arena, megaBytes(8));
  rgroup.temp             = &temp_arena;

  debug_drawer.arena  = subArena(temp_arena, kiloBytes(128));
  debug_drawer.rgroup = &rgroup; // todo have separate render group maybe?
  debug_drawer.at     = -screen_half_dim + v2{10, 10};

  // Game logic //////////////////////////////////////
  f32 &speed          = state.speed;
  i32 &absolute_coord = state.cursor_coord;
  f32 &dt             = target_frame_time_sec;
  f32 &r              = state.revolution;
  r += Tau32 * dt;
  if (r > Tau32) {
    r -= Tau32;
  }

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

  Bitmap raytracing_bitmap = {};
  v3 eye_p_base =  {-1, 1, 3};
  v3 eye_p = eye_p_base + v3{0 * sin(r),
                             0 * sin(r),
                             0 * sin(r)};
  v3 eye_z = noz(eye_p_base);  // NOTE: z comes at you
  v3 eye_x = cross(eye_z, v3{0,0,1});
  v3 eye_y = cross(eye_z, eye_x);

  f32 d_eye_screen = 1920;  // NOTE: it's just fudge
  v3 screen_center = eye_p - d_eye_screen * eye_z;

  {// Ray tracing
    World world = {};
    Material materials[3];
    materials[0] = {.color=v3{0,0,0}};
    materials[1] = {.color=v3{.1,0,0}};
    materials[2] = {.color=v3{0,.1,.1}};
    world.materials = materials;

    Plane planes[2] = {{.N=v3{1,1,1}, .d=0, .mat_index=1},
                       {.N=v3{1,0,1}, .d=0, .mat_index=2}};
    world.planes = planes;

    world.plane_count    = arrayCount(planes);
    world.material_count = arrayCount(materials);

    // rgba in memory order
    raytracing_bitmap.dimx = 512;
    raytracing_bitmap.dimy = 512;
    auto dimx = raytracing_bitmap.dimx;
    auto dimy = raytracing_bitmap.dimy;
    raytracing_bitmap.pitch = 4*dimx;

    u32 *bitmap = pushArray(temp_arena, dimx*dimy, u32);
    u32 *dst = bitmap;
    f32 screen_half_dim_x = 1.f;
    f32 screen_half_dim_y = 1.f;
    for (i32 y=0; y < dimy; y++) {
      // screen_x and screen_y ranges from -1 to +1
      f32 screen_y = -1.f + 2.f * (f32)y / (f32)dimy;

      for (i32 x=0; x < dimx; x++) {
        f32 screen_x = -1.f + 2.f * (f32)x / (f32)dimx;

        v3 screen_p = screen_center + (screen_half_dim_x * screen_x * eye_x +
                                       screen_half_dim_y * screen_y * eye_y);
        v3 ray_origin = eye_p;
        v3 ray_dir    = screen_p - eye_p;
        v3 color = raycast(world, ray_origin, ray_dir);

        *dst++ = pack_sRGBA(toV4(color, 1));
      }
    }
    raytracing_bitmap.memory = bitmap;
  }

  // Render: Build the push buffer ////////////////////////////////////

  // push backdrop
  pushRect(rgroup, rect2{-screen_half_dim, screen_half_dim}, v4{0,0,0,1});

  {// push the editor tree
    v2 min = hadamard(screen_half_dim, {0.5f, -0.0f});
    drawTree(state, rgroup, state.editor_root, min);
  }

  if (state.focused_on_slider)
  {// push cursor
    i32 screen_width_in_tiles = 80;
    i32 cursor_x = absolute_coord % screen_width_in_tiles;
    i32 cursor_y = absolute_coord / screen_width_in_tiles;
    f32 tile_width = 2.f / (f32)screen_width_in_tiles;
    v2 tile_dim = {tile_width, tile_width};
    v2 min = {0 + ((f32)cursor_x + state.cursor_tile_offset.x) * tile_dim.x,
              1 - (f32)(cursor_y+1) * tile_dim.y};
    auto rect = rectMinDim(min, tile_dim);
    pushRect(rgroup, rect, v4{1,0,0,1});
  }

  {// push ray tracing
    // push the canvas rect ///////////////////////////////////////////////
    v2 min = {-500, -250};  // todo don't like this fudgy coordinates, we need a layout system
    v2 max = min + v2i(raytracing_bitmap.dimx, raytracing_bitmap.dimy);
    pushRect(rgroup, rect2{min, max}, TextureIdRayTrace);
    pushRectOutline(rgroup, rect2{min, max}, .04f, warm_yellow);

    // push coordinate system ///////////////////////////////////////////////
    v3 p0 = v3{0,0,0};
    v3 px = v3{1,0,0};
    v3 py = v3{0,1,0};
    v3 pz = v3{0,0,1};

    v3  &screen_N = eye_z;
    f32  screen_d = -dot(eye_p, eye_z) + d_eye_screen;

    auto p0_hit = raycastPlane(eye_p, p0 - eye_p, screen_N, screen_d);
    auto px_hit = raycastPlane(eye_p, px - eye_p, screen_N, screen_d);
    auto py_hit = raycastPlane(eye_p, py - eye_p, screen_N, screen_d);
    auto pz_hit = raycastPlane(eye_p, pz - eye_p, screen_N, screen_d);

    if (p0_hit.hit && px_hit.hit && py_hit.hit && pz_hit.hit) {
      Screen screen = {screen_center, eye_x, eye_y};
      v2 p0_screen = screenProject(screen, p0_hit.p);
      // TODO: inspect p0_screen
      v2 px_screen = screenProject(screen, px_hit.p);
      v2 py_screen = screenProject(screen, py_hit.p);
      v2 pz_screen = screenProject(screen, pz_hit.p);
      pushLine(rgroup, p0_screen, px_screen, 2.f, {.2,  0,  0, 1});
      pushLine(rgroup, p0_screen, py_screen, 2.f, { 0, .2,  0, 1});
      pushLine(rgroup, p0_screen, pz_screen, 2.f, { 0,  0, .2, 1});

      v3 px = px_hit.p;
      pushDebugText("eye_x: %.1f, %.1f, %.1f", eye_x.x, eye_x.y, eye_x.z);
      pushDebugText("eye_y: %.1f, %.1f, %.1f", eye_y.x, eye_y.y, eye_y.z);
    }
  }

  {// push debug text
    pushDebugText("frame time: %.3f ms", input.last_frame_time_sec * 1000);
  }

  // Render: Create GPU commands //////////////////////////////////////////

  GPUCommands gcommands = {};
  gcommands.commands      = subArena(temp_arena, megaBytes(8));
  gcommands.vertex_buffer = subArena(temp_arena, megaBytes(8));

  u8 *next = rgroup.commands.base;
  u8 *end  = rgroup.commands.base + rgroup.commands.used;
  while (next != end) {
    auto &header = *EAT_TYPE(next, RenderEntryHeader);

    switch (header.type) {
      case RenderEntryTypeRectangle: {
        auto &entry = *EAT_TYPE(next, RenderEntryRectangle);

        auto c0 = entry.color;
        if (c0.r == 0 && c0.g == 0 && c0.b == 0 && c0.a == 1) {
          breakhere;
        }

        i32 vertex_count = 4;
        auto out = pushVertices(gcommands, vertex_count);
        auto c = entry.color;
        auto cs = simd_float4{c.r, c.g, c.b, c.a};
        auto min = hadamard(pixel_to_clip, entry.rect.min);
        auto max = hadamard(pixel_to_clip, entry.rect.max);

        // TODO: excuse me but why are these bitmaps flipped?
        out.vertices[0] = {{min.x, min.y}, {0.f, 1.f}, cs};
        out.vertices[1] = {{min.x, max.y}, {0.f, 0.f}, cs};
        out.vertices[2] = {{max.x, min.y}, {1.f, 1.f}, cs};
        out.vertices[3] = {{max.x, max.y}, {1.f, 0.f}, cs};

        auto &command = *pushGPUCommand(gcommands, TriangleStrip);
        command.vertex_start = out.vertex_start;
        command.vertex_count = vertex_count;
        command.texture      = entry.texture;

        break;
      }

        // TODO cutnpaste
      case RenderEntryTypeQuad: {
        auto &entry = *EAT_TYPE(next, RenderEntryQuad);

        i32 vertex_count = 4;
        auto out = pushVertices(gcommands, vertex_count);
        auto c = entry.color;
        auto cs = simd_float4{c.r, c.g, c.b, c.a};

        out.vertices[0] = {toFloat2(hadamard(pixel_to_clip, entry.x0y0)), {0.f, 1.f}, cs};
        out.vertices[1] = {toFloat2(hadamard(pixel_to_clip, entry.x0y1)), {0.f, 0.f}, cs};
        out.vertices[2] = {toFloat2(hadamard(pixel_to_clip, entry.x1y0)), {1.f, 1.f}, cs};
        out.vertices[3] = {toFloat2(hadamard(pixel_to_clip, entry.x1y1)), {1.f, 0.f}, cs};

        auto &command = *pushGPUCommand(gcommands, TriangleStrip);
        command.vertex_start = out.vertex_start;
        command.vertex_count = vertex_count;
        command.texture      = entry.texture;

        break;
      }

        invalidDefaultCase;
    }
  }
  return GameOutput{.gcommands=gcommands, .raytracing_bitmap=raytracing_bitmap};
}
