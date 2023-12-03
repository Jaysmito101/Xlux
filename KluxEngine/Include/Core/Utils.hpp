#pragma once
#include "Core/Types.hpp"

namespace klux
{
	namespace utils
	{
		KLUX_API String GetExecutablePath();
		KLUX_API String GetExecutableDirectory();

		KLUX_API List<U8> ReadBinaryFie(const String& filepath);
	}
}