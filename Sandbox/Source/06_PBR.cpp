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
	xlux::math::Vec3 tangent;
	xlux::math::Vec3 bitangent;

	VertexInData(xlux::math::Vec3 pos, xlux::math::Vec3 normal, xlux::math::Vec3 texCoord)
		: position(pos), texCoord(texCoord), normal(normal), tangent(0.0f, 0.0f, 0.0f), bitangent(0.0f, 0.0f, 0.0f)
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
	xlux::math::Vec3 tangent = xlux::math::Vec3(0.0f, 0.0f, 0.0f);
	xlux::math::Vec3 bitangent = xlux::math::Vec3(0.0f, 0.0f, 0.0f);

	inline VertexOutData Scaled(xlux::F32 scale) const
	{
		return VertexOutData(position * scale, texCoord * scale, normal * scale, tangent * scale, bitangent * scale);
	}

	inline void Add(const VertexOutData& other)
	{
		position += other.position;
		texCoord += other.texCoord;
		normal += other.normal;
		tangent += other.tangent;
		bitangent += other.bitangent;
	}
};

struct XTexture
{
	XTexture(const std::string& path_, xlux::RawPtr<xlux::Device> device_)
	{
		device = device_;
		xlux::log::Info("Loading Texture: {}", path_);
		stbi_set_flip_vertically_on_load(true);
		xlux::I32 tWidth = 0, tHeight = 0, tChannels = 0;
		auto textureData = stbi_loadf(path_.c_str(), &tWidth, &tHeight, &tChannels, 3);
		xlux::log::Info("Texture Size: {}x{}", tWidth, tHeight);

		if (!textureData)
		{
			xlux::log::Error("Failed to load texture: {}", path_);
			return;
		}

		texture = device->CreateTexture2D(tWidth, tHeight, xlux::TexelFormat_RGB);
		auto totalSize = texture->GetSizeInBytes();
		deviceMemory = device->AllocateMemory(totalSize);

		buffer = device->CreateBuffer(texture->GetSizeInBytes());
		buffer->BindMemory(deviceMemory);
		texture->BindBuffer(buffer);
		texture->SetData(textureData, texture->GetSizeInBytes(), 0);
		stbi_image_free(textureData);

		xlux::log::Info("Texture Loaded: {}", path_);
	}

	void Destroy()
	{
		device->DestroyTexture(texture);
		device->DestroyBuffer(buffer);
		device->FreeMemory(deviceMemory);
	}

	xlux::RawPtr<xlux::Device> device = nullptr;
	xlux::RawPtr<xlux::Buffer> buffer = nullptr;
	xlux::RawPtr<xlux::Texture2D> texture = nullptr;
	xlux::RawPtr<xlux::DeviceMemory> deviceMemory = nullptr;
};

