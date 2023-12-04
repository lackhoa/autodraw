#pragma once

#include "kv-math.h"

struct Bitmap {
  u32 *memory;
  i32 dimx;
  i32 dimy;
  i32 pitch;
};

inline v4
linearToSrgb(v4 linear)
{
    v4 result;
    result.r = squareRoot(linear.r);
    result.g = squareRoot(linear.g);
    result.b = squareRoot(linear.b);
    result.a = linear.a;
    return result;
}

inline u32
pack_sRGBA(v4 color)
{
  // linear to srgb
  color.r = squareRoot(color.r);
  color.g = squareRoot(color.g);
  color.b = squareRoot(color.b);
  u32 result = ((u32)(color.a*255.0f + 0.5f) << 24
                | (u32)(color.b*255.0f + 0.5f) << 16
                | (u32)(color.g*255.0f + 0.5f) << 8
                | (u32)(color.r*255.0f + 0.5f));
  return result;
}
