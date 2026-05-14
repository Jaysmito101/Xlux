#include "Core/Logger.hpp"
#include "Core/Types.hpp"
#include "Impl/Renderer.hpp"
#include "Impl/DeviceMemory.hpp"
#include "Impl/Buffer.hpp"
#include "Impl/Pipeline.hpp"
#include "Impl/Shader.hpp"
#include "Impl/Framebuffer.hpp"

namespace xlux {

Renderer::Renderer() {
  
  m_VertexShaderJob = CreateRawPtr<VertexShaderWorker>();
  m_VertexShaderThreadPool = CreateRawPtr<
  ThreadPool<k_VertexShaderWorkerCount, VertexShaderWorkerInput, U32>>(
    m_VertexShaderJob);
    
  m_FragmentWorker = CreateScope<FragmentWorkerPoolType>();
  m_FrameClearWorker = CreateScope<FrameClearWorkerPoolType>(); 

  m_VertexToFragmentDataAllocator = CreateRawPtr<LinearAllocator>(
      1024 * 1024 * 256, k_VertexShaderWorkerCount);  // 256MB x 8 = 2GB
}

Renderer::~Renderer() {
  m_FragmentWorker.reset();
  m_FrameClearWorker.reset();

  delete m_VertexShaderThreadPool;
  delete m_VertexShaderJob;

  delete m_VertexToFragmentDataAllocator;
}

void Renderer::BeginFrame() {
#if defined(XLUX_VERY_STRICT_CHECKS)
  if (m_IsInFrame) {
    xlux::log::Error(
        "Renderer::BeginFrame() called twice without calling EndFrame()");
  }
#endif

  m_IsInFrame = true;
  m_DetachedRendering = false;
}

void Renderer::EndFrame() {
#if defined(XLUX_VERY_STRICT_CHECKS)
  if (!m_IsInFrame) {
    xlux::log::Error(
        "Renderer::EndFrame() called without calling BeginFrame()");
  }
#endif

  Flush();

  m_ActiveViewport.reset();
  m_ActiveFramebuffer = nullptr;
  m_ActivePipeline = nullptr;
  m_VertexToFragmentDataAllocator->Reset();

  m_IsInFrame = false;
}

void Renderer::Flush() {
#if defined(XLUX_VERY_STRICT_CHECKS)
  if (!m_IsInFrame) {
    xlux::log::Error(
        "Renderer::EndFrame() called without calling BeginFrame()");
  }
#endif
  m_VertexShaderThreadPool->WaitJobDone();
  m_FrameClearWorker->WaitForIdle();
  m_FragmentWorker->WaitForIdle();
  m_VertexToFragmentDataAllocator->Reset();
}

void Renderer::BindFramebuffer(RawPtr<IFramebuffer> fbo) {
#if defined(XLUX_VERY_STRICT_CHECKS)
  if (!m_IsInFrame) {
    xlux::log::Error(
        "Renderer::BindFramebuffer() called without calling BeginFrame()");
  }
#endif

  m_ActiveFramebuffer = fbo;
}

void Renderer::BindPipeline(RawPtr<Pipeline> pipeline) {
  m_ActivePipeline = pipeline;
}

void Renderer::Clear(Bool color, Bool depth) {
#if defined(XLUX_VERY_STRICT_CHECKS)
  if (!m_ActiveViewport.has_value()) {
    xlux::log::Error("Renderer::Clear() called without calling SetViewport()");
  }
#endif

  const auto startX = m_ActiveViewport->x;
  const auto startY = m_ActiveViewport->y;
  const auto endX = m_ActiveViewport->x + m_ActiveViewport->width;
  const auto endY = m_ActiveViewport->y + m_ActiveViewport->height;

  FrameClearWorkerInput input = {
    .slotId = 0,
    .clearColor = m_ClearColor,
    .framebuffer = m_ActiveFramebuffer,
    .shouldClearColor = color,
    .shouldClearDepth = depth
  };

  auto tiles = m_ActiveFramebuffer->GetOverlappingTiles(startX, startY,
                                               endX - startX, endY - startY);
  for (auto tileId : tiles) {
    input.slotId = tileId;
    m_FrameClearWorker->AddJob(input);
  }
 m_FragmentWorker->WaitForIdle();
}

void Renderer::SetViewport(I32 x, I32 y, I32 width, I32 height) {
#if defined(XLUX_VERY_STRICT_CHECKS)
  if (!m_ActiveFramebuffer) {
    xlux::log::Error(
        "Renderer::SetViewport() called without calling BindFramebuffer()");
  }

  if (width <= 0 || height <= 0) {
    xlux::log::Error(
        "Renderer::SetViewport() called with invalid viewport size");
  }

  if (x < 0 || y < 0) {
    xlux::log::Error(
        "Renderer::SetViewport() called with invalid viewport position");
  }

  if (x + width > m_ActiveFramebuffer->GetWidth() ||
      y + height > m_ActiveFramebuffer->GetHeight()) {
    xlux::log::Error(
        "Renderer::SetViewport() called with invalid viewport size");
  }
#endif

  m_ActiveViewport = Viewport{x, y, width, height};
}

void Renderer::DrawIndexed(RawPtr<Buffer> vertexBuffer,
                           RawPtr<Buffer> indexBuffer, U32 indexCount,
                           U32 startingVertex, U32 startingIndex) {
#if defined(XLUX_VERY_STRICT_CHECKS)
  if (!m_IsInFrame) {
    xlux::log::Error(
        "Renderer::DrawIndexed() called without calling BeginFrame()");
  }

  if (!m_ActiveFramebuffer) {
    xlux::log::Error(
        "Renderer::DrawIndexed() called without calling BindFramebuffer()");
  }

  if (!m_ActivePipeline) {
    xlux::log::Error(
        "Renderer::DrawIndexed() called without calling BindPipeline()");
  }

  if (!m_ActiveViewport.has_value()) {
    xlux::log::Error(
        "Renderer::DrawIndexed() called without calling SetViewport()");
  }

  if (indexCount == 0) {
    xlux::log::Error("Renderer::DrawIndexed() called with 0 index count");
  }

  if (indexCount % 3 != 0) {
    xlux::log::Error("Renderer::DrawIndexed() called with invalid index count");
  }
#endif


  auto vertexShaderJob =
      reinterpret_cast<RawPtr<VertexShaderWorker>>(m_VertexShaderJob);
  vertexShaderJob->SetVertexToFragmentDataAllocator(
      m_VertexToFragmentDataAllocator);

  for (auto i = 0; i < static_cast<I32>(indexCount); i += 3) {
    m_VertexShaderThreadPool->AddJob(
        {.indexStart = i,
         .userData = m_RendererUserData,

         .startingVertex = startingVertex,
         .startingIndex = startingIndex,

         .vertexBuffer = vertexBuffer,
         .indexBuffer = indexBuffer,

         .pipeline = m_ActivePipeline,
         .framebuffer = m_ActiveFramebuffer,

         .rasterizer = std::bind(&Renderer::PassTriangleToFragmentShader, this,
                                 std::placeholders::_1)});
  }

  if (!m_DetachedRendering) {
    m_VertexShaderThreadPool->WaitJobDone();
    m_FragmentWorker->WaitForIdle();
  }
}

void Renderer::DrawIndexedOrdered(RawPtr<Buffer> vertexBuffer,
                                  RawPtr<Buffer> indexBuffer, U32 indexCount,
                                  U32 startingVertex, U32 startingIndex) {
#if defined(XLUX_VERY_STRICT_CHECKS)
  if (!m_IsInFrame) {
    xlux::log::Error(
        "Renderer::DrawIndexed() called without calling BeginFrame()");
  }

  if (!m_ActiveFramebuffer) {
    xlux::log::Error(
        "Renderer::DrawIndexed() called without calling BindFramebuffer()");
  }

  if (!m_ActivePipeline) {
    xlux::log::Error(
        "Renderer::DrawIndexed() called without calling BindPipeline()");
  }

  if (!m_ActiveViewport.has_value()) {
    xlux::log::Error(
        "Renderer::DrawIndexed() called without calling SetViewport()");
  }

  if (indexCount == 0) {
    xlux::log::Error("Renderer::DrawIndexed() called with 0 index count");
  }

  if (indexCount % 3 != 0) {
    xlux::log::Error("Renderer::DrawIndexed() called with invalid index count");
  }
#endif


  auto vertexShaderJob =
      reinterpret_cast<RawPtr<VertexShaderWorker>>(m_VertexShaderJob);

  vertexShaderJob->SetVertexToFragmentDataAllocator(
      m_VertexToFragmentDataAllocator);

  for (auto i = 0; i < static_cast<I32>(indexCount); i += 3) {
    m_VertexShaderThreadPool->AddJobTo(
        {.indexStart = i,
         .userData = m_RendererUserData,

         .startingVertex = startingVertex,
         .startingIndex = startingIndex,

         .vertexBuffer = vertexBuffer,
         .indexBuffer = indexBuffer,

         .pipeline = m_ActivePipeline,
         .framebuffer = m_ActiveFramebuffer,

         .rasterizer = std::bind(&Renderer::PassTriangleToFragmentShader, this,
                                 std::placeholders::_1)},
        0);
  }

  if (!m_DetachedRendering) {
    m_VertexShaderThreadPool->WaitJobDone();
    m_FragmentWorker->WaitForIdle();
  }
}

Bool Renderer::PassTriangleToFragmentShader(ShaderTriangleRef triangle) {
  auto boundingBox = triangle.GetBoundingBox();  // (xmin, ymin, xmax, ymax)

  FragmentShaderWorkerInput input = {
    .triangle = triangle,
    .slotId = 0,
    .pipeline = m_ActivePipeline,
    .framebuffer = m_ActiveFramebuffer,
  };

  auto minX = static_cast<I32>(std::floor(boundingBox[0]));
  auto minY = static_cast<I32>(std::floor(boundingBox[1]));
  auto maxX = static_cast<I32>(std::ceil(boundingBox[2]));
  auto maxY = static_cast<I32>(std::ceil(boundingBox[3]));

  auto startX = static_cast<U32>(std::max(0, minX));
  auto startY = static_cast<U32>(std::max(0, minY));
  auto endX = static_cast<U32>(std::max(0, maxX));
  auto endY = static_cast<U32>(std::max(0, maxY));

  auto tiles = m_ActiveFramebuffer->GetOverlappingTiles(
      startX, startY,
      endX > startX ? endX - startX : 0,
      endY > startY ? endY - startY : 0);
  for (auto tileId : tiles) {
    input.slotId = tileId;
    m_FragmentWorker->AddJob(input);
  }
  
  return false;
}

}  // namespace xlux