struct XMesh
{
	XMesh(const std::string& path_, xlux::RawPtr<xlux::Device> device_)
	{
		device = device_;

		// Load the model
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		xlux::log::Info("Loading Model: {}", path_);
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path_.c_str()))
		{
			xlux::log::Error("Failed to load model: {}", err);
			return;
		}

		if (!warn.empty())
		{
			xlux::log::Warn("{}", warn);
		}

		xlux::log::Info("Number of shapes: {}", shapes.size());
		const auto shape = shapes[0]; // only bother with the first shape

		xlux::log::Info("Processing vertices...");
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


		xlux::log::Info("Vertices in mesh: {}", vertices.size());
		xlux::log::Info("Indices in mesh: {}", indices.size());

		auto totalSize = sizeof(VertexInData) * vertices.size() + sizeof(xlux::U32) * indices.size();
		deviceMemory = device->AllocateMemory(totalSize);

		vertexBuffer = device->CreateBuffer(sizeof(VertexInData) * vertices.size());
		vertexBuffer->BindMemory(deviceMemory, 0);
		vertexBuffer->SetData(vertices.data(), sizeof(VertexInData) * vertices.size());

		indexBuffer = device->CreateBuffer(sizeof(xlux::U32) * indices.size());
		indexBuffer->BindMemory(deviceMemory, sizeof(VertexInData) * vertices.size());
		indexBuffer->SetData(indices.data(), sizeof(xlux::U32) * indices.size());

		vertexCount = static_cast<xlux::U32>(vertices.size());
		indexCount = static_cast<xlux::U32>(indices.size());

		CalculateTangentBasis();

		xlux::log::Info("Model Loaded: {}", path_);
	}

	void CalculateTangentBasis()
	{
		xlux::log::Info("Calculating Tangent Basis...");
        for (auto i = 0u; i < indices.size(); i += 3)
		{
			auto v0 = vertices[indices[i + 0]];
			auto v1 = vertices[indices[i + 1]];
			auto v2 = vertices[indices[i + 2]];
			auto edge1 = v1.position - v0.position;
			auto edge2 = v2.position - v0.position;
			auto deltaUV1 = v1.texCoord - v0.texCoord;
			auto deltaUV2 = v2.texCoord - v0.texCoord;
			float f = 1.0f / (deltaUV1[0] * deltaUV2[1] - deltaUV2[0] * deltaUV1[1]);
			auto tangent = xlux::math::Vec3(
                f * (deltaUV2[1] * edge1[0] - deltaUV1[1] * edge2[0]),
				f * (deltaUV2[1] * edge1[1] - deltaUV1[1] * edge2[1]),
				f * (deltaUV2[1] * edge1[2] - deltaUV1[1] * edge2[2])
			);
			auto bitangent = xlux::math::Vec3(
				f * (-deltaUV2[0] * edge1[0] + deltaUV1[0] * edge2[0]),
				f * (-deltaUV2[0] * edge1[1] + deltaUV1[0] * edge2[1]),
				f * (-deltaUV2[0] * edge1[2] + deltaUV1[0] * edge2[2])
			);
			vertices[indices[i + 0]].tangent = tangent;
			vertices[indices[i + 1]].tangent = tangent;
			vertices[indices[i + 2]].tangent = tangent;
			vertices[indices[i + 0]].bitangent = bitangent;
			vertices[indices[i + 1]].bitangent = bitangent;
			vertices[indices[i + 2]].bitangent = bitangent;
		}
		xlux::log::Info("Tangent Basis Calculated");
		vertexBuffer->SetData(vertices.data(), sizeof(VertexInData) * vertices.size());

	}

	void Draw(xlux::RawPtr<xlux::Renderer> renderer)
	{
		renderer->DrawIndexed(vertexBuffer, indexBuffer, indexCount);
	}

	void Destroy()
	{
		device->DestroyBuffer(vertexBuffer);
		device->DestroyBuffer(indexBuffer);
		device->FreeMemory(deviceMemory);
	}


	xlux::List<VertexInData> vertices = {};
	xlux::List<xlux::U32> indices = {};
	xlux::RawPtr<xlux::Device> device = nullptr;
	xlux::RawPtr<xlux::Buffer> vertexBuffer = nullptr;
	xlux::RawPtr<xlux::Buffer> indexBuffer = nullptr;
	xlux::RawPtr<xlux::DeviceMemory> deviceMemory = nullptr;
	xlux::U32 vertexCount = 0;
	xlux::U32 indexCount = 0;
};

struct Light
{
	Light(xlux::math::Vec3 position_, xlux::math::Vec3 color_)
	{
		position = position_;
		color = color_;
	}

	Light() = default;

	xlux::math::Vec3 position = xlux::math::Vec3(0.0f, 0.0f, 0.0f);
	xlux::math::Vec3 color = xlux::math::Vec3(1.0f, 1.0f, 1.0f);
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
		dataOut->normal.Normalize();
		dataOut->tangent = model.Mul(xlux::math::Vec4(dataIn->tangent, 0.0f));
		dataOut->tangent.Normalize();
		dataOut->bitangent = model.Mul(xlux::math::Vec4(dataIn->bitangent, 0.0f));
		dataOut->bitangent.Normalize();
		builtIn->Position = xlux::math::Vec4(pos, 1.0f);
		return true;
	}
};

class HelloWorldFShader : public xlux::IShaderG<VertexOutData, xlux::FragmentShaderOutput>
{
public:
	xlux::RawPtr<xlux::Texture2D> diffuseT;
	xlux::RawPtr<xlux::Texture2D> metalT;
	xlux::RawPtr<xlux::Texture2D> roughnessT;
	xlux::RawPtr<xlux::Texture2D> normalT;

	xlux::math::Vec3 cameraPosition;
	Light light[10] = {};
	xlux::U32 lightCount = 0;

	xlux::math::Vec3 Mix(const xlux::math::Vec3& a, const xlux::math::Vec3& b, const xlux::F32 t)
	{
		return a * (1.0f - t) + b * t;
	}

