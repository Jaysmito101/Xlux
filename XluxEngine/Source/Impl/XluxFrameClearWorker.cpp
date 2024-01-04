#include "Impl/FrameClearWorker.hpp"
#include "Impl/Framebuffer.hpp"

namespace xlux
{
	Bool FrameClearWorker::Execute(FrameClearWorkerInput payload, U32& result, Size threadID)
	{
		(void)result, (void)threadID;

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

}