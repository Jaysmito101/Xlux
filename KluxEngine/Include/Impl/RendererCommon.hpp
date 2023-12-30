#pragma once

#include "Core/Core.hpp"
#include "Core/ThreadPool.hpp"
#include "Math/Math.hpp"
#include "Impl/Buffer.hpp"

namespace klux
{

	class IFramebuffer;
	class Device;
	class Buffer;
	class Pipeline;

	struct Viewport
	{
		I32 x = 0;
		I32 y = 0;
		I32 width = 0;
		I32 height = 0;
	};


}