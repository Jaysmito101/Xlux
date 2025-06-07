#include "Impl/VertexShaderWorker.hpp"
#include "Impl/DeviceMemory.hpp"
#include "Impl/Buffer.hpp"
#include "Impl/Pipeline.hpp"
#include "Impl/Interpolator.hpp"
#include "Impl/Shader.hpp"
#include "Impl/Framebuffer.hpp"

namespace xlux
{
	Bool VertexShaderWorker::Execute(VertexShaderWorkerInput payload, U32& result, Size threadID)
	{
		(void)result;

		const auto indexBufferPtr = m_IndexBuffer->GetDataPtrWithOffset(m_StartingIndex * sizeof(U32));
		const auto vertexBufferPtr = m_VertexBuffer->GetDataPtrWithOffset(m_StartingIndex * m_Pipeline->m_CreateInfo.vertexItemSize);

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


		auto seedTraingle = ShaderTriangleRef(m_VertexToFragmentDataAllocator, m_Pipeline->m_CreateInfo.vertexToFragmentDataSize, threadID);

		// Vertex Shader Call
		for (auto i = 0; i < 3; ++i)
		{
			seedTraingle.GetBuiltInRef(i)->Reset();
			seedTraingle.GetBuiltInRef(i)->VertexIndex = payload.indexStart * 3 + i;
			m_Pipeline->m_CreateInfo.vertexShader->Execute(vertexData[i], seedTraingle.GetVertexData(i), seedTraingle.GetBuiltInRef(i));
			seedTraingle.GetBuiltInRef(i)->Position /= seedTraingle.GetBuiltInRef(i)->Position[3];
		}

		// Backface Culling
		if (m_Pipeline->m_CreateInfo.enableBackfaceCulling)
		{
			if (!IsTriangleFacingCamera(seedTraingle.GetBuiltInRef(0)->Position, seedTraingle.GetBuiltInRef(1)->Position, seedTraingle.GetBuiltInRef(2)->Position))
			{
				return false;
			}
		}

		// Clipping
		// auto triangles = List<ShaderTriangleRef>{ seedTraingle };
		ShaderTriangleRef trianglesBuffer[2][16];
		auto triangleBufferIndex = 0;
		auto triangles = trianglesBuffer[triangleBufferIndex];
		triangles[0] = seedTraingle;
		Size triangleCount = 1;

		// The way the fragment shader has been implemented, this renderer
		// doesnt really need to clip triangles as it can automatically
		// discard fragments that are outside the viewport.
		// However, clipping is still implemented for the sake of completeness.
		if (m_Pipeline->m_CreateInfo.enableClipping)
		{

			// normal, point on plane (these are the planes of the cube from {-1, -1, -1} to {1, 1, 1})
			static const Array<math::Vec3, 6> clipPlanes = {
				math::Vec3(1.0f, 0.0f, 0.0f),
				math::Vec3(0.0f, 1.0f, 0.0f),
				math::Vec3(0.0f, 0.0f, 1.0f),
				math::Vec3(-1.0f, 0.0f, 0.0f),
				math::Vec3(0.0f, -1.0f, 0.0f),
				math::Vec3(0.0f, 0.0f, -1.0f)
			};

			for (const auto& plane : clipPlanes)
			{
				// triangles = ClipTrianglesAgainstPlane(triangles, plane, plane, threadID);
				triangleCount = ClipTrianglesAgainstPlane(trianglesBuffer[triangleBufferIndex], triangleCount, plane, plane, threadID, trianglesBuffer[1 - triangleBufferIndex]);
				triangleBufferIndex = 1 - triangleBufferIndex;
			}
		}

		triangles = trianglesBuffer[triangleBufferIndex];


		// Rasterization
		// if (m_Rasterizer) // this will be true if everything is ok so just skip the check for performance
		{
			// for (auto& triangle : triangles)
			for (auto ti = 0 ; ti < triangleCount; ++ti)
			{
				auto& triangle = triangles[ti];
				// Transform NDC to Screen space
				for (auto i = 0; i < 3; ++i)
				{
					auto& position = triangle.GetBuiltInRef(i)->Position;
					//position = position / position[3];
					position = position * 0.5f + math::Vec4(0.5f);

					position[0] = position[0] * m_Framebuffer->GetWidth();
					position[1] = position[1] * m_Framebuffer->GetHeight();
					position[3] = 1.0f;

				}
				m_Rasterizer(triangle);
			}
		}

		return false;
	}

