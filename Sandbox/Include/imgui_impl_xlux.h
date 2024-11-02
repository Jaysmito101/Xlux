// dear imgui: Renderer Backend for Xlux 

#pragma once

#include "imgui.h"
#include "Xlux.hpp"

#ifndef IMGUI_DISABLE

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
IMGUI_IMPL_API bool     ImGui_ImplXlux_Init(xlux::RawPtr<xlux::Device> device);
IMGUI_IMPL_API void     ImGui_ImplXlux_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplXlux_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplXlux_RenderDrawData(xlux::RawPtr<ImDrawData> draw_data, xlux::RawPtr<xlux::Renderer> renderer);

// (Optional) Called by Init/NewFrame/Shutdown
IMGUI_IMPL_API bool     ImGui_ImplXlux_CreateFontsTexture();
IMGUI_IMPL_API void     ImGui_ImplXlux_DestroyFontsTexture();
IMGUI_IMPL_API bool     ImGui_ImplXlux_CreateDeviceObjects();
IMGUI_IMPL_API void     ImGui_ImplXlux_DestroyDeviceObjects();
IMGUI_IMPL_API bool     ImGui_ImplXlux_RecreateVtxIdxBuffers(xlux::Size vertex_buffer_size, xlux::Size index_buffer_size);

#endif // IMGUI_DISABLE
