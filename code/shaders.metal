#import <metal_stdlib>
#import "shader-interface.h"

using namespace metal;

struct VertexOutput
{
  float4 position [[position]];
  float2 uv;
  float3 color;
};

vertex VertexOutput vert(VertexInput in [[stage_in]])
{
  VertexOutput out;
  out.position = float4(in.position, 0.0, 1.f);
  out.color    = in.color;
  out.uv       = in.uv;
  return out;
}

fragment float4 frag(VertexOutput in [[stage_in]],
                     texture2d<float> tex [[texture(0)]],
                     sampler sam [[sampler(0)]])
{
  float4 out = tex.sample(sam, in.uv);
  out.r *= in.color.r;
  out.g *= in.color.g;
  out.b *= in.color.b;
  return out;
}
