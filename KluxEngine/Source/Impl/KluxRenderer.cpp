#include "Impl/Renderer.hpp"
#include "Impl/DeviceMemory.hpp"
#include "Impl/Buffer.hpp"
#include "Impl/Pipeline.hpp"
#include "Impl/Shader.hpp"
#include "Impl/Framebuffer.hpp"

namespace klux
{

	class FrameClearWorker : public IJob<FrameClearWorkerInput, U32>
	{
	public:
		FrameClearWorker() = default;
		~FrameClearWorker() = default;

		Bool Execute(const FrameClearWorkerInput& payload, U32& result) override
		{
			(void)result;
			const auto maxX = std::min(payload.x + payload.width, static_cast<U32>(m_Framebuffer->GetWidth()));
			const auto maxY = std::min(payload.y + payload.height, static_cast<U32>(m_Framebuffer->GetHeight()));

			if (m_ClearColor)
			{
				for (auto ch = 0; ch < (I32)m_Framebuffer->GetColorAttachmentCount(); ++ch)
				{
					for (auto y = payload.y; y < maxY; ++y)
					{
						for (auto x = payload.x; x < maxX; ++x)
						{
							m_Framebuffer->SetColorPixel(ch, x, y, m_Color[0], m_Color[1], m_Color[2], m_Color[3]);
						}
					}
				}
			}

			if (m_ClearDepth && m_Framebuffer->HasDepthAttachment())
			{
				for (auto y = payload.y; y < maxY; ++y)
				{
					for (auto x = payload.x; x < maxX; ++x)
					{
						m_Framebuffer->SetDepthPixel(x, y, 10000000.0f);
					}
				}
			}


			return false;
		}

		inline void SetFramebuffer(RawPtr<IFramebuffer> fbo)
		{
			m_Framebuffer = fbo;
		}

		inline void SetClearColor(const math::Vec4& color)
		{
			m_Color = color;
		}

		inline void SetEnableClearDepth(Bool clearDepth)
		{
			m_ClearDepth = clearDepth;
		}

		inline void SetEnableClearColor(Bool clearColor)
		{
			m_ClearColor = clearColor;
		}

	private:
		RawPtr<IFramebuffer> m_Framebuffer = nullptr;
		Bool m_ClearColor = true;
		Bool m_ClearDepth = true;
		math::Vec4 m_Color = { 0.0f, 0.0f, 0.0f, 1.0f };
	};

	class VertexShaderWorker : public IJob<VertexShaderWorkerInput, U32>
	{
	public:
		VertexShaderWorker() = default;
		~VertexShaderWorker() = default;

		Bool Execute(const VertexShaderWorkerInput& payload, U32& result) override
		{
			(void)result;

			const auto indexBufferPtr = m_IndexBuffer->GetDataPtr();
			const auto vertexBufferPtr = m_VertexBuffer->GetDataPtr();
			const U32 triangleIndex[3] = {
				((U32*)indexBufferPtr)[payload.indexStart * 3 + 0],
				((U32*)indexBufferPtr)[payload.indexStart * 3 + 1],
				((U32*)indexBufferPtr)[payload.indexStart * 3 + 2]
			};

			void* vertexData[3] = {
				&((U8*)vertexBufferPtr)[triangleIndex[0] * m_Pipeline->m_CreateInfo.vertexItemSize],
				&((U8*)vertexBufferPtr)[triangleIndex[1] * m_Pipeline->m_CreateInfo.vertexItemSize],
				&((U8*)vertexBufferPtr)[triangleIndex[2] * m_Pipeline->m_CreateInfo.vertexItemSize]
			};

			
			auto seedTraingle = ShaderTriangleRef(m_VertexToFragmentDataAllocator, m_Pipeline->m_CreateInfo.vertexToFragmentDataSize);

			for (auto i = 0; i < 3; ++i)
			{
				seedTraingle.GetBuiltInRef(i)->Reset();
				seedTraingle.GetBuiltInRef(i)->VertexIndex = payload.indexStart * 3 + i;
				m_Pipeline->m_CreateInfo.vertexShader->Execute(vertexData[i], seedTraingle.GetVertexData(i), seedTraingle.GetBuiltInRef(i));
				seedTraingle.GetBuiltInRef(i)->Position /= seedTraingle.GetBuiltInRef(i)->Position[3];
			}
			seedTraingle.Log();




			// TODO: Clip
			// TODO: Cull
			// TODO: Send to rasterizer

			
			return false;
		}