	Size VertexShaderWorker::ClipTrianglesAgainstPlane(const ShaderTriangleRef* triangles, Size tianglesCountIn, const math::Vec3& planeNormal, const math::Vec3& planePoint, Size threadID, ShaderTriangleRef* result)
	{
		Size tianglesCountOut = 0;
		// for (const auto& triangle : triangles)
		for (auto i = 0; i < tianglesCountIn; ++i)
		{
			tianglesCountOut = ClipTriangleAgainstPlane(triangles[i], planeNormal, planePoint, threadID, tianglesCountOut, result);
		}
		return tianglesCountOut;
	}


	Size VertexShaderWorker::ClipTriangleAgainstPlane(const ShaderTriangleRef& triangle, const math::Vec3& planeNormal, const math::Vec3& planePoint, Size threadID, Size tianglesCount, ShaderTriangleRef* result)
	{
		//auto result = List<ShaderTriangleRef>();
		auto intepolator = m_Pipeline->m_CreateInfo.interpolator;

		Array<Pair<math::Vec4, U32>, 3> insidePoints;
		Array<Pair<math::Vec4, U32>, 3> outsidePoints;
		auto insidePointsCount = 0;
		auto outsidePointsCount = 0;

		auto distance = [&](const math::Vec3& point) {
			return planeNormal.Dot(point - planePoint);
		};

		auto pushInsidePoint = [&](const math::Vec4& point, U32 index) {
			insidePoints[insidePointsCount++] = { point, index };
		};

		auto pushOutsidePoint = [&](const math::Vec4& point, U32 index) {
			outsidePoints[outsidePointsCount++] = { point, index };
		};

		auto pushPoint = [&](const math::Vec4& point, U32 index) {
			if (distance(point) <= 0.0f)
				pushInsidePoint(point, index);
			else
				pushOutsidePoint(point, index);
		};

		pushPoint(triangle.GetBuiltInRef(0)->Position, 0);
		pushPoint(triangle.GetBuiltInRef(1)->Position, 1);
		pushPoint(triangle.GetBuiltInRef(2)->Position, 2);


		if (insidePointsCount == 3)
		{
			// result.push_back(triangle);
			result[tianglesCount++] = triangle;
		}
		else if (insidePointsCount == 2)
		{
			auto& insidePoint0 = insidePoints[0];
			auto& insidePoint1 = insidePoints[1];
			auto& outsidePoint0 = outsidePoints[0];

			auto [interSectionPoint0, t0] = PlaneIntersection(planeNormal, planePoint, math::Vec3(insidePoint0.x), math::Vec3(outsidePoint0.x));
			auto [interSectionPoint1, t1] = PlaneIntersection(planeNormal, planePoint, math::Vec3(insidePoint1.x), math::Vec3(outsidePoint0.x));

			auto triangle0 = ShaderTriangleRef(m_VertexToFragmentDataAllocator, m_Pipeline->m_CreateInfo.vertexToFragmentDataSize, threadID);
			auto triangle1 = ShaderTriangleRef(m_VertexToFragmentDataAllocator, m_Pipeline->m_CreateInfo.vertexToFragmentDataSize, threadID);

			{

				std::memcpy(triangle0.GetVertexData(0), triangle.GetVertexData(insidePoint0.y), m_Pipeline->m_CreateInfo.vertexToFragmentDataSize);
				std::memcpy(triangle0.GetVertexData(1), triangle.GetVertexData(insidePoint1.y), m_Pipeline->m_CreateInfo.vertexToFragmentDataSize);

				intepolator->Reset(triangle0.GetVertexData(2));
				intepolator->ScaleAndAdd(triangle0.GetVertexData(2), triangle.GetVertexData(insidePoint1.y), 1.0f - t1);
				intepolator->ScaleAndAdd(triangle0.GetVertexData(2), triangle.GetVertexData(outsidePoint0.y), t1);

				triangle0.GetBuiltInRef(0)->Position = insidePoint0.x;
				triangle0.GetBuiltInRef(1)->Position = insidePoint1.x;
				triangle0.GetBuiltInRef(2)->Position = math::Vec4(interSectionPoint1, 1.0f);

				//result.push_back(triangle0);
				result[tianglesCount++] = triangle0;
			}

			{

				std::memcpy(triangle1.GetVertexData(0), triangle.GetVertexData(insidePoint0.y), m_Pipeline->m_CreateInfo.vertexToFragmentDataSize);
				std::memcpy(triangle1.GetVertexData(1), triangle0.GetVertexData(2), m_Pipeline->m_CreateInfo.vertexToFragmentDataSize);

				intepolator->Reset(triangle1.GetVertexData(2));
				intepolator->ScaleAndAdd(triangle1.GetVertexData(2), triangle.GetVertexData(insidePoint0.y), 1.0f - t0);
				intepolator->ScaleAndAdd(triangle1.GetVertexData(2), triangle.GetVertexData(outsidePoint0.y), t0);

				triangle1.GetBuiltInRef(0)->Position = insidePoint0.x;
				triangle1.GetBuiltInRef(1)->Position = math::Vec4(interSectionPoint1, 1.0f);
				triangle1.GetBuiltInRef(2)->Position = math::Vec4(interSectionPoint0, 1.0f);

				// result.push_back(triangle1);
				result[tianglesCount++] = triangle1;
			}
		}
		else if (insidePointsCount == 1)
		{
			auto& insidePoint0 = insidePoints[0];
			auto& outsidePoint0 = outsidePoints[0];
			auto& outsidePoint1 = outsidePoints[1];

			auto [interSectionPoint0, t0] = PlaneIntersection(planeNormal, planePoint, math::Vec3(insidePoint0.x), math::Vec3(outsidePoint0.x));
			auto [interSectionPoint1, t1] = PlaneIntersection(planeNormal, planePoint, math::Vec3(insidePoint0.x), math::Vec3(outsidePoint1.x));

			auto triangle0 = ShaderTriangleRef(m_VertexToFragmentDataAllocator, m_Pipeline->m_CreateInfo.vertexToFragmentDataSize, threadID);

			std::memcpy(triangle0.GetVertexData(0), triangle.GetVertexData(insidePoint0.y), m_Pipeline->m_CreateInfo.vertexToFragmentDataSize);

			intepolator->Reset(triangle0.GetVertexData(1));
			intepolator->ScaleAndAdd(triangle0.GetVertexData(1), triangle.GetVertexData(insidePoint0.y), 1.0f - t0);
			intepolator->ScaleAndAdd(triangle0.GetVertexData(1), triangle.GetVertexData(outsidePoint0.y), t0);

			intepolator->Reset(triangle0.GetVertexData(2));
			intepolator->ScaleAndAdd(triangle0.GetVertexData(2), triangle.GetVertexData(insidePoint0.y), 1.0f - t1);
			intepolator->ScaleAndAdd(triangle0.GetVertexData(2), triangle.GetVertexData(outsidePoint1.y), t1);


			triangle0.GetBuiltInRef(0)->Position = insidePoint0.x;
			triangle0.GetBuiltInRef(1)->Position = math::Vec4(interSectionPoint0, 1.0f);
			triangle0.GetBuiltInRef(2)->Position = math::Vec4(interSectionPoint1, 1.0f);

			// result.push_back(triangle0);
			result[tianglesCount++] = triangle0;
		}
		else
		{
			// all points are outside so no triangle is generated
		}

		return tianglesCount;
	}

	Pair<math::Vec3, F32> VertexShaderWorker::PlaneIntersection(const math::Vec3& planeNormal, const math::Vec3& planePoint, const math::Vec3& lineStart, const math::Vec3& lineEnd)
	{
		auto planeDistance = planeNormal.Dot(planePoint - lineStart);
		auto lineDistance = planeNormal.Dot(lineEnd - lineStart);
		auto t = planeDistance / lineDistance;
		return { lineStart + (lineEnd - lineStart) * t, t };
	}

	Bool VertexShaderWorker::IsTriangleFacingCamera(const math::Vec4& v0, const math::Vec4& v1, const math::Vec4& v2)
	{
		auto v0v1 = math::Vec3(v1 - v0);
		auto v0v2 = math::Vec3(v2 - v0);

		/*
		auto normal = v0v1.Cross(v0v2);
		static const auto cameraToTriangle = math::Vec3(0.0f, 0.0f, -1.0f);
		return normal.Dot(cameraToTriangle) < 0.0f;
		*/

		auto sign = (v0v1[0] * v0v2[1] - v0v1[1] * v0v2[0]);
		return sign >= 0.0f;
	}


}