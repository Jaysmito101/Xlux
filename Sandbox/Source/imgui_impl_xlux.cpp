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
    xlux::RawPtr<xlux::IShader>       XluxVertexShader;
    xlux::RawPtr<xlux::IShader>       XluxFragmentShader;

    xlux::RawPtr<xlux::Texture2D>     FontTexture;
    xlux::RawPtr<xlux::Buffer>        FontTextureBuffer;
    xlux::RawPtr<xlux::DeviceMemory>  FontTextureMemory;


    xlux::RawPtr<xlux::Buffer>        VertexBuffer;
    xlux::RawPtr<xlux::Buffer>        IndexBuffer;
    xlux::RawPtr<xlux::DeviceMemory>  VertexIndexBufferMemory;

    ImGui_ImplXlux_Data() { memset((void*)this, 0, sizeof(*this)); }
};


struct ImGui_ImplXlux_VertexInData
{
	xlux::math::Vec3 position;
	xlux::math::Vec2 uv;
    xlux::math::Vec4 color;

	ImGui_ImplXlux_VertexInData(xlux::math::Vec3 position, xlux::math::Vec2 uv, xlux::math::Vec4 color)
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
		builtIn->Position = proj_mtx.Mul(xlux::math::Vec4(dataIn->position, 1.0f));
		return true;
	}
};

class ImGui_ImplXlux_FragmentShader : public xlux::IShaderG<ImGui_ImplXlux_VertexOutData, xlux::FragmentShaderOutput>
{
public:
    xlux::RawPtr<xlux::Texture2D> texture = nullptr;

public:
	xlux::Bool Execute(const xlux::RawPtr<ImGui_ImplXlux_VertexOutData> dataIn, xlux::RawPtr<xlux::FragmentShaderOutput> dataOut, xlux::RawPtr<xlux::ShaderBuiltIn> builtIn)
	{
		(void)builtIn; // unused
		dataOut->Color[0] = dataIn->frag_color * texture->Sample(xlux::math::Vec3(dataIn->frag_uv, 0.0f));
		return true;
	}
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


static void ImGui_ImplXlux_SetupRenderState(xlux::RawPtr<ImDrawData> draw_data, xlux::RawPtr<xlux::Renderer> renderer)
{
    auto bd = ImGui_ImplXlux_GetBackendData();
    // Setup viewport, orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
    float L = draw_data->DisplayPos.x;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float T = draw_data->DisplayPos.y;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    static_cast<xlux::RawPtr<ImGui_ImplXlux_VertexShader>>(bd->XluxVertexShader)->proj_mtx
        = xlux::math::Mat4x4::Orthographic(L, R, B, T, 0.01f, 1000.0f);
	renderer->BindPipeline(bd->XluxPipeline);
}

void ImGui_ImplXlux_RenderDrawData(xlux::RawPtr<ImDrawData> draw_data, xlux::RawPtr<xlux::Renderer> renderer)
{
     // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0)
        return;

    // backup state
    auto current_pipeline = renderer->GetActivePipeline();
    auto viewport = renderer->GetActiveViewport();

    auto bd = ImGui_ImplXlux_GetBackendData();


    ImGui_ImplXlux_SetupRenderState(draw_data, renderer);

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const auto draw_list = draw_data->CmdLists[n];

        auto vertex_buffer_size = draw_list->VtxBuffer.Size * sizeof(ImGui_ImplXlux_VertexInData);
        auto index_buffer_size = draw_list->IdxBuffer.Size * sizeof(xlux::U32);
        if (bd->VertexBuffer->GetSize() < vertex_buffer_size || bd->IndexBuffer->GetSize() < index_buffer_size)
        {
            ImGui_ImplXlux_RecreateVtxIdxBuffers(vertex_buffer_size, index_buffer_size);
        }

        auto vtx_dst = static_cast<ImGui_ImplXlux_VertexInData*>(bd->VertexBuffer->Map(vertex_buffer_size, 0));
        auto idx_dst = static_cast<xlux::U32*>(bd->IndexBuffer->Map(index_buffer_size, 0));

        for (int i = 0; i < draw_list->VtxBuffer.Size; i++)
        {
            vtx_dst[i].position = xlux::math::Vec3(draw_list->VtxBuffer[i].pos.x, draw_list->VtxBuffer[i].pos.y, 0.0);
            vtx_dst[i].uv = xlux::math::Vec2(draw_list->VtxBuffer[i].uv.x, draw_list->VtxBuffer[i].uv.y);
            auto color = ImColor(draw_list->VtxBuffer[i].col);
            vtx_dst[i].color = xlux::math::Vec4(color.Value.x, color.Value.y, color.Value.z, color.Value.w);
        }

        for (int i = 0; i < draw_list->IdxBuffer.Size / 3; i++)
        {
            idx_dst[i * 3 + 2] = draw_list->IdxBuffer[i * 3 + 0];
            idx_dst[i * 3 + 1] = draw_list->IdxBuffer[i * 3 + 1];
            idx_dst[i * 3 + 0] = draw_list->IdxBuffer[i * 3 + 2];
        }

        bd->VertexBuffer->Unmap();
        bd->IndexBuffer->Unmap();

        for (int cmd_i = 0; cmd_i < draw_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &draw_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != nullptr)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    ImGui_ImplXlux_SetupRenderState(draw_data, renderer);
                else
                    pcmd->UserCallback(draw_list, pcmd);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

				renderer->SetViewport((int)clip_min.x, (int)((float)fb_height - clip_max.y), (int)(clip_max.x - clip_min.x), (int)(clip_max.y - clip_min.y));

                // Bind texture, Draw
                static_cast<xlux::RawPtr<ImGui_ImplXlux_FragmentShader>>(bd->XluxFragmentShader)->texture = (xlux::RawPtr<xlux::Texture2D>)(intptr_t)pcmd->GetTexID();

                renderer->DrawIndexedOrdered(bd->VertexBuffer, bd->IndexBuffer, pcmd->ElemCount, 0, pcmd->IdxOffset);
            }
        }
    }

	// Restore pipeline
	renderer->BindPipeline(current_pipeline);
	renderer->SetViewport(viewport.x, viewport.y, viewport.width, viewport.height);

}


