#pragma once

#include "Core/Types.hpp"

#include <thread>
#include <mutex>
#include <cstdlib>

namespace klux
{

	class LinearAllocator
	{
	public:
		LinearAllocator(Size maxSize);
		~LinearAllocator();

		RawPtr<U8> Allocate(Size size);
		void Reset();

	private:
		Size m_MaxSize = 0;
		Size m_CurrentOffset = 0;
		RawPtr<U8> m_Data = nullptr; 
		std::mutex m_Mutex;
	};

}