#include "Impl/VertexShaderWorker.hpp"
#include "Impl/DeviceMemory.hpp"
#include "Impl/Buffer.hpp"
#include "Impl/Pipeline.hpp"
#include "Impl/Shader.hpp"
#include "Impl/Framebuffer.hpp"

namespace xlux
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
		
		// Clipping
		// auto triangles = List<ShaderTriangleRef> { seedTraingle };

		// normal, point on plane (these are the planes of the cube from {-1, -1, -1} to {1, 1, 1})
		/*
		const List<math::Vec3> clipPlanes = {
			math::Vec3(1.0f, 0.0f, 0.0f),
			math::Vec3(0.0f, 1.0f, 0.0f),
			math::Vec3(0.0f, 0.0f, 1.0f),
			math::Vec3(-1.0f, 0.0f, 0.0f),
			math::Vec3(0.0f, -1.0f, 0.0f),
			math::Vec3(0.0f, 0.0f, -1.0f)
		};
		*/
		//for (const auto& plane : clipPlanes) {
//			// triangles = ClipTrianglesAgainstPlane(triangles, plane, plane);
	//	}



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

	List<ShaderTriangleRef> VertexShaderWorker::ClipTrianglesAgainstPlane(const List<ShaderTriangleRef>& triangles, const math::Vec3& planeNormal, const math::Vec3& planePoint)
	{
		auto result = List<ShaderTriangleRef>();
		for (const auto& triangle : triangles)
		{
			auto clipped = ClipTriangleAgainstPlane(triangle, planeNormal, planePoint);
			result.insert(result.end(), clipped.begin(), clipped.end());
		}
		return result;
	}

	List<ShaderTriangleRef> VertexShaderWorker::ClipTriangleAgainstPlane(const ShaderTriangleRef& triangle, const math::Vec3& planeNormal, const math::Vec3& planePoint)
	{
		auto result = List<ShaderTriangleRef>();

		Array<Pair<math::Vec3, U32>, 3> insidePoints;
		Array<Pair<math::Vec3, U32>, 3> outsidePoints;
		auto insidePointsCount = 0;
		auto outsidePointsCount = 0;

		auto distance = [&](const math::Vec3& point) {
			return planeNormal.Dot(point - planePoint);
		};

		auto pushInsidePoint = [&](const math::Vec3& point, U32 index) {
			insidePoints[insidePointsCount++] = { point, index };
		};

		auto pushOutsidePoint = [&](const math::Vec3& point, U32 index) {
			outsidePoints[outsidePointsCount++] = { point, index };
		};

		auto pushPoint = [&](const math::Vec3& point, U32 index) {
			if (distance(point) <= 0.0f)
				pushInsidePoint(point, index);
			else
				pushOutsidePoint(point, index);
		};

		pushPoint(triangle.GetBuiltInRef(0)->Position, 0);
		pushPoint(triangle.GetBuiltInRef(1)->Position, 1);
		pushPoint(triangle.GetBuiltInRef(2)->Position, 2);

		

		xlux::log::Info("insidePointsCount: {}", insidePointsCount);
		xlux::log::Info("outsidePointsCount: {}", outsidePointsCount);

		// exit(0);

		return result;
	}


}