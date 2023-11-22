#import <metal_stdlib>
#import "shader-interface.h"

using namespace metal;

struct VertexOutput
{
  float4 position [[position]];
  int    type;
};

vertex VertexOutput vert(VertexInput in [[stage_in]])
{
  VertexOutput out;
  out.position = float4(in.position, 0.0, 1.f);
  out.type     = in.type;
  return out;
}

fragment float4 frag(VertexOutput in [[stage_in]])
{
  if (in.type == 0)
    return float4(0,0,0,1);  // background
  else
    return float4(1,0,0,1);  // cursor
}
