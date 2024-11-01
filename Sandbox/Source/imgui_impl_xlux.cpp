// dear imgui: Renderer Backend for Xlux 
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"

#ifndef IMGUI_DISABLE
#include "imgui_impl_Xlux.h"

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option" // warning: ignore unknown flags
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning: use of old-style cast
#pragma clang diagnostic ignored "-Wsign-conversion"        // warning: implicit conversion changes signedness
#pragma clang diagnostic ignored "-Wunused-macros"          // warning: macro is not used
#pragma clang diagnostic ignored "-Wnonportable-system-include-path"
#pragma clang diagnostic ignored "-Wcast-function-type"     // warning: cast between incompatible function types (for loader)
#endif
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"                  // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wunknown-warning-option"   // warning: unknown warning group 'xxx'
#pragma GCC diagnostic ignored "-Wcast-function-type"       // warning: cast between incompatible function types (for loader)
#endif


// Xlux includes
#include "Xlux.hpp"

// OpenGL Data
struct ImGui_ImplXlux_Data
{
    xlux::RawPtr<xlux::Device>        XluxDevice;
    xlux::RawPtr<xlux::Pipeline>      XluxPipeline;

    xlux::RawPtr<xlux::Texture2D>     FontTexture;
    xlux::RawPtr<xlux::Buffer>        FontTextureBuffer;
    xlux::RawPtr<xlux::DeviceMemory>  FontTextureMemory;


    ImGui_ImplXlux_Data() { memset((void*)this, 0, sizeof(*this)); }
};

// Backend data stored in io.BackendRendererUserData to allow support for multiple Dear ImGui contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
static auto ImGui_ImplXlux_GetBackendData()
{
    return ImGui::GetCurrentContext() ? static_cast<xlux::RawPtr<ImGui_ImplXlux_Data>>(ImGui::GetIO().BackendRendererUserData) : nullptr;
}

// Functions
bool ImGui_ImplXlux_Init(xlux::RawPtr<xlux::Device> device)
{
    ImGuiIO& io = ImGui::GetIO();
    IMGUI_CHECKVERSION();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");
    
    // Setup backend capabilities flags
    auto bd = IM_NEW(ImGui_ImplXlux_Data)();
    bd->XluxDevice = device;

    io.BackendRendererUserData = static_cast<void*>(bd);
    io.BackendRendererName = "imgui_impl_xlux";
    return true;
}

