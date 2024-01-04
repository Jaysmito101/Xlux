#include "Impl/FragmentShaderWorker.hpp"
#include "Impl/DeviceMemory.hpp"
#include "Impl/Buffer.hpp"
#include "Impl/Pipeline.hpp"
#include "Impl/Interpolator.hpp"
#include "Impl/Shader.hpp"
#include "Impl/Framebuffer.hpp"
#include "Impl/RendererCommon.hpp"

namespace xlux
{
	// follows top-left rule
	Bool FragmentShaderWorker::PointInTriangle(const math::Vec2& p, const math::Vec4& p0, const math::Vec4& p1, const math::Vec4& p2)
	{
		const auto BAIS = 0.525f;

        const auto v0 = math::Vec2(p2[0] - p0[0], p2[1] - p0[1]);
		const auto v1 = math::Vec2(p1[0] - p0[0], p1[1] - p0[1]);
		const auto v2 = math::Vec2(p[0] - p0[0], p[1] - p0[1]);
		const auto dot00 = v0.Dot(v0);
		const auto dot01 = v0.Dot(v1);
		const auto dot02 = v0.Dot(v2);
		const auto dot11 = v1.Dot(v1);
		const auto dot12 = v1.Dot(v2);
		const auto invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
		const auto u = (dot11 * dot02 - dot01 * dot12) * invDenom;
		const auto v = (dot00 * dot12 - dot01 * dot02) * invDenom;
		return (u > -BAIS && v > -BAIS && u + v < 1.0f + BAIS);
	}

	math::Vec3 FragmentShaderWorker::CalculateBarycentric(const math::Vec2& p, const math::Vec4& a, const math::Vec4& b, const math::Vec4& c)
	{
		const auto ab = math::Vec3(b[0] - a[0], b[1] - a[1], 0.0f);
		const auto ac = math::Vec3(c[0] - a[0], c[1] - a[1], 0.0f);
		const auto ap = math::Vec3(p[0] - a[0], p[1] - a[1], 0.0f);
		const auto bp = math::Vec3(p[0] - b[0], p[1] - b[1], 0.0f);

		const F32 areaTriangle = ab.Cross(ac).Length();// *0.5f;
		const F32 areaPBC = ab.Cross(bp).Length();// *0.5f;
		const F32 areaPCA = ac.Cross(ap).Length();// *0.5f;

		const F32 u = areaPBC / areaTriangle;
		const F32 v = areaPCA / areaTriangle;
		const F32 w = 1.0f - u - v;

		return math::Vec3(w, v, u);
	}

	Bool FragmentShaderWorker::Execute(FragmentShaderWorkerInput payload, U32& result, Size threadID)
	{
		(void)result, (void)threadID;

		// payload.triangle.Log();

		auto boundingBox = payload.triangle.GetBoundingBox();

		const auto startX = std::max(payload.startX, static_cast<U32>(boundingBox[0]));
		const auto startY = std::max(payload.startY, static_cast<U32>(boundingBox[1]));
		const auto endX = std::min (payload.startX + payload.width, static_cast<U32>(boundingBox[2]));
		const auto endY = std::min (payload.startY + payload.height, static_cast<U32>(boundingBox[3]));

		auto interpolator = m_Pipeline->m_CreateInfo.interpolator;

		auto framebuffer = m_Framebuffer;

		FragmentShaderOutput fragmentShaderOutput = {};
		U8 fragmentInterpolatedInput[1024];

		auto& p0 = payload.triangle.GetBuiltInRef(0)->Position;
		auto& p1 = payload.triangle.GetBuiltInRef(1)->Position;
		auto& p2 = payload.triangle.GetBuiltInRef(2)->Position;
		auto vertexData = payload.triangle.GetVertexData();

		for (U32 y = startY; y < endY; ++y)
		{
			for (U32 x = startX; x < endX; ++x)
			{
				auto p = math::Vec2((F32)x, (F32)y);
				if (PointInTriangle(p, p0, p1, p2))
				{
					auto baycentric = CalculateBarycentric(p, p0, p1, p2);
					interpolator->Reset(fragmentInterpolatedInput);

					interpolator->ScaleAndAdd(fragmentInterpolatedInput, vertexData[0], baycentric[0]);
					interpolator->ScaleAndAdd(fragmentInterpolatedInput, vertexData[1], baycentric[1]);
					interpolator->ScaleAndAdd(fragmentInterpolatedInput, vertexData[2], baycentric[2]);

					fragmentShaderOutput.Depth = p0[2] * baycentric[0] + p1[2] * baycentric[1] + p2[2] * baycentric[2];
					m_Pipeline->m_CreateInfo.fragmentShader->Execute(fragmentInterpolatedInput, &fragmentShaderOutput);

					auto px = x, py = framebuffer->GetHeight() - 1 - y;

					if (BlendAndApplyDepth(px, py, framebuffer, fragmentShaderOutput.Depth))
					{
						BlendAndApplyColor(px, py, framebuffer, fragmentShaderOutput);
					}

				}
			}
		}

		return false;
	}

