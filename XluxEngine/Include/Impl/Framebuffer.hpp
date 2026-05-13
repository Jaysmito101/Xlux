#pragma once

#include "Core/Core.hpp"

namespace xlux {

class IFramebuffer {
 public:
  inline I32 GetWidth() const { return GetSize().x; }
  inline I32 GetHeight() const { return GetSize().y; }

  virtual U32 GetColorAttachmentCount() const = 0;
  virtual Bool HasDepthAttachment() const = 0;
  virtual Pair<U32, U32> GetSize() const = 0;
  virtual void SetColorPixel(I32 channel, I32 x, I32 y, F32 r, F32 g, F32 b,
                             F32 a) = 0;
  virtual void SetDepthPixel(I32 x, I32 y, F32 depth) {
    (void)x, (void)y, (void)depth, throw std::runtime_error("Not implemented");
  }
  virtual void GetColorPixel(I32 channel, I32 x, I32 y, F32& r, F32& g, F32& b,
                             F32& a) const = 0;
  virtual void GetDepthPixel(I32 x, I32 y, F32& depth) const {
    (void)x, (void)y, (void)depth, throw std::runtime_error("Not implemented");
  }

  // This function can be used by the renderer to determine the optimal tiling
  // configuration for the framebuffer. The default implementation returns a
  // tile size of 64x64. The purpose for this is to allow the render to 
  // creae optimal memory and synchronization slots for the framebuffer, so
  // that it matches the inherent memory layout of the framebuffer, 
  // which can significantly improve the performance of the renderer.
  constexpr virtual Pair<U32, U32> GetOptimalTilingConfig() const {
    return MakePair<U32, U32>(64, 64);
  }

  virtual ~IFramebuffer() = default;
};
}  // namespace xlux