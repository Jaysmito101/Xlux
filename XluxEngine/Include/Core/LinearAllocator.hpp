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
		LinearAllocator(Size maxSize);
		~LinearAllocator();

		RawPtr<U8> Allocate(Size size);
		void Reset();
		
		inline Bool IsAllocationIdValid(Size id) const { return id == m_AllocationID;}
		inline Size GetAllocationId() const { return m_AllocationID; }

	private:
		Size m_MaxSize = 0;
		Size m_CurrentOffset = 0;
		Size m_AllocationID = 0;
		RawPtr<U8> m_Data = nullptr; 
		std::mutex m_Mutex;
	};

}