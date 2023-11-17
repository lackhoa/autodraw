#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

#define TAU 6.28318530718
uniform float uTime = 0.0;

float divisions = 5.0;
float angle = 0.0;

vec2 VectorRotateTime(vec2 v, float speed)
{
  float time = uTime * speed;
  float localTime = fract(time);

  if (false) {}
  else if (localTime < 0.25)                          angle = 0.0;
  else if ((localTime >= 0.25) && (localTime < 0.50)) angle = TAU/8*sin(TAU*localTime - TAU/4);
  else if ((localTime >= 0.50) && (localTime < 0.75)) angle = TAU/8;
  else if (localTime >= 0.75)                         angle = TAU/8*sin(TAU*localTime);

  v -= .5;  // todo what does this do?
  v =  mat2(cos(angle), -sin(angle), sin(angle), cos(angle)) * v;
  v += .5;

  return v;
}

float Rectangle(in vec2 st, in float size, in float fill)
{
  float roundSize = .5 - size / 2.f;
  float left = step(roundSize, st.x);
  float top = step(roundSize, st.y);
  float bottom = step(roundSize, 1.0 - st.y);
  float right = step(roundSize, 1.0 - st.x);

  return (left * bottom * right * top) * fill;
}

void main()
{
  vec2 fragPos = fragTexCoord;
  fragPos.xy += uTime / 9.f;

  fragPos *= divisions;
  vec2 ipos = floor(fragPos);
  vec2 fpos = fract(fragPos);

  // fpos = VectorRotateTime(fpos, .2);
  fpos = fpos;

  float alpha = Rectangle(fpos, .216, 1.f);
  vec3 color = vec3(.2, .5, .7);

  finalColor = vec4(color, alpha);
}
