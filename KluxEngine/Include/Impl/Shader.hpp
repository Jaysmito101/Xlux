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