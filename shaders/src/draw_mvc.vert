#include <preamble.glsl>

// Stage declarations
layout(location = 0) in  vec2 in_vert;
layout(location = 0) out vec2 out_value;

// Uniform buffer declarations
layout(binding = 0) uniform b_buffer_settings {
  mat4 projection;
  bool draw_lines;
  uint draw_method;
} settings;

void main() {
  out_value   = in_vert;
  gl_Position = settings.projection * vec4(in_vert * 2.f - 1.f , 0, 1);
}