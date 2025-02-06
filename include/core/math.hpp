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

// Extensions to Eigen's existing classes are inserted through header files
#ifndef EIGEN_ARRAYBASE_PLUGIN
  #define EIGEN_ARRAYBASE_PLUGIN  "ext/eigen_arraybase.ext"
#endif

#ifndef EIGEN_MATRIXBASE_PLUGIN
  #define EIGEN_MATRIXBASE_PLUGIN "ext/eigen_matrixbase.ext"
#endif

#ifndef EIGEN_ARRAY_PLUGIN
  #define EIGEN_ARRAY_PLUGIN "ext/eigen_array.ext"
#endif

#ifndef EIGEN_MATRIX_PLUGIN
  #define EIGEN_MATRIX_PLUGIN "ext/eigen_matrix.ext"
#endif

// Include eigen headers after plugin extensions are specified
#include <Eigen/Dense>
#include <Eigen/Geometry>

namespace prg {
  // Introduce shorthands in the 'prg' namespace
  using byte   = std::byte;
  using uint   = unsigned int;
  using schar  = signed   char;
  using uchar  = unsigned char;
  using ushort = unsigned short;

  // Introduce 'eig' namespace shorthand in 'prg' namespace
  namespace eig = Eigen;
} // namespace prg

namespace Eigen {
  namespace dtl {
    // Manual alignment of 2->8, 3->16, 4->16
    template <size_t D> constexpr size_t vector_align() {
      return D >= 3 ? 16
           : D == 2 ? 8
           : 4;
    }
  } // namespace dtl

  // Introduce forcibly aligned Eigen::AlArray type
  template <class Type, size_t Size>
  class alignas(dtl::vector_align<Size>()) AlArray 
  : public Array<Type, Size, 1> {
    using Base = Array<Type, Size, 1>;

  public:
    using Base::Base;

    AlArray() : Base() 
    { }

    // This constructor allows you to construct MyVectorType from Eigen expressions
    template <typename Other>
    AlArray(const ArrayBase<Other>& o)
    : Base(o)
    { }

    template<typename Other>
    AlArray& operator=(const ArrayBase <Other>& o)
    {
      this->Base::operator=(o);
      return *this;
    }
  };

  // Introduce forcibly aligned Eigen::AlVector type
  template <class Type, size_t Size>
  class alignas(dtl::vector_align<Size>()) AlVector 
  : public Vector<Type, Size> {
    using Base = Vector<Type, Size>;

  public:
    AlVector() : Base() 
    { }

    // This constructor allows you to construct MyVectorType from Eigen expressions
    template <typename Other>
    AlVector(const MatrixBase<Other>& o)
    : Base(o)
    { }

    template<typename Other>
    AlVector& operator=(const MatrixBase <Other>& o)
    {
      this->Base::operator=(o);
      return *this;
    }
  };

  /* Define some useful conversions */
  
  // Sourced from glm, actuallly
  inline
  Projective3f ortho(float left, float right, float bottom, float top, float near, float far) {
    const float _00 =   2.f / (right - left);
    const float _11 =   2.f / (top - bottom);
    const float _22 =   2.f / (far - near);
    const float _30 = - (right + left) / (right - left);
    const float _31 = - (top + bottom) / (top - bottom);
    const float _32 = - (far + near)   / (far - near);

    Projective3f trf;
    trf.matrix() << _00,   0,   0, _30,
                      0, _11,   0, _31,
                      0,   0, _22, _32,
                      0,   0,   0,   1;
    return trf;
  }

  // Convert a screen-space vector in [0, 1] to world space
  inline
  Vector3f screen_to_world_space(const Vector2f      &v,
                                 const Projective3f  &mat) {
    Array2f v_ = (v.array() - 0.5f) * 2.f;
    Array4f trf = mat.inverse() * (Vector4f() << v_, 0, 1).finished();
    return trf.head<3>() / trf[3];
  }

  inline
  Vector2f window_to_screen_space(const Array2f &v,      // window-space vector
                                  const Array2f &offs,   // window offset
                                  const Array2f &size) { // window size
    auto v_ = ((v - offs) / size).eval();
    return { v_.x(), 1.f - v_.y() };
  }

  inline
  Vector2u window_to_pixel(const Array2f &v,      // window-space vector
                           const Array2f &offs,   // window offset
                           const Array2f &size) { // window size
    auto v_ = (v - offs).cast<unsigned>().eval();
    return { v_.x(), size.y() - 1 - v_.y() };
  }

  // Convert a world-space vector to screen space in [0, 1]
  inline
  Vector2f world_to_screen_space(const Vector3f     &v,     // world-space vector
                                 const Projective3f &mat) { // camera view/proj matrix
    Array4f trf = mat * (Vector4f() << v, 1).finished();
    return trf.head<2>() / trf.w() * .5f + .5f;
  }

  // Convert a screen-space vector in [0, 1] to window space
  inline
  Vector2f screen_to_window_space(const Array2f &v,      // screen-space vector
                                  const Array2f &offs,   // window offset
                                  const Array2f &size) { // window size
    return offs + size * Array2f(v.x(), 1.f - v.y());
  }

  // Convert a world-space vector to window space
  inline
  Vector2f world_to_window_space(const Vector3f     &v,      // world-space vector
                                 const Projective3f &mat,    // camera view/proj matrix
                                 const Vector2f     &offs,   // window offset
                                 const Vector2f     &size) { // window size
    return screen_to_window_space(world_to_screen_space(v, mat), offs, size);
  }
  
  /* Define useful integer types */

  using Array1us = Array<unsigned short, 1, 1>;
  using Array2us = Array<unsigned short, 2, 1>;
  using Array3us = Array<unsigned short, 3, 1>;
  using Array4us = Array<unsigned short, 4, 1>;
  
  using Array1s = Array<short, 1, 1>;
  using Array2s = Array<short, 2, 1>;
  using Array3s = Array<short, 3, 1>;
  using Array4s = Array<short, 4, 1>;

  using Array1u = Array<unsigned int, 1, 1>;
  using Array2u = Array<unsigned int, 2, 1>;
  using Array3u = Array<unsigned int, 3, 1>;
  using Array4u = Array<unsigned int, 4, 1>;

  using Vector1u = Matrix<unsigned int, 1, 1>;
  using Vector2u = Matrix<unsigned int, 2, 1>;
  using Vector3u = Matrix<unsigned int, 3, 1>;
  using Vector4u = Matrix<unsigned int, 4, 1>;

  /* Define common aligned vector types */
  
  using AlArray3s  = AlArray<short, 3>;
  using AlArray3us = AlArray<unsigned short, 3>;
  using AlArray3u  = AlArray<unsigned int, 3>;
  using AlArray3i  = AlArray<int, 3>;
  using AlArray3f  = AlArray<float, 3>;
  using AlVector3f = AlVector<float, 3>;
  
  /* Define (rarely) useful 1-component types */

  using Array1i  = Array<int, 1, 1>;
  using Array1u  = Array<unsigned int, 1, 1>;
  using Array1s  = Array<short, 1, 1>;
  using Array1us = Array<unsigned short, 1, 1>;
  using Array1f  = Array<float, 1, 1>;
} // namespace Eigen
