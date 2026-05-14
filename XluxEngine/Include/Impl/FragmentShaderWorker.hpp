#pragma once

#include "Core/Core.hpp"
#include "Core/ThreadPool.hpp"
#include "Math/Math.hpp"
#include "Impl/RendererCommon.hpp"

namespace xlux {

struct FragmentShaderWorkerInput {
  ShaderTriangleRef triangle;
  U32 slotId = 0;
  RawPtr<Pipeline> pipeline = nullptr;
  RawPtr<IFramebuffer> framebuffer = nullptr;
};

class FragmentShaderWorker {
 public:
  Bool Execute(FragmentShaderWorkerInput payload, U32 threadID);

 private:
  Bool PointInTriangle(const math::Vec2& p, const math::Vec4& p0,
                       const math::Vec4& p1, const math::Vec4& p2);
  math::Vec3 CalculateBarycentric(const math::Vec2& p, const math::Vec4& a,
                                  const math::Vec4& b, const math::Vec4& c);
  Bool BlendAndApplyDepth(U32 px, U32 py, RawPtr<IFramebuffer> fbo, RawPtr<Pipeline> pipeline, F32 depth);
  void BlendAndApplyColor(U32 px, U32 py, RawPtr<IFramebuffer> fbo, RawPtr<Pipeline> pipeline,
                          const FragmentShaderOutput& output);
  F32 CalculateBlendFactor(F32 srcAlpha, F32 dstAlpha,
                           EBlendFunction blendFunction);
};
}  // namespace xlux