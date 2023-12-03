#pragma once
#include "Core/Core.hpp"

#include "Impl/DeviceMemory.hpp"

namespace klux
{
	class KLUX_API Device
	{
	public:
		static RawPtr<Device> Create();
		static void Destroy(RawPtr<Device> device);

		RawPtr<DeviceMemory> AllocateMemory(Size size);
		void FreeMemory(RawPtr<DeviceMemory> memory);

	private:
		Device();
		~Device();

	private:
		List<RawPtr<DeviceMemory>> m_AllocatedMemoryList;
	};
}