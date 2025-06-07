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
	};


	class VertexShaderWorker : public IJob<VertexShaderWorkerInput, U32>
	{
	public:
		VertexShaderWorker() = default;
		~VertexShaderWorker() = default;

		Bool Execute(VertexShaderWorkerInput payload, U32& result, Size threadID) override;

		inline void SetVertexBuffer(RawPtr<Buffer> buffer, Size startingVertex) { m_VertexBuffer = buffer; m_StartingVertex = startingVertex; }
		inline void SetIndexBuffer(RawPtr<Buffer> buffer, Size startingIndex) { m_IndexBuffer = buffer; m_StartingIndex = startingIndex; }
		inline void SetPipeline(RawPtr<Pipeline> pipeline) { m_Pipeline = pipeline; }
		inline void SetVertexToFragmentDataAllocator(RawPtr<LinearAllocator> allocator) { m_VertexToFragmentDataAllocator = allocator; }
		inline void SetFramebuffer(RawPtr<IFramebuffer> framebuffer) { m_Framebuffer = framebuffer; }
		inline void SetRasterizer(std::function<Bool(ShaderTriangleRef)> rasterizer) { m_Rasterizer = rasterizer; }

	private:
		Size ClipTrianglesAgainstPlane(const ShaderTriangleRef* triangles, Size tianglesCountIn, const math::Vec3& planeNormal, const math::Vec3& planePoint, Size threadID, ShaderTriangleRef* result);
		Size ClipTriangleAgainstPlane(const ShaderTriangleRef& triangle, const math::Vec3& planeNormal, const math::Vec3& planePoint, Size threadID, Size tianglesCount, ShaderTriangleRef* result);
		Pair<math::Vec3, F32> PlaneIntersection(const math::Vec3& planeNormal, const math::Vec3& planePoint, const math::Vec3& lineStart, const math::Vec3& lineEnd);
		Bool IsTriangleFacingCamera(const math::Vec4& v0, const math::Vec4& v1, const math::Vec4& v2);

	private:
		Size m_StartingVertex = 0;
		Size m_StartingIndex = 0;

		RawPtr<Buffer> m_VertexBuffer = nullptr;
		RawPtr<Buffer> m_IndexBuffer = nullptr;
		

		RawPtr<Pipeline> m_Pipeline = nullptr;
		RawPtr<IFramebuffer> m_Framebuffer = nullptr;
		RawPtr<LinearAllocator> m_VertexToFragmentDataAllocator = nullptr;

		std::function<Bool(ShaderTriangleRef)> m_Rasterizer;
	};



}