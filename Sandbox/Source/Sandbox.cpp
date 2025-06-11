#include "Xlux.hpp"
#include "Window.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#pragma warning(push, 0)
#include "ufbx.h"
#pragma warning(pop)

struct ResourceData {
	xlux::RawPtr<xlux::Texture2D> texture = nullptr;
	xlux::math::Mat4x4 modelMatrix = xlux::math::Mat4x4::Identity();
	xlux::math::Mat4x4 inverseTransposeModelMatrix = xlux::math::Mat4x4::Identity();
};

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
	xlux::RawPtr<ResourceData> resourceData = nullptr;

	inline VertexOutData Scaled(xlux::F32 scale) const
	{
		return VertexOutData(position * scale, texCoord * scale, normal * scale, resourceData);
	}

	inline void Add(const VertexOutData& other)
	{
		position += other.position;
		texCoord += other.texCoord;
		normal += other.normal;
		resourceData = other.resourceData;
	}
};

class HelloWorldVShader : public xlux::IShaderG<VertexInData, VertexOutData>
{
public:
	xlux::math::Mat4x4 viewProj = xlux::math::Mat4x4::Identity();


public:
	xlux::Bool Execute(const xlux::RawPtr<VertexInData> dataIn, xlux::RawPtr<VertexOutData> dataOut, xlux::RawPtr<xlux::ShaderBuiltIn> builtIn)
	{
		dataOut->texCoord = dataIn->texCoord;
		auto rd = static_cast<xlux::RawPtr<ResourceData>>(builtIn->UserData);

		auto pos = viewProj.Mul(rd->modelMatrix.Mul(xlux::math::Vec4(dataIn->position, 1.0f)));
		dataOut->position = pos;
		dataOut->normal = rd->inverseTransposeModelMatrix.Mul(xlux::math::Vec4(dataIn->normal, 0.0f)).Normalized();
		dataOut->resourceData = rd;
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
		const auto lightPos = xlux::math::Vec3(00.0f, 50.0f, 00.0f);
		const auto lightDir = (lightPos - dataIn->position).Normalized();
		const auto lightIntensity = std::max(0.0f, lightDir.Dot(dataIn->normal * 1.0f));
		if (dataIn->resourceData->texture) {
			dataOut->Color[0] = dataIn->resourceData->texture->SampleInterpolated(dataIn->texCoord) * (lightIntensity + 0.3f);
		}
		else {
			dataOut->Color[0] = xlux::math::Vec4(1.0, 1.0, 1.0) * (lightIntensity + 0.3f);
		}
		dataOut->Color[0][3] = 1.0f;
		//dataOut->Color[0] = dataOut->Color[0].Pow(1.0f / 2.2f);
		return true;
	}
};


struct Mesh {
	xlux::Size startVertex = 0;
	xlux::Size startIndex = 0;
	xlux::Size numIndices = 0;

	xlux::RawPtr<xlux::Texture2D> texture = nullptr;
};

struct Node {
	xlux::math::Mat4x4 transform = xlux::math::Mat4x4::Identity();
	std::string name = "Unnamed Node";
	xlux::List<xlux::U32> children;
	xlux::U32 parent; // Parent node ID, SIZE_MAX if no parent
	xlux::U32 mesh;
};

struct Scene {
	xlux::List<Node> nodes;
	xlux::U32 rootNode;

	xlux::UnorderedMap<xlux::U32, Mesh> meshes;
	xlux::List<VertexInData> vertices;
	xlux::List<xlux::U32> indices;
};

