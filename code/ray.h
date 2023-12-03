#pragma once

struct Material {
  v3 color;
};

struct Plane {
  v3  N;
  f32 d;
  u32 mat_index;
};

struct Sphere {
  v3  P;
  f32 r;
  u32 mat_index;
};

struct World {
  Material *materials;
  u32 material_count;

  u32 plane_count;
  Plane *planes;

  u32 sphere_count;
  Sphere *spheres;
};
