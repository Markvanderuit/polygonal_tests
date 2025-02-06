#include <preamble.glsl>

// Buffer layout declarations
layout(std140) uniform;
layout(std430) buffer;

// Storage buffer declarations; we'll be doing vertex pulling
layout(binding = 0) restrict readonly buffer b_buffer_verts {
  vec2 data[];
} verts;
layout(binding = 1) restrict readonly buffer b_buffer_colrs {
  vec3 data[];
} colrs;

// Uniform buffer declarations
layout(binding = 0) uniform b_buffer_settings {
  bool draw_exterior;
} settings;

// Stage input/output declarations
layout(location = 0) in  vec2 in_value;
layout(location = 0) out vec4 out_value;


float cross_2d(vec2 a, vec2 b) {
    // return cross(vec3(a, 0), vec3(b, 0)).z;
    return a.x * b.y - a.y * b.x;
}

float get_angle(vec2 a, vec2 b) {
    return atan(cross_2d(a, b), dot(a, b));
}

// Hardcoded maximum nr. of weights
const uint N_MAX = 8u;

float[N_MAX] mvc(vec2 p) {
    // Output weights and cumulative sum used to normnalize
    float[N_MAX] weights;
    float weights_sum = 0.0;

    // Actual polytope size is buffer size
    const uint n = verts.data.length();

    // Iterate polytope vertices
    for (int i = 0; i < n; i++) {
      // Get current vertex, as well as prev/next vertices in polytope
      vec2 vt      = verts.data[i];
      vec2 vt_prev = verts.data[(i - 1 + n) % n];
      vec2 vt_next = verts.data[(i + 1    ) % n];

      // Floater's paper, listing 2.1
      /* {
        // Get unit vector (p -> vt), store length
        vec2  vt_dir  = vt - p;
        float vt_norm = distance(vt, p);
        vt_dir /= vt_norm;

        // Get angles between (p -> vt) and (p -> prev/next)
        float angle_prev = acos(dot(vt_dir, normalize(vt_prev - p)));
        float angle_next = acos(dot(vt_dir, normalize(vt_next - p)));

        // Compute w_i 
        float t_prev = tan(angle_prev * .5f);
        float t_next = tan(angle_next * .5f);
        weights[i] = (t_prev + t_next) / vt_norm;
      } */

      // Alternatively, Hormannm's paper, section 4's first listing
      {
        // Compute necessary vectors from p to prev, center, next
        vec2 dir_vt   = vt - p;
        vec2 dir_next = vt_next - p;
        vec2 dir_prev = vt_prev - p;
        float vt_norm = length(dir_vt);

        // Compute areas of parallelograms (prev, center) and (center, next)
        float A_prev = cross_2d(dir_vt, dir_prev);
        float A_next = cross_2d(dir_next, dir_vt);
        
        // Compute w_i
        float t_prev = .5f * (vt_norm * length(dir_prev) - dot(dir_vt, dir_prev)) / A_prev;
        float t_next = .5f * (vt_norm * length(dir_next) - dot(dir_vt, dir_next)) / A_next;
        weights[i] = (t_prev + t_next) / vt_norm;
      }
      
      // Cumulative sum
      weights_sum += weights[i];
    }

    // Listing 2.1, normalize w_i over sum of w_j
    float rcp = 1.f / weights_sum;
    for (uint i = 0; i < n; ++i)
      weights[i] *= rcp;

    // Set unused weights to 0
    for (uint i = n; i < N_MAX; ++i)
      weights[i] = 0.f;

    return weights;
}

vec3 mvc_colr(in vec2 p) {
  float[N_MAX] weights = mvc(p);
  const uint N = verts.data.length();

  vec3 colr = vec3(0);

  for (uint i = 0; i < N; ++i) {
    // Catch negative weights as being exterior
    if (weights[i] < 0 && !settings.draw_exterior) {
      colr = vec3(0);
      return colr;
    }
    
    // Grid lines, bad hack
    // if (fract(weights[i] * 30) < 0.1)
      colr += weights[i] * colrs.data[i];
  }

  return colr;
}

void main() {
  out_value = vec4(mvc_colr(in_value), 1);
}