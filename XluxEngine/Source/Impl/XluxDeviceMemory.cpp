#include "Impl/DeviceMemory.hpp"

namespace xlux
{

	DeviceMemory::DeviceMemory(Size size)
		: m_Size(size)
	{
		m_Data = new U8[size];
	}

	DeviceMemory::~DeviceMemory()
	{
		delete[] m_Data;
	}

	RawPtr<U8> DeviceMemory::Map(Size offset, Size size)
	{
		(void) size;
		return m_Data + offset;
	}

	void DeviceMemory::Unmap()
	{
		return;
	}

}