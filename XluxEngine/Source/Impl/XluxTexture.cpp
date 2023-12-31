#include "Impl/Texture.hpp"
#include "Impl/Buffer.hpp"

namespace xlux
{

	Texture2D::Texture2D(U32 width, U32 height, ETexelFormat format)
		: m_Width(width), m_Height(height), m_Format(format), m_Buffer(nullptr)
	{
	}

	Texture2D::~Texture2D()
	{
	}

	Bool Texture2D::SetData(const void* data, Size size, Size offset)
	{
		if (size + offset > m_Buffer->GetSize())
			return false;

		m_Buffer->SetData(data, size, offset);
		return true;
	}

	Bool Texture2D::GetData(void* data, Size size, Size offset) const
	{
		if (size + offset > m_Buffer->GetSize())
			return false;

		m_Buffer->GetData(data, size, offset);
		return true;
	}

	Bool Texture2D::SetPixel(U32 x, U32 y, U32 z, F32 r, F32 g, F32 b, F32 a)
	{
		(void)z;

		if (x < 0u || x >= m_Width || y < 0u || y >= m_Height)
			return false;

		Size pixelSize = GetPixelSize() * sizeof(F32);
		Size offset = (x + y * m_Width) * pixelSize;

		if (offset + pixelSize > m_Buffer->GetSize())
			return false;

		F32 pixel[4] = { r, g, b, a };
		m_Buffer->SetData(pixel, pixelSize, offset);

		return true;
	}

	Bool Texture2D::GetPixel(U32 x, U32 y, U32 z, F32& r, F32& g, F32& b, F32& a) const
	{
		(void)z;
		if (x < 0 || x >= m_Width || y < 0 || y >= m_Height)
			return false;

		Size pixelSize = GetPixelSize() * sizeof(F32);
		Size offset = (x + y * m_Width) * pixelSize;

		if (offset + pixelSize > m_Buffer->GetSize())
			return false;

		F32 pixel[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		m_Buffer->GetData(pixel, pixelSize, offset);

		r = pixel[0];
		g = pixel[1];
		b = pixel[2];
		a = pixel[3];

		return true;
	}


	math::Vec4 Texture2D::Sample(const math::Vec3& uvw) const
	{
		U32 x = std::clamp((U32)(uvw[0] * m_Width), 0u, m_Width - 1);
		U32 y = std::clamp((U32)(uvw[1] * m_Height), 0u, m_Height - 1);

		F32 r, g, b, a;

		GetPixel(x, y, 0, r, g, b, a);

		return math::Vec4(r, g, b, a);
	}
}