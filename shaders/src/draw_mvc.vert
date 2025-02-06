#include <preamble.glsl>

vec2 verts[4] = vec2[](
  vec2(-1, -1),
  vec2(-1,  1),
  vec2( 1, -1),
  vec2( 1,  1)
);

// Stage output declarations
layout(location = 0) out vec2 out_value;

void main() {
  // Pull vertex from correct element in triangle strip
  vec2 vert = verts[gl_VertexID];

  // Set outputs; pass vertex position through, and set output to [-1, 1]
  out_value   = vert;
  gl_Position = vec4(vert * 2.f - 1.f, 0, 1);
}