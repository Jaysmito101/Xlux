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
        //klux::log::Info("vShader[{}]: {} {} {}", builtIn->VertexIndex, dataIn->position[0], dataIn->position[1], dataIn->position[2]);

        dataOut->position = dataIn->position;
        builtIn->Position = klux::math::Vec4(dataIn->position, 1.0f);
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
        .SetVertexItemSize(sizeof(VertexInData))
        .SetVertexToFragmentDataSize(sizeof(VertexOutData));

    auto pipeline = device->CreatePipeline(createInfo);

    const auto vertices = std::vector<VertexInData>{
        VertexInData ( klux::math::Vec3(-0.5f, -0.5f, 0.0f)),
        VertexInData ( klux::math::Vec3( 0.5f, -0.5f, 0.0f)),
        VertexInData ( klux::math::Vec3( 0.0f,  0.5f, 0.0f))    
	};

    const auto indices = std::vector<klux::U32>{
        0, 1, 2
    };

    auto totalSize = sizeof(VertexInData) * vertices.size() + sizeof(klux::U32) * indices.size();
    auto deviceMemory = device->AllocateMemory(totalSize);

    auto vertexBuffer = device->CreateBuffer(sizeof(VertexInData) * vertices.size());
    vertexBuffer->BindMemory(deviceMemory, 0);
    vertexBuffer->SetData(vertices.data(), sizeof(VertexInData) * vertices.size());

    auto indexBuffer = device->CreateBuffer(sizeof(klux::U32) * indices.size());
    indexBuffer->BindMemory(deviceMemory, sizeof(VertexInData) * vertices.size());
    indexBuffer->SetData(indices.data(), sizeof(klux::U32) * indices.size());

    auto renderer = device->CreateRenderer();


    auto prevTime = klux::utils::GetTime(), currTime = klux::utils::GetTime(), deltaTime = 0.0f;

    while (!Window::HasClosed())
    {
        currTime = klux::utils::GetTime();
        deltaTime = currTime - prevTime;
        prevTime = currTime;

        Window::SetTitle("Klux Engine Sandbox - Jaysmito Mukherjee - FPS: " + std::to_string(1.0f / deltaTime));

        tm += 0.01f;

        renderer->BeginFrame();

        renderer->BindFramebuffer(framebuffer);
        renderer->SetViewport(0, 0, framebuffer->GetWidth(), framebuffer->GetHeight());
        renderer->SetClearColor(0.3f, 0.2f, 0.2f, 1.0f);
        renderer->Clear();
        renderer->BindPipeline(pipeline);
        renderer->DrawIndexed(vertexBuffer, indexBuffer, static_cast<klux::U32>(indices.size()));


        renderer->EndFrame();

        std::this_thread::sleep_for (std::chrono::milliseconds(100));


		Window::SwapBuffer();
		Window::Update();
    }

    device->DestroyRenderer(renderer);

    device->DestroyPipeline(pipeline);

    device->DestroyBuffer(vertexBuffer);
    device->DestroyBuffer(indexBuffer);

    device->FreeMemory(deviceMemory);

    delete vertexShader;
    delete fragmentShader;
    delete interpolator;

    klux::Device::Destroy(device);

    Window::Destroy();
    klux::Logger::Shutdown();
}