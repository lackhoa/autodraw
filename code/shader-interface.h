#pragma once
#import <simd/simd.h>

struct VertexInput {
  simd_float2 position [[attribute(0)]];
  simd_float2 uv       [[attribute(1)]];
  int         type     [[attribute(2)]];
} __attribute__((packed));
