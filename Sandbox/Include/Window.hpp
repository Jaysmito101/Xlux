#pragma once
#include "Klux.hpp"

enum WindowEvent : klux::U32
{
	WindowResize = 0x0ABF01,
	WindowClose = 0x0ABF02
};

class WindowFramebuffer;

class Window
{
public:
	inline static void Create(const klux::String& title, klux::I32 width, klux::I32 height) { if (!s_Instance) s_Instance = new Window(title, width, height); }
	static void Destroy() { delete s_Instance; }
	static klux::Bool HasClosed();
	static void Update();
	static void SwapBuffer();
	static void Clear(klux::F32 r, klux::F32 g, klux::F32 b, klux::F32 a);
	static void SetPixel(klux::F32 x, klux::F32 y, klux::F32 r, klux::F32 g, klux::F32 b, klux::F32 a);

	inline static klux::I32 GetWidth() { return s_Instance->m_Width; }
	inline static klux::I32 GetHeight(){ return s_Instance->m_Height; }
	inline static void SetWidth(klux::I32 width) { s_Instance->m_Width = width; }
	inline static void SetHeight(klux::I32 height) { s_Instance->m_Height = height; }
	inline static klux::RawPtr<WindowFramebuffer> GetFramebuffer() { return s_Instance->m_Framebuffer; }

private:
	Window(const klux::String& title, klux::I32 width, klux::I32 height);
	~Window();

private:
	klux::String m_Title;
	klux::I32 m_Width, m_Height;
	klux::RawPtr<WindowFramebuffer> m_Framebuffer;

	static klux::RawPtr<Window> s_Instance;
};

class WindowFramebuffer : public klux::IFramebuffer
{
public:
	WindowFramebuffer() = default;
	~WindowFramebuffer() = default;

	virtual klux::Pair<klux::U32, klux::U32> GetSize() const override;
	virtual void SetColorPixel(klux::I32 channel, klux::I32 x, klux::I32 y, klux::F32 r, klux::F32 g, klux::F32 b, klux::F32 a) override;
	virtual void SetDepthPixel(klux::I32 x, klux::I32 y, klux::F32 depth) override;

};