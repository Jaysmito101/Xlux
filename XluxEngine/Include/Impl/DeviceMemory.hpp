#pragma once
#include "Core/Core.hpp"

namespace xlux
{
	class Device;

	class XLUX_API DeviceMemory
	{
	public:
		RawPtr<U8> Map( Size offset, Size size );
		void Unmap();

		inline Size GetSize() const { return m_Size; }


		friend class Device;

	private:
		DeviceMemory(Size size);
		~DeviceMemory();


	private:
		RawPtr<U8> m_Data = nullptr;
		Size m_Size = 0;
	};
}