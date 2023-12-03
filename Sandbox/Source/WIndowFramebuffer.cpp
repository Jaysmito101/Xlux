#include "Window.hpp"

klux::Pair<klux::U32, klux::U32> WindowFramebuffer::GetSize() const
{
	return klux::MakePair<klux::U32, klux::U32>(Window::GetWidth(), Window::GetHeight());
}

void WindowFramebuffer::SetColorPixel(klux::I32 channel, klux::I32 x, klux::I32 y, klux::F32 r, klux::F32 g, klux::F32 b, klux::F32 a)
{
	if (channel != 0)
	{
		klux::log::Error("Invalid channel for Window Framebuffer");
	}

	Window::SetPixel(
		(klux::F32)x / (klux::F32)Window::GetWidth(),
		(klux::F32)y / (klux::F32)Window::GetHeight(),
		r, g, b, a
	);
}

void WindowFramebuffer::SetDepthPixel(klux::I32 x, klux::I32 y, klux::F32 depth)
{
	(void)x, (void)y, (void)depth;
	return;
}