		inline void SetVertexBuffer(RawPtr<Buffer> buffer)
		{
			m_VertexBuffer = buffer;
		}

		inline void SetIndexBuffer(RawPtr<Buffer> buffer)
		{
			m_IndexBuffer = buffer;
		}

		inline void SetPipeline(RawPtr<Pipeline> pipeline)
		{
			m_Pipeline = pipeline;
		}

		inline void SetVertexToFragmentDataAllocator(RawPtr<LinearAllocator> allocator)
		{
			m_VertexToFragmentDataAllocator = allocator;
		}

	private:
		RawPtr<Buffer> m_VertexBuffer = nullptr;
		RawPtr<Buffer> m_IndexBuffer = nullptr;
		RawPtr<Pipeline> m_Pipeline = nullptr;
		RawPtr<LinearAllocator> m_VertexToFragmentDataAllocator = nullptr;
	};

	Renderer::Renderer()
	{
		m_FrameClearJob = CreateRawPtr<FrameClearWorker>();
		m_FrameClearThreadPool = CreateRawPtr<ThreadPool<8, FrameClearWorkerInput, U32>>(m_FrameClearJob);

		m_VertexShaderJob = CreateRawPtr<VertexShaderWorker>();
		m_VertexShaderThreadPool = CreateRawPtr<ThreadPool<8, VertexShaderWorkerInput, U32>>(m_VertexShaderJob);

		m_VertexToFragmentDataAllocator = CreateRawPtr<LinearAllocator>(1024 * 1024 * 128); // 128 MB
	}

	Renderer::~Renderer()
	{
		delete m_FrameClearThreadPool;
		delete m_FrameClearJob;

		delete m_VertexShaderThreadPool;
		delete m_VertexShaderJob;

		delete m_VertexToFragmentDataAllocator;
	}

	void Renderer::BeginFrame()
	{
#if defined(KLUX_VERY_STRICT_CHECKS)
		if (m_IsInFrame)
		{
			klux::log::Error("Renderer::BeginFrame() called twice without calling EndFrame()");
		}
#endif

		m_IsInFrame = true;
	}

	void Renderer::EndFrame()
	{
#if defined(KLUX_VERY_STRICT_CHECKS)
		if (!m_IsInFrame)
		{
			klux::log::Error("Renderer::EndFrame() called without calling BeginFrame()");
		}
#endif

		m_ActiveViewport.reset();
		m_ActiveFramebuffer = nullptr;
		m_ActivePipeline = nullptr;
		m_VertexToFragmentDataAllocator->Reset();

		m_IsInFrame = false;
	}

	void Renderer::BindFramebuffer(RawPtr<IFramebuffer> fbo)
	{
#if defined(KLUX_VERY_STRICT_CHECKS)
		if (!m_IsInFrame)
		{
			klux::log::Error("Renderer::BindFramebuffer() called without calling BeginFrame()");
		}
#endif

		m_ActiveFramebuffer = fbo;
	}

	void Renderer::BindPipeline(RawPtr<Pipeline> pipeline)
	{
		m_ActivePipeline = pipeline;
	}

