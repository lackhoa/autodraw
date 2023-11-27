#pragma once

#include "kv_v3.h"

typedef struct
{
    union
    {
        struct
        {
            r32 x, y, z, w;
        };
        struct
        {
            r32 r, g, b, a;
        };
        struct
        {
            V3 rgb;
            r32 a_ignored;
        };
        struct
        {
            V3 xyz;
            r32 w_ignored;
        };
        struct
        {
            V2 xy;
            V2 yz_ignored;
        };
        r32 E[4];
    };
} V4;

inline V4
v4(r32 x, r32 y, r32 z, r32 w)
{
    V4 result = { x, y, z , w };
    return result;
}

inline V4
hadamard(V4 u, V4 v)
{
    V4 result;
    result.x = u.x * v.x;
    result.y = u.y * v.y;
    result.z = u.z * v.z;
    result.w = u.w * v.w;
    return result;
}

inline V4
clampMax(V4 v, r32 max)
{
    V4 result = { clampMax(v.x, max),
                  clampMax(v.y, max),
                  clampMax(v.z, max),
                  clampMax(v.w, max) };
    return result;
}

inline V4
operator*(r32 c, V4 v)
{
    V4 result = {c * v.x, c * v.y, c * v.z, c * v.w};
    return result;
}

inline V4
operator*(V4 v, r32 c)
{
    V4 result = {c * v.x, c * v.y, c * v.z, c * v.w};
    return result;
}

inline V4 &
operator*=(V4 &v, r32 c)
{
    v = c * v;
    return v;
}

inline V4
operator+(V4 u, V4 v)
{
    V4 result = {u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w};
    return result;
}

inline V4
operator-(V4 u, V4 v)
{
    V4 result = {u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w};
    return result;
}

inline V4
lerp(V4 a, r32 t, V4 b)
{
    V4 result;
    result = a + t*(b - a);
    return result;
}

inline b32
inRange(V4 v, r32 min, r32 max)
{
    b32 result = (inRange(v.x, min, max) &&
                  inRange(v.y, min, max) &&
                  inRange(v.z, min, max) &&
                  inRange(v.w, min, max));
    return result;
}
