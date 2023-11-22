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
            V2 xy;
            f32 ignored;
        };
        f32 E[3];
    };
} V3;

inline V3
v3(f32 x, f32 y, f32 z)
{
    V3 result = { x, y, z };
    return result;
}

inline V3
v3i(i32 x, i32 y, i32 z)
{
    V3 result;
    result.x = (f32)x;
    result.y = (f32)y;
    result.z = (f32)z;
    return result;
}

inline V3
toV3(V2 xy, f32 z)
{
    V3 result;
    result.xy = xy;
    result.z = z;
    return result;
}

inline V3
v3All(f32 c)
{
    return v3(c, c, c);
}

inline V3
v3z(f32 z)
{
    V3 result = {0,0,z};
    return result;
}

inline V3
v3xy(V2 v)
{
    V3 result = {v.x, v.y, 0};
    return result;
}

inline V3
operator-(V3 u, V3 v)
{
    V3 result;
    result.x = u.x - v.x;
    result.y = u.y - v.y;
    result.z = u.z - v.z;
    return result;
}

inline b32
operator<(V3 u, V3 v)
{
    b32 result = ((u.x < v.x) && (u.y < v.y) && (u.z < v.z));
    return result;
}

inline b32
operator<=(V3 u, V3 v)
{
    b32 result = ((u.x <= v.x) && (u.y <= v.y) && (u.z <= v.z));
    return result;
}

inline b32
operator>(V3 u, V3 v)
{
    b32 result = ((u.x > v.x) && (u.y > v.y) && (u.z > v.z));
    return result;
}

inline b32
operator>=(V3 u, V3 v)
{
    b32 result = ((u.x >= v.x) && (u.y >= v.y) && (u.z >= v.z));
    return result;
}

inline b32
operator==(V3 u, V3 v)
{
    b32 result;
    result = (u.x == v.x) && (u.y == v.y) && (u.z == v.z);
    return result;
}

inline b32
operator!=(V3 u, V3 v)
{
    b32 result;
    result = (u.x != v.x) || (u.y != v.y) || (u.z != v.z);
    return result;
}

inline V3
operator+(V3 u, V3 v)
{
    V3 result;
    result.x = u.x + v.x;
    result.y = u.y + v.y;
    result.z = u.z + v.z;
    return result;
}

inline V3 &
operator+=(V3 &v, V3 u)
{
    v = u + v;
    return v;
}

inline V3
operator-=(V3 &v, V3 u)
{
    v = v - u;
    return v;
}


inline V3
operator-(V3 v)
{
    V3 result;
    result.x = -v.x;
    result.y = -v.y;
    result.z = -v.z;
    return result;
}

inline V3
operator*(f32 c, V3 v)
{
    V3 result;
    result.x = c * v.x;
    result.y = c * v.y;
    result.z = c * v.z;
    return result;
}

inline V3
operator*(V3 v, f32 c)
{
    V3 result = c*v;
    return result;
}

inline V3 &
operator*=(V3 &v, f32 c)
{
    v = c * v;
    return v;
}

inline V3
operator/(V3 v, f32 c)
{
    V3 result;
    result.x = v.x / c;
    result.y = v.y / c;
    result.z = v.z / c;
    return result;
}

inline f32
inner(V3 v, V3 u)
{
    f32 result = v.x*u.x + v.y*u.y + v.z*u.z;
    return result;
}

inline V3
hadamard(V3 v, V3 u)
{
    V3 result;
    result.x = v.x*u.x;
    result.y = v.y*u.y;
    result.z = v.z*u.z;
    return result;
}

inline f32
lengthSq(V3 v)
{
    f32 result = square(v.x) + square(v.y) + square(v.z);
    return result;
}

inline f32
length(V3 v)
{
    f32 result = squareRoot(lengthSq(v));
    assert(((v.x == 0.0f) && (v.y == 0.0f)) || (result != 0));
    return result;
}

inline V3
normalize(V3 v)
{
    V3 result;
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

inline V3
project(V3 onto, V3 v)
{
    f32 innerProd = inner(onto, v);
    V3 result = (innerProd / lengthSq(onto)) * onto;
    return result;
}

inline V3
toBilateral(V3 v)
{
    V3 result;
    result.x = toBilateral(v.x);
    result.y = toBilateral(v.y);
    result.z = toBilateral(v.z);
    return result;
}
