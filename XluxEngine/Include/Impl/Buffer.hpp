#pragma once

#include "Core/Core.hpp"
#include "Impl/DeviceMemory.hpp"

namespace xlux
{
	class Device;

	class XLUX_API Buffer
	{
	public:
		void SetData(const void* data, Size size, Size offset = 0);
		void GetData(void* data, Size size, Size offset = 0);
		void BindMemory(RawPtr<DeviceMemory> deviceMemory, Size offset = 0);


		inline Bool IsUsable() const { return m_DeviceMemory != nullptr; }
		inline Size GetSize() const { return m_Size; }
		inline Size GetOffset() const { return m_Offset; }

		inline void* Map(Size size, Size offset = 0) { return m_DeviceMemory->Map(m_Offset + offset, size); }
		inline void Unmap() { m_DeviceMemory->Unmap(); }


		friend class Device;
		friend class VertexShaderWorker;
		friend class FragmentShaderWorker;
	private:
		Buffer(Size size);
		~Buffer();

		void* GetDataPtr() const;

	private:
		RawPtr<DeviceMemory> m_DeviceMemory = nullptr;
		Size m_Size = 0;
		Size m_Offset = 0;
	};

}
