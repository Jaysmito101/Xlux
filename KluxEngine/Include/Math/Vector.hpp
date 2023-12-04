#pragma once

#include "Core/Core.hpp"

#include <immintrin.h>


namespace klux
{

	namespace math_normal
	{

		template <typename T>
		concept VectorValueType = std::is_arithmetic_v<T>;

		template <Size N, typename ValueTypeT = ValueType>
		class Vec
		{
		public:

			KLUX_FORCE_INLINE Vec()
			{
			}

			KLUX_FORCE_INLINE Vec(const ValueTypeT& value)
			{
				for (Size i = 0; i < N; ++i)
				{
					m_Data[i] = value;
				}
			}

			KLUX_FORCE_INLINE Vec(const ValueTypeT* data)
			{
				std::memcpy(m_Data, data, sizeof(m_Data));
			}

			template <Size M, VectorValueType... Args>
			KLUX_FORCE_INLINE Vec(const Vec<M>& other, Args... args)
			{
				if constexpr ((M + sizeof...(args)) < N) {
					throw std::logic_error("Invalid number of arguments");
				}
				else
				{
					if constexpr (sizeof...(args) == 0)
					{
						std::memcpy(m_Data, other.m_Data, sizeof(other.m_Data));
					}
					else
					{
						std::memcpy(m_Data, other.m_Data, std::clamp(M, Size(0), N) * sizeof(ValueTypeT));
						ValueTypeT data[] = { args... };
						std::memcpy(m_Data + M, data, sizeof(data));
					}
				}
			}

			template <VectorValueType... Args>
			KLUX_FORCE_INLINE Vec(Args... args)
			{
				static_assert(sizeof...(args) == N, "Invalid number of arguments");
				ValueTypeT data[] = { args... };
				std::memcpy(m_Data, data, sizeof(data));
			}



			KLUX_FORCE_INLINE Vec(Vec<N, ValueTypeT>&& other)
			{
				std::memcpy(m_Data, other.m_Data, sizeof(m_Data));
			}

			KLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator=(const Vec<N, ValueTypeT>& other)
			{
				std::memcpy(m_Data, other.m_Data, sizeof(m_Data));
				return *this;
			}

			KLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator=(Vec<N, ValueTypeT>&& other)
			{
				std::memcpy(m_Data, other.m_Data, sizeof(m_Data));
				return *this;
			}

			KLUX_FORCE_INLINE ValueTypeT& operator[](Size index)
			{
				return m_Data[index];
			}

			KLUX_FORCE_INLINE const ValueTypeT& operator[](Size index) const
			{
				return m_Data[index];
			}

			KLUX_FORCE_INLINE Vec<N, ValueTypeT> operator+(const Vec<N, ValueTypeT>& other) const
			{
				Vec<N, ValueTypeT> result;
				for (Size i = 0; i < N; ++i)
				{
					result[i] = m_Data[i] + other[i];
				}
				return result;
			}

			KLUX_FORCE_INLINE Vec<N, ValueTypeT> operator-(const Vec<N, ValueTypeT>& other) const
			{
				Vec<N, ValueTypeT> result;
				for (Size i = 0; i < N; ++i)
				{
					result[i] = m_Data[i] - other[i];
				}
				return result;
			}

			KLUX_FORCE_INLINE Vec<N, ValueTypeT> operator*(const Vec<N, ValueTypeT>& other) const
			{
				Vec<N, ValueTypeT> result;
				for (Size i = 0; i < N; ++i)
				{
					result[i] = m_Data[i] * other[i];
				}
				return result;
			}

			KLUX_FORCE_INLINE Vec<N, ValueTypeT> operator/(const Vec<N, ValueTypeT>& other) const
			{
				Vec<N, ValueTypeT> result;
				for (Size i = 0; i < N; ++i)
				{
					result[i] = m_Data[i] / other[i];
				}
				return result;
			}

			KLUX_FORCE_INLINE Vec<N, ValueTypeT> operator*(const ValueTypeT& value) const
			{
				Vec<N, ValueTypeT> result;
				for (Size i = 0; i < N; ++i)
				{
					result[i] = m_Data[i] * value;
				}
				return result;
			}

			KLUX_FORCE_INLINE Vec<N, ValueTypeT> operator/(const ValueTypeT& value) const
			{
				Vec<N, ValueTypeT> result;
				for (Size i = 0; i < N; ++i)
				{
					result[i] = m_Data[i] / value;
				}
				return result;
			}

			KLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator+=(const Vec<N, ValueTypeT>& other)
			{
				for (Size i = 0; i < N; ++i)
				{
					m_Data[i] += other[i];
				}
				return *this;
			}

			KLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator-=(const Vec<N, ValueTypeT>& other)
			{
				for (Size i = 0; i < N; ++i)
				{
					m_Data[i] -= other[i];
				}
				return *this;
			}

			KLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator*=(const Vec<N, ValueTypeT>& other)
			{
				for (Size i = 0; i < N; ++i)
				{
					m_Data[i] *= other[i];
				}
				return *this;
			}

			KLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator/=(const Vec<N, ValueTypeT>& other)
			{
				for (Size i = 0; i < N; ++i)
				{
					m_Data[i] /= other[i];
				}
				return *this;
			}

			KLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator*=(const ValueTypeT& value)
			{
				for (Size i = 0; i < N; ++i)
				{
					m_Data[i] *= value;
				}
				return *this;
			}

			KLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator/=(const ValueTypeT& value)
			{
				for (Size i = 0; i < N; ++i)
				{
					m_Data[i] /= value;
				}
				return *this;
			}

			KLUX_FORCE_INLINE bool operator==(const Vec<N, ValueTypeT>& other) const
			{
				for (Size i = 0; i < N; ++i)
				{
					if ((m_Data[i] - other[i]) > std::numeric_limits<ValueTypeT>::epsilon())
					{
						return false;
					}
				}
				return true;
			}

			KLUX_FORCE_INLINE bool operator!=(const Vec<N, ValueTypeT>& other) const
			{
				return !(*this == other);
			}

			KLUX_FORCE_INLINE ValueTypeT Length() const
			{
				ValueTypeT  result = 0;
				for (Size i = 0; i < N; ++i)
				{
					result += m_Data[i] * m_Data[i];
				}
				return static_cast<ValueTypeT>(std::sqrt(result));
			}

			KLUX_FORCE_INLINE ValueTypeT  LengthSquared() const
			{
				ValueTypeT  result = 0;
				for (Size i = 0; i < N; ++i)
				{
					result += m_Data[i] * m_Data[i];
				}
				return result;
			}

			KLUX_FORCE_INLINE ValueTypeT  Dot(const Vec<N, ValueTypeT>& other) const
			{
				ValueTypeT  result = 0;
				for (Size i = 0; i < N; ++i)
				{
					result += m_Data[i] * other[i];
				}
				return result;
			}

			KLUX_FORCE_INLINE Vec<N, ValueTypeT> Cross(const Vec<N, ValueTypeT>& other) const
			{
				static_assert(N == 3, "Cross product is only defined for 3D vectors");
				return Vec<N, ValueTypeT>(
					m_Data[1] * other[2] - m_Data[2] * other[1],
					m_Data[2] * other[0] - m_Data[0] * other[2],
					m_Data[0] * other[1] - m_Data[1] * other[0]
				);
			}

			KLUX_FORCE_INLINE Vec<N, ValueTypeT> Normalized() const
			{
				return *this / Length();
			}

			KLUX_FORCE_INLINE void Normalize()
			{
				*this /= Length();
			}

			KLUX_FORCE_INLINE ValueTypeT* RawData()
			{
				return m_Data;
			}

			KLUX_FORCE_INLINE const ValueTypeT* RawData() const
			{
				return m_Data;
			}

			KLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator<<(const ValueTypeT& value)
			{
				for (Size i = 0; i < N - 1; ++i)
				{
					m_Data[i] = m_Data[i + 1];
				}
				m_Data[N - 1] = value;
				return *this;
			}

			KLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator>>(const ValueTypeT& value)
			{
				for (Size i = N - 1; i > 0; --i)
				{
					m_Data[i] = m_Data[i - 1];
				}
				m_Data[0] = value;
				return *this;
			}

			template <Size M, typename ValueTypeT2>
			friend std::ostream& operator<<(std::ostream& os, const Vec<M, ValueTypeT2>& vec);

			template <Size M, typename ValueTypeT2>
			friend class Vec;


		private:
			ValueTypeT m_Data[N];
		};

		template <Size N, typename ValueTypeT>
		KLUX_FORCE_INLINE std::ostream& operator<<(std::ostream& os, const Vec<N, ValueTypeT>& vec)
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