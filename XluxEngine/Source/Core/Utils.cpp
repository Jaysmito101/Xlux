#include "Core/Utils.hpp"
#include "Core/Core.hpp"

namespace xlux
{
	namespace utils
	{
		XLUX_API String GetExecutablePath()
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

		XLUX_API String GetExecutableDirectory()
		{
			String path = GetExecutablePath();
			return path.substr(0, path.find_last_of("\\/"));
		}

		XLUX_API List<U8> ReadBinaryFie(const String& filepath)
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

		XLUX_API F32 GetTime()
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

		XLUX_API Bool IsPointInTriangle(const math::Vec2& p, const math::Vec2& a, const math::Vec2& b, const math::Vec2& c)
		{
			// Compute vectors        
			math::Vec2 v0 = c - a;
			math::Vec2 v1 = b - a;
			math::Vec2 v2 = p - a;

			// Compute dot products
			F32 dot00 = v0.Dot(v0);
			F32 dot01 = v0.Dot(v1);
			F32 dot02 = v0.Dot(v2);
			F32 dot11 = v1.Dot(v1);
			F32 dot12 = v1.Dot(v2);

			// Compute barycentric coordinates
			F32 invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
			F32 u = (dot11 * dot02 - dot01 * dot12) * invDenom;
			F32 v = (dot00 * dot12 - dot01 * dot02) * invDenom;

			// Check if point is in triangle
			return (u >= 0) && (v >= 0) && (u + v < 1);
		}

	}
}