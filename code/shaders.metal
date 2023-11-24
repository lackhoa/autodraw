#import <metal_stdlib>
#import "shader-interface.h"

using namespace metal;

struct VertexOutput
{
  float4 position [[position]];
  int    type;
  float2 uv;
};

vertex VertexOutput vert(VertexInput in [[stage_in]])
{
  VertexOutput out;
  out.position = float4(in.position, 0.0, 1.f);
  out.type     = in.type;
  out.uv       = in.uv;
  return out;
}

fragment float4 frag(VertexOutput in [[stage_in]],
                     texture2d<float> color_texture [[texture(0)]],
                     sampler sam [[sampler(0)]])
{
  if (in.type == 0) {
    return float4(0,.5f,.5f,1);  // background
  } else if (in.type == 1) {
    return float4(1,0,0,1);  // cursor
  } else {
    return color_texture.sample(sam, in.uv);  // textured
  }
}
