#include "Xlux.hpp"
#include "Window.hpp"


struct VertexInData
{
	xlux::math::Vec3 position;
	xlux::math::Vec3 color;

	VertexInData(xlux::math::Vec3 pos, xlux::math::Vec3 col)
		: position(pos), color(col)
	{}
};

struct VertexOutData
{
	xlux::math::Vec3 position = xlux::math::Vec3(0.0f, 0.0f, 0.0f);
	xlux::math::Vec3 color = xlux::math::Vec3(0.0f, 0.0f, 0.0f);

	inline VertexOutData Scaled(xlux::F32 scale) const
	{
		return VertexOutData(position * scale, color * scale);
	}

	inline void Add(const VertexOutData& other)
	{
		position += other.position;
		color += other.color;
	}
};

class HelloWorldVShader : public xlux::IShaderG<VertexInData, VertexOutData>
{
public:
	xlux::Bool Execute(const xlux::RawPtr<VertexInData> dataIn, xlux::RawPtr<VertexOutData> dataOut, xlux::RawPtr<xlux::ShaderBuiltIn> builtIn)
	{
		dataOut->position = dataIn->position;
		dataOut->color = dataIn->color;
		auto pos = dataIn->position;
		builtIn->Position = xlux::math::Vec4(pos, 1.0f);
		return true;
	}
};

class HelloWorldFShader : public xlux::IShaderG<VertexOutData, xlux::FragmentShaderOutput>
{
public:
	xlux::Bool Execute(const xlux::RawPtr<VertexOutData> dataIn, xlux::RawPtr<xlux::FragmentShaderOutput> dataOut, xlux::RawPtr<xlux::ShaderBuiltIn> builtIn)
	{
		(void)builtIn;
		dataOut->Color[0] = xlux::math::Vec4(dataIn->color, 1.0f);
		return true;
	}
};

int main()
{
	xlux::Logger::Init();
	Window::Create("Xlux Engine Sandbox - Jaysmito Mukherjee", 640, 480);
	xlux::log::Info("Xlux - High Performance Software Renderer Device");

	auto device = xlux::Device::Create();

	auto framebuffer = Window::GetFramebuffer();

	auto vertexShader = xlux::CreateRawPtr<HelloWorldVShader>();
	auto fragmentShader = xlux::CreateRawPtr<HelloWorldFShader>();
	auto interpolator = xlux::CreateRawPtr<xlux::BasicInterpolator<VertexOutData>>();

	auto createInfo = xlux::PipelineCreateInfo()
		.SetShader(vertexShader, xlux::ShaderStage_Vertex)
		.SetShader(fragmentShader, xlux::ShaderStage_Fragment)
		.SetInterpolator(interpolator)
		.SetVertexItemSize(sizeof(VertexInData))
		.SetVertexToFragmentDataSize(sizeof(VertexOutData));

	auto pipeline = device->CreatePipeline(createInfo);

	const auto vertices = std::vector<VertexInData>{
		VertexInData(xlux::math::Vec3(-0.5f, -0.5f, 1.0f), xlux::math::Vec3(1.0f, 0.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(0.5f, -0.5f, 0.0f), xlux::math::Vec3(0.0f, 1.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(-0.5f,  0.5f, 0.0f), xlux::math::Vec3(0.0f, 0.0f, 1.0f)),
		VertexInData(xlux::math::Vec3(0.5f,  0.5f, 0.0f), xlux::math::Vec3(0.0f, 0.0f, 1.0f))
	};

	const auto indices = std::vector<xlux::U32>{
		0, 1, 2
	};

	auto totalSize = sizeof(VertexInData) * vertices.size() + sizeof(xlux::U32) * indices.size();
	auto deviceMemory = device->AllocateMemory(totalSize);

	auto vertexBuffer = device->CreateBuffer(sizeof(VertexInData) * vertices.size());
	vertexBuffer->BindMemory(deviceMemory, 0);
	vertexBuffer->SetData(vertices.data(), sizeof(VertexInData) * vertices.size());

	auto indexBuffer = device->CreateBuffer(sizeof(xlux::U32) * indices.size());
	indexBuffer->BindMemory(deviceMemory, sizeof(VertexInData) * vertices.size());
	indexBuffer->SetData(indices.data(), sizeof(xlux::U32) * indices.size());

	auto renderer = device->CreateRenderer();


	auto prevTime = xlux::utils::GetTime(), currTime = xlux::utils::GetTime(), deltaTime = 0.0f;

	while (!Window::HasClosed())
	{
		currTime = xlux::utils::GetTime();
		deltaTime = currTime - prevTime;
		prevTime = currTime;

		Window::SetTitle("Xlux Engine [Hello Triangle] - Jaysmito Mukherjee - FPS: " + std::to_string(1.0f / deltaTime));

		renderer->BeginFrame();
		renderer->BindFramebuffer(framebuffer);
		renderer->SetViewport(0, 0, framebuffer->GetWidth(), framebuffer->GetHeight());
		renderer->SetClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		renderer->Clear();
		renderer->BindPipeline(pipeline);
		renderer->DrawIndexed(vertexBuffer, indexBuffer, static_cast<xlux::U32>(indices.size()));
		renderer->EndFrame();


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

	xlux::Device::Destroy(device);
	Window::Destroy();
	xlux::Logger::Shutdown();
}