#pragma once
#include "Core/Types.hpp"
#include "Math/Math.hpp"

namespace klux
{
	namespace utils
	{
		KLUX_API String GetExecutablePath();
		KLUX_API String GetExecutableDirectory();

		KLUX_API List<U8> ReadBinaryFie(const String& filepath);

		KLUX_API F32 GetTime();

		KLUX_API Bool IsPointInTriangle(const math::Vec2& p, const math::Vec2& a, const math::Vec2& b, const math::Vec2& c);

	}
}