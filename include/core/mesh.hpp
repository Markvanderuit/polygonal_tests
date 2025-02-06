#pragma once

#include <core/math.hpp>
#include <core/utility.hpp>
#include <numeric>
#include <numbers>

namespace prg {
  namespace dtl {
    float outer_product(eig::Vector2f a, eig::Vector2f b) {
      return (a * b.transpose())(0,0);
    }

    eig::Vector3f get_barycentric_coords(eig::Vector2f a, eig::Vector2f b, eig::Vector2f c, eig::Vector2f p) {
      eig::Vector2f ab = b - a, ac = c - a;
      float a_tri = std::abs(.5f * outer_product(ac, ab));
      float a_ab  = std::abs(.5f * outer_product(p - a, ab));
      float a_ac  = std::abs(.5f * outer_product(ac, p  - a));
      float a_bc  = std::abs(.5f * outer_product(c - p, b - p));
      return (eig::Vector3f(a_bc, a_ac, a_ab) / a_tri).eval();
    }
    bool is_inside_triangle(eig::Vector2f a, eig::Vector2f b, eig::Vector2f c, eig::Vector2f p) {
      auto abc = get_barycentric_coords(a, b, c, p).array().eval();
      return (abc <= 1.0f).all() && (abc >= 0.f).all();

      // // First, edge vectors w.r.t. an arbitrary vertex
      // auto e0 = (c - a).eval();
      // auto e1 = (b - a).eval();
      // auto e2 = (p - a).eval();
      
      // // Next, dot products between respective angles
      // float d00 = e0.dot(e0);
      // float d01 = e0.dot(e1);
      // float d02 = e0.dot(e2);
      // float d11 = e1.dot(e1);
      // float d12 = e1.dot(e2);
      
      // // Then, compute barycentric coordinates, which should
      // // be partially zero or negative for p
      // float denom = d00 * d11 - d01 * d01;
      // if (std::abs(denom) < 1e-20)
      //   return true;
      // float rcp_denom = 1.f / denom;
      // float u = (d11 * d02 - d01 * d12) * rcp_denom;
      // float v = (d00 * d12 - d01 * d02) * rcp_denom;

      // // Check for boundedness
      // return u >= 0 && v >= 0 && (u + v <= 1);    
    }
  } // namespace dtl

  // Triangulate a polygon that is based solely on an ordered set of vertices,
  // and is possibly concave. Uses ear-clipping algorithm, and badly at that haha!
  inline
  std::vector<eig::Array3u> triangulate_polygon(std::span<const eig::Vector2f> verts) {
    // Establish initial index list of the polygon's exterior edgess
    std::vector<uint> elems_polygon(verts.size());
    std::iota(range_iter(elems_polygon), 0u);

    // Establish empty list of triangle connections, to be filled
    std::vector<eig::Array3u> elems_triangles;
    
    // Loop until the polygon description holds no triangles
    while (elems_polygon.size() > 2) {
      // Iterate to search for a valid, clippable triangle
      uint i_ = 0;
      for (; i_ < elems_polygon.size(); ++i_) {
        uint i_prev = (int(i_) - 1 + elems_polygon.size()) % elems_polygon.size();
        uint i_next = (int(i_) + 1) % elems_polygon.size();
        
        // Indices of previous, next, and their vertices
        uint i = elems_polygon[i_prev], j = elems_polygon[i_], k = elems_polygon[i_next];
        auto vert_i = verts[i], vert_j = verts[j], vert_k = verts[k];
        
        // Test local convexity of resulting triangle w.r.t CCW orientation
        /* {
          eig::Vector2f a = vert_i - vert_j, b = vert_k - vert_j;
          float angle = std::atan2(dtl::outer_product(b, a), b.dot(a));
          if (angle < 0.f)
            angle += 2.f * std::numbers::pi_v<float>;
          guard_continue(angle <= std::numbers::pi_v<float>);
        } */

        // Test potential overlap of other vertices inside resulting triangle
        {
          bool is_overlapping = false;
          for (uint l = 0; l < elems_polygon.size(); ++l) {
            guard_continue(l != i_ && l != i_prev && l != i_next);
            auto p = verts[elems_polygon[l]];
            if (dtl::is_inside_triangle(vert_i, vert_j, vert_k, p)) {
              is_overlapping = true;
              break;
            }
          }
          guard_continue(!is_overlapping);
        }

        // If triangle is an ear, clip it from the polygon
        elems_polygon.erase(elems_polygon.begin() + i_);
        elems_triangles.push_back({ i, j, k });
        break;
      }

      // If this was reached, no triangulation is available
      if (i_ == elems_polygon.size())
        return {};
    }

    return elems_triangles;
  }
} // namespace prg