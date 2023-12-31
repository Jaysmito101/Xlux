#include "Core/LinearAllocator.hpp"
#include "Core/Logger.hpp"

namespace xlux
{

	LinearAllocator::LinearAllocator(Size maxSize)
		: m_MaxSize(maxSize)
	{
		m_Data = new U8[maxSize];
		if (m_Data == nullptr)
		{
			xlux::log::Error("Failed to allocate memory");
		}
	}

	LinearAllocator::~LinearAllocator()
	{
		delete[] m_Data;
	}

	RawPtr<U8> LinearAllocator::Allocate(Size size)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		if (m_CurrentOffset + size > m_MaxSize)
		{
			xlux::log::Error("LinearAllocator is full");
		}

		auto ptr = m_Data + m_CurrentOffset;
		m_CurrentOffset += size;

		return ptr;
	}

	void LinearAllocator::Reset()
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		m_AllocationID++;
		m_CurrentOffset = 0;
	}

}