#pragma once

#include "Core/Core.hpp"
#include "Core/ThreadPool.hpp"
#include "Math/Math.hpp"
#include "Impl/RendererCommon.hpp"

namespace xlux
{

	
	struct FragmentShaderWorkerInput
	{
		ShaderTriangleRef triangle;
		U32 startX = 0;
		U32 startY = 0;
		U32 width = 0;
		U32 height = 0;

		FragmentShaderWorkerInput() {}
	};


	class FragmentShaderWorker : public IJob<FragmentShaderWorkerInput, U32>
	{
	public:
		FragmentShaderWorker() = default;
		~FragmentShaderWorker() = default;

		Bool Execute(FragmentShaderWorkerInput payload, U32& result) override;

		inline void SetPipeline(RawPtr<Pipeline> pipeline) { m_Pipeline = pipeline; }
		inline void SetFramebuffer(RawPtr<IFramebuffer> framebuffer) { m_Framebuffer = framebuffer; }

	private:
		Bool PointInTriangle(const math::Vec2& p, const math::Vec4& p0, const math::Vec4& p1, const math::Vec4& p2);
		math::Vec3 CalculateBarycentric(const math::Vec2& p, const math::Vec4& a, const math::Vec4& b, const math::Vec4& c);
		void BlendAndApplyDepth(U32 px, U32 py, RawPtr<IFramebuffer> fbo, F32 depth);
		void BlendAndApplyColor(U32 px, U32 py, RawPtr<IFramebuffer> fbo, const FragmentShaderOutput& output);
		F32 CalculateBlendFactor(F32 srcAlpha, F32 dstAlpha, EBlendFunction blendFunction);

	private:
		RawPtr<Pipeline> m_Pipeline = nullptr;
		RawPtr<IFramebuffer> m_Framebuffer = nullptr;
	};
}