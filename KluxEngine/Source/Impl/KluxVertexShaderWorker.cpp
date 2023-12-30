#include "Impl/VertexShaderWorker.hpp"
#include "Impl/DeviceMemory.hpp"
#include "Impl/Buffer.hpp"
#include "Impl/Pipeline.hpp"
#include "Impl/Shader.hpp"
#include "Impl/Framebuffer.hpp"

namespace klux
{
	Bool VertexShaderWorker::Execute(VertexShaderWorkerInput payload, U32& result)
	{
		(void)result;

		const auto indexBufferPtr = m_IndexBuffer->GetDataPtr();
		const auto vertexBufferPtr = m_VertexBuffer->GetDataPtr();

		const U32 triangleIndex[3] = {
			((U32*)indexBufferPtr)[payload.indexStart + 0],
			((U32*)indexBufferPtr)[payload.indexStart + 1],
			((U32*)indexBufferPtr)[payload.indexStart + 2]
		};

		void* vertexData[3] = {
			&((U8*)vertexBufferPtr)[triangleIndex[0] * m_Pipeline->m_CreateInfo.vertexItemSize],
			&((U8*)vertexBufferPtr)[triangleIndex[1] * m_Pipeline->m_CreateInfo.vertexItemSize],
			&((U8*)vertexBufferPtr)[triangleIndex[2] * m_Pipeline->m_CreateInfo.vertexItemSize]
		};


		auto seedTraingle = ShaderTriangleRef(m_VertexToFragmentDataAllocator, m_Pipeline->m_CreateInfo.vertexToFragmentDataSize);

		// Vertex Shader Call
		for (auto i = 0; i < 3; ++i)
		{
			seedTraingle.GetBuiltInRef(i)->Reset();
			seedTraingle.GetBuiltInRef(i)->VertexIndex = payload.indexStart * 3 + i;
			m_Pipeline->m_CreateInfo.vertexShader->Execute(vertexData[i], seedTraingle.GetVertexData(i), seedTraingle.GetBuiltInRef(i));
			seedTraingle.GetBuiltInRef(i)->Position /= seedTraingle.GetBuiltInRef(i)->Position[3];
		}

		// TODO: Cull
		// TODO: Clip

		// Transform NDC to Screen space
		for (auto i = 0; i < 3; ++i)
		{
			auto& position = seedTraingle.GetBuiltInRef(i)->Position;
			position = position * 0.5f + math::Vec4(0.5f);

			position[0] = position[0] * m_Framebuffer->GetWidth();
			position[1] = position[1] * m_Framebuffer->GetHeight();
			position[3] = 1.0f;
		}

		// Rasterization
		// if (m_Rasterizer) // this will be true if everything is ok so just skip the check for performance
		{
			m_Rasterizer(seedTraingle);	
		}

		return false;
	}


}