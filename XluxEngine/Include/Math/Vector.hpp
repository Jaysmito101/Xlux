#pragma once

#include "Core/Types.hpp"
#include "Core/Logger.hpp"


namespace xlux
{

	namespace math_normal
	{

		template <typename T>
		concept VectorValueType = std::is_arithmetic_v<T>;

		template <Size N, typename ValueTypeT = ValueType>
		class Vec
		{
		public:
			XLUX_FORCE_INLINE Vec(const ValueTypeT& value)
			{
				for (Size i = 0; i < N; ++i)
				{
					m_Data[i] = value;
				}
			}

			XLUX_FORCE_INLINE Vec(const ValueTypeT* data)
			{
				std::memcpy(m_Data, data, sizeof(m_Data));
			}

			/*template <Size M, VectorValueType... Args>
			static XLUX_FORCE_INLINE Vec<N, ValueTypeT> From(const Vec<M>& other, Args... args)
			{
				auto result = Vec<N, ValueTypeT>();
				if constexpr ((M + sizeof...(args)) < N) {
					throw std::logic_error("Invalid number of arguments");
				}
				else
				{
					if constexpr (sizeof...(args) == 0)
					{
						std::memcpy(result.m_Data, other.m_Data, sizeof(other.m_Data));
					}
					else
					{
						std::memcpy(result.m_Data, other.m_Data, std::clamp(M, Size(0), N) * sizeof(ValueTypeT));
						ValueTypeT data[] = { args... };
						std::memcpy(result.m_Data + M, data, sizeof(data));
					}
				}
				return result;
			}*/

			template <Size M, VectorValueType... Args>
			XLUX_FORCE_INLINE Vec(const Vec<M>& other, Args... args)
			{
				//if constexpr ((M + sizeof...(args)) < N) {
					//throw std::logic_error("Invalid number of arguments");
				//}
				//else
				{
					if constexpr (sizeof...(args) == 0)
					{
						std::memcpy(m_Data, other.m_Data, std::clamp(M, Size(0), N) * sizeof(ValueTypeT));
					}
					else if constexpr ((M + sizeof...(args)) >= N)
					{
						std::memcpy(m_Data, other.m_Data, std::clamp(M, Size(0), N) * sizeof(ValueTypeT));
						ValueTypeT data[] = { args... };
						std::memcpy(m_Data + M, data, sizeof(data));
					}
					else
					{
						std::memcpy(m_Data, other.m_Data, std::clamp(M, Size(0), N) * sizeof(ValueTypeT));
						ValueTypeT data[] = { args... };
						std::memcpy(m_Data + M, data, sizeof(data));

						for (Size i = M + sizeof...(args); i < N; ++i)
						{
							m_Data[i] = ValueTypeT();
						}
					}
				}
			}

			template <VectorValueType... Args>
			XLUX_FORCE_INLINE Vec(Args... args)
			{
				static_assert(sizeof...(args) <= N, "Invalid number of arguments");
				if constexpr (sizeof...(args) == 0)
				{
					for (Size i = 0; i < N; ++i)
					{
						m_Data[i] = ValueTypeT();
					}
				}
				else
				{
					// static_cast all args to ValueTypeT
					ValueTypeT data[] = { static_cast<ValueTypeT>(args)... };
					std::memcpy(m_Data, data, sizeof(data));
				}
			}



			/*XLUX_FORCE_INLINE Vec(Vec<N, ValueTypeT>&& other)
			{
				std::memcpy(m_Data, other.m_Data, sizeof(m_Data));
			}*/

			XLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator=(const Vec<N, ValueTypeT>& other)
			{
				std::memcpy(m_Data, other.m_Data, sizeof(m_Data));
				return *this;
			}

			/*XLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator=(Vec<N, ValueTypeT>&& other)
			{
				std::memcpy(m_Data, other.m_Data, sizeof(m_Data));
				return *this;
			}*/

			XLUX_FORCE_INLINE ValueTypeT& operator[](Size index)
			{
				return m_Data[index];
			}

