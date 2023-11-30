#pragma once

#include "kv-v3.h"

struct Rect3
{
    v3 min;
    v3 max;
};

inline b32
verifyRect(Rect3 rect)
{
    b32 result = ((rect.min.x <= rect.max.x)
                  && (rect.min.y <= rect.max.y)
                  && (rect.min.z <= rect.max.z));
    return result;
}

inline Rect3
rectRadius(v3 radius)
{
    return {-radius, radius};
}

inline Rect3
rectCenterRadius(v3 center, v3 radius)
{
    assert((radius.x >= 0) && (radius.y >= 0) && (radius.z >= 0));
    Rect3 result;
    result.min = center - radius;
    result.max = center + radius;
    return result;
}

inline b32
inRectangle(Rect3 rect, v3 p)
{
    b32 result = ((p.x >= rect.min.x)
                  && (p.y >= rect.min.y)
                  && (p.z >= rect.min.z)
                  && (p.x < rect.max.x)
                  && (p.y < rect.max.y)
                  && (p.z < rect.max.z));
    return result;
}

inline Rect3
rectCenterDim(v3 center, v3 dim)
{
    Rect3 result = rectCenterRadius(center, 0.5f*dim);
    return result;
}

inline Rect3
rectDim(v3 dim)
{
    Rect3 result = rectCenterDim(v3All(0), dim);
    return result;
}

inline Rect3
minkowskiGrow(Rect3 a, Rect3 b)
{
    Rect3 result;
    v3 bDiameter = b.max - b.min;
    v3 bRadius = 0.5f * bDiameter;
    result.min = a.min - bRadius;
    result.max = a.max + bRadius;
    return result;
}

inline v3
getRectCenter(Rect3 rect)
{
    v3 result;
    result = 0.5f*(rect.min + rect.max);
    return result;
}

inline v3
getRectRadius(Rect3 rect)
{
    v3 result;
    result = 0.5f * (rect.max - rect.min);
    return result;
}

inline b32
rectOverlap(Rect3 a, Rect3 b)
{
    b32 separate = ((a.max.x <= b.min.x) || (a.min.x >= b.max.x)
                    || (a.max.y <= b.min.y) || (a.min.y >= b.max.y)
                    || (a.max.z <= b.min.z) || (a.min.z >= b.max.z));
    return !separate;
}

inline b32
rectInside(Rect3 in, Rect3 out)
{
    b32 result = ((in.min >= out.min) && (in.max <= out.max));
    return result;
}

inline Rect3
addRadiusTo(Rect3 a, v3 radius)
{
    Rect3 result = Rect3{a.min - radius, a.max + radius};
    return result;
}

inline v3
getBarycentricCoordinate(Rect3 rect, v3 pos)
{
    v3 result;
    v3 dim = rect.max - rect.min;
    result.x = ((pos.x - rect.min.x) / dim.x);
    result.y = ((pos.y - rect.min.y) / dim.y);
    result.z = ((pos.z - rect.min.z) / dim.z);
    return result;
}
