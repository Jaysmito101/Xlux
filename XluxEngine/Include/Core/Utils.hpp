#pragma once
#include "Core/Types.hpp"
#include "Math/Math.hpp"

namespace xlux
{
	namespace utils
	{
		XLUX_API String GetExecutablePath();
		XLUX_API String GetExecutableDirectory();

		XLUX_API List<U8> ReadBinaryFie(const String& filepath);

		XLUX_API F32 GetTime();

		XLUX_API Bool IsPointInTriangle(const math::Vec2& p, const math::Vec2& a, const math::Vec2& b, const math::Vec2& c);

	}
}