#include "Klux.hpp"
#include "Window.hpp"

int main()
{
    klux::Logger::Init();
    Window::Create("Klux Engine Sandbox - Jaysmito Mukherjee", 640, 480);

    klux::log::Info("Klux - High Performance Software Renderer Device");
    klux::F32 tm = 0.0f;

    auto device = klux::Device::Create();

    auto framebuffer = Window::GetFramebuffer();

    auto memory = device->AllocateMemory(framebuffer->GetWidth() * framebuffer->GetHeight() * 4);

    device->FreeMemory(memory);

    auto prevTime = klux::utils::GetTime(), currTime = klux::utils::GetTime(), deltaTime = 0.0f;

    while (!Window::HasClosed())
    {
        currTime = klux::utils::GetTime();
        deltaTime = currTime - prevTime;
        prevTime = currTime;

        Window::Clear(0.2f, 0.3f, 0.8f, 1.0f);
		
        Window::SetPixel(0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f);

        tm += 0.01f;
        for (int i = 0; i < framebuffer->GetWidth(); i++)
        {
            for (int j = 0; j < framebuffer->GetHeight(); j++)
            {
				// klux::F32 fx = (klux::F32)i / Window::GetWidth();
				// klux::F32 fy = (klux::F32)j / Window::GetHeight();
				// Window::SetPixel(fx, fy, fabsf(sinf(tm + fx)), fabsf(sinf(tm + fy)), fabsf(sinf(tm)), 1.0f);
                framebuffer->SetColorPixel(
                    0, i, j,
                    fabsf(sinf(tm + (klux::F32)i / framebuffer->GetWidth())),
                    fabsf(sinf(tm + (klux::F32)j / framebuffer->GetHeight())),
                    fabsf(sinf(tm)), 1.0f
                );
			}
		}

		Window::SwapBuffer();
		Window::Update();
    }

    klux::Device::Destroy(device);

    Window::Destroy();
    klux::Logger::Shutdown();
}