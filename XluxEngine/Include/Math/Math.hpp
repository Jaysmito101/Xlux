#pragma once

#include "Core/Types.hpp"
#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"

// #include "Math/VectorSIMD.hpp"
// #include "Math/MatrixSIMD.hpp"

namespace xlux {

namespace math {
using namespace math_normal;
// using namespace math_simd;

struct PackedColor {
  U32 rgba;
  constexpr XLUX_FORCE_INLINE PackedColor() : rgba(0) {}
  constexpr XLUX_FORCE_INLINE PackedColor(U32 rgba) : rgba(rgba) {}
  constexpr XLUX_FORCE_INLINE PackedColor(U8 r, U8 g, U8 b, U8 a) {
    rgba = (static_cast<U32>(r) << 24) | (static_cast<U32>(g) << 16) |
           (static_cast<U32>(b) << 8) | static_cast<U32>(a);
  }
  XLUX_FORCE_INLINE PackedColor(const math::Vec4& color) {
    U8 r = static_cast<U8>(std::clamp(color[0] * 255.0f, 0.0f, 255.0f));
    U8 g = static_cast<U8>(std::clamp(color[1] * 255.0f, 0.0f, 255.0f));
    U8 b = static_cast<U8>(std::clamp(color[2] * 255.0f, 0.0f, 255.0f));
    U8 a = static_cast<U8>(std::clamp(color[3] * 255.0f, 0.0f, 255.0f));
    rgba = (static_cast<U32>(r) << 24) | (static_cast<U32>(g) << 16) |
           (static_cast<U32>(b) << 8) | static_cast<U32>(a);
  }

  XLUX_FORCE_INLINE math::Vec4 ToVec4() const {
    U8 r = (rgba >> 24) & 0xFF;
    U8 g = (rgba >> 16) & 0xFF;
    U8 b = (rgba >> 8) & 0xFF;
    U8 a = rgba & 0xFF;
    return math::Vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
  }

  constexpr U32 ToRGBA() const { return rgba; }
};

constexpr F32 PI = 3.14159265358979323846f;

XLUX_FORCE_INLINE F32 ToRadians(F32 degrees) {
  return degrees * (F32)PI / 180.0f;
}
}  // namespace math
}  // namespace xlux
