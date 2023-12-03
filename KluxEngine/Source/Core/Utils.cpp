#include "Core/Utils.hpp"
#include "Core/Core.hpp"

namespace klux
{
	namespace utils
	{
		KLUX_API String GetExecutablePath()
		{
#if defined(PLATFORM_WINDOWS)
			static CHAR buffer[2048];
			::GetModuleFileNameA(NULL, buffer, 2048);
			return String(buffer);
#else
			static I8 buffer[2048];
			readlink("/proc/self/exe", buffer, 2048);
			return String(buffer);
#endif
		}

		KLUX_API String GetExecutableDirectory()
		{
			String path = GetExecutablePath();
			return path.substr(0, path.find_last_of("\\/"));
		}

		KLUX_API List<U8> ReadBinaryFie(const String& filepath)
		{
			List<U8> result;

            std::ifstream file;
			file.open(filepath, std::ios::binary | std::ios::ate);
			if (file.is_open())
			{
				std::streampos size = file.tellg();
				result.resize(size);
				file.seekg(0, std::ios::beg);
				file.read(reinterpret_cast<char*>(result.data()), size);
				file.close();
			}
			else
			{
				log::Error("Failed to open file '{}'", filepath);
			}
			return result;

		}

	}
}