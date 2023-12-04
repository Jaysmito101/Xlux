#include "Impl/Device.hpp"


namespace klux
{

	RawPtr<Device> Device::Create()
	{
		return new Device();
	}

	void Device::Destroy(RawPtr<Device> device)
	{
		delete device;
	}

	Device::Device()
	{
	}

	Device::~Device()
	{
		// destroy all pipelines
		for (I32 i = (I32)m_PipelineList.size() - 1; i >= 0; --i)
		{
			DestroyPipeline(m_PipelineList[i]);
		}

		// free all memory
		for (I32 i = (I32)m_AllocatedMemoryList.size() - 1; i >= 0; --i)
		{
			FreeMemory(m_AllocatedMemoryList[i]);
		}
	}

	RawPtr<DeviceMemory> Device::AllocateMemory(Size size)
	{
		auto memory = new DeviceMemory(size);
		m_AllocatedMemoryList.push_back(memory);
		return memory;
	}

	void Device::FreeMemory(RawPtr<DeviceMemory> memory)
	{
		auto it = std::find(m_AllocatedMemoryList.begin(), m_AllocatedMemoryList.end(), memory);
		if (it != m_AllocatedMemoryList.end())
		{
			m_AllocatedMemoryList.erase(it);
		}
		delete memory;
	}

	RawPtr<Pipeline> Device::CreatePipeline(const PipelineCreateInfo& createInfo)
	{
		auto pipeline = new Pipeline(createInfo);
		m_PipelineList.push_back(pipeline);
		return pipeline;
	}
	
	void Device::DestroyPipeline(RawPtr<Pipeline> pipeline)
	{
		auto it = std::find(m_PipelineList.begin(), m_PipelineList.end(), pipeline);
		if (it != m_PipelineList.end())
		{
			m_PipelineList.erase(it);
		}
		delete pipeline;
	}
}