	Bool FragmentShaderWorker::BlendAndApplyDepth(U32 px, U32 py, RawPtr<IFramebuffer> framebuffer, F32 depth)
	{
		if (!m_Pipeline->m_CreateInfo.depthTestEnable) return true;
		if (!framebuffer->HasDepthAttachment()) return true;



		F32 currentDepth = 0.0f;
		framebuffer->GetDepthPixel(px, py, currentDepth);

		Bool testResult = false;

		switch (m_Pipeline->m_CreateInfo.depthCompareFunction)
		{
		case CompareFunction_Never:
		{
			testResult = false;
			break;
		}
		case CompareFunction_Less:
		{
			// Passes if the incoming depth value is less than the stored depth value.
			testResult = (depth < currentDepth);
			break;
		}
		case CompareFunction_Equal:
		{
			// Passes if the incoming depth value is equal to the stored depth value.
			testResult = (depth == currentDepth);
			break;
		}
		case CompareFunction_LessEqual:
		{
			// Passes if the incoming depth value is less than or equal to the stored depth value.
			testResult = (depth <= currentDepth);
			break;
		}
		case CompareFunction_Greater:
		{
			// Passes if the incoming depth value is greater than the stored depth value.
			testResult = (depth > currentDepth);
			break;
		}
		case CompareFunction_NotEqual:
		{
			// Passes if the incoming depth value is not equal to the stored depth value.
			testResult = (depth != currentDepth);
			break;
		}
		case CompareFunction_GreaterEqual:
		{
			// Passes if the incoming depth value is greater than or equal to the stored depth value.
			testResult = (depth >= currentDepth);
			break;
		}
		case CompareFunction_Always:
		{
			testResult = true;
			break;
		}
		default:
		{
			testResult = false;
			break;
		}
		}
		if (testResult) framebuffer->SetDepthPixel(px, py, depth);
		return testResult;
	}

	void FragmentShaderWorker::BlendAndApplyColor(U32 px, U32 py, RawPtr<IFramebuffer> framebuffer, const FragmentShaderOutput& output)
	{
		math::Vec4 dstColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		math::Vec4 blendedColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		for (U32 i = 0; i < std::min(framebuffer->GetColorAttachmentCount(), 4u); ++i)
		{
			auto srcColor = output.Color[i];

			if (m_Pipeline->m_CreateInfo.blendEnable)
			{
				framebuffer->GetColorPixel(i, px, py, dstColor[0], dstColor[1], dstColor[2], dstColor[3]);


				auto blendEquation = m_Pipeline->m_CreateInfo.blendEquation;
				auto srcBelndFunc = m_Pipeline->m_CreateInfo.srcBlendFunction;
				auto dstBlendFunc = m_Pipeline->m_CreateInfo.dstBlendFunction;
				F32 srcBlendFactor = CalculateBlendFactor(srcColor[3], dstColor[3], srcBelndFunc);
				F32 dstBlendFactor = CalculateBlendFactor(srcColor[3], dstColor[3], dstBlendFunc);

				switch (blendEquation)
				{
				case xlux::BlendMode_Add:
				{
					blendedColor = srcColor * srcBlendFactor + dstColor * dstBlendFactor;
					break;
				}
				case xlux::BlendMode_Subtract:
				{
					blendedColor = srcColor * srcBlendFactor - dstColor * dstBlendFactor;
					break;
				}
				case xlux::BlendMode_ReverseSubtract:
				{
					blendedColor = dstColor * dstBlendFactor - srcColor * srcBlendFactor;
					break;
				}
				case xlux::BlendMode_Min:
				{
					blendedColor[0] = std::min(srcColor[0] * srcBlendFactor, dstColor[0] * dstBlendFactor);
					blendedColor[1] = std::min(srcColor[1] * srcBlendFactor, dstColor[1] * dstBlendFactor);
					blendedColor[2] = std::min(srcColor[2] * srcBlendFactor, dstColor[2] * dstBlendFactor);
					blendedColor[3] = std::min(srcColor[3] * srcBlendFactor, dstColor[3] * dstBlendFactor);
					break;
				}
				case xlux::BlendMode_Max:
				{
					blendedColor[0] = std::max(srcColor[0] * srcBlendFactor, dstColor[0] * dstBlendFactor);
					blendedColor[1] = std::max(srcColor[1] * srcBlendFactor, dstColor[1] * dstBlendFactor);
					blendedColor[2] = std::max(srcColor[2] * srcBlendFactor, dstColor[2] * dstBlendFactor);
					blendedColor[3] = std::max(srcColor[3] * srcBlendFactor, dstColor[3] * dstBlendFactor);
					break;
				}
				default:
				{
					blendedColor = srcColor;
					break;
				}
				}
			}
			else
			{
				blendedColor = srcColor;
			}

			framebuffer->SetColorPixel(i, px, py,
				blendedColor[0],
				blendedColor[1],
				blendedColor[2],
				blendedColor[3]);
		}

	}

	F32 FragmentShaderWorker::CalculateBlendFactor(F32 srcAlpha, F32 dstAlpha, EBlendFunction blendFunction)
	{
		F32 factor = 1.0f;

		switch (blendFunction)
		{
		case BlendFunction_Zero:
		{
			factor = 0.0f;
			break;
		}
		case BlendFunction_One:
		{
			factor = 1.0f;
			break;
		}
		case BlendFunction_SrcAlpha:
		{
			factor = srcAlpha;
			break;
		}
		case BlendFunction_OneMinusSrcAlpha:
		{
			factor = 1.0f - srcAlpha;
			break;
		}
		case BlendFunction_DstAlpha:
		{
			factor = dstAlpha;
			break;
		}
		case BlendFunction_OneMinusDstAlpha:
		{
			factor = 1.0f - dstAlpha;
			break;
		}
		default:
		{
			break;
		}
		}

		return factor;
	}


}