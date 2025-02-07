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

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/compile.h>
#include <fmt/ranges.h>
#include <concepts>
#include <iterator>
#include <span>
#include <source_location>
#include <utility>

// Simple guard statement syntactic sugar
#define guard(expr,...)                if (!(expr)) { return __VA_ARGS__ ; }
#define guard_continue(expr)           if (!(expr)) { continue; }
#define guard_break(expr)              if (!(expr)) { break; }
#define guard_constexpr(expr,...)      if constexpr (!(expr)) { return __VA_ARGS__ ; }
#define guard_constexpr_continue(expr) if constexpr (!(expr)) { continue; }
#define guard_constexpr_break(expr)    if constexpr (!(expr)) { break; }

// Simple range-like syntactic sugar
#define range_iter(c)  c.begin(), c.end()
#define range_riter(c) c.rbegin(), c.rend()

// Utility debug shorthands
#if defined(NDEBUG) || defined(PRG_ENABLE_ASSERTIONS)
  #define prg_debug_insert(x)    x
  #define prg_debug_select(x, y) x
  #define prg_enable_debug       true
#else
  #define prg_debug_insert(x)
  #define prg_debug_select(x, y) y
  #define prg_enable_debug       false
#endif

namespace prg {
  // Interpret a sized contiguous container as a span of type T
  template <class T, class C>
  constexpr
  std::span<T> cnt_span(C &c) {
    auto data = c.data();
    guard(data, {});
    using value_type = typename C::value_type;
    return { reinterpret_cast<T*>(data), (c.size() * sizeof(value_type)) / sizeof(T) };
  }

  // Interpret an object as a span of type T
  template <class T, class O>
  constexpr
  std::span<T> obj_span(O &o) {
    return { reinterpret_cast<T*>(&o), sizeof(O) / sizeof(T) };
  }

  // Interpret a span of U to a span of type T
  template <class T, class U>
  std::span<T> cast_span(std::span<U> s) {
    auto data = s.data();
    guard(data, {});
    return { reinterpret_cast<T*>(data), s.size_bytes() / sizeof(T) };
  }

  // Take a pair of integers, cast to same type, and do a ceiling divide
  template <typename T, typename T_>
  constexpr inline T ceil_div(T n, T_ div) {
    return (n + static_cast<T>(div) - T(1)) / static_cast<T>(div);
  }

  // Helpers for debug utility
  namespace dtl {
    // Message buffer which accepts keyed strings
    class Message {
      std::string _buffer;

    public:
      void put(std::string_view key, std::string_view message) {
        fmt::format_to(std::back_inserter(_buffer),
                      FMT_COMPILE("  {:<8} : {}\n"), 
                      key, 
                      message);
      }
      
      std::string get() const {
        return _buffer;
      }
    };

    // Exception class which accepts keyed strings, which are
    // output in the order in which they were provided.
    class Exception : public std::exception, public Message {
      mutable std::string _what;

    public:
      const char * what() const noexcept override {
        _what = fmt::format("Exception thrown\n{}", get());
        return _what.c_str();
      }
    };
  } // namespace dtl

  // Debug utility
  namespace dbg {
    // Evaluate a boolean expression, throwing a detailed exception pointing
    // to the expression's origin if said expression fails
    // Note: can be removed on release builds
#if defined(NDEBUG) || defined(MET_ENABLE_EXCEPTIONS)
    inline
    void check_expr(bool expr,
                    const std::string_view &msg = "",
                    const std::source_location sl = std::source_location::current()) {
      guard(!expr);

      dtl::Exception e;
      e.put("src", "dbg::check_expr(...) failed, expression evaluated to false");
      e.put("message", msg);
      e.put("in file", fmt::format("{}({}:{})", sl.file_name(), sl.line(), sl.column()));
      throw e;
    }
#else
  #define check_expr(expr, msg, sl)
#endif
  } // namespace dbg
} // namespace prg