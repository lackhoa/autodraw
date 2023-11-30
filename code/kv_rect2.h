#pragma once

#include "kv_v2.h"

struct rect2 {
    v2 min;
    v2 max;
};

inline b32
inRectangle(rect2 rect, v2 point)
{
    return ((point.x >= rect.min.x) && (point.y >= rect.min.y)
            && (point.x < rect.max.x) && (point.y < rect.max.y));
}

inline rect2
rectRadius(v2 radius)
{
    return {-radius, radius};
}

inline v2
getRectDim(rect2 rect)
{
    return (rect.max - rect.min);
}

inline rect2
rectCenterRadius(v2 center, v2 radius)
{
    assert((radius.x >= 0) && (radius.y >= 0));
    rect2 result;
    result.min = center - radius;
    result.max = center + radius;
    return result;
}

inline rect2
rectCenterDim(v2 center, v2 dim)
{
    rect2 result = rectCenterRadius(center, 0.5f*dim);
    return result;
}

inline rect2
rectMinDim(v2 min, v2 dim)
{
  rect2 out = rect2{.min=min, .max=min+dim};
  return out;
}

inline rect2
intersect(rect2 a, rect2 b)
{
    rect2 result;
    result.min.x = maximum(a.min.x, b.min.x);
    result.min.y = maximum(a.min.y, b.min.y);
    result.max.x = minimum(a.max.x, b.max.x);
    result.max.y = minimum(a.max.y, b.max.y);
    return result;
}
