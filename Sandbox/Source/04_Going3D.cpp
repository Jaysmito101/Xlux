#include "Xlux.hpp"
#include "Window.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


struct VertexInData
{
	xlux::math::Vec3 position;
	xlux::math::Vec3 normal;
	xlux::math::Vec3 texCoord;

	VertexInData(xlux::math::Vec3 pos, xlux::math::Vec3 normal, xlux::math::Vec3 texCoord)
		: position(pos), texCoord(texCoord), normal(normal)
	{}
};

struct VertexOutData
{
	xlux::math::Vec3 position = xlux::math::Vec3(0.0f, 0.0f, 0.0f);
	xlux::math::Vec3 texCoord = xlux::math::Vec3(0.0f, 0.0f, 0.0f);
	xlux::math::Vec3 normal = xlux::math::Vec3(0.0f, 0.0f, 0.0f);

	inline VertexOutData Scaled(xlux::F32 scale) const
	{
		return VertexOutData(position * scale, texCoord * scale, normal * scale);
	}

	inline void Add(const VertexOutData& other)
	{
		position += other.position;
		texCoord += other.texCoord;
		normal += other.normal;
	}
};

class HelloWorldVShader : public xlux::IShaderG<VertexInData, VertexOutData>
{
public:
	xlux::math::Mat4x4 model = xlux::math::Mat4x4::Identity();
	xlux::math::Mat4x4 viewProj = xlux::math::Mat4x4::Identity();

public:
	xlux::Bool Execute(const xlux::RawPtr<VertexInData> dataIn, xlux::RawPtr<VertexOutData> dataOut, xlux::RawPtr<xlux::ShaderBuiltIn> builtIn)
	{
		dataOut->texCoord = dataIn->texCoord;
		dataOut->position = dataIn->position;
		dataOut->normal = dataIn->normal;
		auto pos = viewProj.Mul(model.Mul(xlux::math::Vec4(dataIn->position, 1.0f)));
		//auto pos = dataIn->position;
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
		const auto lightPos = xlux::math::Vec3(0.0f, 1.0f, 0.0f);
		const auto lightColor = xlux::math::Vec3(1.0f, 1.0f, 1.0f);
		const auto lightDir = (lightPos - dataIn->position).Normalized();
		const auto lightIntensity = std::max(0.0f, lightDir.Dot(dataIn->normal * -1.0f));
		dataOut->Color[0] = texture->Sample(dataIn->texCoord) * lightIntensity * lightColor;
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
		.SetDepthTestEnable(true)
		//.SetClippingEnable(true)
		.SetDepthCompareFunction(xlux::CompareFunction_Less)
		.SetVertexItemSize(sizeof(VertexInData))
		.SetVertexToFragmentDataSize(sizeof(VertexOutData));

	auto pipeline = device->CreatePipeline(createInfo);
	
	// Vectices for a cube with vertex Normals (all 36 vertices)
	const auto vertices = std::vector<VertexInData>{
        // Front
		VertexInData(xlux::math::Vec3(-0.5f, -0.5f, 0.5f), xlux::math::Vec3(0.0f, 0.0f, 1.0f), xlux::math::Vec3(0.0f, 0.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(0.5f, -0.5f, 0.5f), xlux::math::Vec3(0.0f, 0.0f, 1.0f), xlux::math::Vec3(1.0f, 0.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(0.5f, 0.5f, 0.5f), xlux::math::Vec3(0.0f, 0.0f, 1.0f), xlux::math::Vec3(1.0f, 1.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(0.5f, 0.5f, 0.5f), xlux::math::Vec3(0.0f, 0.0f, 1.0f), xlux::math::Vec3(1.0f, 1.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(-0.5f, 0.5f, 0.5f), xlux::math::Vec3(0.0f, 0.0f, 1.0f), xlux::math::Vec3(0.0f, 1.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(-0.5f, -0.5f, 0.5f), xlux::math::Vec3(0.0f, 0.0f, 1.0f), xlux::math::Vec3(0.0f, 0.0f, 0.0f)),
		// Back
        VertexInData(xlux::math::Vec3(-0.5f, -0.5f, -0.5f), xlux::math::Vec3(0.0f, 0.0f, -1.0f), xlux::math::Vec3(0.0f, 0.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(0.5f, -0.5f, -0.5f), xlux::math::Vec3(0.0f, 0.0f, -1.0f), xlux::math::Vec3(1.0f, 0.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(0.5f, 0.5f, -0.5f), xlux::math::Vec3(0.0f, 0.0f, -1.0f), xlux::math::Vec3(1.0f, 1.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(0.5f, 0.5f, -0.5f), xlux::math::Vec3(0.0f, 0.0f, -1.0f), xlux::math::Vec3(1.0f, 1.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(-0.5f, 0.5f, -0.5f), xlux::math::Vec3(0.0f, 0.0f, -1.0f), xlux::math::Vec3(0.0f, 1.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(-0.5f, -0.5f, -0.5f), xlux::math::Vec3(0.0f, 0.0f, -1.0f), xlux::math::Vec3(0.0f, 0.0f, 0.0f)),
		// Left
        VertexInData(xlux::math::Vec3(-0.5f, 0.5f, -0.5f), xlux::math::Vec3(-1.0f, 0.0f, 0.0f), xlux::math::Vec3(1.0f, 0.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(-0.5f, 0.5f, 0.5f), xlux::math::Vec3(-1.0f, 0.0f, 0.0f), xlux::math::Vec3(1.0f, 1.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(-0.5f, -0.5f, 0.5f), xlux::math::Vec3(-1.0f, 0.0f, 0.0f), xlux::math::Vec3(0.0f, 1.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(-0.5f, -0.5f, 0.5f), xlux::math::Vec3(-1.0f, 0.0f, 0.0f), xlux::math::Vec3(0.0f, 1.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(-0.5f, -0.5f, -0.5f), xlux::math::Vec3(-1.0f, 0.0f, 0.0f), xlux::math::Vec3(0.0f, 0.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(-0.5f, 0.5f, -0.5f), xlux::math::Vec3(-1.0f, 0.0f, 0.0f), xlux::math::Vec3(1.0f, 0.0f, 0.0f)),
		// Right
        VertexInData(xlux::math::Vec3(0.5f, 0.5f, 0.5f), xlux::math::Vec3(1.0f, 0.0f, 0.0f), xlux::math::Vec3(1.0f, 0.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(0.5f, 0.5f, -0.5f), xlux::math::Vec3(1.0f, 0.0f, 0.0f), xlux::math::Vec3(1.0f, 1.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(0.5f, -0.5f, -0.5f), xlux::math::Vec3(1.0f, 0.0f, 0.0f), xlux::math::Vec3(0.0f, 1.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(0.5f, -0.5f, -0.5f), xlux::math::Vec3(1.0f, 0.0f, 0.0f), xlux::math::Vec3(0.0f, 1.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(0.5f, -0.5f, 0.5f), xlux::math::Vec3(1.0f, 0.0f, 0.0f), xlux::math::Vec3(0.0f, 0.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(0.5f, 0.5f, 0.5f), xlux::math::Vec3(1.0f, 0.0f, 0.0f), xlux::math::Vec3(1.0f, 0.0f, 0.0f)),
		// Bottom
        VertexInData(xlux::math::Vec3(-0.5f, -0.5f, -0.5f), xlux::math::Vec3(0.0f, -1.0f, 0.0f), xlux::math::Vec3(0.0f, 1.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(0.5f, -0.5f, -0.5f), xlux::math::Vec3(0.0f, -1.0f, 0.0f), xlux::math::Vec3(1.0f, 1.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(0.5f, -0.5f, 0.5f), xlux::math::Vec3(0.0f, -1.0f, 0.0f), xlux::math::Vec3(1.0f, 0.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(0.5f, -0.5f, 0.5f), xlux::math::Vec3(0.0f, -1.0f, 0.0f), xlux::math::Vec3(1.0f, 0.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(-0.5f, -0.5f, 0.5f), xlux::math::Vec3(0.0f, -1.0f, 0.0f), xlux::math::Vec3(0.0f, 0.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(-0.5f, -0.5f, -0.5f), xlux::math::Vec3(0.0f, -1.0f, 0.0f), xlux::math::Vec3(0.0f, 1.0f, 0.0f)),
		// Top
        VertexInData(xlux::math::Vec3(-0.5f, 0.5f, -0.5f), xlux::math::Vec3(0.0f, 1.0f, 0.0f), xlux::math::Vec3(0.0f, 1.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(0.5f, 0.5f, -0.5f), xlux::math::Vec3(0.0f, 1.0f, 0.0f), xlux::math::Vec3(1.0f, 1.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(0.5f, 0.5f, 0.5f), xlux::math::Vec3(0.0f, 1.0f, 0.0f), xlux::math::Vec3(1.0f, 0.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(0.5f, 0.5f, 0.5f), xlux::math::Vec3(0.0f, 1.0f, 0.0f), xlux::math::Vec3(1.0f, 0.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(-0.5f, 0.5f, 0.5f), xlux::math::Vec3(0.0f, 1.0f, 0.0f), xlux::math::Vec3(0.0f, 0.0f, 0.0f)),
		VertexInData(xlux::math::Vec3(-0.5f, 0.5f, -0.5f), xlux::math::Vec3(0.0f, 1.0f, 0.0f), xlux::math::Vec3(0.0f, 1.0f, 0.0f)),
	};

	// for 0 - 36 all
	std::vector<xlux::U32> indices = {
        0, 1, 2, 3, 4, 5, // Front
		6, 7, 8, 9, 10, 11, // Back
		12, 13, 14, 15, 16, 17, // Left
		18, 19, 20, 21, 22, 23, // Right
		24, 25, 26, 27, 28, 29, // Bottom
		30, 31, 32, 33, 34, 35 // Top	
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


	auto renderer = device->CreateRenderer();

	auto prevTime = xlux::utils::GetTime(), currTime = xlux::utils::GetTime(), deltaTime = 0.0f;

	auto proj = xlux::math::Mat4x4::Perspective(xlux::math::ToRadians(66.0f), (float)framebuffer->GetWidth() / (float)framebuffer->GetHeight(), 0.1f, 100.0f);
	auto view = xlux::math::Mat4x4::LookAt(xlux::math::Vec3(2.0f, 2.0f, 2.0f), xlux::math::Vec3(0.0f, 0.0f, 0.0f), xlux::math::Vec3(0.0f, 1.0f, 0.0f));

	fragmentShader->texture = texture;

	while (!Window::HasClosed())
	{
		currTime = xlux::utils::GetTime();
		deltaTime = currTime - prevTime;
		prevTime = currTime;

		Window::SetTitle("Xlux Engine [Going 3D] - Jaysmito Mukherjee - FPS: " + std::to_string(1.0f / deltaTime));

		proj = xlux::math::Mat4x4::Perspective(xlux::math::ToRadians(66.0f), (float)framebuffer->GetWidth() / (float)framebuffer->GetHeight(), 0.1f, 100.0f);
		vertexShader->model = xlux::math::Mat4x4::RotateY(currTime * 0.5f);
		vertexShader->viewProj = proj.Mul(view);

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