	xlux::math::Vec3 fresnelSchlick(const xlux::math::Vec3& F0, const xlux::F32 cosTheta)
	{
		return F0 + (xlux::math::Vec3(1.0f) - F0) * std::pow(1.0f - cosTheta, 5.0f);
	}

	xlux::F32 ndfGGX(xlux::F32 cosLh, xlux::F32 roughness)
	{
		auto alpha = roughness * roughness;
		auto alpha2 = alpha * alpha;

		auto denom = cosLh * cosLh * (alpha2 - 1.0f) + 1.0f;
		return alpha2 / (denom * denom * xlux::math::PI);
	}

	xlux::F32 gaSchlickG1(xlux::F32 cosTheta, xlux::F32 k)
	{
		return cosTheta / (cosTheta * (1.0f - k) + k);
	}

	xlux::F32 gaSchlickGGX(xlux::F32 cosLh, xlux::F32 cosLo, xlux::F32 roughness)
	{
		auto r = (roughness + 1.0f);
		auto k = (r * r) / 8.0f;
		return gaSchlickG1(cosLh, k) * gaSchlickG1(cosLo, k);
	}

	xlux::math::Vec3 aces(const xlux::math::Vec3& x)
	{
		const auto a = 2.51f;
		const auto b = 0.03f;
		const auto c = 2.43f;
		const auto d = 0.59f;
		const auto e = 0.14f;
		auto result = (x * (x * a + b)) / (x * (x * c + d) + e);
		return xlux::math::Vec3(std::clamp(result[0], 0.0f, 1.0f), std::clamp(result[1], 0.0f, 1.0f), std::clamp(result[2], 0.0f, 1.0f));
	}

public:
	xlux::Bool Execute(const xlux::RawPtr<VertexOutData> dataIn, xlux::RawPtr<xlux::FragmentShaderOutput> dataOut, xlux::RawPtr<xlux::ShaderBuiltIn> builtIn)
	{
		using namespace xlux::math;
		(void)builtIn;

		auto albedo = Vec3(diffuseT->Sample(dataIn->texCoord));
		auto metalness = metalT->Sample(dataIn->texCoord)[0];
		auto roughness = roughnessT->Sample(dataIn->texCoord)[0];

		auto Lo = (cameraPosition - dataIn->position).Normalized();
				
		auto tangentBasis = Mat4x4(Vec4(dataIn->tangent, 0.0f), Vec4(dataIn->bitangent, 0.0f), Vec4(dataIn->normal, 0.0f), Vec4(0.0f, 0.0f, 0.0f, 1.0f));
		auto N = Vec3( normalT->Sample(dataIn->texCoord) * 2.0f - Vec4(1.0f));
		N = Vec3(tangentBasis.Mul(Vec4(N, 1.0f)).Normalized());

		// auto N = Vec3(dataIn->normal);

		auto cosLo = std::max(0.0f, N.Dot(Lo));

		auto Lr = N * 2.0f * cosLo - Lo;

		static const auto Fdielectric = Vec3(0.04f);

		auto F0 = Mix(Fdielectric, albedo, metalness);

		auto directLighting = Vec3(0.0f);

		for (auto i = 0; i < 1; i++) 
		{
			auto lightToPixel = (dataIn->position - light[i].position);
			auto lightDistance = lightToPixel.Length();

			auto Li = -lightToPixel.Normalized();
			auto Lradiance = light[i].color;

			auto Lh = (Li + Lo).Normalized();

			auto cosLi = std::max(0.0f, N.Dot(Li));
			auto cosLh = std::max(0.0f, N.Dot(Lh));

			auto F = fresnelSchlick(F0, cosLh);
			auto D = ndfGGX(cosLh, roughness);
			auto G = gaSchlickGGX(cosLh, cosLi, roughness);

			auto kd = Mix(Vec3(1.0f) - F, Vec3(1.0f), metalness);

			auto diffuseBRDF = kd * albedo;

			auto specularBRDF = (F * D * G) / std::max(0.00001f, 4.0f * cosLi * cosLo);


			directLighting += (diffuseBRDF + specularBRDF) * Lradiance * cosLi / (lightDistance * lightDistance);

		}

		// ambient
		directLighting += Vec3(0.05f) * albedo;

		directLighting = directLighting.Pow(1.0f / 2.2f);

		directLighting = aces(directLighting);

		dataOut->Color[0] = Vec4(directLighting, 1.0f);
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
		.SetBackfaceCullingEnable(true)
		.SetDepthCompareFunction(xlux::CompareFunction_Less)
		.SetVertexItemSize(sizeof(VertexInData))
		.SetVertexToFragmentDataSize(sizeof(VertexOutData));

	auto pipeline = device->CreatePipeline(createInfo);

	// Vectices for a cube with vertex Normals (all 36 vertices)
	std::vector<VertexInData> vertices = {};
	std::vector<xlux::U32> indices = {};

	// Load the model
	
	auto diffuseTexture = XTexture(xlux::utils::GetExecutableDirectory() + "/nile/diffuse.png", device);
	auto metalnessTexture = XTexture(xlux::utils::GetExecutableDirectory() + "/nile/metal.png", device);
	auto roughnessTexture = XTexture(xlux::utils::GetExecutableDirectory() + "/nile/roughness.png", device);
	auto normalTexture = XTexture(xlux::utils::GetExecutableDirectory() + "/nile/normal.png", device);
	auto zeus = XMesh(xlux::utils::GetExecutableDirectory() + "/nile/model.obj", device);
	
	auto renderer = device->CreateRenderer();

	auto prevTime = xlux::utils::GetTime(), currTime = xlux::utils::GetTime(), deltaTime = 0.0f;

	auto proj = xlux::math::Mat4x4::Perspective(xlux::math::ToRadians(66.0f), (float)framebuffer->GetWidth() / (float)framebuffer->GetHeight(), 0.1f, 100.0f);
	auto view = xlux::math::Mat4x4::LookAt(xlux::math::Vec3(12.0f, 12.0f, 12.0f), xlux::math::Vec3(0.0f, 0.0f, 0.0f), xlux::math::Vec3(0.0f, 1.0f, 0.0f));

	fragmentShader->diffuseT = diffuseTexture.texture;
	fragmentShader->metalT = metalnessTexture.texture;
	fragmentShader->roughnessT = roughnessTexture.texture;
	fragmentShader->normalT = normalTexture.texture;

	fragmentShader->light[0] = Light(xlux::math::Vec3(5.0f, 5.0f, 0.0f), xlux::math::Vec3(1.0f, 1.0f, 1.0f) * 1000.0f);
	fragmentShader->lightCount = 1;


	const auto dist = 35.0f;
	const auto speed = 0.3f;
	const auto offset = -3.5f;


	while (!Window::HasClosed())
	{
		currTime = xlux::utils::GetTime();
		deltaTime = currTime - prevTime;
		prevTime = currTime;

		Window::SetTitle("Xlux Engine [PBR] - Jaysmito Mukherjee - FPS: " + std::to_string(1.0f / deltaTime));
		// vertexShader->model = xlux::math::Mat4x4::RotateY(currTime * 0.5f);
		fragmentShader->cameraPosition = xlux::math::Vec3(dist * cos(currTime * speed * 0.2f + offset), 12.0f, dist * sin(currTime * speed * 0.2f + offset));
		view = xlux::math::Mat4x4::LookAt(fragmentShader->cameraPosition, xlux::math::Vec3(0.0f, 5.0f, 0.0f), xlux::math::Vec3(0.0f, 1.0f, 0.0f));
		proj = xlux::math::Mat4x4::Perspective(xlux::math::ToRadians(66.0f), (float)framebuffer->GetWidth() / (float)framebuffer->GetHeight(), 0.1f, 10000.0f);
		vertexShader->viewProj = proj.Mul(view);

		renderer->BeginFrame();
		renderer->BindFramebuffer(framebuffer);
		renderer->SetViewport(0, 0, framebuffer->GetWidth(), framebuffer->GetHeight());
		renderer->SetClearColor(0.02f, 0.02f, 0.02f, 1.0f);
		renderer->Clear();
		renderer->BindPipeline(pipeline);
		zeus.Draw(renderer);
		renderer->EndFrame();


		Window::SwapBuffer();
		Window::Update();
	}

	device->DestroyRenderer(renderer);
	device->DestroyPipeline(pipeline);

	diffuseTexture.Destroy();
	zeus.Destroy();

	delete vertexShader;
	delete fragmentShader;
	delete interpolator;

	xlux::Device::Destroy(device);
	Window::Destroy();
	xlux::Logger::Shutdown();
}