#import <metal_stdlib>
#import "shader-interface.h"

using namespace metal;

struct VertexOutput
{
  float4 position [[position]];
  float2 uv;
};

vertex VertexOutput vert(VertexInput in [[stage_in]])
{
  VertexOutput out;
  out.position = float4(in.position, 0.0, 1.f);
  out.uv       = in.uv;
  return out;
}

fragment float4 frag(VertexOutput in [[stage_in]],
                     texture2d<float> tex0 [[texture(0)]],
                     sampler sam [[sampler(0)]])
{
  return tex0.sample(sam, in.uv);  // textured
}
