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


	class ITexture
	{
	public:

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
			return GetWidth() * GetHeight();
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
	};

	class Texture2D : public ITexture
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

		friend class Device;
	private:
		Texture2D(U32 width, U32 height, ETexelFormat format, RawPtr<Buffer> data);
		~Texture2D();

	private:
		RawPtr<Buffer> m_Buffer;
		ETexelFormat m_Format;
		U32 m_Width = 0, m_Height = 0;
	};
}