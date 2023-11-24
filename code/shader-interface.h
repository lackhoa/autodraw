#pragma once
#import <simd/simd.h>

struct VertexInput {
  simd_float2 position [[attribute(0)]];
  int         type     [[attribute(1)]];
  simd_float2 uv       [[attribute(2)]];        
};
