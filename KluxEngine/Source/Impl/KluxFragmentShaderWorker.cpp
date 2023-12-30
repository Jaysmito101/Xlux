#include "Impl/FragmentShaderWorker.hpp"
#include "Impl/DeviceMemory.hpp"
#include "Impl/Buffer.hpp"
#include "Impl/Pipeline.hpp"
#include "Impl/Interpolator.hpp"
#include "Impl/Shader.hpp"
#include "Impl/Framebuffer.hpp"
#include "Impl/RendererCommon.hpp"

namespace klux
{
	Bool FragmentShaderWorker::PointInTriangle(const math::Vec2& p, const math::Vec4& p0, const math::Vec4& p1, const math::Vec4& p2)
	{
		// Compute vectors        
		math::Vec2 v0 = { p2[0] - p0[0], p2[1] - p0[1] };
        math::Vec2 v1 = { p1[0] - p0[0], p1[1] - p0[1] };
		math::Vec2 v2 = { p[0] - p0[0], p[1] - p0[1] };
		// Compute dot products
		F32 dot00 = v0.Dot(v0);
		F32 dot01 = v0.Dot(v1);
		F32 dot02 = v0.Dot(v2);
		F32 dot11 = v1.Dot(v1);
		F32 dot12 = v1.Dot(v2);

		// Compute barycentric coordinates
		F32 invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
		F32 u = (dot11 * dot02 - dot01 * dot12) * invDenom;
		F32 v = (dot00 * dot12 - dot01 * dot02) * invDenom;
		// Check if point is in triangle
		return (u >= 0.0f) && (v >= 0.0f) && (u + v < 1.0f);

	}

	Bool FragmentShaderWorker::Execute(FragmentShaderWorkerInput payload, U32& result)
	{
		(void)result;

		// payload.triangle.Log();

		auto boundingBox = payload.triangle.GetBoundingBox();

		payload.startX = std::max(payload.startX, static_cast<U32>(boundingBox[0]));
		payload.startY = std::max(payload.startY, static_cast<U32>(boundingBox[1]));
		payload.width = std::min(payload.width, static_cast<U32>(boundingBox[2] - payload.startX));
		payload.height = std::min(payload.height, static_cast<U32>(boundingBox[3] - payload.startY));
	
		auto interpolator = m_Pipeline->m_CreateInfo.interpolator;

		auto framebuffer = m_Framebuffer;

		FragmentShaderOutput fragmentShaderOutput = {};
		U8 fragmentInterpolatedInput[1024];

		for (U32 y = payload.startY; y < payload.startY + payload.height; ++y)
		{
			for (U32 x = payload.startX; x < payload.startX + payload.width; ++x)
			{
				// klux::log::Info("FragmentShaderWorker::Execute({},{})", x, y);
				auto p = math::Vec2((F32)x, (F32)y);
				auto& p0 = payload.triangle.GetBuiltInRef(0)->Position;
				auto& p1 = payload.triangle.GetBuiltInRef(1)->Position;
				auto& p2 = payload.triangle.GetBuiltInRef(2)->Position;
				auto vertexData = payload.triangle.GetVertexData();
				if (PointInTriangle(p, p0, p1, p2))
				{
					auto baycentric = CalculateBarycentric(p, p0, p1, p2);
					interpolator->Reset(fragmentInterpolatedInput);

					interpolator->ScaleAndAdd(fragmentInterpolatedInput, vertexData[0], baycentric[0]);
					interpolator->ScaleAndAdd(fragmentInterpolatedInput, vertexData[1], baycentric[1]);
					interpolator->ScaleAndAdd(fragmentInterpolatedInput, vertexData[2], baycentric[2]);

					m_Pipeline->m_CreateInfo.fragmentShader->Execute(fragmentInterpolatedInput, &fragmentShaderOutput);

					auto px =  x, py = framebuffer->GetHeight() - 1 - y;

					if (framebuffer->HasDepthAttachment()) framebuffer->SetDepthPixel(px, py, p0[3] * baycentric[0] + p1[3] * baycentric[1] + p2[3] * baycentric[2]);
					for (U32 i = 0; i < std::min(framebuffer->GetColorAttachmentCount(), 4u); ++i)
					{
						framebuffer->SetColorPixel(i, px, py,
							fragmentShaderOutput.Color[i][0],
							fragmentShaderOutput.Color[i][1],
							fragmentShaderOutput.Color[i][2],
							fragmentShaderOutput.Color[i][3]);
					}
				}
			}
		}
		
		return false;
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
		
		return math::Vec3( w, v, u );
	}


}