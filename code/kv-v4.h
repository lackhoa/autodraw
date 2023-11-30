#pragma once

#include "kv-v3.h"

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
            v3 rgb;
            r32 a_ignored;
        };
        struct
        {
            v3 xyz;
            r32 w_ignored;
        };
        struct
        {
            v2 xy;
            v2 yz_ignored;
        };
        r32 E[4];
    };
} v4;

inline v4
toV4(r32 x, r32 y, r32 z, r32 w)
{
    v4 result = { x, y, z , w };
    return result;
}

inline v4
hadamard(v4 u, v4 v)
{
    v4 result;
    result.x = u.x * v.x;
    result.y = u.y * v.y;
    result.z = u.z * v.z;
    result.w = u.w * v.w;
    return result;
}

inline v4
clampMax(v4 v, r32 max)
{
    v4 result = { clampMax(v.x, max),
                  clampMax(v.y, max),
                  clampMax(v.z, max),
                  clampMax(v.w, max) };
    return result;
}

inline v4
operator*(r32 c, v4 v)
{
    v4 result = {c * v.x, c * v.y, c * v.z, c * v.w};
    return result;
}

inline v4
operator*(v4 v, r32 c)
{
    v4 result = {c * v.x, c * v.y, c * v.z, c * v.w};
    return result;
}

inline v4 &
operator*=(v4 &v, r32 c)
{
    v = c * v;
    return v;
}

inline v4
operator+(v4 u, v4 v)
{
    v4 result = {u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w};
    return result;
}

inline v4
operator-(v4 u, v4 v)
{
    v4 result = {u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w};
    return result;
}

inline v4
lerp(v4 a, r32 t, v4 b)
{
    v4 result;
    result = a + t*(b - a);
    return result;
}

inline b32
inRange(v4 v, r32 min, r32 max)
{
    b32 result = (inRange(v.x, min, max) &&
                  inRange(v.y, min, max) &&
                  inRange(v.z, min, max) &&
                  inRange(v.w, min, max));
    return result;
}
