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

	class ShaderTriangleRef
	{
	public:
		ShaderTriangleRef(RawPtr<LinearAllocator> alloc, Size vertexDataSize)
		{
			m_VertexDataSize = vertexDataSize;
			m_VertexData[0] = alloc->Allocate( vertexDataSize + sizeof(ShaderBuiltIn) );
			m_VertexData[1] = alloc->Allocate( vertexDataSize + sizeof(ShaderBuiltIn) );
			m_VertexData[2] = alloc->Allocate( vertexDataSize + sizeof(ShaderBuiltIn) );

			m_BuiltInRefs[0] = reinterpret_cast<RawPtr<ShaderBuiltIn>>(reinterpret_cast<U8*>(m_VertexData[0]) + vertexDataSize);
			m_BuiltInRefs[1] = reinterpret_cast<RawPtr<ShaderBuiltIn>>(reinterpret_cast<U8*>(m_VertexData[1]) + vertexDataSize);
			m_BuiltInRefs[2] = reinterpret_cast<RawPtr<ShaderBuiltIn>>(reinterpret_cast<U8*>(m_VertexData[2]) + vertexDataSize);
		}

		inline RawPtr<ShaderBuiltIn>* GetBuiltInRefs() { return m_BuiltInRefs; }
		inline RawPtr<void>* GetVertexData() { return m_VertexData; }

		inline RawPtr<void> GetVertexData(U32 index) { return m_VertexData[index]; }
		inline RawPtr<ShaderBuiltIn> GetBuiltInRef(U32 index) { return m_BuiltInRefs[index]; }

#ifndef NDEBUG
		inline void Log()
		{
			log::Info("TriangleRef: {{\n\tPos[0]: ({}, {}, {})\n\tPos[1]: ({}, {}, {})\n\tPos[2]: ({}, {}, {})\n}}",
				m_BuiltInRefs[0]->Position[0], m_BuiltInRefs[0]->Position[1], m_BuiltInRefs[0]->Position[2],
				m_BuiltInRefs[1]->Position[0], m_BuiltInRefs[1]->Position[1], m_BuiltInRefs[1]->Position[2],
				m_BuiltInRefs[2]->Position[0], m_BuiltInRefs[2]->Position[1], m_BuiltInRefs[2]->Position[2]);
		}
#endif

	private:
		RawPtr<void> m_VertexData[3] = { nullptr, nullptr, nullptr };
		RawPtr<ShaderBuiltIn> m_BuiltInRefs[3] = { nullptr, nullptr, nullptr };
		Size m_VertexDataSize = 0;
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