bool ImGui_ImplXlux_RecreateVtxIdxBuffers(xlux::Size vertex_buffer_size, xlux::Size index_buffer_size) 
{
    auto bd = ImGui_ImplXlux_GetBackendData();

    if (bd->VertexBuffer) { bd->XluxDevice->DestroyBuffer(bd->VertexBuffer); bd->VertexBuffer = nullptr; }
    if (bd->IndexBuffer) { bd->XluxDevice->DestroyBuffer(bd->IndexBuffer); bd->IndexBuffer = nullptr; }
    if (bd->VertexIndexBufferMemory) { bd->XluxDevice->FreeMemory(bd->VertexIndexBufferMemory); bd->VertexIndexBufferMemory = nullptr; }

    bd->VertexBuffer = bd->XluxDevice->CreateBuffer(vertex_buffer_size);
    bd->IndexBuffer = bd->XluxDevice->CreateBuffer(index_buffer_size);
    bd->VertexIndexBufferMemory = bd->XluxDevice->AllocateMemory(vertex_buffer_size + index_buffer_size);

    bd->IndexBuffer->BindMemory(bd->VertexIndexBufferMemory, 0);
	bd->VertexBuffer->BindMemory(bd->VertexIndexBufferMemory, bd->IndexBuffer->GetSize());

    return true;
}

bool ImGui_ImplXlux_CreateDeviceObjects()
{
    auto bd = ImGui_ImplXlux_GetBackendData();

    // DO NOT FREE THESE! They are managed by bd->XluxPipeline
    bd->XluxVertexShader = xlux::CreateRawPtr<ImGui_ImplXlux_VertexShader>();
    bd->XluxFragmentShader = xlux::CreateRawPtr<ImGui_ImplXlux_FragmentShader>();

    auto pipeline_create_info = xlux::PipelineCreateInfo()
        .SetShader(bd->XluxVertexShader, xlux::ShaderStage_Vertex)
        .SetShader(bd->XluxFragmentShader, xlux::ShaderStage_Fragment)
        .SetInterpolator(xlux::CreateRawPtr<xlux::BasicInterpolator<ImGui_ImplXlux_VertexOutData>>())
		.SetVertexItemSize(sizeof(ImGui_ImplXlux_VertexInData))
		.SetVertexToFragmentDataSize(sizeof(ImGui_ImplXlux_VertexOutData))
        .SetBlendEnable(true)
        .SetBlendEquation(xlux::EBlendEquation::BlendMode_Add)
        .SetSrcBlendFunction(xlux::EBlendFunction::BlendFunction_SrcAlpha)
        .SetDstBlendFunction(xlux::EBlendFunction::BlendFunction_OneMinusSrcAlpha)
		.SetSrcBlendFunctionAlpha(xlux::EBlendFunction::BlendFunction_One)
		.SetDstBlendFunctionAlpha(xlux::EBlendFunction::BlendFunction_OneMinusSrcAlpha)
        .SetBackfaceCullingEnable(false)
        .SetDepthTestEnable(false)
        .SetClippingEnable(false);        

    bd->XluxPipeline = bd->XluxDevice->CreatePipeline(pipeline_create_info);

    ImGui_ImplXlux_RecreateVtxIdxBuffers(10000, 10000);
    ImGui_ImplXlux_CreateFontsTexture();

    return true;
}

void ImGui_ImplXlux_DestroyDeviceObjects()
{
    auto bd = ImGui_ImplXlux_GetBackendData();

    if (bd->XluxPipeline) { bd->XluxDevice->DestroyPipeline(bd->XluxPipeline); bd->XluxPipeline = nullptr; }
    if (bd->VertexBuffer) { bd->XluxDevice->DestroyBuffer(bd->VertexBuffer); bd->VertexBuffer = nullptr; }  
    if (bd->IndexBuffer) { bd->XluxDevice->DestroyBuffer(bd->IndexBuffer); bd->IndexBuffer = nullptr; }
    if (bd->VertexIndexBufferMemory) { bd->XluxDevice->FreeMemory(bd->VertexIndexBufferMemory); bd->VertexIndexBufferMemory = nullptr; }


    ImGui_ImplXlux_DestroyFontsTexture();
}

#endif // IMGUI_DISABLE