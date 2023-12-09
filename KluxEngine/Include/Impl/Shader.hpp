#pragma once

#include "Core/Core.hpp"

#include "Math/Math.hpp"

namespace klux
{

	class Device;

	class ShaderBuiltIn
	{
	public:
		math::Vec4 Position;
		U32 VertexIndex;

		inline ShaderBuiltIn() = default;

		inline ShaderBuiltIn& SetPosition(const math::Vec4& position) { Position = position; return *this; }
		inline ShaderBuiltIn& SetVertexIndex(U32 vertexIndex) { VertexIndex = vertexIndex; return *this; }

		inline void Reset()
		{
			Position = math::Vec4(0.0f, 0.0f, 0.0f, 1.0f);
			VertexIndex = 0;
		}
	};

	class FragShaderOutput
	{
	public:
		math::Vec4 Color;
	};

	class IShader
	{
	public:
		virtual Bool Execute(const RawPtr<void> dataIn, RawPtr<void> dataOut, RawPtr<ShaderBuiltIn> builtIn = nullptr) = 0;
	};


	template <typename InDataType, typename OutDataType>
	class IShaderG : public IShader
	{
	public:
		Bool Execute(const RawPtr<void> dataIn, RawPtr<void> dataOut, RawPtr<ShaderBuiltIn> builtIn = nullptr) override
		{
			return Execute(static_cast<const RawPtr<InDataType>>(dataIn), static_cast<RawPtr<OutDataType>>(dataOut), builtIn);
		}

		virtual Bool Execute(const RawPtr<InDataType> dataIn, RawPtr<OutDataType> dataOut, RawPtr<ShaderBuiltIn> builtIn = nullptr) = 0;
	};
}