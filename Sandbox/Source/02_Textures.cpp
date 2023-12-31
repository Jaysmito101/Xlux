#include "Xlux.hpp"
#include "Window.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


struct VertexInData
{
	xlux::math::Vec3 position;
	xlux::math::Vec3 texCoord;

	VertexInData(xlux::math::Vec3 pos, xlux::math::Vec3 texCoord)
		: position(pos), texCoord(texCoord)
	{}
};

struct VertexOutData
{
	xlux::math::Vec3 position = xlux::math::Vec3(0.0f, 0.0f, 0.0f);
	xlux::math::Vec3 texCoord = xlux::math::Vec3(0.0f, 0.0f, 0.0f);

	inline VertexOutData Scaled(xlux::F32 scale) const
	{
		return VertexOutData(position * scale, texCoord * scale);
	}

	inline void Add(const VertexOutData& other)
	{
		position += other.position;
		texCoord += other.texCoord;
	}
};

class HelloWorldVShader : public xlux::IShaderG<VertexInData, VertexOutData>
{
public:
	xlux::Bool Execute(const xlux::RawPtr<VertexInData> dataIn, xlux::RawPtr<VertexOutData> dataOut, xlux::RawPtr<xlux::ShaderBuiltIn> builtIn)
	{
		dataOut->position = dataIn->position;
		dataOut->texCoord = dataIn->texCoord;
		auto pos = dataIn->position;
		builtIn->Position = xlux::math::Vec4(pos, 1.0f);
		return true;
	}
};

class HelloWorldFShader : public xlux::IShaderG<VertexOutData, xlux::FragmentShaderOutput>
{
public:
	xlux::RawPtr<xlux::Texture2D> texture;

public:
	xlux::Bool Execute(const xlux::RawPtr<VertexOutData> dataIn, xlux::RawPtr<xlux::FragmentShaderOutput> dataOut, xlux::RawPtr<xlux::ShaderBuiltIn> builtIn)
	{
		(void)builtIn; // unused
		dataOut->Color[0] = texture->Sample(dataIn->texCoord);
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
		VertexInData(xlux::math::Vec3(-0.5f, -0.5f, 0.0f), xlux::math::Vec3(0.0f, 0.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(0.5f, -0.5f, 0.0f), xlux::math::Vec3(1.0f, 0.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(-0.5f,  0.5f, 0.0f), xlux::math::Vec3(0.0f, 1.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(0.5f,  0.5f, 1.0f), xlux::math::Vec3(1.0f, 1.0f, 0.0f))
	};

	const auto indices = std::vector<xlux::U32>{
		0, 1, 2,
		1, 2, 3
	};

	stbi_set_flip_vertically_on_load(true);
	xlux::I32 tWidth = 0, tHeight = 0, tChannels = 0;
	auto textureData = stbi_loadf((xlux::utils::GetExecutableDirectory() + "/logo.jpg").c_str(), &tWidth, &tHeight, &tChannels, 3);
	auto texture = device->CreateTexture2D(tWidth, tHeight, xlux::TexelFormat_RGB);
	

	auto totalSize = sizeof(VertexInData) * vertices.size() + sizeof(xlux::U32) * indices.size() + texture->GetSizeInBytes();
	auto deviceMemory = device->AllocateMemory(totalSize);

	auto vertexBuffer = device->CreateBuffer(sizeof(VertexInData) * vertices.size());
	vertexBuffer->BindMemory(deviceMemory, 0);
	vertexBuffer->SetData(vertices.data(), sizeof(VertexInData) * vertices.size());

	auto indexBuffer = device->CreateBuffer(sizeof(xlux::U32) * indices.size());
	indexBuffer->BindMemory(deviceMemory, sizeof(VertexInData) * vertices.size());
	indexBuffer->SetData(indices.data(), sizeof(xlux::U32) * indices.size());

	auto textureBuffer = device->CreateBuffer(texture->GetSizeInBytes());
	textureBuffer->BindMemory(deviceMemory, sizeof(VertexInData) * vertices.size() + sizeof(xlux::U32) * indices.size());
	texture->BindBuffer(textureBuffer);
	texture->SetData(textureData, texture->GetSizeInBytes(), 0);
	stbi_image_free(textureData);

	texture->Sample(xlux::math::Vec3(0.5f, 0.5f, 1.0f));

	auto renderer = device->CreateRenderer();


	auto prevTime = xlux::utils::GetTime(), currTime = xlux::utils::GetTime(), deltaTime = 0.0f;

	fragmentShader->texture = texture;

	while (!Window::HasClosed())
	{
		currTime = xlux::utils::GetTime();
		deltaTime = currTime - prevTime;
		prevTime = currTime;

		Window::SetTitle("Xlux Engine [Hello Texture] - Jaysmito Mukherjee - FPS: " + std::to_string(1.0f / deltaTime));

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
	device->DestroyTexture(texture);
	device->DestroyBuffer(textureBuffer);
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