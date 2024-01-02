#include "Xlux.hpp"
#include "Window.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

struct VertexInData
{
	xlux::math::Vec3 position;
	xlux::math::Vec3 normal;
	xlux::math::Vec3 texCoord;

	VertexInData(xlux::math::Vec3 pos, xlux::math::Vec3 normal, xlux::math::Vec3 texCoord)
		: position(pos), texCoord(texCoord), normal(normal)
	{}

	VertexInData()
		: position(0.0f, 0.0f, 0.0f), texCoord(0.0f, 0.0f, 0.0f), normal(0.0f, 0.0f, 0.0f)
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
		auto pos = viewProj.Mul(model.Mul(xlux::math::Vec4(dataIn->position, 1.0f)));
		dataOut->position = pos;
		dataOut->normal = model.Mul(xlux::math::Vec4(dataIn->normal, 0.0f));
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
		(void)builtIn;
		const auto lightPos = xlux::math::Vec3(100.0f, 50.0f, 100.0f);
		const auto lightDir = (lightPos - dataIn->position).Normalized();
		const auto lightIntensity = std::max(0.0f, lightDir.Dot(dataIn->normal * -1.0f));
		dataOut->Color[0] = texture->Sample(dataIn->texCoord) * (lightIntensity + 0.3f);
		dataOut->Color[0][3] = 1.0f;
		dataOut->Color[0] = dataOut->Color[0].Pow(1.0f / 2.2f);
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
		.SetClippingEnable(true)
		.SetDepthCompareFunction(xlux::CompareFunction_Less)
		.SetVertexItemSize(sizeof(VertexInData))
		.SetVertexToFragmentDataSize(sizeof(VertexOutData));

	auto pipeline = device->CreatePipeline(createInfo);
	
	// Vectices for a cube with vertex Normals (all 36 vertices)
	std::vector<VertexInData> vertices = {};
	std::vector<xlux::U32> indices = {};

	// Load the model
	const auto modelPath = xlux::utils::GetExecutableDirectory() + "/model.obj";
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	xlux::log::Info("Loading model from: {}", modelPath);
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str()))
	{
		xlux::log::Error("Failed to load model: {}", err);
		return -1;
	}

	xlux::log::Info("Model loaded successfully");

	const auto shape = shapes[0]; // only bother with the first shape

	auto v_index_offset = 0;
	for (auto face : shape.mesh.num_face_vertices)
	{
		for (auto v = 0u; v < face; v++)
		{
			auto index = shape.mesh.indices[v_index_offset + v];
			auto vertex = VertexInData();
			vertex.position = xlux::math::Vec3(
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			);
			vertex.texCoord = xlux::math::Vec3(
				attrib.texcoords[2 * index.texcoord_index + 0],
				attrib.texcoords[2 * index.texcoord_index + 1],
				0.0f
			);
			vertex.normal = xlux::math::Vec3(
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			);
			
			vertices.push_back(vertex);
			indices.push_back(v_index_offset + v);
		}
		v_index_offset += face;
	}	
	

	xlux::log::Info("Vertices: {}", vertices.size());
	xlux::log::Info("Indices: {}", indices.size());



	stbi_set_flip_vertically_on_load(true);
	xlux::I32 tWidth = 0, tHeight = 0, tChannels = 0;
	auto textureData = stbi_loadf((xlux::utils::GetExecutableDirectory() + "/texture.png").c_str(), &tWidth, &tHeight, &tChannels, 3);
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
	auto view = xlux::math::Mat4x4::LookAt(xlux::math::Vec3(12.0f, 12.0f, 12.0f), xlux::math::Vec3(0.0f, 0.0f, 0.0f), xlux::math::Vec3(0.0f, 1.0f, 0.0f));

	fragmentShader->texture = texture;

	while (!Window::HasClosed())
	{
		currTime = xlux::utils::GetTime();
		deltaTime = currTime - prevTime;
		prevTime = currTime;

		Window::SetTitle("Xlux Engine [Model Loading] - Jaysmito Mukherjee - FPS: " + std::to_string(1.0f / deltaTime));
		const xlux::F32 dist = 220.0f;
		view = xlux::math::Mat4x4::LookAt(xlux::math::Vec3(dist * cos(currTime * 0.5f), 12.0f, dist * sin(currTime * 0.5f)), xlux::math::Vec3(0.0f, 0.0f, 0.0f), xlux::math::Vec3(0.0f, 1.0f, 0.0f));
		proj = xlux::math::Mat4x4::Perspective(xlux::math::ToRadians(66.0f), (float)framebuffer->GetWidth() / (float)framebuffer->GetHeight(), 0.1f, 10000.0f);
		// vertexShader->model = xlux::math::Mat4x4::RotateY(currTime * 0.5f);
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