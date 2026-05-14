#include "Core/Logger.hpp"
#include "Core/Types.hpp"
#include "Impl/FrameClearWorker.hpp"
#include "Impl/Framebuffer.hpp"

namespace xlux {
Bool FrameClearWorker::Execute(FrameClearWorkerInput payload, U32 threadID) {
  (void)threadID;

  auto tileOffset = payload.framebuffer->GetTileOffset(payload.slotId);
  auto tileSize = payload.framebuffer->GetTileSize();
  auto tileEnd = MakePair(std::clamp(tileOffset.x + tileSize.x, 0U,
                                     (U32)payload.framebuffer->GetWidth()),
                          std::clamp(tileOffset.y + tileSize.y, 0U,
                                     (U32)payload.framebuffer->GetHeight()));
  auto pixel = payload.clearColor.ToVec4();

  for (U32 x = tileOffset.x; x < tileEnd.x; ++x) {
    for (U32 y = tileOffset.y; y < tileEnd.y; ++y) {
      if (payload.shouldClearColor) {
        for (U32 ch = 0; ch < payload.framebuffer->GetColorAttachmentCount();
             ++ch) {
          payload.framebuffer->SetColorPixel(ch, x, y, pixel[0], pixel[1],
                                             pixel[2], pixel[3]);
        }
      }

      if (payload.shouldClearDepth &&
          payload.framebuffer->HasDepthAttachment()) {
        payload.framebuffer->SetDepthPixel(x, y, 10000000.0f);
      }
    }
  }

  return false;
}

}  // namespace xlux