#pragma once

#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"

#include "Math/VectorSIMD.hpp"
#include "Math/MatrixSIMD.hpp"

namespace xlux
{

	namespace math
	{
		using namespace math_normal;
		//using namespace math_simd;
		

		constexpr F32 PI = 3.14159265358979323846f;

		XLUX_FORCE_INLINE F32 ToRadians(F32 degrees)
		{
			return degrees * (F32)PI / 180.0f;
		}
	}
}