			XLUX_FORCE_INLINE const ValueTypeT& operator[](Size index) const
			{
				return m_Data[index];
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT> operator+(const Vec<N, ValueTypeT>& other) const
			{
				Vec<N, ValueTypeT> result;
				for (Size i = 0; i < N; ++i)
				{
					result[i] = m_Data[i] + other[i];
				}
				return result;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT> operator-(const Vec<N, ValueTypeT>& other) const
			{
				Vec<N, ValueTypeT> result;
				for (Size i = 0; i < N; ++i)
				{
					result[i] = m_Data[i] - other[i];
				}
				return result;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT> operator*(const Vec<N, ValueTypeT>& other) const
			{
				Vec<N, ValueTypeT> result;
				for (Size i = 0; i < N; ++i)
				{
					result[i] = m_Data[i] * other[i];
				}
				return result;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT> operator/(const Vec<N, ValueTypeT>& other) const
			{
				Vec<N, ValueTypeT> result;
				for (Size i = 0; i < N; ++i)
				{
					result[i] = m_Data[i] / other[i];
				}
				return result;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT> operator*(const ValueTypeT& value) const
			{
				Vec<N, ValueTypeT> result;
				for (Size i = 0; i < N; ++i)
				{
					result[i] = m_Data[i] * value;
				}
				return result;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT> operator/(const ValueTypeT& value) const
			{
				Vec<N, ValueTypeT> result;
				for (Size i = 0; i < N; ++i)
				{
					result[i] = m_Data[i] / value;
				}
				return result;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator+=(const Vec<N, ValueTypeT>& other)
			{
				for (Size i = 0; i < N; ++i)
				{
					m_Data[i] += other[i];
				}
				return *this;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator-=(const Vec<N, ValueTypeT>& other)
			{
				for (Size i = 0; i < N; ++i)
				{
					m_Data[i] -= other[i];
				}
				return *this;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator*=(const Vec<N, ValueTypeT>& other)
			{
				for (Size i = 0; i < N; ++i)
				{
					m_Data[i] *= other[i];
				}
				return *this;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator/=(const Vec<N, ValueTypeT>& other)
			{
				for (Size i = 0; i < N; ++i)
				{
					m_Data[i] /= other[i];
				}
				return *this;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator*=(const ValueTypeT& value)
			{
				for (Size i = 0; i < N; ++i)
				{
					m_Data[i] *= value;
				}
				return *this;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator/=(const ValueTypeT& value)
			{
				for (Size i = 0; i < N; ++i)
				{
					m_Data[i] /= value;
				}
				return *this;
			}

			XLUX_FORCE_INLINE bool operator==(const Vec<N, ValueTypeT>& other) const
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

			XLUX_FORCE_INLINE Vec<N, ValueTypeT> operator-() const
			{
				Vec<N, ValueTypeT> result;
				for (Size i = 0; i < N; ++i)
				{
					result[i] = -m_Data[i];
				}
				return result;
			}


			XLUX_FORCE_INLINE bool operator!=(const Vec<N, ValueTypeT>& other) const
			{
				return !(*this == other);
			}

			XLUX_FORCE_INLINE ValueTypeT Length() const
			{
				ValueTypeT  result = 0;
				for (Size i = 0; i < N; ++i)
				{
					result += m_Data[i] * m_Data[i];
				}
				return static_cast<ValueTypeT>(std::sqrt(result));
			}

			XLUX_FORCE_INLINE ValueTypeT  LengthSquared() const
			{
				ValueTypeT  result = 0;
				for (Size i = 0; i < N; ++i)
				{
					result += m_Data[i] * m_Data[i];
				}
				return result;
			}

			XLUX_FORCE_INLINE ValueTypeT  Dot(const Vec<N, ValueTypeT>& other) const
			{
				ValueTypeT  result = 0;
				for (Size i = 0; i < N; ++i)
				{
					result += m_Data[i] * other[i];
				}
				return result;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT> Pow(F32 power) const
			{
				Vec<N, ValueTypeT> result;
				for (Size i = 0; i < N; ++i)
				{
					result[i] = std::pow(m_Data[i], power);
				}
				return result;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT> Cross(const Vec<N, ValueTypeT>& other) const
			{
				static_assert(N == 3 || N == 2, "Cross product is only defined for 3D vectors");
				if constexpr (N == 2)
				{
					F32 result = m_Data[0] * other[1] - m_Data[1] * other[0];
					return Vec<2, ValueTypeT>(result, 0.0f);
				}
				else if constexpr (N == 3)
				{
					return Vec<3, ValueTypeT>(
						m_Data[1] * other[2] - m_Data[2] * other[1],
						m_Data[2] * other[0] - m_Data[0] * other[2],
						m_Data[0] * other[1] - m_Data[1] * other[0]
					);
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
					m_Data[i] = m_Data[i + 1];
				}
				m_Data[N - 1] = value;
				return *this;
			}

			XLUX_FORCE_INLINE Vec<N, ValueTypeT>& operator>>(const ValueTypeT& value)
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