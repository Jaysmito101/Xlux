#include "Core/LinearAllocator.hpp"
#include "Core/Logger.hpp"

namespace xlux
{

	LinearAllocator::LinearAllocator(Size maxSize, Size threadCount)
		: m_MaxSize(maxSize)
	{
		m_Data = new U8[maxSize * threadCount];
		if (m_Data == nullptr)
		{
			xlux::log::Error("Failed to allocate memory");
		}
	}

	LinearAllocator::~LinearAllocator()
	{
		delete[] m_Data;
	}

	RawPtr<U8> LinearAllocator::Allocate(Size size, Size threadId)
	{
		// std::lock_guard<std::mutex> lock(m_Mutex);
		auto& currentOffset = m_CurrentOffset[threadId];
		
		
		if (currentOffset + size > m_MaxSize)
		{
			xlux::log::Error("LinearAllocator is full");
		}
				
		auto ptr = m_Data + currentOffset + threadId * m_MaxSize;
		currentOffset += size;

		return ptr;
	}

	void LinearAllocator::Reset()
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		m_AllocationID++;
		for (auto& offset : m_CurrentOffset)
		{
			offset = 0;
		}
	}

}