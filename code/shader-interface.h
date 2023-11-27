#pragma once
#import <simd/simd.h>

struct VertexInput {
  simd_float2 position [[attribute(0)]];
  simd_float2 uv       [[attribute(1)]];
} __attribute__((packed));
