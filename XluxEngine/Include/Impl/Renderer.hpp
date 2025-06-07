#pragma once

#include "Core/Core.hpp"
#include "Core/ThreadPool.hpp"
#include "Math/Math.hpp"

#include "Impl/RendererCommon.hpp"
#include "Impl/VertexShaderWorker.hpp"
#include "Impl/FragmentShaderWorker.hpp"
#include "Impl/FrameClearWorker.hpp"

namespace xlux
{

	class XLUX_API Renderer
	{
	public:

		void BeginFrame();
		void EndFrame();

		void BindFramebuffer(RawPtr<IFramebuffer> fbo);
		void BindPipeline(RawPtr<Pipeline> pipeline);
		void Clear(Bool color = true, Bool depth = true);
		void SetViewport(I32 x, I32 y, I32 width, I32 height);

		void DrawIndexed(RawPtr<Buffer> vertexBuffer, RawPtr<Buffer> indexBuffer, U32 indexCount, U32 startingVertex = 0, U32 startingIndex = 0);
		
		inline void SetClearColor(F32 r, F32 g, F32 b, F32 a) { m_ClearColor = { r, g, b, a }; }

		friend class Device;
	private:
		Renderer();
		~Renderer();

		Bool PassTriangleToFragmentShader(ShaderTriangleRef triangle);

	private:
		Bool m_IsInFrame = false;
		math::Vec4 m_ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		RawPtr<IFramebuffer> m_ActiveFramebuffer = nullptr;
		RawPtr<Pipeline> m_ActivePipeline = nullptr;
		std::optional<Viewport> m_ActiveViewport;


		static constexpr U32 k_FragmentShaderWorkerCountX = 8;
		static constexpr U32 k_FragmentShaderWorkerCountY = 6;
		static constexpr U32 k_VertexShaderWorkerCount = 8;
		static constexpr U32 k_FrameClearWorkerCount = 8;
		U32 m_FragmentShaderTileWidth = 0;
		U32 m_FragmentShaderTileHeight = 0;

		RawPtr<ThreadPool<k_FrameClearWorkerCount, FrameClearWorkerInput, U32>> m_FrameClearThreadPool;
		RawPtr<IJob<FrameClearWorkerInput, U32>> m_FrameClearJob;

		RawPtr<ThreadPool<k_VertexShaderWorkerCount, VertexShaderWorkerInput, U32>> m_VertexShaderThreadPool;
		RawPtr<IJob<VertexShaderWorkerInput, U32>> m_VertexShaderJob;

		RawPtr<ThreadPool<k_FragmentShaderWorkerCountX * k_FragmentShaderWorkerCountY, FragmentShaderWorkerInput, U32>> m_FragmentShaderThreadPool;
		RawPtr<IJob<FragmentShaderWorkerInput, U32>> m_FragmentShaderJob;

		RawPtr<LinearAllocator> m_VertexToFragmentDataAllocator;
	};
}