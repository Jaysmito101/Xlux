#pragma once

#include "Core/Core.hpp"
#include "Core/ThreadPool.hpp"
#include "Math/Math.hpp"
#include "Impl/RendererCommon.hpp"
#include "Impl/Shader.hpp"

namespace xlux
{


	struct VertexShaderWorkerInput
	{
		I32 indexStart = 0;
		void* userData = nullptr;

		Size startingVertex = 0;
		Size startingIndex = 0;

		RawPtr<Buffer> vertexBuffer = nullptr;
		RawPtr<Buffer> indexBuffer = nullptr;

		RawPtr<Pipeline> pipeline = nullptr;
		RawPtr<IFramebuffer> framebuffer = nullptr;

		std::function<Bool(ShaderTriangleRef)> rasterizer;
	};


	class VertexShaderWorker : public IJob<VertexShaderWorkerInput, U32>
	{
	public:
		VertexShaderWorker() = default;
		~VertexShaderWorker() = default;

		Bool Execute(VertexShaderWorkerInput payload, U32& result, Size threadID) override;

		inline void SetVertexToFragmentDataAllocator(RawPtr<LinearAllocator> allocator) { m_VertexToFragmentDataAllocator = allocator; }

	private:
		Size ClipTrianglesAgainstPlane(const ShaderTriangleRef* triangles, Size tianglesCountIn, const math::Vec3& planeNormal, const math::Vec3& planePoint, Size threadID, const VertexShaderWorkerInput& payload, ShaderTriangleRef* result);
		Size ClipTriangleAgainstPlane(const ShaderTriangleRef& triangle, const math::Vec3& planeNormal, const math::Vec3& planePoint, Size threadID, Size tianglesCount, const VertexShaderWorkerInput& payload, ShaderTriangleRef* result);
		Pair<math::Vec3, F32> PlaneIntersection(const math::Vec3& planeNormal, const math::Vec3& planePoint, const math::Vec3& lineStart, const math::Vec3& lineEnd);
		Bool IsTriangleFacingCamera(const math::Vec4& v0, const math::Vec4& v1, const math::Vec4& v2);

	private:
		RawPtr<LinearAllocator> m_VertexToFragmentDataAllocator = nullptr;
	};



}