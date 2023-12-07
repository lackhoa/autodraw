#pragma once

#include "kv-utils.h"

// #include <string.h>  // todo importing a string library just to zero out memory?

// inline void
// zeroMemory(void *destination, size_t length)
// {
//     memset(destination, 0, length);
// }

// inline void
// copyMemory(void *dst, void *src, size_t size)
// {
//     memcpy(dst, src, size);
// }

inline i32
absoslute(i32 in)
{
    return ((in >= 0) ? in : -in);
}

inline f32
square(f32 x)
{
    f32 result = x*x;
    return result;
}

inline f32
squareRoot(f32 x)
{
#if COMPILER_MSVC
    f32 result = sqrtf(x);
#else
    f32 result = __builtin_sqrtf(x);
#endif
    return result;
}

inline u32
roundF32ToU32(f32 Real32)
{
#if COMPILER_MSVC
    u32 Result = (u32)roundf(Real32);
#else
    u32 Result = (u32)__builtin_roundf(Real32);
#endif
    return(Result);
}

inline f32
roundF32(f32 Real32)
{
#if COMPILER_MSVC
    f32 Result = roundf(Real32);
#else
    f32 Result = __builtin_roundf(Real32);
#endif
    return(Result);
}

// todo I don't like this, just do the round myself
// inline i32
// roundF32ToI32(f32 Real32)
// {
// #if COMPILER_MSVC
//     i32 Result = (i32)roundf(Real32);
// #else
//     i32 Result = (i32)__builtin_roundf(Real32);
// #endif
//     return(Result);
// }

inline i32
floorF32ToI32(f32 Real32)
{
#if COMPILER_MSVC
    i32 Result = (i32)floorf(Real32);
#else
    i32 Result = (i32)__builtin_floorf(Real32);
#endif
    return(Result);
}

inline i32
ceilF32ToI32(f32 Real32)
{
#if COMPILER_MSVC
    i32 Result = (i32)ceilf(Real32);
#else
    i32 Result = (i32)__builtin_ceilf(Real32);
#endif
    return(Result);
}

// NOTE: weird names to avoid name collision (haizz)
inline f32
kv_sin(f32 angle)
{
#if COMPILER_MSVC
    f32 result = sinf(angle);
#else
    f32 result = __builtin_sinf(angle);
#endif
    return(result);
}

inline f32
kv_cos(f32 angle)
{
#if COMPILER_MSVC
    f32 result = cosf(angle);
#else
    f32 result = __builtin_cosf(angle);
#endif
    return(result);
}

inline f32
kv_atan2(f32 y, f32 x)
{
#if COMPILER_MSVC
    f32 result = atan2f(y, x);
#else
    f32 result = __builtin_atan2f(y, x);
#endif
    return(result);
}

struct bit_scan_result
{
    b32 found;
    u32 index;
};

inline bit_scan_result
findLeastSignificantSetBit(u32 mask)
{
    bit_scan_result result = {};

#if COMPILER_MSVC
    result.found = _BitScanForward((unsigned long *)&result.index, mask);
#elif COMPILER_LLVM
    if (mask != 0)
    {
        result.found = true;
        result.index = __builtin_ctz(mask);
    }
#else
        for (u32 index = 0;
             index < 32;
             index++)
        {
            if((mask & (1 << index)) != 0)
            {
                result.found = true;
                result.index = index;
                return result;
            }
        }
#endif

    return result;
}


inline f32
absolute(f32 x)
{
#if COMPILER_MSVC
    f32 result = (f32)fabs(x);
#else
    f32 result = (f32)__builtin_fabs(x);
#endif
    return result;
}

inline u32
rotateLeft(u32 value, i32 rotateAmount)
{
#if COMPILER_MSVC
    u32 result = _rotl(value, rotateAmount);
#elif COMPILER_LLVM
    u32 result = __builtin_rotateleft32(value, rotateAmount);
#else
    i32 r = rotateAmount & 31;
    u32 result = (value << r) | (value >> (32 - r));
#endif
    return result;
}

inline u32
rotateRight(u32 value, i32 rotateAmount)
{
#if COMPILER_MSVC
    u32 result = _rotr(value, rotateAmount);
#elif COMPILER_LLVM
    u32 result = __builtin_rotateright32(value, rotateAmount);
#else
    i32 r = rotateAmount & 31;
    u32 result = (value >> r) | (value << (32 - r));
#endif
    return result;
}

#if HANDMADE_WIN32
#  define readBarrier  _ReadBarrier()
#  define writeBarrier _WriteBarrier()
#  define atomicCompareExchange _InterlockedCompareExchange
#else
// todo: Other platforms
#endif
