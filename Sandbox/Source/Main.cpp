#include "Klux.hpp"
#include "Window.hpp"


struct VertexInData
{
    klux::math::Vec3 position;

    VertexInData(klux::math::Vec3 pos) 
    {
        position = pos;
    }

    VertexInData(const VertexInData& other) 
	{
		position = other.position;
	}
};

struct VertexOutData
{
    klux::math::Vec3 position = klux::math::Vec3(0.0f, 0.0f, 0.0f);

    inline VertexOutData Scaled(klux::F32 scale) const
	{
		return VertexOutData{position * scale};
	}

    inline VertexOutData Add (const VertexOutData& other) const
    {
        return VertexOutData{position + other.position};
	}
};

class HelloWorldVShader : public klux::IShaderG<VertexInData, VertexOutData>
{
public:
    klux::Bool Execute(const klux::RawPtr<VertexInData> dataIn, klux::RawPtr<VertexOutData> dataOut, klux::RawPtr<klux::ShaderBuiltIn> builtIn) 
    {
        dataOut->position = dataIn->position;
        builtIn->Position = klux::math::Vec4(dataIn->position);
		return true;
    }
};

class HelloWorldFShader : public klux::IShaderG<VertexOutData, klux::FragShaderOutput>
{
public:
	klux::Bool Execute(const klux::RawPtr<VertexOutData> dataIn, klux::RawPtr<klux::FragShaderOutput> dataOut, klux::RawPtr<klux::ShaderBuiltIn> builtIn) 
	{
        (void) dataIn;
        (void) builtIn;
		dataOut->Color = klux::math::Vec4(1.0f, 0.0f, 0.0f, 1.0f);
		return true;
	}
};

int main()
{
    klux::Logger::Init();
    Window::Create("Klux Engine Sandbox - Jaysmito Mukherjee", 640, 480);

    klux::log::Info("Klux - High Performance Software Renderer Device");
    klux::F32 tm = 0.0f;

    auto device = klux::Device::Create();

    auto framebuffer = Window::GetFramebuffer();

    auto vertexShader = klux::CreateRawPtr<HelloWorldVShader>();
    auto fragmentShader = klux::CreateRawPtr<HelloWorldFShader>();
    auto interpolator = klux::CreateRawPtr<klux::BasicInterpolator<VertexOutData>>();

    auto createInfo = klux::PipelineCreateInfo()
        .SetShader(vertexShader, klux::ShaderStage_Vertex)
        .SetShader(vertexShader, klux::ShaderStage_Fragment)
        .SetInterpolator(interpolator)
        .SetVertexItemSize(0);

    auto pipeline = device->CreatePipeline(createInfo);

    const auto vertices = std::vector<VertexInData>{
        VertexInData ( klux::math::Vec3(-0.5f, -0.5f, 0.0f)),
        VertexInData ( klux::math::Vec3( 0.5f, -0.5f, 0.0f)),
        VertexInData ( klux::math::Vec3( 0.0f,  0.5f, 0.0f))    
	};

    const auto indices = std::vector<klux::U32>{
        0, 1, 2
    };

    // TODO: Create Vertex Buffer
    // TODO: Create Index Buffer


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

    device->DestroyPipeline(pipeline);

    delete vertexShader;
    delete fragmentShader;
    delete interpolator;

    klux::Device::Destroy(device);

    Window::Destroy();
    klux::Logger::Shutdown();
}