#pragma once
#include "Core/Core.hpp"

namespace xlux
{
	class Device;
	class IShader;
	class IInterpolator;

	enum EShaderStage
	{
		ShaderStage_Vertex,
		ShaderStage_Fragment,
		ShaderStage_Compute
	};

	enum EBlendEquation
	{
		BlendMode_Add,
		BlendMode_Subtract,
		BlendMode_ReverseSubtract,
		BlendMode_Min,
		BlendMode_Max
	};

	enum EBlendFunction
	{
		BlendFunction_Zero,
		BlendFunction_One,
		BlendFunction_SrcAlpha,
		BlendFunction_OneMinusSrcAlpha,
		BlendFunction_DstAlpha,
		BlendFunction_OneMinusDstAlpha
	};

	enum ECompareFunction
	{
		CompareFunction_Never,
		CompareFunction_Less,
		CompareFunction_Equal,
		CompareFunction_LessEqual,
		CompareFunction_Greater,
		CompareFunction_NotEqual,
		CompareFunction_GreaterEqual,
		CompareFunction_Always
	};

	struct XLUX_API PipelineCreateInfo
	{
		Size vertexItemSize = 0;
		Size vertexToFragmentDataSize = 0;

		RawPtr<IShader> vertexShader = nullptr;
		RawPtr<IShader> fragmentShader = nullptr;
		RawPtr<IInterpolator> interpolator = nullptr;

		EBlendEquation blendEquation = BlendMode_Add;
		EBlendFunction srcBlendFunction = BlendFunction_SrcAlpha;
		EBlendFunction dstBlendFunction = BlendFunction_OneMinusSrcAlpha;

		ECompareFunction depthCompareFunction = CompareFunction_Less;

		Bool cullFaceEnable = false;
		Bool depthTestEnable = false;
		Bool blendEnable = false;
		Bool rasterizerDiscardEnable = false;
		Bool enableClipping = false;

		PipelineCreateInfo() = default;
		~PipelineCreateInfo() = default;

		PipelineCreateInfo& SetVertexItemSize(Size size)
		{
			vertexItemSize = size;
			return *this;
		}

		PipelineCreateInfo& SetVertexToFragmentDataSize(Size size)
		{
			vertexToFragmentDataSize = size;
			return *this;
		}

		PipelineCreateInfo& SetShader(RawPtr<IShader> shader, EShaderStage stage)
		{
			if (stage == ShaderStage_Vertex)
			{
				vertexShader = shader;
			}
			else if (stage == ShaderStage_Fragment)
			{
				fragmentShader = shader;
			}
			else
			{
				xlux::log::Error("Invalid shader stage");
			}
			return *this;
		}

		PipelineCreateInfo& SetClippingEnable(bool enable)
		{
			enableClipping = enable;
			return *this;
		}

		PipelineCreateInfo& SetInterpolator(RawPtr<IInterpolator> in)
		{
			this->interpolator = in;
			return *this;
		}

		PipelineCreateInfo& SetBlendEquation(EBlendEquation equation)
		{
			blendEquation = equation;
			return *this;
		}

		PipelineCreateInfo& SetSrcBlendFunction(EBlendFunction function)
		{
			srcBlendFunction = function;
			return *this;
		}

		PipelineCreateInfo& SetDstBlendFunction(EBlendFunction function)
		{
			dstBlendFunction = function;
			return *this;
		}

		PipelineCreateInfo& SetDepthCompareFunction(ECompareFunction function)
		{
			depthCompareFunction = function;
			return *this;
		}

		PipelineCreateInfo& SetCullFaceEnable(bool enable)
		{
			cullFaceEnable = enable;
			return *this;
		}

		PipelineCreateInfo& SetDepthTestEnable(bool enable)
		{
			depthTestEnable = enable;
			return *this;
		}

		PipelineCreateInfo& SetBlendEnable(bool enable)
		{
			blendEnable = enable;
			return *this;
		}

		PipelineCreateInfo& SetRasterizerDiscardEnable(bool enable)
		{
			rasterizerDiscardEnable = enable;
			return *this;
		}



	};

	class XLUX_API Pipeline
	{
	public:


		friend class Device;

	private:
		Pipeline(const PipelineCreateInfo& createInfo);
		~Pipeline();

		friend class VertexShaderWorker;
		friend class FragmentShaderWorker;
	private:
		PipelineCreateInfo m_CreateInfo;
	};
}