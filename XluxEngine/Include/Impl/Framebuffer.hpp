#pragma once

#include "Core/core.hpp"

namespace xlux
{

	class IFramebuffer
	{
	public:

		inline I32 GetWidth() const { return GetSize().x; }
		inline I32 GetHeight() const { return GetSize().y; }

		virtual U32 GetColorAttachmentCount() const = 0;
		virtual Bool HasDepthAttachment() const = 0;
		virtual Pair<U32, U32> GetSize() const = 0;
		virtual void SetColorPixel(I32 channel, I32 x, I32 y, F32 r, F32 g, F32 b, F32 a) = 0;
		virtual void SetDepthPixel(I32 x, I32 y, F32 depth) { (void)x, (void)y, (void)depth, throw std::runtime_error("Not implemented"); }
		virtual void GetColorPixel(I32 channel, I32 x, I32 y, F32& r, F32& g, F32& b, F32& a) const = 0;
		virtual void GetDepthPixel(I32 x, I32 y, F32& depth) const { (void)x, (void)y, (void)depth, throw std::runtime_error("Not implemented"); }

	};
}