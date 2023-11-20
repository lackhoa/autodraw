struct VertexInput
{
  float2 position [[attribute(0)]];
  float4 color    [[attribute(1)]];
};

struct ShaderInOut
{
  float4 position [[position]];
  float4 color;
};

vertex ShaderInOut vert(VertexInput in [[stage_in]])
{
  ShaderInOut out;
  out.position = float4(in.position, 0.0, 1.f);
  out.color    = in.color;
  return out;
}

fragment float4 frag(ShaderInOut in [[stage_in]])
{
  return in.color;
}
