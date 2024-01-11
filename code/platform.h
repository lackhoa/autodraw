/*
  Header file imported by the game and all platforms.
  Define protocol of communication between the game and the platform.
  IMPORTANT: Minimize burden for the platform! Do not make them do work that the game can do itself.
 */

#pragma once

#include "kv.h"
#include "kv-math.h"
#include "keycodes.h"
#include "kv-bitmap.h"

f32 font_height_px = 128.f;
f32 game_update_hz = 60.f;
f32 target_frame_time_sec = 1.f / game_update_hz;

struct ReadFileResult
{
  u64  size;
  u8  *content;
  operator bool() {
    return content;
  }
};

#define PLATFORM_WRITE_ENTIRE_FILE(NAME) b32 NAME(u8 *content, i32 content_size, char *filename)
typedef PLATFORM_WRITE_ENTIRE_FILE(PlatformWriteEntireFile);

#define PLATFORM_READ_ENTIRE_FILE(NAME) ReadFileResult NAME(KvArena &arena, char *filename)
typedef PLATFORM_READ_ENTIRE_FILE(PlatformReadEntireFile);

struct PlatformCode 
{
  PlatformWriteEntireFile *writeEntireFile;
  PlatformReadEntireFile  *readEntireFile;
};

// NOTE: currently we deal with monospaced fonts only
struct Codepoint {
  i32 width, height;
  f32 width_over_height;
  i32 xoff, yoff;
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

enum GPUCommandType {
  GPUCommandTypeTriangle,
  GPUCommandTypeTriangleStrip,
};

struct GPUCommandHeader {
  GPUCommandType type;
};

struct GPUCommands {
  KvArena commands;
  KvArena vertex_buffer;
  i32   vertex_start;
};

struct GPUCommandTriangle {
  i32 vertex_start;
  i32 vertex_count;
  TextureId texture;
};

struct GPUCommandTriangleStrip {
  i32 vertex_start;
  i32 vertex_count;
  TextureId texture;
};

struct KeyState {
  b32 is_down;
};

struct GameOutput {
  GPUCommands gcommands;
  Bitmap      raytracing_bitmap;
};

// NOTE: GameMemory is the communication interface between the game and platform.
struct GameInput {
  KvArena arena;                  // All memory allocated for the game by the platform

  KeyState key_states[kVK_Count];
  f32      last_frame_time_sec;
  b32      hot_reloaded;
  v2       screen_dim;
  
  b32 test_boolean;
};

#include "generated_ad_platform.h"
