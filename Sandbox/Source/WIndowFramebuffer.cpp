#include "Window.hpp"

xlux::Pair<xlux::U32, xlux::U32> WindowFramebuffer::GetSize() const
{
	return xlux::MakePair<xlux::U32, xlux::U32>(Window::GetWidth(), Window::GetHeight());
}

void WindowFramebuffer::SetColorPixel(xlux::I32 channel, xlux::I32 x, xlux::I32 y, xlux::F32 r, xlux::F32 g, xlux::F32 b, xlux::F32 a)
{
	if (channel != 0)
	{
		xlux::log::Error("Invalid channel for Window Framebuffer");
	}

	Window::SetPixel(
		(xlux::F32)x / (xlux::F32)Window::GetWidth(),
		(xlux::F32)y / (xlux::F32)Window::GetHeight(),
		r, g, b, a
	);
}

void WindowFramebuffer::SetDepthPixel(xlux::I32 x, xlux::I32 y, xlux::F32 depth)
{
	(void)x, (void)y, (void)depth;
	return;
}


void WindowFramebuffer::GetColorPixel(xlux::I32 channel, xlux::I32 x, xlux::I32 y, xlux::F32& r, xlux::F32& g, xlux::F32& b, xlux::F32& a) const
{
	(void)x, (void)y;

	if (channel != 0)
	{
		xlux::log::Error("Invalid channel for Window Framebuffer");
	}

	r = g = b = 0.0f;
	a = 1.0f;
}

void WindowFramebuffer::GetDepthPixel(xlux::I32 x, xlux::I32 y, xlux::F32& depth) const
{
	(void)x, (void)y;
	depth = 0.0f;
}
