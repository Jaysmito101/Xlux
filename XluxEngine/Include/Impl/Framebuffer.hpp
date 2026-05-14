#pragma once

#include <algorithm>
#include "Core/Core.hpp"

namespace xlux {

class IFramebuffer {
 public:
  inline I32 GetWidth() const { return this->GetSize().x; }
  inline I32 GetHeight() const { return this->GetSize().y; }

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

  IFramebuffer() {
    // Initialize slot usage to 0
    for (auto& slot : m_SlotUsage) {
      slot.store(0, std::memory_order_relaxed);
    }
  }
  virtual ~IFramebuffer() = default;

  inline Pair<U32, U32> GetTileCount() const {
    auto tileSize = GetTileSize();
    return MakePair<U32, U32>((GetSize().x + tileSize.x - 1) / tileSize.x,
                              (GetSize().y + tileSize.y - 1) / tileSize.y);
  }

  inline std::set<U32> GetOverlappingTiles(U32 x, U32 y, U32 width,
                                           U32 height) const {
    std::set<U32> overlappingTiles;

    const auto tileSize = GetTileSize();
    const auto startTileX = x / tileSize.x;
    const auto startTileY = y / tileSize.y;
    const auto tileCount = GetTileCount();
    const auto endTileX = std::clamp((x + width + tileSize.x - 1) / tileSize.x,
                                     U32(0), tileCount.x);
    const auto endTileY = std::clamp((y + height + tileSize.y - 1) / tileSize.y,
                                     U32(0), tileCount.y);

    for (U32 tileY = startTileY; tileY < endTileY; ++tileY) {
      for (U32 tileX = startTileX; tileX < endTileX; ++tileX) {
        U32 tileId = tileY * tileCount.x + tileX;
        if (tileId < kMaxSlots) {
          overlappingTiles.insert(tileId);
        }
      }
    }

    return overlappingTiles;
  }

  inline Pair<U32, U32> GetTileSize() const {
    return CalculateTileSize(GetOptimalTilingConfig(), GetSize());
  }

  inline Pair<U32, U32> GetTileOffset(U32 tileId) const {
    const auto tileCount = GetTileCount();
    const auto tileSize = GetTileSize();
    const auto tileX = tileId % tileCount.x;
    const auto tileY = tileId / tileCount.x;
    return MakePair<U32, U32>(tileX * tileSize.x, tileY * tileSize.y);
  }

  inline Bool AcquireSlot(U32 tileId, U32 threadID, U32& currentSlotOwner) {
    if (tileId >= kMaxSlots) {
      throw std::runtime_error("Tile ID exceeds maximum slot count");
    }
    currentSlotOwner = 0;  // Default value indicating no owner
    return m_SlotUsage[tileId].compare_exchange_strong(
        currentSlotOwner, threadID, std::memory_order_acquire,
        std::memory_order_relaxed);
  }

  inline void ReleaseSlot(U32 tileId) {
    if (tileId >= kMaxSlots) {
      throw std::runtime_error("Tile ID exceeds maximum slot count");
    }
    m_SlotUsage[tileId].store(0, std::memory_order_release);
  }

 private:
  static Pair<U32, U32> CalculateTileSize(Pair<U32, U32> optimalTiling,
                                          Pair<U32, U32> viewportSize) {
    if (viewportSize.x == 0 || viewportSize.y == 0 || optimalTiling.x == 0 ||
        optimalTiling.y == 0) {
      return MakePair<U32, U32>(0, 0);
    }

    Pair<U32, U32> tileSize = optimalTiling;
    U32 tilesX = 0;
    U32 tilesY = 0;

    while (true) {
      tilesX = (viewportSize.x + tileSize.x - 1) / tileSize.x;
      tilesY = (viewportSize.y + tileSize.y - 1) / tileSize.y;

      if (tilesX * tilesY <= kMaxSlots) {
        break;
      }

      if (tilesX > tilesY) {
        tileSize.x += optimalTiling.x;
      } else if (tilesY > tilesX) {
        tileSize.y += optimalTiling.y;
      } else {
        tileSize.x += optimalTiling.x;
        tileSize.y += optimalTiling.y;
      }
    }

    return MakePair<U32, U32>(std::move(tileSize.x), std::move(tileSize.y));
  }

  const static U32 kMaxSlots = 4096;
  std::array<std::atomic<U32>, kMaxSlots> m_SlotUsage = {};
};
}  // namespace xlux