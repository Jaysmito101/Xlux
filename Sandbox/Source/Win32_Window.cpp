#if defined(_WIN32) || defined(_WIN64)

#define RGBA(r, g, b, a) ((b) | ((g) << 8) | ((r) << 16) | ((a) << 24))


#include "Window.hpp"

klux::RawPtr<Window> Window::s_Instance = nullptr;

struct Win32_GraphicsBuffer
{
	HBITMAP hbm = NULL;
	klux::RawPtr<klux::U32> data = nullptr;

	Win32_GraphicsBuffer()
	{ }

	Win32_GraphicsBuffer(klux::I32 wd, klux::I32 hgt, klux::Bool onlyMemory = false)
	{
		if (onlyMemory)
		{
			this->data = new klux::U32[wd * hgt];
			memset(this->data, 0, wd * hgt * sizeof(klux::U32));
			if (!this->data)
			{
				klux::log::Error("Failed to allocate memory for graphics buffer");
				return;
			}
		}
		else
		{
			HDC hdcScreen = GetDC(NULL);
			BITMAPINFO bmi = { 0 };
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = wd;
			bmi.bmiHeader.biHeight = -hgt; // top-down
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;
			this->hbm = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, (void**)(&this->data), NULL, NULL);
			ReleaseDC(NULL, hdcScreen);
		}
	}

	~Win32_GraphicsBuffer()
	{
		if (this->hbm)
		{
			DeleteObject(this->hbm);
		}
		else
		{
			delete[] this->data;
		}
	}
};

static klux::Bool s_HasClosed = false;
static HWND s_WindowHandle = NULL;
static HINSTANCE s_ModuleHandle = NULL;
static klux::RawPtr<Win32_GraphicsBuffer> s_FrontBuffer;
static klux::RawPtr<Win32_GraphicsBuffer> s_BackBuffer;
static klux::RawPtr<Window> s_Window = nullptr;


LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		RECT r = { 0 };
		GetClientRect(s_WindowHandle, &r);
		break;
	}
	case WM_PAINT:
	{
        PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		HDC hdcMem = CreateCompatibleDC(hdc);
		SelectObject(hdcMem, s_FrontBuffer->hbm);
		BitBlt(hdc, 0, 0, Window::GetWidth(), Window::GetHeight(), hdcMem, 0, 0, SRCCOPY);
		DeleteDC(hdcMem);
		EndPaint(hwnd, &ps);
		break;
	}
	case WM_SIZE:
	{
		Window::SetWidth(LOWORD(lParam));
		Window::SetHeight(HIWORD(lParam));

		delete s_FrontBuffer;
		delete s_BackBuffer;

		s_FrontBuffer = klux::CreateRawPtr<Win32_GraphicsBuffer>( Window::GetWidth(), Window::GetHeight(), false);
		s_BackBuffer = klux::CreateRawPtr<Win32_GraphicsBuffer>(Window::GetWidth(), Window::GetHeight(), false);

		klux::EventManager<WindowResize, klux::I32, klux::I32>::Get()->RaiseEvent( Window::GetWidth(), Window::GetHeight() );

		break;
	}
	case WM_DESTROY:
	{
		s_HasClosed = true;
		break;
	}
	case WM_CLOSE:
	{
		klux::EventManager<WindowClose>::Get()->RaiseEvent();
		s_HasClosed = true;
		DestroyWindow(hwnd);
		PostQuitMessage(0);
		break;
	}
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}


