#include "kv_scalar.h"

#if !defined(HANDMADE_V2_H)

typedef struct
{
    union
    {
        struct
        {
            f32 x, y;
        };
        f32 E[2];
    };
} V2;

inline V2
v2(f32 x, f32 y)
{
    V2 result;
    result.x = x;
    result.y = y;
    return result;
}

inline V2
v2i(i32 x, i32 y)
{
    return v2((f32)x, (f32)y);
}

inline V2
v2All(f32 c)
{
    return v2(c, c);
}

inline b32
operator==(V2 u, V2 v)
{
    b32 result;
    result = (u.x == v.x) && (u.y == v.y);
    return result;
}

inline b32
operator!=(V2 u, V2 v)
{
    b32 result;
    result = (u.x != v.x) || (u.y != v.y);
    return result;
}

inline V2
operator+(V2 u, V2 v)
{
    V2 result;
    result.x = u.x + v.x;
    result.y = u.y + v.y;
    return result;
}

inline V2 &
operator+=(V2 &v, V2 u)
{
    v = u + v;
    return v;
}

inline V2
operator-(V2 u, V2 v)
{
    V2 result;
    result.x = u.x - v.x;
    result.y = u.y - v.y;
    return result;
}

inline V2
operator-=(V2 &v, V2 u)
{
    v = v - u;
    return v;
}

inline V2
operator-(V2 v)
{
    V2 result;
    result.x = -v.x;
    result.y = -v.y;
    return result;
}

inline V2
operator*(f32 c, V2 v)
{
    V2 result;
    result.x = c * v.x;
    result.y = c * v.y;
    return result;
}

inline V2
operator*(V2 v, f32 c)
{
    V2 result = c*v;
    return result;
}

inline V2 &
operator*=(V2 &v, f32 c)
{
    v = c * v;
    return v;
}

inline V2
operator/(V2 v, f32 c)
{
    V2 result;
    result.x = v.x / c;
    result.y = v.y / c;
    return result;
}

inline f32
inner(V2 v, V2 u)
{
    f32 result = v.x*u.x + v.y*u.y;
    return result;
}

inline f32
lengthSq(V2 v)
{
    f32 result = square(v.x) + square(v.y);
    return result;
}

inline f32
length(V2 v)
{
    f32 result = squareRoot(lengthSq(v));
    return result;
}

inline f32
projectLen(V2 onto, V2 v)
{
    f32 innerProd = inner(onto, v);
    f32 result = (innerProd / length(onto));
    return result;
}

inline V2
project(V2 onto, V2 v)
{
    f32 innerProd = inner(onto, v);
    V2 result = (innerProd / lengthSq(onto)) * onto;
    return result;
}

inline V2
hadamard(V2 v, V2 u)
{
    V2 result;
    result.x = v.x*u.x;
    result.y = v.y*u.y;
    return result;
}

inline V2
normalize(V2 v)
{
    V2 result;
    f32 len = length(v);
    if (len == 0)
    {
        result = v2(0,0);
    }
    else
    {
        result = v * (1.0f / len);
    }
    return result;
}

inline V2
perp(V2 v)
{
    V2 result = {-v.y, v.x};
    return result;
}

inline V2
toBilateral(V2 v)
{
    V2 result = {toBilateral(v.x), toBilateral(v.y)};
    return result;
}

inline b32
inRange(V2 v, f32 min, f32 max)
{
    b32 result = (inRange(v.x, min, max) && inRange(v.y, min, max));
    return result;
}

#define HANDMADE_V2_H
#endif
