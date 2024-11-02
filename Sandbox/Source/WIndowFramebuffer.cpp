#include "Window.hpp"

xlux::Pair<xlux::U32, xlux::U32> WindowFramebuffer::GetSize() const
{
	return xlux::MakePair<xlux::U32, xlux::U32>(Window::GetWidth(), Window::GetHeight());
}

static xlux::F32 s_ColorBuffer[4096][4096][4] = { 0.0f };

void WindowFramebuffer::SetColorPixel(xlux::I32 channel, xlux::I32 x, xlux::I32 y, xlux::F32 r, xlux::F32 g, xlux::F32 b, xlux::F32 a)
{
	if (channel != 0)
	{
		xlux::log::Error("Invalid channel for Window Framebuffer");
	}

	r = std::clamp(r, 0.0f, 0.999f);
	g = std::clamp(g, 0.0f, 0.999f);
	b = std::clamp(b, 0.0f, 0.999f);
	a = std::clamp(a, 0.0f, 0.999f);

	s_ColorBuffer[x][y][0] = r;
	s_ColorBuffer[x][y][1] = g;
	s_ColorBuffer[x][y][2] = b;
	s_ColorBuffer[x][y][3] = a;

	Window::SetPixel(
		(xlux::F32)x / (xlux::F32)Window::GetWidth(),
		(xlux::F32)y / (xlux::F32)Window::GetHeight(),
		r, g, b, a
	);
}



void WindowFramebuffer::GetColorPixel(xlux::I32 channel, xlux::I32 x, xlux::I32 y, xlux::F32& r, xlux::F32& g, xlux::F32& b, xlux::F32& a) const
{
	(void)x, (void)y;

	if (channel != 0)
	{
		xlux::log::Error("Invalid channel for Window Framebuffer");
	}
	
	r = s_ColorBuffer[x][y][0];
	g = s_ColorBuffer[x][y][1];
	b = s_ColorBuffer[x][y][2];
	a = s_ColorBuffer[x][y][3];
}

static xlux::F32 s_DepthBuffer[4096][4096] = { 0.0f };

void WindowFramebuffer::GetDepthPixel(xlux::I32 x, xlux::I32 y, xlux::F32& depth) const
{
	depth = s_DepthBuffer[x][y];
}


void WindowFramebuffer::SetDepthPixel(xlux::I32 x, xlux::I32 y, xlux::F32 depth)
{
	s_DepthBuffer[x][y] = depth;
}