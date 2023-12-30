#pragma once

#include "Core/Core.hpp"
#include "Core/ThreadPool.hpp"
#include "Math/Math.hpp"
#include "Impl/RendererCommon.hpp"
#include "Impl/Shader.hpp"

namespace klux
{
	struct FrameClearWorkerInput
	{
		U32 x = 0;
		U32 y = 0;
		U32 width = 0;
		U32 height = 0;

		FrameClearWorkerInput() {};
	};

	class FrameClearWorker : public IJob<FrameClearWorkerInput, U32>
	{
	public:
		FrameClearWorker() = default;
		~FrameClearWorker() = default;

		Bool Execute(FrameClearWorkerInput payload, U32& result) override;		

		inline void SetFramebuffer(RawPtr<IFramebuffer> fbo) { m_Framebuffer = fbo; }
		inline void SetClearColor(const math::Vec4& color) { m_Color = color; }
		inline void SetEnableClearDepth(Bool clearDepth) { m_ClearDepth = clearDepth; }
		inline void SetEnableClearColor(Bool clearColor) { m_ClearColor = clearColor; }

	private:
		RawPtr<IFramebuffer> m_Framebuffer = nullptr;
		Bool m_ClearColor = true;
		Bool m_ClearDepth = true;
		math::Vec4 m_Color = { 0.0f, 0.0f, 0.0f, 1.0f };
	};

}