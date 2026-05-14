#pragma once

#include "Core/Core.hpp"
#include "Core/ThreadPool.hpp"
#include "Math/Math.hpp"
#include "Impl/RendererCommon.hpp"
#include "Impl/Shader.hpp"

namespace xlux {
struct FrameClearWorkerInput {
  U32 slotId = 0;
  math::PackedColor clearColor = math::PackedColor(0, 255, 255, 255);
  RawPtr<IFramebuffer> framebuffer = nullptr;
  Bool shouldClearColor = true;
  Bool shouldClearDepth = true;
};

class FrameClearWorker {
 public:
  Bool Execute(FrameClearWorkerInput payload, U32 threadID);
};

}  // namespace xlux