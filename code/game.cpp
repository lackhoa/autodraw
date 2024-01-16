/*
  Rules for the renderer:
  - When you push render entries, dimensions are specified in "meter"
  - Textures and bitmaps are srgb premultiplied-alpha
  - v4 colors are in linear space, alpha=1 (so pma doesn't matter)
  - Packed colors are rgba in memory order (i.e abgr in u32 register order)
  pma = pre-multiplied alpha

 */

#import <time.h>
#define KV_IMPLEMENTATION
#include "kv.h"
#include "platform.h"
#include "shader-interface.h"
#include "kv-bitmap.h"
#include "ray.h"

// global_variable Tokenizer init_tokenizer;

enum ZLevel {
  ZLevelBackdrop,
  ZLevelRaytrace,
  ZLevelGeneral,
  ZLevelDebug,

  ZLevelCount_,
};

inline simd_float2
toFloat2(v2 v)
{
  return {v.x, v.y};
}

typedef u32 TreeID;
TreeID mode_tree_id = 0;
struct EditorTree {
  TreeID id;
  String name;

  EditorTree *children;
  EditorTree *parent;
  EditorTree *next_sibling;
  EditorTree *prev_sibling;
};

struct SortEntry {
  i32    buffer_offset;
  ZLevel z_level;
};

struct RenderGroup {
  KvArena      commands;
  i32        z_bucket_count[ZLevelCount_];
  ZLevel     current_z_level;
  KvArena      arena;
  Codepoint *codepoints;
  i32        monospaced_width;
  // We won't specific meter_to_pixel here, it is up to the thing that does the rendering
};

enum RenderEntryType {
  // todo: Is there a fast way to clear the buffer in metal?
  RenderEntryTypeQuad,
};

struct RenderEntryHeader {
  RenderEntryType type;
  ZLevel          z_level;
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
  header->type    = type;
  header->z_level = rgroup.current_z_level;
  rgroup.z_bucket_count[rgroup.current_z_level]++;
  void *body = pushSize(rgroup.commands, size);

  // backward index
  SortEntry *sort_entry = pushStructBackward(rgroup.commands, SortEntry);
  sort_entry->buffer_offset = (u8 *)header - rgroup.commands.base;
  sort_entry->z_level       = rgroup.current_z_level;

  return body;
}

#define pushRenderEntry(rgroup, type) (RenderEntry##type *) pushRenderEntry_(rgroup, sizeof(RenderEntry##type), RenderEntryType##type)

void
pushRect(RenderGroup &rgroup, rect2 rect, TextureId texture, v4 color)
{
  RenderEntryQuad &entry = *pushRenderEntry(rgroup, Quad);
  entry.texture = texture;
  entry.color   = color;
  entry.x0y0 = rect.min;
  entry.x0y1 = {rect.min.x, rect.max.y};
  entry.x1y0 = {rect.max.x, rect.min.y};
  entry.x1y1 = rect.max;
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

void
pushRectOutline(RenderGroup &rgroup, rect2 outline, f32 thickness, v4 color)
{
  kv_soft_assert(thickness >= 2);
  // our outlines are divided in half
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

void
pushLine(RenderGroup &rgroup, v2 p0, v2 p1, f32 thickness, v4 color)
{
  RenderEntryQuad &entry = *pushRenderEntry(rgroup, Quad);
  kv_soft_assert(thickness >= 2);
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
  KvArena        arena;
  RenderGroup *rgroup;
  v2           at;
};

global_variable DebugDrawer debug_drawer;

global_variable f32 todo_text_scale = 0.2f;  // this should make the text tiny, but it isn't

inline f32
pushLetter(RenderGroup &rgroup, v2 min, char character, v4 color)
{
  kv_assert_defend(33 <= character && character <= 126, return 0;);
  auto codepoint = rgroup.codepoints[(u8)character];
  auto dim = todo_text_scale * v2{(f32)codepoint.width, (f32)codepoint.height};
  pushRect(rgroup, rectMinDim(min, dim), codepointTexture(character), color);
  return dim.y;
}

f32
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
    x += todo_text_scale * (f32)rgroup.monospaced_width;
  }
  return min.y + max_dim_y;
}

