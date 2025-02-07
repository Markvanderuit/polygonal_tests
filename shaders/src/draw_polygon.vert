#include <preamble.glsl>

// Stage declarations
layout(location = 0) in  vec2 in_vert;

// Uniform buffer declarations
layout(binding = 0) uniform b_buffer_settings {
  mat4 projection;
  bool draw_exterior;
  uint draw_method;
} settings;

void main() {
  gl_Position = settings.projection * vec4(in_vert * 2.f - 1.f , 0, 1);
}