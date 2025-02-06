#include <preamble.glsl>

// Stage input/output declarations
layout(location = 0) in  vec3 in_colr;
layout(location = 0) out vec4 out_colr;

void main() {
  out_colr = vec4(in_colr, 1);
}