#include <preamble.glsl>

// Stage output declarations
layout(location = 0) in  vec2 in_vert;
layout(location = 1) in  vec3 in_colr;
layout(location = 0) out vec3 out_colr;

// Uniform buffer declarations
layout(binding = 0) uniform b_buffer_settings {
  mat4 projection;
  bool draw_exterior;
  uint draw_method;
} settings;

void main() {
  out_colr = in_colr;
  gl_Position = settings.projection * vec4(in_vert * 2.f - 1.f , 0, 1);
}