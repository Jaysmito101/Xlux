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

		Bool Execute(VertexShaderWorkerInput payload, U32& result) override;

		inline void SetVertexBuffer(RawPtr<Buffer> buffer) { m_VertexBuffer = buffer; }
		inline void SetIndexBuffer(RawPtr<Buffer> buffer) { m_IndexBuffer = buffer; }
		inline void SetPipeline(RawPtr<Pipeline> pipeline) { m_Pipeline = pipeline; }
		inline void SetVertexToFragmentDataAllocator(RawPtr<LinearAllocator> allocator) { m_VertexToFragmentDataAllocator = allocator; }
		inline void SetFramebuffer(RawPtr<IFramebuffer> framebuffer) { m_Framebuffer = framebuffer; }
		inline void SetRasterizer(std::function<Bool(ShaderTriangleRef)> rasterizer) { m_Rasterizer = rasterizer; }

	private:
		List<ShaderTriangleRef> ClipTrianglesAgainstPlane(const List<ShaderTriangleRef>& triangles, const math::Vec3& planeNormal, const math::Vec3& planePoint);
		List<ShaderTriangleRef> ClipTriangleAgainstPlane(const ShaderTriangleRef& triangle, const math::Vec3& planeNormal, const math::Vec3& planePoint);
		Pair<math::Vec3, F32> PlaneIntersection(const math::Vec3& planeNormal, const math::Vec3& planePoint, const math::Vec3& lineStart, const math::Vec3& lineEnd);

	private:
		RawPtr<Buffer> m_VertexBuffer = nullptr;
		RawPtr<Buffer> m_IndexBuffer = nullptr;
		RawPtr<Pipeline> m_Pipeline = nullptr;
		RawPtr<IFramebuffer> m_Framebuffer = nullptr;
		RawPtr<LinearAllocator> m_VertexToFragmentDataAllocator = nullptr;

		std::function<Bool(ShaderTriangleRef)> m_Rasterizer;
	};



}