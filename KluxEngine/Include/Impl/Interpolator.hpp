#pragma once

#include "Core/Core.hpp"

namespace klux
{

	class IInterpolator
	{
	public:
		virtual void ScaleAndAdd(void* dst, const void* src1, float scale) = 0;
		virtual void Reset(void* dst) = 0;
	};

	template<typename T>
	class IInterpolatorG : public IInterpolator
	{
	public:
		virtual void ScaleAndAdd(void* dst, const void* src1, float scale) override
		{
			ScaleAndAdd((T*)dst, (const T*)src1, scale);
		}

		virtual void Reset(void* dst) override
		{
			Reset((T*)dst);
		}

		virtual void ScaleAndAdd(T* dst, const T* src1, float scale) = 0;
		virtual void Reset(T* dst) = 0;
	};


	template<typename T>
	class BasicInterpolator : public IInterpolatorG<T>
	{
	public:
		virtual void ScaleAndAdd(T* dst, const T* src, float scale) override
		{
			dst->Add(src->Scaled(scale));
		}

		virtual void Reset(T* dst) override
		{
			*dst = T();
		}
	};

}