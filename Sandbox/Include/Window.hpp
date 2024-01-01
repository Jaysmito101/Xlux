#pragma once
#include "Xlux.hpp"

enum WindowEvent : xlux::U32
{
	WindowResize = 0x0ABF01,
	WindowClose = 0x0ABF02
};

class WindowFramebuffer;

class Window
{
public:
	inline static void Create(const xlux::String& title, xlux::I32 width, xlux::I32 height) { if (!s_Instance) s_Instance = new Window(title, width, height); }
	static void Destroy() { delete s_Instance; }
	static xlux::Bool HasClosed();
	static void Update();
	static void SwapBuffer();
	static void Clear(xlux::F32 r, xlux::F32 g, xlux::F32 b, xlux::F32 a);
	static void SetPixel(xlux::F32 x, xlux::F32 y, xlux::F32 r, xlux::F32 g, xlux::F32 b, xlux::F32 a);
	static void SetTitle(const xlux::String& title);

	inline static xlux::I32 GetWidth() { return s_Instance->m_Width; }
	inline static xlux::I32 GetHeight(){ return s_Instance->m_Height; }
	inline static void SetWidth(xlux::I32 width) { s_Instance->m_Width = width; }
	inline static void SetHeight(xlux::I32 height) { s_Instance->m_Height = height; }
	inline static xlux::RawPtr<WindowFramebuffer> GetFramebuffer() { return s_Instance->m_Framebuffer; }

private:
	Window(const xlux::String& title, xlux::I32 width, xlux::I32 height);
	~Window();

private:
	xlux::String m_Title;
	xlux::I32 m_Width, m_Height;
	xlux::RawPtr<WindowFramebuffer> m_Framebuffer;

	static xlux::RawPtr<Window> s_Instance;
};

class WindowFramebuffer : public xlux::IFramebuffer
{
public:
	WindowFramebuffer() = default;
	~WindowFramebuffer() = default;

	virtual xlux::U32 GetColorAttachmentCount() const override { return 1; }
	virtual xlux::Bool HasDepthAttachment() const override { return true; }
	virtual xlux::Pair<xlux::U32, xlux::U32> GetSize() const override;
	virtual void SetColorPixel(xlux::I32 channel, xlux::I32 x, xlux::I32 y, xlux::F32 r, xlux::F32 g, xlux::F32 b, xlux::F32 a) override;
	virtual void SetDepthPixel(xlux::I32 x, xlux::I32 y, xlux::F32 depth) override;

	virtual void GetColorPixel(xlux::I32 channel, xlux::I32 x, xlux::I32 y, xlux::F32& r, xlux::F32& g, xlux::F32& b, xlux::F32& a) const;
	virtual void GetDepthPixel(xlux::I32 x, xlux::I32 y, xlux::F32& depth) const;


};