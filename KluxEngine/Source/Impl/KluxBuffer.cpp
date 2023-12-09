#include "Impl/Buffer.hpp"


namespace klux
{


	Buffer::Buffer(Size size)
		: m_Size(size)
	{
	}

	Buffer::~Buffer()
	{
	}

	void Buffer::SetData(const void* data, Size size, Size offset)
	{
		if (m_DeviceMemory == nullptr)
		{
			klux::log::Error("Buffer is not usable");
		}

		if (offset + size > m_Size)
		{
			klux::log::Error("Invalid offset or size");
		}

		auto devData = m_DeviceMemory->Map(m_Offset + offset, size);

		std::memcpy(devData, data, size);

		m_DeviceMemory->Unmap();

	}

	void Buffer::GetData(void* data, Size size, Size offset)
	{
		if (m_DeviceMemory == nullptr)
		{
			klux::log::Error("Buffer is not usable");
		}

		if (offset + size > m_Size)
		{
			klux::log::Error("Invalid offset or size");
		}

		auto devData = m_DeviceMemory->Map(m_Offset + offset, size);

		std::memcpy(data, devData, size);

		m_DeviceMemory->Unmap();
	}

	void Buffer::BindMemory(RawPtr<DeviceMemory> deviceMemory, Size offset)
	{
		if (m_DeviceMemory != nullptr)
		{
			klux::log::Error("Buffer is already bound to memory");
		}

		if (offset + m_Size > deviceMemory->GetSize())
		{
			klux::log::Error("Invalid offset or size");
		}

		m_DeviceMemory = deviceMemory;
		m_Offset = offset;
	}

	void* Buffer::GetDataPtr() const
	{
		return m_DeviceMemory->Map(m_Offset, m_Size);
	}


}