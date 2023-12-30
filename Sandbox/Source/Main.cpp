#include "Klux.hpp"
#include "Window.hpp"


struct VertexInData
{
	klux::math::Vec3 position;
	klux::math::Vec3 color;

	VertexInData(klux::math::Vec3 pos, klux::math::Vec3 col)
		: position(pos), color(col)
	{}
};

struct VertexOutData
{
	klux::math::Vec3 position = klux::math::Vec3(0.0f, 0.0f, 0.0f);
	klux::math::Vec3 color = klux::math::Vec3(0.0f, 0.0f, 0.0f);

	inline VertexOutData Scaled(klux::F32 scale) const
	{
		return VertexOutData(position * scale, color * scale);
	}

	inline void Add(const VertexOutData& other)
	{
		position += other.position;
		color += other.color;

	}
};

class HelloWorldVShader : public klux::IShaderG<VertexInData, VertexOutData>
{
public:
	klux::Bool Execute(const klux::RawPtr<VertexInData> dataIn, klux::RawPtr<VertexOutData> dataOut, klux::RawPtr<klux::ShaderBuiltIn> builtIn)
	{
		//klux::log::Info("vShader[{}]: {} {} {}", builtIn->VertexIndex, dataIn->position[0], dataIn->position[1], dataIn->position[2]);

		dataOut->position = dataIn->position;
		dataOut->color = dataIn->color;

		auto pos = dataIn->position;
		//pos[0] *= std::sinf(klux::utils::GetTime() * 2.0f) + 1.0f;
		//pos[1] *= std::cosf(klux::utils::GetTime() * 2.0f);
		builtIn->Position = klux::math::Vec4(pos, 1.0f);
		return true;
	}
};

class HelloWorldFShader : public klux::IShaderG<VertexOutData, klux::FragmentShaderOutput>
{
public:
	klux::Bool Execute(const klux::RawPtr<VertexOutData> dataIn, klux::RawPtr<klux::FragmentShaderOutput> dataOut, klux::RawPtr<klux::ShaderBuiltIn> builtIn)
	{
		(void)builtIn;
		dataOut->Color[0] = klux::math::Vec4(dataIn->color, 1.0f);
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
		.SetShader(fragmentShader, klux::ShaderStage_Fragment)
		.SetInterpolator(interpolator)
		.SetVertexItemSize(sizeof(VertexInData))
		.SetVertexToFragmentDataSize(sizeof(VertexOutData));

	auto pipeline = device->CreatePipeline(createInfo);

	const auto vertices = std::vector<VertexInData>{
		VertexInData(klux::math::Vec3(-0.5f, -0.5f, 0.0f), klux::math::Vec3(0.0f, 0.0f, 0.0f)),
		VertexInData(klux::math::Vec3(0.5f,  0.5f, 0.0f), klux::math::Vec3(1.0f, 1.0f, 0.0f)),
		VertexInData(klux::math::Vec3(0.5f, -0.5f, 0.0f), klux::math::Vec3(1.0f, 0.0f, 0.0f)),
		VertexInData(klux::math::Vec3(-0.5f,  0.5f, 0.0f), klux::math::Vec3(0.0f, 1.0f, 0.0f))
	};

	const auto indices = std::vector<klux::U32>{
		3, 2, 0,
		3, 1, 2
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

		// klux::log::Trace("---------------------------------- FRAME -------------------------------");
		 //std::this_thread::sleep_for(std::chrono::milliseconds(1000));


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