	void Renderer::Clear(Bool color, Bool depth)
	{
#if defined(KLUX_VERY_STRICT_CHECKS)
		if (!m_ActiveViewport.has_value())
		{
			klux::log::Error("Renderer::Clear() called without calling SetViewport()");
		}
#endif

		reinterpret_cast<RawPtr<FrameClearWorker>>(m_FrameClearJob)->SetFramebuffer(m_ActiveFramebuffer);
		reinterpret_cast<RawPtr<FrameClearWorker>>(m_FrameClearJob)->SetClearColor(m_ClearColor);
		reinterpret_cast<RawPtr<FrameClearWorker>>(m_FrameClearJob)->SetEnableClearColor(color);
		reinterpret_cast<RawPtr<FrameClearWorker>>(m_FrameClearJob)->SetEnableClearDepth(depth);

		const auto startX = m_ActiveViewport->x;
		const auto startY = m_ActiveViewport->y;
		const auto endX = m_ActiveViewport->x + m_ActiveViewport->width;
		const auto endY = m_ActiveViewport->y + m_ActiveViewport->height;

		const auto tileSize = 16;
		static FrameClearWorkerInput input = {};

		for (auto y = startY; y < endY; y += tileSize)
		{
			for (auto x = startX; x < endX; x += tileSize)
			{
				input.x = x;
				input.y = y;
				input.width = tileSize;
				input.height = tileSize;
				m_FrameClearThreadPool->AddJob(input);
			}
		}
		
		m_FrameClearThreadPool->WaitJobDone();
	}

	void Renderer::SetViewport(I32 x, I32 y, I32 width, I32 height)
	{
#if defined(KLUX_VERY_STRICT_CHECKS)
		if (!m_ActiveFramebuffer)
		{
			klux::log::Error("Renderer::SetViewport() called without calling BindFramebuffer()");
		}

		if (width <= 0 || height <= 0)
		{
			klux::log::Error("Renderer::SetViewport() called with invalid viewport size");
		}

		if (x < 0 || y < 0)
		{
			klux::log::Error("Renderer::SetViewport() called with invalid viewport position");
		}

		if (x + width > m_ActiveFramebuffer->GetWidth() || y + height > m_ActiveFramebuffer->GetHeight())
		{
			klux::log::Error("Renderer::SetViewport() called with invalid viewport size");
		}
#endif

		m_ActiveViewport = Viewport{ x, y, width, height };
	}



	void Renderer::DrawIndexed(RawPtr<Buffer> vertexBuffer, RawPtr<Buffer> indexBuffer, U32 indexCount)
	{
#if defined(KLUX_VERY_STRICT_CHECKS)
		if (!m_IsInFrame)
		{
			klux::log::Error("Renderer::DrawIndexed() called without calling BeginFrame()");
		}

		if (!m_ActiveFramebuffer)
		{
			klux::log::Error("Renderer::DrawIndexed() called without calling BindFramebuffer()");
		}

		if (!m_ActivePipeline)
		{
			klux::log::Error("Renderer::DrawIndexed() called without calling BindPipeline()");
		}

		if (!m_ActiveViewport.has_value())
		{
			klux::log::Error("Renderer::DrawIndexed() called without calling SetViewport()");
		}

		if (indexCount == 0)
		{
			klux::log::Error("Renderer::DrawIndexed() called with 0 index count");
		}

		if (indexCount % 3 != 0)
		{
			klux::log::Error("Renderer::DrawIndexed() called with invalid index count");
		}
#endif
		(void)vertexBuffer;
		(void)indexBuffer;
		(void)indexCount;

		reinterpret_cast<RawPtr<VertexShaderWorker>>(m_VertexShaderJob)->SetIndexBuffer(indexBuffer);
		reinterpret_cast<RawPtr<VertexShaderWorker>>(m_VertexShaderJob)->SetVertexBuffer(vertexBuffer);
		reinterpret_cast<RawPtr<VertexShaderWorker>>(m_VertexShaderJob)->SetPipeline(m_ActivePipeline);
		reinterpret_cast<RawPtr<VertexShaderWorker>>(m_VertexShaderJob)->SetVertexToFragmentDataAllocator(m_VertexToFragmentDataAllocator);


		for (auto i = 0; i < static_cast<I32>(indexCount); i += 3)
		{
			m_VertexShaderThreadPool->AddJob({ i });
		}


		// m_VertexShaderThreadPool.AddJob()

		// TODO: Implement this


	}

}