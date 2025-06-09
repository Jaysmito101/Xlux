#pragma once
#include "Core/Core.hpp"

namespace xlux
{
	class Device;
	class Buffer;
	class IShader;
	class IInterpolator;

	enum ETextureType
	{
		TextureType_2D,
		TextureType_Cube
	};

	enum ETexelFormat
	{
		TexelFormat_RGB,
		TexelFormat_RGBA,
		TexelFormat_Depth
	};


	class XLUX_API ITexture
	{
	public:
		virtual ~ITexture() {}

		inline I32 GetWidth() const { return GetSize().x; }
		inline I32 GetHeight() const { return GetSize().y; }

		inline Bool IsDepth() const { return GetFormat() == TexelFormat_Depth; }
		inline Bool IsColor() const { return GetFormat() == TexelFormat_RGB || GetFormat() == TexelFormat_RGBA; }

		inline Size GetPixelSize() const
		{
			switch (GetFormat())
			{
			case TexelFormat_RGB: return 3;
			case TexelFormat_RGBA: return 4;
			case TexelFormat_Depth: return 1;
			default: throw std::runtime_error("Invalid texel format");
			}
		}

		inline Size GetPixelCount() const
		{
			return GetWidth() * GetHeight() * GetDepth();
		}

		inline Size GetSizeInBytes() const
		{
			return GetPixelCount() * GetPixelSize() * sizeof(F32); // internally everything is stored as F32
		}

		virtual Bool SetData(const void* data, Size size, Size offset) = 0;
		virtual Bool GetData(void* data, Size size, Size offset) const = 0;
		virtual Bool SetPixel(U32 x, U32 y, U32 z, F32 r, F32 g, F32 b, F32 a) = 0;
		virtual Bool GetPixel(U32 x, U32 y, U32 z, F32& r, F32& g, F32& b, F32& a) const = 0;
		virtual Pair<U32, U32> GetSize() const = 0;
		virtual U32 GetDepth() const = 0;
		virtual ETexelFormat GetFormat() const = 0;
		virtual ETextureType GetType() const = 0;

		virtual math::Vec4 Sample(const math::Vec3& uvw) const { (void)uvw; return math::Vec4(1.0f, 0.0f, 1.0f, 1.0f); }

		virtual math::Vec4 SampleInterpolated(const math::Vec3& uvw) const
		{
			// sample with trilinear interpolation
			// sample with trilinear interpolation
			F32 u = uvw[0];
			F32 v = uvw[1];
			F32 w = uvw[2];

			F32 width = (F32)GetWidth();
			F32 height = (F32)GetHeight();
			F32 depth = (F32)GetDepth();

			F32 u_pixel = u * width;
			F32 v_pixel = v * height;
			F32 w_pixel = w * depth;

			F32 u_floor = std::floor(u_pixel);
			F32 v_floor = std::floor(v_pixel);
			F32 w_floor = std::floor(w_pixel);

			F32 u_frac = u_pixel - u_floor;
			F32 v_frac = v_pixel - v_floor;
			F32 w_frac = w_pixel - w_floor;

			// The 8 corners of the cube
			math::Vec3 uvw000 = { u_floor / width, v_floor / height, w_floor / depth };
			math::Vec3 uvw100 = { (u_floor + 1) / width, v_floor / height, w_floor / depth };
			math::Vec3 uvw010 = { u_floor / width, (v_floor + 1) / height, w_floor / depth };
			math::Vec3 uvw110 = { (u_floor + 1) / width, (v_floor + 1) / height, w_floor / depth };
			math::Vec3 uvw001 = { u_floor / width, v_floor / height, (w_floor + 1) / depth };
			math::Vec3 uvw101 = { (u_floor + 1) / width, v_floor / height, (w_floor + 1) / depth };
			math::Vec3 uvw011 = { u_floor / width, (v_floor + 1) / height, (w_floor + 1) / depth };
			math::Vec3 uvw111 = { (u_floor + 1) / width, (v_floor + 1) / height, (w_floor + 1) / depth };

			// Sample the 8 corners
			math::Vec4 c000 = Sample(uvw000);
			math::Vec4 c100 = Sample(uvw100);
			math::Vec4 c010 = Sample(uvw010);
			math::Vec4 c110 = Sample(uvw110);
			math::Vec4 c001 = Sample(uvw001);
			math::Vec4 c101 = Sample(uvw101);
			math::Vec4 c011 = Sample(uvw011);
			math::Vec4 c111 = Sample(uvw111);

			// Interpolate along x
			math::Vec4 c00 = c000 * (1 - u_frac) + c100 * u_frac;
			math::Vec4 c01 = c001 * (1 - u_frac) + c101 * u_frac;
			math::Vec4 c10 = c010 * (1 - u_frac) + c110 * u_frac;
			math::Vec4 c11 = c011 * (1 - u_frac) + c111 * u_frac;

			// Interpolate along y
			math::Vec4 c0 = c00 * (1 - v_frac) + c10 * v_frac;
			math::Vec4 c1 = c01 * (1 - v_frac) + c11 * v_frac;

			// Interpolate along z and return
			return c0 * (1 - w_frac) + c1 * w_frac;
		}
	};

	class XLUX_API Texture2D : public ITexture
	{
	public:

		Bool SetData(const void* data, Size size, Size offset) override;
		Bool GetData(void* data, Size size, Size offset) const override;
		Bool SetPixel(U32 x, U32 y, U32 z, F32 r, F32 g, F32 b, F32 a) override;
		Bool GetPixel(U32 x, U32 y, U32 z, F32& r, F32& g, F32& b, F32& a) const override;

		math::Vec4 Sample(const math::Vec3& uvw) const override;


		inline Pair<U32, U32> GetSize() const override { return { m_Width, m_Height }; }
		inline ETexelFormat GetFormat() const override { return m_Format; }
		inline ETextureType GetType() const override { return TextureType_2D; }
		inline U32 GetDepth() const override { return 1; }
		inline void BindBuffer(RawPtr<Buffer> buffer) { m_Buffer = buffer; }

		friend class Device;
	private:
		Texture2D(U32 width, U32 height, ETexelFormat format);
		~Texture2D();

	private:
		RawPtr<Buffer> m_Buffer;
		ETexelFormat m_Format;
		U32 m_Width = 0, m_Height = 0;
	};
}