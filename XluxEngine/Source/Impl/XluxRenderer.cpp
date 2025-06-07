#include "Impl/Renderer.hpp"
#include "Impl/DeviceMemory.hpp"
#include "Impl/Buffer.hpp"
#include "Impl/Pipeline.hpp"
#include "Impl/Shader.hpp"
#include "Impl/Framebuffer.hpp"

namespace xlux
{


	Renderer::Renderer()
	{
		m_FrameClearJob = CreateRawPtr<FrameClearWorker>();
		m_FrameClearThreadPool = CreateRawPtr<ThreadPool<k_FrameClearWorkerCount, FrameClearWorkerInput, U32>>(m_FrameClearJob);

		m_VertexShaderJob = CreateRawPtr<VertexShaderWorker>();
		m_VertexShaderThreadPool = CreateRawPtr<ThreadPool<k_VertexShaderWorkerCount, VertexShaderWorkerInput, U32>>(m_VertexShaderJob);

		m_FragmentShaderJob = CreateRawPtr<FragmentShaderWorker>();
		m_FragmentShaderThreadPool = CreateRawPtr<ThreadPool<k_FragmentShaderWorkerCountX * k_FragmentShaderWorkerCountY, FragmentShaderWorkerInput, U32>>(m_FragmentShaderJob);

		m_VertexToFragmentDataAllocator = CreateRawPtr<LinearAllocator>(1024 * 1024 * 256, k_VertexShaderWorkerCount); // 256MB x 8 = 2GB
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
#if defined(XLUX_VERY_STRICT_CHECKS)
		if (m_IsInFrame)
		{
			xlux::log::Error("Renderer::BeginFrame() called twice without calling EndFrame()");
		}
#endif

		m_IsInFrame = true;
	}

