#include <preamble.glsl>

// Stage output declarations
layout(location = 0) in  vec2 in_vert;
layout(location = 1) in  vec3 in_colr;
layout(location = 0) out vec3 out_colr;

void main() {
  out_colr = in_colr;
  gl_Position = vec4(in_vert * 2.f - 1.f, 0, 1);
}