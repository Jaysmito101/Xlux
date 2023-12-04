#pragma once
#include "Core/Core.hpp"

#include "Impl/DeviceMemory.hpp"
#include "Impl/Pipeline.hpp"

namespace klux
{
	class KLUX_API Device
	{
	public:
		static RawPtr<Device> Create();
		static void Destroy(RawPtr<Device> device);

		RawPtr<DeviceMemory> AllocateMemory(Size size);
		void FreeMemory(RawPtr<DeviceMemory> memory);

		RawPtr<Pipeline> CreatePipeline(const PipelineCreateInfo& createInfo);
		void DestroyPipeline(RawPtr<Pipeline> pipeline);

	private:
		Device();
		~Device();

	private:
		List<RawPtr<DeviceMemory>> m_AllocatedMemoryList;
		List<RawPtr<Pipeline>> m_PipelineList;
	};
}