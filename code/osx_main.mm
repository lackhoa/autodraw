#import <stdio.h>
#import "raylib.h"
#import "raymath.h"
#import "utils.h"

global_variable b32 global_running = true;

int awesomeFunction()
{
  return 0;
}

int main(int argc, const char *argv[])
{
  i32 rendering_width  = 1920 / 2;
  i32 rendering_height = 1080 - 80;
  InitWindow(rendering_width, rendering_height, "Auto Draw");
  SetWindowPosition(0,0);

  Image imBlank = GenImageColor(1024, 1024, BLANK);  // todo: what is this?
  Texture2D texture = LoadTextureFromImage(imBlank);
  UnloadImage(imBlank);

  SetTargetFPS(60);
  const char *shader_file_name = "/Users/khoa/AutoDraw/code/cubes_panning.fs";
  Shader shader = LoadShader(0, shader_file_name);
  i32 timeLoc = GetShaderLocation(shader, "uTime");

  i64 shader_mod_time = awesomeFunction();
  while (global_running)
  {
    if (WindowShouldClose())
    {
      global_running = false;
    }
    else
    {
      i64 new_shader_mod_time = GetFileModTime(shader_file_name);
      if (shader_mod_time != new_shader_mod_time)
      {
        shader_mod_time = new_shader_mod_time;
        shader = LoadShader(0, shader_file_name);
        timeLoc = GetShaderLocation(shader, "uTime");
        printf("Shader reloaded!\n");
      }

      r32 time = (r32)GetTime();
      SetShaderValue(shader, timeLoc, &time, SHADER_UNIFORM_FLOAT);

      {
        BeginDrawing();
        ClearBackground(DARKGRAY);
        {
          BeginShaderMode(shader);
          DrawTexture(texture, 0, 0, BLACK);  // todo how does this work?
          EndShaderMode();
        }
        DrawText("Background is painted and animated on shader!", 10, 10, 20, MAROON);
        EndDrawing();
      }
    }
  }

  printf("objective-c autodraw finished!\n");
}
