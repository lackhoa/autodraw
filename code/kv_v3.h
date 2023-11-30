#pragma once

#include "kv_utils.h"
#include "kv_scalar.h"
#include "kv_v2.h"

typedef struct
{
    union
    {
        struct
        {
            f32 x, y, z;
        };
        struct
        {
            f32 r, g, b;
        };
        struct
        {
            v2 xy;
            f32 ignored;
        };
        f32 E[3];
    };
} v3;

inline v3
toV3(f32 x, f32 y, f32 z)
{
    v3 result = { x, y, z };
    return result;
}

inline v3
v3i(i32 x, i32 y, i32 z)
{
    v3 result;
    result.x = (f32)x;
    result.y = (f32)y;
    result.z = (f32)z;
    return result;
}

inline v3
toV3(v2 xy, f32 z)
{
    v3 result;
    result.xy = xy;
    result.z = z;
    return result;
}

inline v3
v3All(f32 c)
{
    return toV3(c, c, c);
}

inline v3
v3z(f32 z)
{
    v3 result = {0,0,z};
    return result;
}

inline v3
v3xy(v2 v)
{
    v3 result = {v.x, v.y, 0};
    return result;
}

inline v3
operator-(v3 u, v3 v)
{
    v3 result;
    result.x = u.x - v.x;
    result.y = u.y - v.y;
    result.z = u.z - v.z;
    return result;
}

inline b32
operator<(v3 u, v3 v)
{
    b32 result = ((u.x < v.x) && (u.y < v.y) && (u.z < v.z));
    return result;
}

inline b32
operator<=(v3 u, v3 v)
{
    b32 result = ((u.x <= v.x) && (u.y <= v.y) && (u.z <= v.z));
    return result;
}

inline b32
operator>(v3 u, v3 v)
{
    b32 result = ((u.x > v.x) && (u.y > v.y) && (u.z > v.z));
    return result;
}

inline b32
operator>=(v3 u, v3 v)
{
    b32 result = ((u.x >= v.x) && (u.y >= v.y) && (u.z >= v.z));
    return result;
}

inline b32
operator==(v3 u, v3 v)
{
    b32 result;
    result = (u.x == v.x) && (u.y == v.y) && (u.z == v.z);
    return result;
}

inline b32
operator!=(v3 u, v3 v)
{
    b32 result;
    result = (u.x != v.x) || (u.y != v.y) || (u.z != v.z);
    return result;
}

inline v3
operator+(v3 u, v3 v)
{
    v3 result;
    result.x = u.x + v.x;
    result.y = u.y + v.y;
    result.z = u.z + v.z;
    return result;
}

inline v3 &
operator+=(v3 &v, v3 u)
{
    v = u + v;
    return v;
}

inline v3
operator-=(v3 &v, v3 u)
{
    v = v - u;
    return v;
}


inline v3
operator-(v3 v)
{
    v3 result;
    result.x = -v.x;
    result.y = -v.y;
    result.z = -v.z;
    return result;
}

inline v3
operator*(f32 c, v3 v)
{
    v3 result;
    result.x = c * v.x;
    result.y = c * v.y;
    result.z = c * v.z;
    return result;
}

inline v3
operator*(v3 v, f32 c)
{
    v3 result = c*v;
    return result;
}

inline v3 &
operator*=(v3 &v, f32 c)
{
    v = c * v;
    return v;
}

inline v3
operator/(v3 v, f32 c)
{
    v3 result;
    result.x = v.x / c;
    result.y = v.y / c;
    result.z = v.z / c;
    return result;
}

inline f32
inner(v3 v, v3 u)
{
    f32 result = v.x*u.x + v.y*u.y + v.z*u.z;
    return result;
}

inline v3
hadamard(v3 v, v3 u)
{
    v3 result;
    result.x = v.x*u.x;
    result.y = v.y*u.y;
    result.z = v.z*u.z;
    return result;
}

inline f32
lengthSq(v3 v)
{
    f32 result = square(v.x) + square(v.y) + square(v.z);
    return result;
}

inline f32
length(v3 v)
{
    f32 result = squareRoot(lengthSq(v));
    assert(((v.x == 0.0f) && (v.y == 0.0f)) || (result != 0));
    return result;
}

inline v3
normalize(v3 v)
{
    v3 result;
    f32 len = length(v);
    if (len == 0)
    {
        result = v3i(0,0,0);
    }
    else
    {
        result = v * (1.0f / len);
    }
    return result;
}

inline v3
project(v3 onto, v3 v)
{
    f32 innerProd = inner(onto, v);
    v3 result = (innerProd / lengthSq(onto)) * onto;
    return result;
}

inline v3
toBilateral(v3 v)
{
    v3 result;
    result.x = toBilateral(v.x);
    result.y = toBilateral(v.y);
    result.z = toBilateral(v.z);
    return result;
}
