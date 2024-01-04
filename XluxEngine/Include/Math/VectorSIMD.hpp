#pragma once

#include "Core/Types.hpp"
#include "Core/Logger.hpp"


#if defined(_WIN32) || defined(_WIN64)
#include <intrin.h>
#elif defined(__linux__)
#include <x86intrin.h>
#endif

namespace xlux
{

	namespace math_simd
	{

		template <typename T>
		concept VectorValueType = std::is_same_v<T, F32> || std::is_same_v<T, I32>;

		template <Size N, typename ValueTypeT = ValueType>
		class Vec
		{
			static_assert (N <= 4, " Vec<N, ValueTypeT> SIMD version is only defined for N < 4");
		public:
			XLUX_FORCE_INLINE Vec(const ValueTypeT& value)
			{
				for (Size i = 0; i < N; ++i)
				{
					if constexpr (std::is_same_v<ValueTypeT, F32>)
					{
						m_Data.m128_f32[i] = value;
					}
					else if constexpr (std::is_same_v<ValueTypeT, I32>)
					{
						m_Data.m128_i32[i] = value;
					}
				}
			}

			XLUX_FORCE_INLINE Vec(const ValueTypeT* data)
			{
				if constexpr (std::is_same_v<ValueTypeT, F32>)
				{
					m_Data = _mm_loadu_ps(data);
				}
				else if constexpr (std::is_same_v<ValueTypeT, I32>)
				{

				}
			}

			template <Size M, VectorValueType... Args>
			XLUX_FORCE_INLINE Vec(const Vec<M>& other, Args... args)
			{
				m_Data = other.m_Data;

				if constexpr (M < N && M == 1) {
					static_assert(sizeof...(args) == N - 1, "Invalid number of arguments");
					auto data = { static_cast<ValueTypeT>(args)... };
					if constexpr (std::is_same_v<ValueTypeT, F32>)
					{
						m_Data.m128_f32[M] = *data.begin();
					}
					else if constexpr (std::is_same_v<ValueTypeT, I32>)
					{
						m_Data.m128_i32[M] = *data.begin();
					}
				}
				else if constexpr (M < N && M == 2) {
					static_assert(sizeof...(args) == N - 2, "Invalid number of arguments");
					auto data = { static_cast<ValueTypeT>(args)... };
					if constexpr (std::is_same_v<ValueTypeT, F32>)
					{
						m_Data.m128_f32[M] = *data.begin();
						m_Data.m128_f32[M + 1] = *(data.begin() + 1);
					}
					else if constexpr (std::is_same_v<ValueTypeT, I32>)
					{
						m_Data.m128_i32[M] = *data.begin();
						m_Data.m128_i32[M + 1] = *(data.begin() + 1);
					}
				}
				else if constexpr (M < N && M == 3) {
					static_assert(sizeof...(args) == N - 3, "Invalid number of arguments");
					auto data = { static_cast<ValueTypeT>(args)... };
					if constexpr (std::is_same_v<ValueTypeT, F32>)
					{
						m_Data.m128_f32[M] = *data.begin();
						m_Data.m128_f32[M + 1] = *(data.begin() + 1);
						m_Data.m128_f32[M + 2] = *(data.begin() + 2);
					}
					else if constexpr (std::is_same_v<ValueTypeT, I32>)
					{
						m_Data.m128_i32[M] = *data.begin();
						m_Data.m128_i32[M + 1] = *(data.begin() + 1);
						m_Data.m128_i32[M + 2] = *(data.begin() + 2);
					}
				}
			}

			template <VectorValueType... Args>
			XLUX_FORCE_INLINE Vec(Args... args)
			{
				static_assert(sizeof...(args) <= N, "Invalid number of arguments");

				if constexpr (sizeof...(args) == 0) 
				{
					m_Data = _mm_setzero_ps();
				}
				else 
				{
					auto data = { static_cast<ValueTypeT>(args)... };
					for (Size i = 0; i < sizeof...(args); ++i)
					{
						if constexpr (std::is_same_v<ValueTypeT, F32>)
						{
							m_Data.m128_f32[i] = *(data.begin() + i);
						}
						else if constexpr (std::is_same_v<ValueTypeT, I32>)
						{
							m_Data.m128_i32[i] = *(data.begin() + i);
						}
					}
				}

			}


			XLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator=(const Vec<N, ValueTypeT>& other)
			{
				m_Data = other.m_Data;
				return *this;
			}

			XLUX_FORCE_INLINE ValueTypeT& operator[](Size index)
			{
				if constexpr (std::is_same_v<ValueTypeT, F32>)
				{
					return m_Data.m128_f32[index];
				}
				else if constexpr (std::is_same_v<ValueTypeT, I32>)
				{
					return m_Data.m128_i32[index];
				}
			}

			XLUX_FORCE_INLINE const ValueTypeT& operator[](Size index) const
			{
				if constexpr (std::is_same_v<ValueTypeT, F32>)
				{
					return m_Data.m128_f32[index];
				}
				else if constexpr (std::is_same_v<ValueTypeT, I32>)
				{
					return m_Data.m128_i32[index];
				}
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT> operator+(const Vec<N, ValueTypeT>& other) const
			{
				Vec<N, ValueTypeT> result;
				result.m_Data = _mm_add_ps(m_Data, other.m_Data);
				return result;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT> operator-(const Vec<N, ValueTypeT>& other) const
			{
				Vec<N, ValueTypeT> result;
				result.m_Data = _mm_sub_ps(m_Data, other.m_Data);
				return result;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT> operator*(const Vec<N, ValueTypeT>& other) const
			{
				Vec<N, ValueTypeT> result;
				result.m_Data = _mm_mul_ps(m_Data, other.m_Data);
				return result;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT> operator/(const Vec<N, ValueTypeT>& other) const
			{
				Vec<N, ValueTypeT> result;
				result.m_Data = _mm_div_ps(m_Data, other.m_Data);
				return result;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT> operator*(const ValueTypeT& value) const
			{
				auto scalar = _mm_set1_ps(value);
				Vec<N, ValueTypeT> result;
				result.m_Data = _mm_mul_ps(m_Data, scalar);
				return result;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT> operator/(const ValueTypeT& value) const
			{
				auto scalar = _mm_set1_ps(1.0f / value);
				Vec<N, ValueTypeT> result;
				result.m_Data = _mm_mul_ps(m_Data, scalar);
				return result;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator+=(const Vec<N, ValueTypeT>& other)
			{
				m_Data = _mm_add_ps(m_Data, other.m_Data);
				return *this;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator-=(const Vec<N, ValueTypeT>& other)
			{
				m_Data = _mm_sub_ps(m_Data, other.m_Data);
				return *this;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator*=(const Vec<N, ValueTypeT>& other)
			{
				m_Data = _mm_mul_ps(m_Data, other.m_Data);
				return *this;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator/=(const Vec<N, ValueTypeT>& other)
			{
				m_Data = _mm_div_ps(m_Data, other.m_Data);
				return *this;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator*=(const ValueTypeT& value)
			{
				auto scalar = _mm_set1_ps(value);
				m_Data = _mm_mul_ps(m_Data, scalar);
				return *this;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator/=(const ValueTypeT& value)
			{
				auto scalar = _mm_set1_ps(1.0f / value);
				m_Data = _mm_mul_ps(m_Data, scalar);
				return *this;
			}

			XLUX_FORCE_INLINE bool operator==(const Vec<N, ValueTypeT>& other) const
			{
				for (Size i = 0; i < N; ++i)
				{
					if (m_Data.m128_f32[i] != other.m_Data.m128_f32[i])
					{
						return false;
					}
				}
				return true;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT> operator-() const
			{
				Vec<N, ValueTypeT> result;
				result.m_Data = _mm_sub_ps(_mm_setzero_ps(), m_Data);
				return result;
			}


			XLUX_FORCE_INLINE bool operator!=(const Vec<N, ValueTypeT>& other) const
			{
				return !(*this == other);
			}

			XLUX_FORCE_INLINE ValueTypeT Length() const
			{
				return std::sqrt(LengthSquared());
			}

			XLUX_FORCE_INLINE ValueTypeT  LengthSquared() const
			{
				return Dot(*this);
			}

			XLUX_FORCE_INLINE ValueTypeT  Dot(const Vec<N, ValueTypeT>& other) const
			{
				auto result = _mm_mul_ps(m_Data, other.m_Data);
				__m128 shuf = _mm_shuffle_ps(result, result, _MM_SHUFFLE(2, 3, 0, 1));
				__m128 sums = _mm_add_ps(result, shuf);
				shuf = _mm_movehl_ps(shuf, sums);
				sums = _mm_add_ss(sums, shuf);
				return _mm_cvtss_f32(sums);
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT> Pow(F32 power) const
			{
				Vec<N, ValueTypeT> result;
				for (Size i = 0; i < N; ++i)
				{
					result[i] = std::pow(m_Data.m128_f32[i], power);
				}
				return result;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT> Cross(const Vec<N, ValueTypeT>& other) const
			{
				static_assert(N == 3 || N == 2, "Cross product is only defined for 3D vectors");
				if constexpr (N == 2)
				{
					F32 result = m_Data.m128_f32[0] * other.m_Data.m128_f32[1] - m_Data.m128_f32[1] * other.m_Data.m128_f32[0];
					return Vec<2, ValueTypeT>(result, 0.0f);
				}
				else if constexpr (N == 3)
				{
					Vec<3, ValueTypeT> result;
					__m128 tmp0 = _mm_shuffle_ps(m_Data, m_Data, _MM_SHUFFLE(3, 0, 2, 1));
					__m128 tmp1 = _mm_shuffle_ps(other.m_Data, other.m_Data, _MM_SHUFFLE(3, 1, 0, 2));
					__m128 tmp2 = _mm_mul_ps(tmp0, other.m_Data);
					__m128 tmp3 = _mm_mul_ps(tmp0, tmp1);
					__m128 tmp4 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(3, 0, 2, 1));
					result.m_Data = _mm_sub_ps(tmp3, tmp4);
					return result;
				}
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT> Normalized() const
			{
				return *this / Length();
			}

			XLUX_FORCE_INLINE void Normalize()
			{
				*this /= Length();
			}

			XLUX_FORCE_INLINE ValueTypeT* RawData()
			{
				return m_Data;
			}

			XLUX_FORCE_INLINE const ValueTypeT* RawData() const
			{
				return m_Data;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator<<(const ValueTypeT& value)
			{
				for (Size i = 0; i < N - 1; ++i)
				{
					m_Data.m128_f32[i] = m_Data.m128_f32[i + 1];
				}
				m_Data.m128_f32[N - 1] = value;
				return *this;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator>>(const ValueTypeT& value)
			{
				for (Size i = N - 1; i > 0; --i)
				{
					m_Data.m128_f32[i] = m_Data.m128_f32[i - 1];
				}
				m_Data.m128_f32[0] = value;
				return *this;
			}

			template <Size M, typename ValueTypeT2>
			friend std::ostream& operator<<(std::ostream& os, const Vec<M, ValueTypeT2>& vec);

			template <Size M, typename ValueTypeT2>
			friend class Vec;


		private:
			__m128 m_Data;
		};

		template <Size N, typename ValueTypeT>
		XLUX_FORCE_INLINE std::ostream& operator<<(std::ostream& os, const Vec<N, ValueTypeT>& vec)
		{
			os << "(";
			for (Size i = 0; i < N; ++i)
			{
				os << vec[i];
				if (i != N - 1)
				{
					os << ", ";
				}
			}
			os << ")";
			return os;
		};


		using Vec2 = Vec<2, ValueType>;
		using Vec3 = Vec<3, ValueType>;
		using Vec4 = Vec<4, ValueType>;

		using IVec2 = Vec<2, I32>;
		using IVec3 = Vec<3, I32>;
		using IVec4 = Vec<4, I32>;
	}

}