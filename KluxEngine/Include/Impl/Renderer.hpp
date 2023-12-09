#pragma once

#include "Core/Core.hpp"
#include "Core/ThreadPool.hpp"
#include "Math/Math.hpp"

namespace klux
{
	class IFramebuffer;
	class Device;
	class Buffer;
	class Pipeline;

	struct Viewport
	{
		I32 x = 0;
		I32 y = 0;
		I32 width = 0;
		I32 height = 0;
	};

	struct FrameClearWorkerInput
	{
		U32 x = 0;
		U32 y = 0;
		U32 width = 0;
		U32 height = 0;
	};

	struct VertexShaderWorkerInput
	{
		I32 indexStart = 0;
	};

	struct FragmentShaderWorkerInput
	{
		math::Vec3 barycentric;
	};

	class KLUX_API Renderer
	{
	public:

		void BeginFrame();
		void EndFrame();

		void BindFramebuffer(RawPtr<IFramebuffer> fbo);
		void BindPipeline(RawPtr<Pipeline> pipeline);
		void Clear(Bool color = true, Bool depth = true);
		void SetViewport(I32 x, I32 y, I32 width, I32 height);

		void DrawIndexed(RawPtr<Buffer> vertexBuffer, RawPtr<Buffer> indexBuffer, U32 indexCount);


		
		inline void SetClearColor(F32 r, F32 g, F32 b, F32 a) { m_ClearColor = { r, g, b, a }; }

		friend class Device;
	private:
		Renderer();
		~Renderer();

	private:
		Bool m_IsInFrame = false;
		math::Vec4 m_ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		RawPtr<IFramebuffer> m_ActiveFramebuffer = nullptr;
		RawPtr<Pipeline> m_ActivePipeline = nullptr;
		std::optional<Viewport> m_ActiveViewport;
		RawPtr<ThreadPool<8, FrameClearWorkerInput, U32>> m_FrameClearThreadPool;
		RawPtr<IJob<FrameClearWorkerInput, U32>> m_FrameClearJob;

		RawPtr<ThreadPool<8, VertexShaderWorkerInput, U32>> m_VertexShaderThreadPool;
		RawPtr<IJob<VertexShaderWorkerInput, U32>> m_VertexShaderJob;

		RawPtr<LinearAllocator> m_VertexToFragmentDataAllocator;
	};
}