void ImGui_ImplXlux_Shutdown()
{
    auto bd = ImGui_ImplXlux_GetBackendData();
    IM_ASSERT(bd != nullptr && "No renderer backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplXlux_DestroyDeviceObjects();
    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
    io.BackendFlags &= ~ImGuiBackendFlags_RendererHasVtxOffset;
    IM_DELETE(bd);
}

void ImGui_ImplXlux_NewFrame()
{
    auto bd = ImGui_ImplXlux_GetBackendData();
    IM_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call ImGui_ImplXlux_Init()?");

    if (!bd->XluxPipeline)
        ImGui_ImplXlux_CreateDeviceObjects();
    if (!bd->FontTexture)
        ImGui_ImplXlux_CreateFontsTexture();
}

bool ImGui_ImplXlux_CreateFontsTexture()
{
    ImGuiIO& io = ImGui::GetIO();
    auto bd = ImGui_ImplXlux_GetBackendData();

    // Build texture atlas
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

    // convert to F32
    auto textureData = new xlux::F32[width * height * 4];
    for (int i = 0; i < width * height * 4; i++)
        textureData[i] = static_cast<xlux::F32>(pixels[i]) / 255.0f;

    // Create texture
    bd->FontTexture = bd->XluxDevice->CreateTexture2D(width, height, xlux::TexelFormat_RGBA);
    bd->FontTextureBuffer = bd->XluxDevice->CreateBuffer(bd->FontTexture->GetSizeInBytes());
    bd->FontTextureMemory = bd->XluxDevice->AllocateMemory(bd->FontTexture->GetSizeInBytes());
    bd->FontTextureBuffer->BindMemory(bd->FontTextureMemory, 0);
    bd->FontTexture->BindBuffer(bd->FontTextureBuffer);
    bd->FontTexture->SetData(textureData, bd->FontTexture->GetSizeInBytes(), 0);

    delete[] textureData;

    // Store our identifier
    io.Fonts->SetTexID((ImTextureID)(intptr_t)bd->FontTexture);

    return true;
}

void ImGui_ImplXlux_DestroyFontsTexture()
{
    ImGuiIO& io = ImGui::GetIO();
    auto bd = ImGui_ImplXlux_GetBackendData();
    if (bd->FontTexture)
    {
        bd->XluxDevice->DestroyTexture(bd->FontTexture); bd->FontTexture = nullptr;
        bd->XluxDevice->DestroyBuffer(bd->FontTextureBuffer); bd->FontTextureBuffer = nullptr;
        bd->XluxDevice->FreeMemory(bd->FontTextureMemory); bd->FontTextureMemory = nullptr;
        io.Fonts->SetTexID(0);
    }
}

void ImGui_ImplXlux_RenderDrawData(xlux::RawPtr<ImDrawData> draw_data)
{
    (void)draw_data;
    xlux::log::Warn("ImGui_ImplXlux_RenderDrawData not implemented");
}


struct ImGui_ImplXlux_VertexInData
{
	xlux::math::Vec2 position;
	xlux::math::Vec2 uv;
    xlux::math::Vec4 color;

	ImGui_ImplXlux_VertexInData(xlux::math::Vec2 position, xlux::math::Vec2 uv, xlux::math::Vec4 color)
		: position(position), uv(uv), color(color)
	{}
};

struct ImGui_ImplXlux_VertexOutData
{
	xlux::math::Vec2 frag_uv;
	xlux::math::Vec4 frag_color;

    ImGui_ImplXlux_VertexOutData()
        : frag_uv(xlux::math::Vec2(0.0f, 0.0f)), frag_color(xlux::math::Vec4(0.0f, 0.0f, 0.0f, 0.0f))
    {}

    ImGui_ImplXlux_VertexOutData(xlux::math::Vec2 frag_uv, xlux::math::Vec4 frag_color)
        : frag_uv(frag_uv), frag_color(frag_color)
    {}

	inline auto Scaled(xlux::F32 scale) const
	{
		return ImGui_ImplXlux_VertexOutData(frag_uv * scale, frag_color * scale);
	}

	inline void Add(const ImGui_ImplXlux_VertexOutData& other)
	{
		frag_uv += other.frag_uv;
		frag_color += other.frag_color;
	}
};

class ImGui_ImplXlux_VertexShader : public xlux::IShaderG<ImGui_ImplXlux_VertexInData, ImGui_ImplXlux_VertexOutData>
{
public:
    xlux::math::Mat4x4 proj_mtx;

	xlux::Bool Execute(const xlux::RawPtr<ImGui_ImplXlux_VertexInData> dataIn, xlux::RawPtr<ImGui_ImplXlux_VertexOutData> dataOut, xlux::RawPtr<xlux::ShaderBuiltIn> builtIn)
	{
		dataOut->frag_uv = dataIn->uv;
		dataOut->frag_color = dataIn->color;
		builtIn->Position = proj_mtx.Mul(xlux::math::Vec4(dataIn->position, 0.0f, 1.0f));
		return true;
	}
};

class ImGui_ImplXlux_FragmentShader : public xlux::IShaderG<ImGui_ImplXlux_VertexOutData, xlux::FragmentShaderOutput>
{
public:
	xlux::RawPtr<xlux::Texture2D> texture;

public:
	xlux::Bool Execute(const xlux::RawPtr<ImGui_ImplXlux_VertexOutData> dataIn, xlux::RawPtr<xlux::FragmentShaderOutput> dataOut, xlux::RawPtr<xlux::ShaderBuiltIn> builtIn)
	{
		(void)builtIn; // unused
		dataOut->Color[0] = dataIn->frag_color * texture->Sample(xlux::math::Vec3(dataIn->frag_uv, 0.0f));
		return true;
	}
};

bool ImGui_ImplXlux_CreateDeviceObjects()
{
    auto bd = ImGui_ImplXlux_GetBackendData();

    auto pipeline_create_info = xlux::PipelineCreateInfo()
        .SetShader(xlux::CreateRawPtr<ImGui_ImplXlux_VertexShader>(), xlux::ShaderStage_Vertex)
        .SetShader(xlux::CreateRawPtr<ImGui_ImplXlux_FragmentShader>(), xlux::ShaderStage_Fragment)
        .SetInterpolator(xlux::CreateRawPtr<xlux::BasicInterpolator<ImGui_ImplXlux_VertexOutData>>())
		.SetVertexItemSize(sizeof(ImGui_ImplXlux_VertexInData))
		.SetVertexToFragmentDataSize(sizeof(ImGui_ImplXlux_VertexOutData));

    bd->XluxPipeline = bd->XluxDevice->CreatePipeline(pipeline_create_info);

    ImGui_ImplXlux_CreateFontsTexture();

    return true;
}

void ImGui_ImplXlux_DestroyDeviceObjects()
{
    auto bd = ImGui_ImplXlux_GetBackendData();

    if (bd->XluxPipeline) { bd->XluxDevice->DestroyPipeline(bd->XluxPipeline); bd->XluxPipeline = nullptr; }


    ImGui_ImplXlux_DestroyFontsTexture();
}

#endif // IMGUI_DISABLE