#pragma once
// #import <simd/simd.h>
// TODO: Can I include my math library in here? That'd be kinda sick!

// These types ship directly to the metal shaders
typedef __attribute__((__ext_vector_type__(2))) float simd_float2;
typedef __attribute__((__ext_vector_type__(3))) float simd_float3;
typedef __attribute__((__ext_vector_type__(4))) float simd_float4;

struct VertexInput {
  simd_float2 position [[attribute(0)]];
  simd_float2 uv       [[attribute(1)]];
  simd_float4 color    [[attribute(2)]];  // tint color (todo: could be packed)
} __attribute__((packed));
