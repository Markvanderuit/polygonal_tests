#include <preamble.glsl>

// Stage output declarations
layout(location = 0) in  vec2 in_vert;
layout(location = 0) out vec2 out_vert;

void main() {
  out_vert = in_vert;
  gl_Position = vec4(in_vert * 2.f - 1.f, 0, 1);
}