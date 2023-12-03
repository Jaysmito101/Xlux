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

		KLUX_API F32 GetTime()
		{
            #if defined(PLATFORM_WINDOWS)
			static LARGE_INTEGER frequency;
			static BOOL use_qpc = QueryPerformanceFrequency(&frequency);
			if (use_qpc)
			{
				static LONGLONG startTime = 0;
				LARGE_INTEGER now;
				QueryPerformanceCounter(&now);
				if (startTime == 0) startTime = now.QuadPart;
				// return (F32)((double)now.QuadPart / (double)frequency.QuadPart);
				return (F32)((F64)(now.QuadPart - startTime) / (F64)frequency.QuadPart);
			}
			else
			{
				static ULONGLONG startTime = 0;
				if (startTime == 0) startTime = GetTickCount64();
				return (F32) ((F64)(GetTickCount64() - startTime) / 1000.0);
			}
			#else
			static timespec startTime = { 0, 0 };

			clock_gettime(CLOCK_MONOTONIC, &startTime);
			timespec now;
			clock_gettime(CLOCK_MONOTONIC, &now);
			if (startTime.tv_sec == 0 && startTime.tv_nsec == 0) startTime = now;
			return (F32)(now.tv_sec - startTime.tv_sec) + (F32)(now.tv_nsec - startTime.tv_nsec) / 1000000000.0f;

			#endif
		}

	}
}