// Copyright (c) 2025 Mark van de Ruit, Delft University of Technology

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <core/math.hpp>

#define IM_VEC2_CLASS_EXTRA                                    \
  ImVec2(const prg::eig::Vector2f &v) : x(v.x()), y(v.y()) { } \
  ImVec2(const prg::eig::Vector2i &v) : x(v.x()), y(v.y()) { } \
  ImVec2(const prg::eig::Array2f &v)  : x(v.x()), y(v.y()) { } \
  ImVec2(const prg::eig::Array2i &v)  : x(v.x()), y(v.y()) { } \
  operator prg::eig::Vector2f() const { return { x, y }; }     \
  operator prg::eig::Vector2i() const { return {               \
    static_cast<int>(x), static_cast<int>(y) }; }              \
  operator prg::eig::Array2f() const { return { x, y }; }      \
  operator prg::eig::Array2i() const { return {                \
    static_cast<int>(x), static_cast<int>(y) }; }

#define IM_VEC4_CLASS_EXTRA                                                        \
  ImVec4(const prg::eig::Vector4f &v) : x(v.x()), y(v.y()), z(v.z()), w(v.w()) { } \
  ImVec4(const prg::eig::Vector4i &v) : x(v.x()), y(v.y()), z(v.z()), w(v.w()) { } \
  ImVec4(const prg::eig::Array4f &v)  : x(v.x()), y(v.y()), z(v.z()), w(v.w()) { } \
  ImVec4(const prg::eig::Array4i &v)  : x(v.x()), y(v.y()), z(v.z()), w(v.w()) { } \
  operator prg::eig::Vector4f() const { return { x, y, z, w }; }                   \
  operator prg::eig::Vector4i() const { return {                                   \
    static_cast<int>(x), static_cast<int>(y),                                      \
    static_cast<int>(z), static_cast<int>(w) }; }                                  \
  operator prg::eig::Array4f() const { return { x, y, z, w }; }                    \
  operator prg::eig::Array4i() const { return {                                    \
    static_cast<int>(x), static_cast<int>(y),                                      \
    static_cast<int>(z), static_cast<int>(w) }; }

#include <core/math.hpp>
#include <small_gl/fwd.hpp>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

namespace ImGui {
  template <typename T>
  constexpr void * to_ptr(T t) { 
    return (void *) static_cast<size_t>(t);
  }

  void Initialize(const gl::Window &window);
  void Destroy();
  void BeginFrame();
  void DrawFrame();

  class Gizmo2D {
    using trf = Eigen::Affine3f;

    bool m_is_active = false;
    trf  m_init_trf;
    trf  m_delta_trf;

  public:
    // Begin/eval/end functions, s.t. eval() returns a delta transform applied to the current
    // transform over every frame, and the user can detect changes
    bool begin_delta(const gl::Window &window, trf init_trf);
    std::pair<bool, trf> 
         eval_delta();
    bool end_delta();

    // Eval function, s.t. the current_trf variable is modified over every frame
    // void eval(trf &current_trf);

    // Helpers
    bool is_over() const;                                  // True if a active gizmo is moused over
    bool is_active() const { return m_is_active; }         // Whether guizmo input is handled, ergo if begin_delta() was called
    void set_active(bool active) { m_is_active = active; }
  };
}