#import <metal_stdlib>

using namespace metal;

struct VertexInput
{
  float2 position [[attribute(0)]];
  float2 uv       [[attribute(1)]];
};

struct ShaderOutput
{
  float4 position [[position]];
  float2 uv;
};

vertex ShaderOutput vert(VertexInput in [[stage_in]])
{
  ShaderOutput out;
  out.position = float4(in.position, 0.0, 1.f);
  out.uv       = in.uv;
  return out;
}

fragment float4 frag(ShaderOutput in [[stage_in]],
                     texture2d<float> colorTexture [[texture(0)]],
                     sampler sam [[sampler(0)]])
{
  return colorTexture.sample(sam, in.uv);
}
