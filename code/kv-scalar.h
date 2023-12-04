#include "kv-utils.h"
#include "kv-intrinsics.h"

#pragma once

#define Pi32 3.14159265359f
#define Tau32 6.28318530717958647692f

inline f32
toBilateral(f32 r)
{
    return (r * 2.0f) - 1.0f;
}

inline f32
toUnilateral(f32 r)
{
    return (r * 0.5f) + 0.5f;
}

inline f32
lerp(f32 a, f32 t, f32 b)
{
    f32 result;
    result = a + t*(b - a);
    return result;
}

inline f32
unlerp(f32 a, f32 v, f32 b)
{
  f32 range = (b - a);
  f32 result = (range != 0.0f) ? ((v - a) / range) : 0.0f;
  return result;
}

inline b32
inRange(f32 value, f32 min, f32 max)
{
  return ((min <= value) && (value <= max));
}

inline b32
inRange(i32 value, i32 min, i32 max)
{
    return ((min <= value) && (value <= max));
}

#define inRange01(v) inRange(v, 0.f, 1.f)

inline f32
clamp(f32 value, f32 min, f32 max)
{
    assert(max >= min);
    if (value > max)
    {
        return max;
    }
    else if (value < min)
    {
        return min;
    }
    return value;
}

inline f32
clamp01(f32 r)
{
    return clamp(r, 0, 1);
}

inline f32
clampMin(f32 r, f32 min)
{
    f32 result = maximum(r, min);
    return result;
}

inline i32
clampMin(i32 r, i32 min)
{
    i32 result = maximum(r, min);
    return result;
}

inline f32
clampMax(f32 r, f32 max)
{
    f32 result = minimum(r, max);
    return result;
}

inline i32
clampMax(i32 r, i32 max)
{
    i32 result = minimum(r, max);
    return result;
}