xlux::math::Mat4x4 RotationMatrixFromQuaternion(const ufbx_quat& rot) {
	// Pre-calculate squared values
	float x2 = (float)(rot.x * rot.x);
	float y2 = (float)(rot.y * rot.y);
	float z2 = (float)(rot.z * rot.z);

	// Pre-calculate products
	float xy = (float)(rot.x * rot.y);
	float xz = (float)(rot.x * rot.z);
	float yz = (float)(rot.y * rot.z);
	float wx = (float)(rot.w * rot.x);
	float wy = (float)(rot.w * rot.y);
	float wz = (float)(rot.w * rot.z);

	// Construct the 4x4 rotation matrix
	return xlux::math::Mat4x4(
		1.0f - 2.0f * (y2 + z2), 2.0f * (xy - wz), 2.0f * (xz + wy), 0.0f,
		2.0f * (xy + wz), 1.0f - 2.0f * (x2 + z2), 2.0f * (yz - wx), 0.0f,
		2.0f * (xz - wy), 2.0f * (yz + wx), 1.0f - 2.0f * (x2 + y2), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}


xlux::math::Mat4x4 TranslateMatrix(const ufbx_transform& transform) {
	auto translation = xlux::math::Mat4x4::Translate(xlux::math::Vec3(transform.translation.x, transform.translation.y, transform.translation.z));
	auto rotation = RotationMatrixFromQuaternion(transform.rotation);
	auto scale = xlux::math::Mat4x4::Scale(xlux::math::Vec3(transform.scale.x, transform.scale.y, transform.scale.z));
	return translation.Mul(rotation).Mul(scale);
}

xlux::math::Vec3 TranslateVec3(const ufbx_vec3& v) {
	return xlux::math::Vec3(v.x, v.y, v.z);
}

xlux::math::Vec2 TranslateVec2(const ufbx_vec2& v) {
	return xlux::math::Vec2(v.x, v.y);
}


static void ProcessNode(ufbx_node* node, Node& nodeData, xlux::U32 parentId) {
	nodeData.name = node->name.data ? node->name.data : "Unnamed Node";
	nodeData.transform = TranslateMatrix(node->local_transform);
	nodeData.parent = parentId;
	nodeData.mesh = node->mesh ? node->mesh->element_id : UINT_MAX;
}


static xlux::U32 VisitNode(ufbx_node* node, xlux::RawPtr<Scene> sceneData, xlux::U32 parentId) {
	xlux::log::Info("{}Node: {} [{}] {}", std::string(node->node_depth, ' '), node->name.data ? node->name.data : "Unnamed Node", node->mesh != NULL, (uintptr_t)sceneData->meshes[5].texture);
	xlux::U32 nodeId = (xlux::U32)sceneData->nodes.size();
	sceneData->nodes.push_back(Node());

	ProcessNode(node, sceneData->nodes[nodeId], parentId);
	if (node->is_root) {
		sceneData->rootNode = nodeId;
	}

	for (auto child : node->children) {
		auto childId = VisitNode(child, sceneData, nodeId);
		sceneData->nodes[nodeId].children.push_back(childId);
	}

	return nodeId;
}

static void ProcessMesh(Scene& sceneData, const ufbx_mesh* mesh, xlux::RawPtr<xlux::Device> device) {
	static uint32_t triIndices[64];


	auto startIndex = (xlux::U32)sceneData.indices.size();

	for (xlux::Size fi = 0; fi < mesh->num_faces; fi++) {
		ufbx_face face = mesh->faces.data[fi];
		size_t numTris = ufbx_triangulate_face(triIndices, sizeof(triIndices) / sizeof(triIndices[0]), mesh, face);

		for (size_t vi = 0; vi < numTris * 3; vi++) {
			uint32_t ix = triIndices[vi];

			VertexInData vertexData = {};
			vertexData.position = TranslateVec3(ufbx_get_vertex_vec3(&mesh->vertex_position, ix));
			vertexData.normal = TranslateVec3(ufbx_get_vertex_vec3(&mesh->vertex_normal, ix));
			vertexData.texCoord = mesh->vertex_uv.exists ? TranslateVec2(ufbx_get_vertex_vec2(&mesh->vertex_uv, ix)) : xlux::math::Vec2(0.0);

			sceneData.vertices.push_back(vertexData);
			sceneData.indices.push_back((xlux::U32)sceneData.vertices.size() - 1);

		}
	}


	auto meshData = Mesh();
	meshData.startVertex = 0;
	meshData.startIndex = startIndex;
	meshData.numIndices = (xlux::U32)sceneData.indices.size() - meshData.startIndex;
	meshData.texture = nullptr;

	if (mesh->materials.count > 0) {
		// only use the first material
		auto material = mesh->materials.data[0];
		if (material->textures.count > 0) {
			// only use the first texture
			auto texture = material->textures.data[0].texture;
			if (texture->filename.length > 0) {
				// only load it if the texture is embedded
				int tWidth, tHeight, tChannels;
				auto imageData = stbi_loadf(texture->filename.data, &tWidth, &tHeight, &tChannels, 3);
				if (!imageData) {
					xlux::log::Error("Failed to load texture: {}", texture->filename.data);
				}
				else {
					auto textureData = device->CreateTexture2D(tWidth, tHeight, xlux::TexelFormat_RGB);

					// deviceMemory and textureBuffer ownership will be managed by the device 
					auto deviceMemory = device->AllocateMemory(textureData->GetSizeInBytes());
					auto textureBuffer = device->CreateBuffer(textureData->GetSizeInBytes());
					textureBuffer->BindMemory(deviceMemory);
					textureData->BindBuffer(textureBuffer);
					textureData->SetData(imageData, textureData->GetSizeInBytes(), 0);
					stbi_image_free(imageData);
					meshData.texture = textureData;
				}
			}
		}
	}

	sceneData.meshes[mesh->element_id] = meshData;
}

static bool LoadMesh(const std::string& path, Scene& sceneData, xlux::RawPtr<xlux::Device> device)
{

	ufbx_load_opts opts = {
		.load_external_files = true,
		.target_axes = {
			.right = UFBX_COORDINATE_AXIS_POSITIVE_X,
			.up = UFBX_COORDINATE_AXIS_POSITIVE_Y,
			.front = UFBX_COORDINATE_AXIS_POSITIVE_Z
		},
		.target_unit_meters = 1.0f
	};
	ufbx_error error;
	ufbx_scene* scene = ufbx_load_file(path.c_str(), &opts, &error);
	if (!scene) {
		xlux::log::Error("Failed to load FBX file: {}", error.description.data);
		return false;
	}

	VisitNode(scene->root_node, &sceneData, 100000);

	for (const auto mesh : scene->meshes) {
		ProcessMesh(sceneData, mesh, device);
	}



	ufbx_free_scene(scene);
	return true;
}

static void DrawMesh(
	xlux::Renderer* renderer,
	const Scene& sceneData,
	HelloWorldVShader* vertexShader,
	HelloWorldFShader* fragmentShader,
	xlux::Buffer* vertexBuffer,
	xlux::Buffer* indexBuffer,
	const xlux::U32 nodeId,
	xlux::UnorderedMap<xlux::U32, ResourceData>& resources,
	xlux::math::Mat4x4 model
)
{
	const auto& node = sceneData.nodes[nodeId];
	model = model.Mul(node.transform);
	if (node.mesh != UINT_MAX) {
		const auto& mesh = sceneData.meshes.at(node.mesh);
		
		auto resource = resources.find(node.mesh);
		// Since we arent animating cache the matrices
		if (resource == resources.end()) {
			ResourceData resourceData;
			resourceData.texture = mesh.texture;
			resourceData.modelMatrix = model;
			resourceData.inverseTransposeModelMatrix = model.FastInverse().Transpose();
			resourceData.inverseTransposeModelMatrix.ResetTranslation();
			resources[node.mesh] = resourceData;
		}
		renderer->SetRendererUserData(&resources.at(node.mesh));
		renderer->DrawIndexed(vertexBuffer, indexBuffer, static_cast<xlux::U32>(mesh.numIndices), (xlux::U32)mesh.startVertex, (xlux::U32)mesh.startIndex);
	}

	for (auto child : node.children) {
		DrawMesh(renderer, sceneData, vertexShader, fragmentShader, vertexBuffer, indexBuffer, child, resources, model);
	}

}

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

	Scene sceneData = {};

	const auto path = xlux::utils::GetExecutableDirectory() + "/SM_Deccer_Cubes_Textured_Complex.fbx";
	//const auto path = xlux::utils::GetExecutableDirectory() + "/model3.fbx";
	if (!LoadMesh(path, sceneData, device))
	{
		xlux::log::Error("Failed to load mesh from {}", path);
		return EXIT_FAILURE;
	}



	xlux::log::Info("Vertices: {}", sceneData.vertices.size());
	xlux::log::Info("Indices: {}", sceneData.indices.size());



	stbi_set_flip_vertically_on_load(true);
	xlux::I32 tWidth = 0, tHeight = 0, tChannels = 0;
	auto textureData = stbi_loadf((xlux::utils::GetExecutableDirectory() + "/texture.png").c_str(), &tWidth, &tHeight, &tChannels, 3);
	auto texture = device->CreateTexture2D(tWidth, tHeight, xlux::TexelFormat_RGB);


	auto totalSize = sizeof(VertexInData) * sceneData.vertices.size() + sizeof(xlux::U32) * sceneData.indices.size() + texture->GetSizeInBytes();
	auto deviceMemory = device->AllocateMemory(totalSize);

	auto vertexBuffer = device->CreateBuffer(sizeof(VertexInData) * sceneData.vertices.size());
	vertexBuffer->BindMemory(deviceMemory, 0);
	vertexBuffer->SetData(sceneData.vertices.data(), sizeof(VertexInData) * sceneData.vertices.size());

	auto indexBuffer = device->CreateBuffer(sizeof(xlux::U32) * sceneData.indices.size());
	indexBuffer->BindMemory(deviceMemory, sizeof(VertexInData) * sceneData.vertices.size());
	indexBuffer->SetData(sceneData.indices.data(), sizeof(xlux::U32) * sceneData.indices.size());

	auto textureBuffer = device->CreateBuffer(texture->GetSizeInBytes());
	textureBuffer->BindMemory(deviceMemory, sizeof(VertexInData) * sceneData.vertices.size() + sizeof(xlux::U32) * sceneData.indices.size());
	texture->BindBuffer(textureBuffer);
	texture->SetData(textureData, texture->GetSizeInBytes(), 0);
	stbi_image_free(textureData);


	auto renderer = device->CreateRenderer();

	auto prevTime = xlux::utils::GetTime(), currTime = xlux::utils::GetTime(), deltaTime = 0.0f;

	auto proj = xlux::math::Mat4x4::Perspective(xlux::math::ToRadians(66.0f), (float)framebuffer->GetWidth() / (float)framebuffer->GetHeight(), 0.1f, 100.0f);
	auto view = xlux::math::Mat4x4::LookAt(xlux::math::Vec3(12.0f, 12.0f, 12.0f), xlux::math::Vec3(0.0f, 0.0f, 0.0f), xlux::math::Vec3(0.0f, 1.0f, 0.0f));
	auto model = xlux::math::Mat4x4::Identity();

	const xlux::F32 dist = 13.0f;


	xlux::UnorderedMap<xlux::U32, ResourceData> resources;

	while (!Window::HasClosed())
	{
		currTime = xlux::utils::GetTime();
		deltaTime = currTime - prevTime;
		prevTime = currTime;

		//auto timeDriver = 2.40;
		auto timeDriver = currTime;

		Window::SetTitle("Xlux Engine [Model Loading] - Jaysmito Mukherjee - FPS: " + std::to_string(1.0f / deltaTime));
		view = xlux::math::Mat4x4::LookAt(xlux::math::Vec3(dist * cos(timeDriver * 0.2f), dist, dist * sin(timeDriver * 0.2f)), xlux::math::Vec3(0.0f, 0.0f, 0.0f), xlux::math::Vec3(0.0f, 1.0f, 0.0f));
		proj = xlux::math::Mat4x4::Perspective(xlux::math::ToRadians(66.0f), (float)framebuffer->GetWidth() / (float)framebuffer->GetHeight(), 0.1f, 10000.0f);
		vertexShader->viewProj = proj.Mul(view);

		renderer->BeginFrame();
		renderer->BindFramebuffer(framebuffer);
		renderer->SetViewport(0, 0, framebuffer->GetWidth(), framebuffer->GetHeight());
		renderer->SetClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		renderer->Clear();
		renderer->SetDetachedRendering(true);
		renderer->BindPipeline(pipeline);

		DrawMesh(
			renderer,
			sceneData,
			vertexShader,
			fragmentShader,
			vertexBuffer,
			indexBuffer,
			sceneData.rootNode,
			resources,
			xlux::math::Mat4x4::Identity()
		);


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