inline f32
pushTextFormat(RenderGroup &rgroup, v2 min, v4 color, char *format, ...)
{
  va_list args;
  va_start(args, format);
  auto string = printVA(rgroup.arena, format, args);
  va_end(args);
  return pushText(rgroup, min, string, color);
}

inline void
pushDebugText(char *format, ...)
{
  auto &d = debug_drawer;
  d.rgroup->current_z_level = ZLevelDebug;
  defer(d.rgroup->current_z_level = ZLevelGeneral;);
  va_list args;
  va_start(args, format);
  auto string = printVA(debug_drawer.arena, format, args);
  v4 debug_text_color = {.5, .5, .5, 1};
  pushText(*d.rgroup, d.at, string, debug_text_color);
  d.at.y += todo_text_scale * font_height_px;
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

struct OldScene {
  u32 magic_number;
  u64 version;
  v3  eye_position;
  b32 eye_inited;
};

// IMPORTANT: This struct is saved to a file, so don't touch it without bumping the version!
u64 current_scene_file_format_version = 1;
u32 scene_file_format_magic_number = *(u32 *)"scen";

struct Scene {
  u32 magic_number;
  u64 version;
  v3 eye_position;
  b32 eye_inited;
} __attribute__((packed));

struct GameState 
{
  KvArena arena;
  KvArena frame_arena;
  Codepoint *codepoints;

  PlatformCode platform;

  String data_path;
  String scene_filename;  // todo: rename to "scene_path"

  TreeID editing_item;

  i32 cursor_coord;
  EditorTree *first_tree;
  EditorTree *hot_item;

  i32   key_held_frames[kVK_Count];
  v3    cursor_tile_offset;
  v3    tree_tile_offset;
  Scene scene;
  b32   successfully_read_scene_file;

  f32 revolution;
};

v4 warm_yellow = v4{.5, .5, .3, 1};

f32 editor_margin      = 5;
f32 editor_indentation = 20;

rect2
drawSingleTree(GameState &state, RenderGroup &rgroup, EditorTree &tree, v2 min);

rect2
drawTreeAndSiblings(GameState &state, RenderGroup &rgroup, EditorTree *tree, v2 trees_min)
{
  v2 max = trees_min;
  while (tree) {
    v2 tree_min = {trees_min.x,
                   trees_min.y - todo_text_scale * font_height_px};
    rect2 rect = drawSingleTree(state, rgroup, *tree, tree_min);

    max.x = maximum(rect.max.x, max.x);
    trees_min.y = rect.min.y - editor_margin;

    tree = tree->next_sibling;
  }
  return rect2{trees_min, max};
}

rect2
drawSingleTree(GameState &state, RenderGroup &rgroup, EditorTree &tree, v2 tree_min)
{
  f32 data_dim_x  = todo_text_scale * (f32)rgroup.monospaced_width * (f32)tree.name.length;
  f32 data_max_y = pushText(rgroup, tree_min, tree.name, warm_yellow);

  v2 children_min = {tree_min.x + editor_indentation,
                     tree_min.y};
  rect2 children = drawTreeAndSiblings(state, rgroup, tree.children, children_min);
  v2 max = {maximum(tree_min.x + data_dim_x, children.max.x),
            data_max_y + editor_margin};
  tree_min.y = children.min.y - editor_margin;

  if (tree.id == state.hot_item->id) {
    v4 color = warm_yellow;
    if (state.editing_item) {
      color.r += .5f;
    }
    f32 outline_thickness = 2;
    pushRectOutline(rgroup, rect2{tree_min, max}, outline_thickness, color);
  }

  return rect2{tree_min, max};
}

inline void
moveTree(GameState &state, i32 dx, i32 dy)
{
  auto hi = state.hot_item;
  kv_soft_assert(dx == 0 || dy == 0);
  i32 iterations = maximum(absoslute(dx), absoslute(dy));

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

// todo: Don't pass the state in whole sale like that
inline void
moveCursor(GameState &state, i32 dx)
{
  i32 &coord = state.cursor_coord;
  f32 &offset = state.cursor_tile_offset.x;
  coord += dx;
  if ((coord < 0) || (coord == 0 && offset < 0)) {
    // clamp cursor to left side of the screen
    coord  = 0;
    offset = 0;
  }
  if ((coord > 30) || (coord == 30 && offset > 0)) {
    coord  = 30;
    offset = 0;
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

struct RaycastOutput {
  b32 hit;
  v3  p;
};

RaycastOutput
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

RaycastOutput
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

v3
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

void
pushLine(RenderGroup rgroup, v3 p0, v3 p1, f32 thickness_px, v4 color)
{
  // v2 p0_screen = eye, p0;
  rect2 rect = {};
  pushRect(rgroup, rect, color);
}

struct Screen {
  v3 center;
  v3 x_axis;
  v3 y_axis;
};

inline v2
screenProject(Screen screen, v3 p)
{
  v3 d = p-screen.center;
  return v2{dot(screen.x_axis, d),
            dot(screen.y_axis, d)};
}

internal void
initScene(Scene &scene) {
  scene.magic_number = scene_file_format_magic_number;
  scene.version      = current_scene_file_format_version;
}

function_typedef("generated_ad_platform.h")
DLL_EXPORT void
gameInitialize(KvArena &init_arena, PlatformCode &platform, String autodraw_path, Codepoint *codepoints)
{
  auto &state = *pushStruct(init_arena, GameState);
  state.arena = subArena(init_arena, megaBytes(512));
  state.frame_arena = subArenaWithRemainingMemory(init_arena);
  state.codepoints  = codepoints;
  state.platform    = platform;
  
  state.data_path      = concatenate(state.arena, autodraw_path, "/../data");
  state.scene_filename = concatenate(state.arena, state.data_path, "/scene.ad");

  {// readSceneFile
    auto temp_marker = beginTemporaryMemory(state.arena);
    defer(endTemporaryMemory(temp_marker));
    auto &arena = state.arena;

    char *scene_filename = state.scene_filename.chars;

    // Read into a temp buffer first
    ReadFileResult scene_file = state.platform.readEntireFile(arena, scene_filename);
    if (!scene_file) {
      printf("scene file %s empty or doesn't exist", scene_filename);
    }
    else if (scene_file.size >= 12)
    {
      state.scene = *(Scene *)scene_file.content;

      if (state.scene.magic_number != *(u32 *)"scen") {
        printf("file %s has wrong magic number!\n", scene_filename);
      }
      else if (state.scene.version == current_scene_file_format_version)  {
        if (scene_file.size != sizeof(Scene)) {
          printf("wrong file size (something went horribly wrong!)\n");
        } else {
          printf("successfully loaded data from file %s\n", scene_filename);
          state.successfully_read_scene_file = true;
        }
      } else if (state.scene.version == current_scene_file_format_version - 1) {
        if (scene_file.size != sizeof(OldScene)) {
          printf("wrong file size (something went horribly wrong!)\n");
        } else {
          // upgrade file version
          // bookmark: Automatically create a backup file
          printf("scene file is old, updating it\n");
          printf("creating a backup file\n");

          char backup_name[256];

          time_t rawtime;
          time(&rawtime);
          struct tm *timeinfo = localtime(&rawtime);
          // todo write to a backup dir, to avoid flooding the data dir
          size_t strftime_result = strftime(backup_name, sizeof(backup_name), "../data/scene-%d-%m-%Y-%H-%M-%S.ad", timeinfo);
          if (strftime_result == 0) {
            printf("strftime failed... I don't even know!\n");
          } else {
            b32 backup_succeed = platform.writeEntireFile(scene_file.content,
                                                          scene_file.size,
                                                          backup_name);
            if (!backup_succeed) {
              printf("failed to create backup file\n");
            } else {
              OldScene old_scene = *(OldScene *)scene_file.content;
              state.scene.version = current_scene_file_format_version;

              state.scene.eye_position = old_scene.eye_position;
              state.scene.eye_inited   = old_scene.eye_inited;

              state.successfully_read_scene_file = true;
            }
          }
        }
      } else {
        printf("cannot recognize scene file version %s\n", scene_filename);
        initScene(state.scene);
      }
    }
  }

  #if 0
{// parseTopLevel
    auto file = platform.readEntireFile(state.arena, "../data/hello_world.ad");
    initTokenizer(init_tokenizer, (char *)file.content);
    TK = &init_tokenizer;

    Token token = nextToken();
    while (hasMore()) {
      auto tl_temp = beginTemporaryMemory(state.arena);
      defer(endTemporaryMemory(tl_temp));
      auto &arena = state.arena;
      error_buffer = subArena(arena, 2048);

      Typer typer = {};

      if (isIdentifier(token))
      {
        Token after_name = nextToken();

        switch (after_name.kind)
        {
          case ':':
          {
            pushContext("variable: NAME : TYPE = VALUE");
            if (Term *type = parseAndBuildGlobal(typer)) {
              if (requireString("=")) {
                if (Term *rhs = parseAndBuildGlobal(typer, type)) {
                  addGlobalBinding(token, rhs);
                }
              }
            }
          } break;

          case Token_ColonEqual:
          {
            pushContext("variable: NAME := VALUE");
            if (Term *rhs = parseAndBuildGlobal(typer)) {
              addGlobalBinding(token, rhs);
            }
            popContext();
          } break;

          default:
          {
            tokenError("unexpected token after identifier");
          } break;
        }
      }
      else {
        tokenError("unexpected token to begin top-level form");
      }
    }
  }
#endif
}

function_typedef("generated_ad_platform.h")
DLL_EXPORT GameOutput gameUpdateAndRender(GameInput &input)
{
  unused_var b32 hot_reloaded = input.hot_reloaded;
  GameState &state = *(GameState *)input.arena.base;
  auto &s = state;

  auto temp_marker = beginTemporaryMemory(s.frame_arena);
  defer(endTemporaryMemory(temp_marker));
  auto &arena = s.frame_arena;

  v2 &screen_dim      = input.screen_dim;
  v2  screen_half_dim = 0.5f * screen_dim;
  v2 pixel_to_clip = {1.f / screen_half_dim.x,
                      1.f / screen_half_dim.y};

  EditorTree &eye    = *pushStruct(arena, EditorTree);
  auto &mom    = *pushStruct(arena, EditorTree);
  auto &slider = *pushStruct(arena, EditorTree);

  eye    = {.id=1, .name=toString(arena, "eye"),
            .next_sibling=&mom};
  mom    = {.id=2, .name=toString(arena, "mom"),
            .children=&slider, .prev_sibling=&eye};
  slider = {.id=3, .name=toString(arena, "slider"),
            .parent=&mom};

  s.first_tree = &eye;
  if (!s.hot_item) s.hot_item = &eye;

  RenderGroup rgroup = {};
  rgroup.codepoints       = s.codepoints;
  rgroup.monospaced_width = (f32)s.codepoints[(u8)'a'].width;
  rgroup.commands         = subArena(arena, megaBytes(8));
  rgroup.arena            = subArena(arena, kiloBytes(128));
  rgroup.current_z_level  = ZLevelGeneral;

  debug_drawer.arena  = subArena(arena, kiloBytes(128));
  debug_drawer.rgroup = &rgroup;
  debug_drawer.at     = -screen_half_dim + v2{10, 10};

  {// Input processing //////////////////////////////////////
    f32 &dt = target_frame_time_sec;
    auto &in = input;

    for (i32 keyi=0; keyi < kVK_Count; keyi++) {
      // Processing held time
      if (in.key_states[keyi].is_down) {
        s.key_held_frames[keyi]++;
      } else {
        s.key_held_frames[keyi] = 0;
      }
    }

    kVK save_key = kVK_ANSI_W;
    if (in.key_states[save_key].is_down && (s.key_held_frames[save_key] == 1)) {
      // write data to a save file
      if (!state.successfully_read_scene_file) {
        printf("WARNING: we won't overwrite the file, since we weren't able to read it");
      } else {
        b32 write_result = s.platform.writeEntireFile((u8 *)&s.scene, sizeof(s.scene), state.scene_filename.chars);
        if (write_result) {
          printf("scene data written to file: %s\n", state.scene_filename.chars);
        } else {
          printf("failed to write to file: %s\n", state.scene_filename.chars);
        }
      }
    }

    v3 direction = {};
    i32 direction_key_held_frames = 0;  // todo this is a #hack to implement the "new-key" behavior
    // todo: cutnpaste
    if (in.key_states[kVK_ANSI_L].is_down) {
      direction.E[0] = 1;
      direction_key_held_frames = maximum(direction_key_held_frames, s.key_held_frames[kVK_ANSI_L]);
    } else if (in.key_states[kVK_ANSI_H].is_down) {
      direction.E[0] = -1;
      direction_key_held_frames = maximum(direction_key_held_frames, s.key_held_frames[kVK_ANSI_H]);
    }
    if (in.key_states[kVK_ANSI_K].is_down) {
      direction.E[1] = 1;
      direction_key_held_frames = maximum(direction_key_held_frames, s.key_held_frames[kVK_ANSI_K]);
    } else if (in.key_states[kVK_ANSI_J].is_down) {
      direction.E[1] = -1;
      direction_key_held_frames = maximum(direction_key_held_frames, s.key_held_frames[kVK_ANSI_J]);
    }
    if (in.key_states[kVK_ANSI_O].is_down) {
      direction.E[2] = 1;
      direction_key_held_frames = maximum(direction_key_held_frames, s.key_held_frames[kVK_ANSI_O]);
    } else if (in.key_states[kVK_ANSI_I].is_down) {
      direction.E[2] = -1;
      direction_key_held_frames = maximum(direction_key_held_frames, s.key_held_frames[kVK_ANSI_I]);
    }

    // bookmark: We need to record the speed for each UI element, which is kind of an ordeal
    // Maybe we only save the speed optionally? (Maybe we should just stick to a good speed, and allow rapid acceleration to both ends (you can press the A-D keys to adjust the speed beforehand, and it'll show you the speed))
    // We're in control adjustment territory, and I'm not sure I wanna go down this rabit hole right now
    // todo: Let's just implement the control, note down the speed, and see what happens
    b32 accelerate = false;
    if (in.key_states[kVK_ANSI_A].is_down) {
      accelerate = true;
    }

    f32 &r = s.revolution;
    r += Tau32 * dt;
    if (r > Tau32) {
      r -= Tau32;
    }

    if (in.key_states[kVK_ANSI_E].is_down) {
      s.editing_item = s.hot_item->id;
    }
    if (in.key_states[kVK_Escape].is_down) {
      s.editing_item = mode_tree_id;
    }

    // We will process all 3 dimensions of control (regardless of the particular tool)
    v3 *offset_ = &s.tree_tile_offset;
    if      (s.editing_item == slider.id) offset_ = &s.cursor_tile_offset;
    else if (s.editing_item == eye.id) {
      offset_ = &s.scene.eye_position;
    }
    v3 &offset = *offset_;

    if (direction.E[0] != 0 || direction.E[1] != 0 || direction.E[2] != 0) {
      if (direction_key_held_frames == 1) {
        // new key-press
        offset += direction;
      } else if (direction_key_held_frames < 8) {
        // hold still for some time
      } else {
        // regular continuous movement
        f32 speed = 10.f;
        if      (s.editing_item == slider.id) speed = 20.f;
        else if (s.editing_item == eye.id)    speed = 1000.f;

        if (accelerate) {
          speed *= 10;
        }

        offset += speed * dt * direction;
      }
    } else if (s.editing_item == slider.id || s.editing_item == mode_tree_id) {
      offset = v3{};
    }

    // NOTE: "offset_int" only applicable for tiled movement
    v3 offset_int = {};
    for (i32 dim_i=0; dim_i < 3; dim_i++) {
      offset_int.E[dim_i] = (offset.E[dim_i] == -0.5f) ? 0 : roundF32(offset.E[dim_i]);
    }
    if (s.editing_item == slider.id || s.editing_item == mode_tree_id) {
      offset -= offset_int;
    }

    // Update game state according to movement
    if (s.editing_item == slider.id) {
      moveCursor(s, offset_int.x);
      pushDebugText("cursor offset: %1.f", s.cursor_tile_offset.x);
      pushDebugText("cursor coord: %1.d", s.cursor_coord);
    } else if (s.editing_item == 0) {// mode tree navigation
      moveTree(s, offset_int.x, offset_int.y);
    }
  }
  
  f32 E3 = 1000.f;
  World world = {};
  Material materials[3];
  materials[0] = {.color=v3{0,0,0}};
  materials[1] = {.color=v3{.1,0,0}};
  materials[2] = {.color=v3{0,.1,.1}};
  world.materials = materials;

  Plane planes[2] = {{.N=v3{1,1,1}, .d=E3 * -0.1f, .mat_index=1},
                     {.N=v3{1,0,1}, .d=E3 * -0.2f, .mat_index=2}};
  world.planes = planes;

  world.plane_count    = arrayCount(planes);
  world.material_count = arrayCount(materials);

  // Render: Build the push buffer ////////////////////////////////////

  {// push backdrop
    rgroup.current_z_level = ZLevelBackdrop;
    v4 bg_color = input.test_boolean ?  v4{1,1,1,1} : v4{0,0,0,1};
    pushRect(rgroup, rect2{-screen_half_dim, screen_half_dim}, bg_color);
    rgroup.current_z_level = ZLevelGeneral;
  }

  if (s.editing_item == slider.id)
  {// push cursor
    v2 O = {0, screen_half_dim.y};
    rgroup.current_z_level = ZLevelGeneral;
    v2 cursor_dim = {20, 20};
    v2 min = O + v2{(s.cursor_coord + s.cursor_tile_offset.x) * cursor_dim.x,
                    - cursor_dim.y};
    auto rect = rectMinDim(min, cursor_dim);
    pushRect(rgroup, rect, v4{1,0,0,1});
    pushDebugText("cursor: %d", s.cursor_coord);
  } else {// push the editor tree
    v2 min = hadamard(screen_half_dim, {0.5f, -0.0f});
    drawTreeAndSiblings(s, rgroup, s.first_tree, min);
  }

  {// push debug text
    pushDebugText("frame time: %.3f ms", input.last_frame_time_sec * 1000);
  }

  Bitmap ray_bitmap = {};
  {// Ray tracing
    auto &scene = state.scene;
    v3   &eye_p = scene.eye_position;
    if (!scene.eye_inited) {
      scene.eye_inited = true;
      eye_p = E3 * v3{-1, -.2f, 3};
    }
    v3 eye_z = noz(eye_p);  // NOTE: z comes at you
    v3 eye_x = cross(eye_z, v3{0,0,1});
    v3 eye_y = cross(eye_z, eye_x);

    f32 d_eye_screen = 2000.f;  // NOTE: based on real life distance
    v3 screen_center = eye_p - d_eye_screen * eye_z;

    // rgba in memory order
    ray_bitmap.dim = {1024, 720};
    auto bitmap_dim = ray_bitmap.dim;
    ray_bitmap.pitch = 4*bitmap_dim.x;

    u32 *bitmap = pushArray(arena, bitmap_dim.x*bitmap_dim.y, u32);
    u32 *dst = bitmap;
    v2 ray_bitmap_half_dim = .5f * ray_bitmap.dim;
    for (i32 y = (i32)-ray_bitmap_half_dim.y;
         y < (i32)ray_bitmap_half_dim.y;
         y++)
    {
      for (i32 x = (i32)-ray_bitmap_half_dim.x;
           x < (i32)ray_bitmap_half_dim.x;
           x++)
      {
        v3 screen_p = screen_center + ((f32)x * eye_x + (f32)y * eye_y);
        v3 ray_origin = eye_p;
        v3 ray_dir    = screen_p - eye_p;
        v3 color = raycast(world, ray_origin, ray_dir);

        *dst++ = pack_sRGBA(toV4(color, 1));
      }
    }
    ray_bitmap.memory = bitmap;

    {// push ray tracing
      // push the canvas rect ///////////////////////////////////////////////
      rgroup.current_z_level = ZLevelRaytrace;
      defer(rgroup.current_z_level = ZLevelGeneral;);
      v2 layout_center = {-350, 0};
      rect2 rect = rectCenterDim(layout_center, bitmap_dim);
      pushRect(rgroup, rect, TextureIdRayTrace);
      pushRectOutline(rgroup, rect, 2.f, warm_yellow);

      // push coordinate system ///////////////////////////////////////////////
      v3 p0 = E3 * v3{0,0,0};
      v3 px = E3 * v3{1,0,0};
      v3 py = E3 * v3{0,1,0};
      v3 pz = E3 * v3{0,0,1};

      v3  &screen_N = eye_z;
      f32  screen_d = -dot(eye_p, eye_z) + d_eye_screen;

      auto p0_hit = raycastPlane(eye_p, p0 - eye_p, screen_N, screen_d);
      auto px_hit = raycastPlane(eye_p, px - eye_p, screen_N, screen_d);
      auto py_hit = raycastPlane(eye_p, py - eye_p, screen_N, screen_d);
      auto pz_hit = raycastPlane(eye_p, pz - eye_p, screen_N, screen_d);

      if (p0_hit.hit && px_hit.hit && py_hit.hit && pz_hit.hit) {
        Screen screen = {screen_center, eye_x, eye_y};
        v2 p0_screen = screenProject(screen, p0_hit.p);
        v2 px_screen = screenProject(screen, px_hit.p);
        v2 py_screen = screenProject(screen, py_hit.p);
        v2 pz_screen = screenProject(screen, pz_hit.p);
        auto O = layout_center;
        pushLine(rgroup, p0_screen+O, px_screen+O, 2.f, {.2,  0,  0, 1});
        pushLine(rgroup, p0_screen+O, py_screen+O, 2.f, { 0, .2,  0, 1});
        pushLine(rgroup, p0_screen+O, pz_screen+O, 2.f, { 0,  0, .2, 1});

        pushDebugText("eye_p: %2.f, %2.f, %2.f", eye_p.x, eye_p.y, eye_p.z);
        // v3 px = px_hit.p;
        // pushDebugText("p0_hit: %.1f, %.1f, %.1f", p0_hit.p.x, p0_hit.p.y, p0_hit.p.z);
        // pushDebugText("p0_screen: %.1f, %.1f", p0_screen.x, p0_screen.y);
      }
    }
  }

  // Render: Create GPU commands //////////////////////////////////////////

  GPUCommands gcommands = {};
  gcommands.commands      = subArena(arena, megaBytes(8));
  gcommands.vertex_buffer = subArena(arena, megaBytes(8));

  i32 render_command_count = 0;
  for (i32 i=0; i < ZLevelCount_; i++) {
    render_command_count += rgroup.z_bucket_count[i];
  }

  i32 next_z_bucket_index[ZLevelCount_] = {};
  i32 *z_buckets[ZLevelCount_];
  for (i32 bucket_i=0; bucket_i < ZLevelCount_; bucket_i++) {
    z_buckets[bucket_i] = pushArray(arena, rgroup.z_bucket_count[bucket_i], i32);
  }

  for (i32 command_i=0; command_i < render_command_count; command_i++) {
    // index pass: figure out which render group entry go where
    SortEntry *backward_base = (SortEntry *)(rgroup.commands.base + rgroup.commands.original_cap);
    auto sort_entry = (backward_base - 1 - command_i);
    i32 index = next_z_bucket_index[sort_entry->z_level]++;
    z_buckets[sort_entry->z_level][index] = sort_entry->buffer_offset;
  }

  for (i32 z_level=0; z_level < ZLevelCount_; z_level++) {
    i32  bucket_count = rgroup.z_bucket_count[z_level];
    i32 *bucket       = z_buckets[z_level];
    for (i32 offset_i=0; offset_i < bucket_count; offset_i++) {
      u8 *at = rgroup.commands.base + bucket[offset_i];
      auto header = EAT_TYPE(at, RenderEntryHeader);
      switch (header->type) {
        case RenderEntryTypeQuad: {
          auto entry = EAT_TYPE(at, RenderEntryQuad);

          i32 vertex_count = 4;
          auto out = pushVertices(gcommands, vertex_count);
          auto c = entry->color;
          auto cs = simd_float4{c.r, c.g, c.b, c.a};

          out.vertices[0] = {toFloat2(hadamard(pixel_to_clip, entry->x0y0)), {0.f, 0.f}, cs};
          out.vertices[1] = {toFloat2(hadamard(pixel_to_clip, entry->x0y1)), {0.f, 1.f}, cs};
          out.vertices[2] = {toFloat2(hadamard(pixel_to_clip, entry->x1y0)), {1.f, 0.f}, cs};
          out.vertices[3] = {toFloat2(hadamard(pixel_to_clip, entry->x1y1)), {1.f, 1.f}, cs};

          auto &command = *pushGPUCommand(gcommands, TriangleStrip);
          command.vertex_start = out.vertex_start;
          command.vertex_count = vertex_count;
          command.texture      = entry->texture;

          break;
        }

          invalidDefaultCase;
      }
    }
  }
  return GameOutput{.gcommands=gcommands, .raytracing_bitmap=ray_bitmap};
}