Window::Window(const std::string& title, int width, int height)
{
	m_Title = title;
	m_Width = width;
	m_Height = height;
	s_Instance = this;

	s_ModuleHandle = GetModuleHandle(NULL);

	m_Framebuffer = klux::CreateRawPtr<WindowFramebuffer>();


	WNDCLASSEX wincl = { 0 };
	wincl.hInstance = s_ModuleHandle;
	wincl.lpszClassName = m_Title.c_str();
	wincl.lpfnWndProc = WindowProcedure;
	wincl.style = CS_DBLCLKS;
	wincl.cbSize = sizeof(WNDCLASSEX);
	wincl.hIcon = NULL;
	wincl.hIconSm = NULL;
	wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wincl.lpszMenuName = NULL;
	wincl.cbClsExtra = 0;
	wincl.cbWndExtra = 0;
	wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

	if (!RegisterClassEx(&wincl))
	{
		klux::log::Error("Failed to register window class");
		return;
	}


	s_WindowHandle = CreateWindowEx(
		0,                   /* Extended possibilites for variation */
		m_Title.c_str(),         /* Classname */
		m_Title.c_str(),       /* Title Text */
		WS_OVERLAPPEDWINDOW, /* default window */
		CW_USEDEFAULT,       /* Windows decides the position */
		CW_USEDEFAULT,       /* where the window ends up on the screen */
		m_Width,                 /* The programs width */
		m_Height,                 /* and height in pixels */
		HWND_DESKTOP,        /* The window is a child-window to desktop */
		NULL,                /* No menu */
		s_ModuleHandle,       /* Program Instance handler */
		NULL                 /* No Window Creation data */
	);

	ShowWindow(s_WindowHandle, SW_SHOW);

	s_HasClosed = false;


	s_FrontBuffer = klux::CreateRawPtr<Win32_GraphicsBuffer>(m_Width, m_Height, false);
	s_BackBuffer = klux::CreateRawPtr<Win32_GraphicsBuffer>(m_Width, m_Height, true);

	s_Window = this;


}

klux::Bool Window::HasClosed()
{
	return s_HasClosed;
}

void Window::Update()
{
	MSG messages = { 0 };
	while (GetMessage(&messages, NULL, 0, 0))
	{
		// klux::log::Info("Mesage : {0}", Win32MessageToString(messages.message));
		TranslateMessage(&messages);
		DispatchMessage(&messages);
		break;
	}
}

void Window::SwapBuffer()
{
	memcpy(s_FrontBuffer->data, s_BackBuffer->data, s_Instance->GetWidth() * s_Instance->GetHeight() * sizeof(klux::U32));
	InvalidateRect(s_WindowHandle, NULL, FALSE);
}

void Window::Clear(klux::F32 r, klux::F32 g, klux::F32 b, klux::F32 a)
{
	int wd = Window::GetWidth(), hgt = Window::GetHeight();
	int pvr = (int)(r * 255.0f), pvg = (int)(g * 255.0f), pvb = (int)(b * 255.0f), pva = (int)(a * 255.0f);
	unsigned int pixel = RGBA(pvr, pvg, pvb, pva);
	for (int i = 0; i < wd * hgt; ++i) s_BackBuffer->data[i] = pixel;
}

void Window::SetPixel(klux::F32 x, klux::F32 y, klux::F32 r, klux::F32 g, klux::F32 b, klux::F32 a)
{
	int wd = Window::GetWidth(), hgt = Window::GetHeight();
	int px = (int)(x * (wd - 1)), py = (int)(y * (hgt - 1));
	if (px < 0 || px >= wd || py < 0 || py >= hgt) return;
	int index = py * wd + px;
	int pvr = (int)(r * 255.0f), pvg = (int)(g * 255.0f), pvb = (int)(b * 255.0f), pva = (int)(a * 255.0f);
	s_BackBuffer->data[index] = RGBA(pvr, pvg, pvb, pva);
}

void Window::SetTitle(const klux::String& title)
{
	SetWindowText(s_WindowHandle, title.c_str());
}

Window::~Window()
{
	delete m_Framebuffer;

	s_Window = nullptr;
	s_HasClosed = false;
	s_ModuleHandle = NULL;
	s_WindowHandle = NULL;
	delete s_FrontBuffer;
	delete s_BackBuffer;
}

#endif