	void Renderer::EndFrame()
	{
#if defined(XLUX_VERY_STRICT_CHECKS)
		if (!m_IsInFrame)
		{
			xlux::log::Error("Renderer::EndFrame() called without calling BeginFrame()");
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
#if defined(XLUX_VERY_STRICT_CHECKS)
		if (!m_IsInFrame)
		{
			xlux::log::Error("Renderer::BindFramebuffer() called without calling BeginFrame()");
		}
#endif

		m_ActiveFramebuffer = fbo;

		m_FragmentShaderTileWidth = static_cast<U32>(std::ceil(static_cast<F32>(fbo->GetWidth()) / k_FragmentShaderWorkerCountX));
		m_FragmentShaderTileHeight = static_cast<U32>(std::ceil(static_cast<F32>(fbo->GetHeight()) / k_FragmentShaderWorkerCountY));
	}

	void Renderer::BindPipeline(RawPtr<Pipeline> pipeline)
	{
		m_ActivePipeline = pipeline;
	}

	void Renderer::Clear(Bool color, Bool depth)
	{
#if defined(XLUX_VERY_STRICT_CHECKS)
		if (!m_ActiveViewport.has_value())
		{
			xlux::log::Error("Renderer::Clear() called without calling SetViewport()");
		}
#endif

		auto frameClearJob = reinterpret_cast<RawPtr<FrameClearWorker>>(m_FrameClearJob);

		frameClearJob->SetFramebuffer(m_ActiveFramebuffer);
		frameClearJob->SetClearColor(m_ClearColor);
		frameClearJob->SetEnableClearColor(color);
		frameClearJob->SetEnableClearDepth(depth);

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
#if defined(XLUX_VERY_STRICT_CHECKS)
		if (!m_ActiveFramebuffer)
		{
			xlux::log::Error("Renderer::SetViewport() called without calling BindFramebuffer()");
		}

		if (width <= 0 || height <= 0)
		{
			xlux::log::Error("Renderer::SetViewport() called with invalid viewport size");
		}

		if (x < 0 || y < 0)
		{
			xlux::log::Error("Renderer::SetViewport() called with invalid viewport position");
		}

		if (x + width > m_ActiveFramebuffer->GetWidth() || y + height > m_ActiveFramebuffer->GetHeight())
		{
			xlux::log::Error("Renderer::SetViewport() called with invalid viewport size");
		}
#endif

		m_ActiveViewport = Viewport{ x, y, width, height };
	}


	void Renderer::DrawIndexed(RawPtr<Buffer> vertexBuffer, RawPtr<Buffer> indexBuffer, U32 indexCount, U32 startingVertex, U32 startingIndex)
	{
#if defined(XLUX_VERY_STRICT_CHECKS)
		if (!m_IsInFrame)
		{
			xlux::log::Error("Renderer::DrawIndexed() called without calling BeginFrame()");
		}

		if (!m_ActiveFramebuffer)
		{
			xlux::log::Error("Renderer::DrawIndexed() called without calling BindFramebuffer()");
		}

		if (!m_ActivePipeline)
		{
			xlux::log::Error("Renderer::DrawIndexed() called without calling BindPipeline()");
		}

		if (!m_ActiveViewport.has_value())
		{
			xlux::log::Error("Renderer::DrawIndexed() called without calling SetViewport()");
		}

		if (indexCount == 0)
		{
			xlux::log::Error("Renderer::DrawIndexed() called with 0 index count");
		}

		if (indexCount % 3 != 0)
		{
			xlux::log::Error("Renderer::DrawIndexed() called with invalid index count");
		}
#endif

		m_VertexToFragmentDataAllocator->Reset();

		auto vertexShaderJob = reinterpret_cast<RawPtr<VertexShaderWorker>>(m_VertexShaderJob);

		vertexShaderJob->SetIndexBuffer(indexBuffer, startingIndex);
		vertexShaderJob->SetVertexBuffer(vertexBuffer, startingVertex);
		vertexShaderJob->SetPipeline(m_ActivePipeline);
		vertexShaderJob->SetFramebuffer(m_ActiveFramebuffer);
		vertexShaderJob->SetVertexToFragmentDataAllocator(m_VertexToFragmentDataAllocator);

		vertexShaderJob->SetRasterizer(std::bind(&Renderer::PassTriangleToFragmentShader, this, std::placeholders::_1));

		for (auto i = 0; i < static_cast<I32>(indexCount); i += 3)
		{
			m_VertexShaderThreadPool->AddJob({ i });
		}



		m_VertexShaderThreadPool->WaitJobDone();
		m_FragmentShaderThreadPool->WaitJobDone();
	}

	void Renderer::DrawIndexedOrdered(RawPtr<Buffer> vertexBuffer, RawPtr<Buffer> indexBuffer, U32 indexCount, U32 startingVertex, U32 startingIndex)
	{
#if defined(XLUX_VERY_STRICT_CHECKS)
		if (!m_IsInFrame)
		{
			xlux::log::Error("Renderer::DrawIndexed() called without calling BeginFrame()");
		}

		if (!m_ActiveFramebuffer)
		{
			xlux::log::Error("Renderer::DrawIndexed() called without calling BindFramebuffer()");
		}

		if (!m_ActivePipeline)
		{
			xlux::log::Error("Renderer::DrawIndexed() called without calling BindPipeline()");
		}

		if (!m_ActiveViewport.has_value())
		{
			xlux::log::Error("Renderer::DrawIndexed() called without calling SetViewport()");
		}

		if (indexCount == 0)
		{
			xlux::log::Error("Renderer::DrawIndexed() called with 0 index count");
		}

		if (indexCount % 3 != 0)
		{
			xlux::log::Error("Renderer::DrawIndexed() called with invalid index count");
		}
#endif

		m_VertexToFragmentDataAllocator->Reset();

		auto vertexShaderJob = reinterpret_cast<RawPtr<VertexShaderWorker>>(m_VertexShaderJob);

		vertexShaderJob->SetIndexBuffer(indexBuffer, startingIndex);
		vertexShaderJob->SetVertexBuffer(vertexBuffer, startingVertex);
		vertexShaderJob->SetPipeline(m_ActivePipeline);
		vertexShaderJob->SetFramebuffer(m_ActiveFramebuffer);
		vertexShaderJob->SetVertexToFragmentDataAllocator(m_VertexToFragmentDataAllocator);

		vertexShaderJob->SetRasterizer(std::bind(&Renderer::PassTriangleToFragmentShader, this, std::placeholders::_1));

		for (auto i = 0; i < static_cast<I32>(indexCount); i += 3)
		{
			m_VertexShaderThreadPool->AddJobTo({ i }, 0);
		}



		m_VertexShaderThreadPool->WaitJobDone();
		m_FragmentShaderThreadPool->WaitJobDone();
	}


	Bool Renderer::PassTriangleToFragmentShader(ShaderTriangleRef triangle)
	{
		auto boundingBox = triangle.GetBoundingBox(); // (xmin, ymin, xmax, ymax)
		// boundingBox *= math::Vec4(static_cast<F32>(m_ActiveFramebuffer->GetWidth()), static_cast<F32>(m_ActiveFramebuffer->GetHeight()), static_cast<F32>(m_ActiveFramebuffer->GetWidth()), static_cast<F32>(m_ActiveFramebuffer->GetHeight()));

		auto fragmentShaderJob = static_cast<RawPtr<FragmentShaderWorker>>(m_FragmentShaderJob);

		fragmentShaderJob->SetFramebuffer(m_ActiveFramebuffer);
		fragmentShaderJob->SetPipeline(m_ActivePipeline);
		
		FragmentShaderWorkerInput input = {};
		input.triangle = triangle;


		for (auto y = 0; y < k_FragmentShaderWorkerCountY; ++y)
		{
			for (auto x = 0; x < k_FragmentShaderWorkerCountX; ++x)
			{
				input.startX = std::max(static_cast<I32>(x * m_FragmentShaderTileWidth), m_ActiveViewport.value().x);
				input.startY = std::max(static_cast<I32>(y * m_FragmentShaderTileHeight), m_ActiveViewport.value().y);
				input.width = static_cast<I32>(m_FragmentShaderTileWidth);
				input.height = static_cast<I32>(m_FragmentShaderTileHeight);

				if ((I32)input.width + (I32)input.startX > m_ActiveViewport.value().x + m_ActiveViewport.value().width)
				{
					input.width = m_ActiveViewport.value().x + m_ActiveViewport.value().width - input.startX;
				}

				if ((I32)input.height + (I32)input.startY > m_ActiveViewport.value().y + m_ActiveViewport.value().height)
				{
					input.height = m_ActiveViewport.value().y + m_ActiveViewport.value().height - input.startY;
				}

				if (boundingBox[0] >= input.startX + input.width || boundingBox[2] <= input.startX ||
					boundingBox[1] >= input.startY + input.height || boundingBox[3] <= input.startY)
				{
					continue;
				}

				m_FragmentShaderThreadPool->AddJobTo(input, y * k_FragmentShaderWorkerCountX + x);
			}
		}

		return false;
	}

}