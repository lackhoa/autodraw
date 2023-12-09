// We don't know if it will even improve startup time yet!

#include <stdio.h>
#include "kv-utils.h"

internal void
makeCodepointTextures(Arena arena) {
  auto temp = beginTemporaryMemory(arena);
  defer(endTemporaryMemory(temp));

  auto read_file = osxReadEntireFile("../resources/fonts/LiberationMono-Regular.ttf");
  u8 *ttf_buffer = read_file.content;
  if (!ttf_buffer) {
    todoErrorReport;
    return;
  }
  stbtt_fontinfo font;
  stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));
  f32 pixel_height = stbtt_ScaleForPixelHeight(&font, font_height_px);

  for (i32 ascii_char=33; ascii_char <= 126; ascii_char++) {
    i32 width, height, xoff, yoff;
    u8 *mono_bitmap = stbtt_GetCodepointBitmap(&font, 0,pixel_height, ascii_char,
                                               &width, &height, &xoff, &yoff);
    assert(width != 0 && height != 0);
    u8 *bitmap = (u8 *)pushSize(arena, 4 * width * height);
    // Blow it out to rgba bitmap
    u32 *dst = (u32 *)bitmap;
    i32 pitch = width;
    u8 *src_row  = mono_bitmap + (height-1)*pitch;  // read backward in y
    for (i32 y=0; y < height; y++) {
      u8 *src = src_row;
      for (i32 x=0; x < width; x++) {
        u32 au = *src++;
        // assert(au < 256);
        f32 c = (f32)au / 255.f;
        // pre-multiplied alpha (NOTE: we assume color is white)
        c = square(c);
        u32 cu = (u32)(255.f*c + 0.5f) ;
        *dst++ = (au << 24) | (cu << 16) | (cu << 8) | (cu << 0);
      }
      src_row -= pitch;
    }

    stbtt_FreeBitmap(mono_bitmap, 0);
    // Note: The color is white so srgb doesn't matter
    metal_textures[ascii_char] = metal_sRGBATexture(bitmap, width, height);
    codepoints[ascii_char]     = {width, height, (r32)width/(r32)height, xoff, yoff};
  }
}

int main()
{
  // Allocate memory
  size_t size_in_bytes = megaBytes(1);
  u8* memory = (u8 *)malloc(size_in_bytes);
  if (memory == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    return 1; // Return an error code
  }
  defer(free(memory));

  Arena arena = newArena(memory, size_in_bytes);
  writeCodepointBitmaps();

  return 0;
}
