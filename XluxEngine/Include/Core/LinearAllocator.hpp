#pragma once

#include "Core/Types.hpp"

#include <thread>
#include <mutex>
#include <cstdlib>

namespace xlux
{

	class LinearAllocator
	{
	public:
		LinearAllocator(Size maxSize, Size threadCount);
		~LinearAllocator();

		RawPtr<U8> Allocate(Size size, Size threadId);
		void Reset();
		
		inline Bool IsAllocationIdValid(Size id) const { return id == m_AllocationID;}
		inline Size GetAllocationId() const { return m_AllocationID; }

	private:
		Size m_MaxSize = 0;
		Array<Size, 1024> m_CurrentOffset = {};
		Size m_AllocationID = 0;
		RawPtr<U8> m_Data = nullptr; 
		std::mutex m_Mutex;
	};

}