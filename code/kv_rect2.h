#pragma once

#include "kv_v2.h"

struct Rect2
{
    V2 min;
    V2 max;
};

inline b32
inRectangle(Rect2 rect, V2 point)
{
    return ((point.x >= rect.min.x) && (point.y >= rect.min.y)
            && (point.x < rect.max.x) && (point.y < rect.max.y));
}

inline Rect2
rectRadius(V2 radius)
{
    return {-radius, radius};
}

inline V2
getRectDim(Rect2 rect)
{
    return (rect.max - rect.min);
}

inline Rect2
rectCenterRadius(V2 center, V2 radius)
{
    assert((radius.x >= 0) && (radius.y >= 0));
    Rect2 result;
    result.min = center - radius;
    result.max = center + radius;
    return result;
}

inline Rect2
rectCenterDim(V2 center, V2 dim)
{
    Rect2 result = rectCenterRadius(center, 0.5f*dim);
    return result;
}

inline Rect2
intersect(Rect2 a, Rect2 b)
{
    Rect2 result;
    result.min.x = maximum(a.min.x, b.min.x);
    result.min.y = maximum(a.min.y, b.min.y);
    result.max.x = minimum(a.max.x, b.max.x);
    result.max.y = minimum(a.max.y, b.max.y);
    return result;
}
