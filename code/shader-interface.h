#pragma once
#import <simd/simd.h>

struct VertexInput {
  simd_float2 position [[attribute(0)]];
  simd_float2 uv       [[attribute(1)]];
  simd_float3 color    [[attribute(2)]];  // tint color (todo: could be packed)
} __attribute__((packed));
