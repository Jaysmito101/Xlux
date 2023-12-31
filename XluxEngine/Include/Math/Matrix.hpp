#pragma once

#include "Core/Types.hpp"
#include "Core/Logger.hpp"
#include "Math/Vector.hpp"

namespace xlux
{

	namespace math_normal
	{
		template <Size M, Size N, typename ValueTypeT = ValueType>
		class Mat
		{
		public:

			XLUX_FORCE_INLINE Mat()
			{
				for (Size i = 0; i < M; ++i)
				{
					for (Size j = 0; j < N; ++j)
					{
						At(i, j) = ValueTypeT(0);
					}
				}
			}


			XLUX_FORCE_INLINE Mat(const ValueTypeT& value)
			{
				for (Size i = 0; i < M; ++i)
				{
					for (Size j = 0; j < N; ++j)
					{
						At(i, j) = value;
					}
				}
			}

			XLUX_FORCE_INLINE Mat(const ValueTypeT* data)
			{
				std::memcpy(m_Data, data, sizeof(m_Data));
			}

			XLUX_FORCE_INLINE Mat(const Mat& other)
			{
				std::memcpy(m_Data, other.m_Data, sizeof(m_Data));
			}

			XLUX_FORCE_INLINE Mat<M, N, ValueTypeT>& operator=(const Mat<M, N, ValueTypeT>& other)
			{
				std::memcpy(m_Data, other.m_Data, sizeof(m_Data));
				return *this;
			}

			XLUX_FORCE_INLINE Mat<M, N, ValueTypeT>& operator+=(const Mat<M, N, ValueTypeT>& other)
			{
				for (Size i = 0; i < M; ++i)
				{
					for (Size j = 0; j < N; ++j)
					{
						At(i, j) += other.At(i, j);
					}
				}
				return *this;
			}

			XLUX_FORCE_INLINE Mat<M, N, ValueTypeT>& operator-=(const Mat<M, N, ValueTypeT>& other)
			{
				for (Size i = 0; i < M; ++i)
				{
					for (Size j = 0; j < N; ++j)
					{
						At(i, j) -= other.At(i, j);
					}
				}
				return *this;
			}

			XLUX_FORCE_INLINE Mat<M, N, ValueTypeT>& operator*=(const ValueTypeT& value)
			{
				for (Size i = 0; i < M; ++i)
				{
					for (Size j = 0; j < N; ++j)
					{
						m_Data[i][j] *= value;
					}
				}
				return *this;
			}

			XLUX_FORCE_INLINE Mat<M, N, ValueTypeT>& operator/=(const ValueTypeT& value)
			{
				for (Size i = 0; i < M; ++i)
				{
					for (Size j = 0; j < N; ++j)
					{
						At(i, j) /= value;
					}
				}
				return *this;
			}

			XLUX_FORCE_INLINE Mat<M, N, ValueTypeT> operator+(const Mat<M, N, ValueTypeT>& other) const
			{
				Mat<M, N, ValueTypeT> result = *this;
				result += other;
				return result;
			}

			XLUX_FORCE_INLINE Mat<M, N, ValueTypeT> operator-(const Mat<M, N, ValueTypeT>& other) const
			{
				Mat<M, N, ValueTypeT> result = *this;
				result -= other;
				return result;
			}

			XLUX_FORCE_INLINE Mat<M, N, ValueTypeT> operator*(const ValueTypeT& value) const
			{
				Mat<M, N, ValueTypeT> result = *this;
				result *= value;
				return result;
			}

			XLUX_FORCE_INLINE Mat<M, N, ValueTypeT> operator/(const ValueTypeT& value) const
			{
				Mat<M, N, ValueTypeT> result = *this;
				result /= value;
				return result;
			}

			XLUX_FORCE_INLINE Mat<N, M, ValueTypeT> Transpose() const
			{
				Mat<N, M, ValueTypeT> result;
				for (Size i = 0; i < M; ++i)
				{
					for (Size j = 0; j < N; ++j)
					{
						result.At(j, i) = At(i, j);
					}
				}
				return result;
			}

			inline const ValueTypeT& At(Size i, Size j) const
			{
				return m_Data[i * N + j];
			}

			inline ValueTypeT& At(Size i, Size j)
			{
				return m_Data[i * N + j];
			}

			inline const ValueTypeT& operator()(Size i, Size j) const
			{
				return At(i, j);
			}

			inline ValueTypeT& operator()(Size i, Size j)
			{
				return At(i, j);
			}

			inline const ValueTypeT* operator[](Size i) const
			{
				return m_Data + i * N;
			}

			inline ValueTypeT* operator[](Size i)
			{
				return m_Data + i * N;
			}

		protected:
			ValueTypeT m_Data[M * N];
		};

		class Mat4x4 : public Mat<4, 4, ValueType>
		{
		public:

			XLUX_FORCE_INLINE Mat4x4()
			{
				std::memset(m_Data, 0, sizeof(m_Data));
				m_Data[0] = m_Data[5] = m_Data[10] = m_Data[15] = 1.0f;
			}

			XLUX_FORCE_INLINE Mat4x4(
				F32 m00, F32 m01, F32 m02, F32 m03,
				F32 m10, F32 m11, F32 m12, F32 m13,
				F32 m20, F32 m21, F32 m22, F32 m23,
				F32 m30, F32 m31, F32 m32, F32 m33)
			{
				this->At(0, 0) = m00; this->At(0, 1) = m01; this->At(0, 2) = m02; this->At(0, 3) = m03;
				this->At(1, 0) = m10; this->At(1, 1) = m11; this->At(1, 2) = m12; this->At(1, 3) = m13;
				this->At(2, 0) = m20; this->At(2, 1) = m21; this->At(2, 2) = m22; this->At(2, 3) = m23;
				this->At(3, 0) = m30; this->At(3, 1) = m31; this->At(3, 2) = m32; this->At(3, 3) = m33;
			}

			XLUX_FORCE_INLINE Mat4x4(const Vec<4, ValueType>& v0, const Vec<4, ValueType>& v1, const Vec<4, ValueType>& v2, const Vec<4, ValueType>& v3)
			{
				this->At(0, 0) = v0[0]; this->At(0, 1) = v0[1]; this->At(0, 2) = v0[2]; this->At(0, 3) = v0[3];
				this->At(1, 0) = v1[0]; this->At(1, 1) = v1[1]; this->At(1, 2) = v1[2]; this->At(1, 3) = v1[3];
				this->At(2, 0) = v2[0]; this->At(2, 1) = v2[1]; this->At(2, 2) = v2[2]; this->At(2, 3) = v2[3];
				this->At(3, 0) = v3[0]; this->At(3, 1) = v3[1]; this->At(3, 2) = v3[2]; this->At(3, 3) = v3[3];
			}

			XLUX_FORCE_INLINE Mat4x4 Mul(const Mat<4, 4, ValueType>& other) const
			{
				Mat4x4 result;
				for (Size i = 0; i < 4; ++i)
				{
					for (Size j = 0; j < 4; ++j)
					{
						result.At(i, j) = this->At(i, 0) * other.At(0, j) +
							this->At(i, 1) * other.At(1, j) +
							this->At(i, 2) * other.At(2, j) +
							this->At(i, 3) * other.At(3, j);
					}
				}
				return result;
			}

			XLUX_FORCE_INLINE Vec<4, ValueType> Mul(const Vec<4, ValueType>& v) const
			{
				Vec<4, ValueType> result;
				for (Size i = 0; i < 4; ++i)
				{
					result[i] = this->At(i, 0) * v[0] +
						this->At(i, 1) * v[1] +
						this->At(i, 2) * v[2] +
						this->At(i, 3) * v[3];
				}
				return result;
			}

			static Mat4x4 Identity()
			{
				return Mat4x4(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f);
			}

			static Mat4x4 Translate(const Vec<3, ValueType>& v)
			{
				return Mat4x4(
					1.0f, 0.0f, 0.0f, v[0],
					0.0f, 1.0f, 0.0f, v[1],
					0.0f, 0.0f, 1.0f, v[2],
					0.0f, 0.0f, 0.0f, 1.0f);
			}

			static Mat4x4 Scale(const Vec<3, ValueType>& v)
			{
				return Mat4x4(
					v[0], 0.0f, 0.0f, 0.0f,
					0.0f, v[1], 0.0f, 0.0f,
					0.0f, 0.0f, v[2], 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f);
			}

			static Mat4x4 RotateX(F32 angle)
			{
				F32 c = std::cos(angle);
				F32 s = std::sin(angle);
				return Mat4x4(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, c, -s, 0.0f,
					0.0f, s, c, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f);
			}

			static Mat4x4 RotateY(F32 angle)
			{
				F32 c = std::cos(angle);
				F32 s = std::sin(angle);
				return Mat4x4(
					c, 0.0f, s, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					-s, 0.0f, c, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f);
			}

			static Mat4x4 RotateZ(F32 angle)
			{
				F32 c = std::cos(angle);
				F32 s = std::sin(angle);
				return Mat4x4(
					c, -s, 0.0f, 0.0f,
					s, c, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f);
			}

			static Mat4x4 LookAt(const Vec<3, ValueType>& eye, const Vec<3, ValueType>& center, const Vec<3, ValueType>& up)
			{
				Vec<3, ValueType> f = (center - eye).Normalized();
				Vec<3, ValueType> s = f.Cross(up).Normalized();
				Vec<3, ValueType> u = s.Cross(f);

				return Mat4x4(
					s[0], s[1], s[2], -s.Dot(eye),
					u[0], u[1], u[2], -u.Dot(eye),
					-f[0], -f[1], -f[2], f.Dot(eye),
					0.0f, 0.0f, 0.0f, 1.0f);
			}

			static Mat4x4 Perspective(F32 fov, F32 aspect, F32 zNear, F32 zFar)
			{
				F32 tanHalfFov = std::tan(fov / 2.0f);
				F32 zRange = zNear - zFar;
				return Mat4x4(
					1.0f / (aspect * tanHalfFov), 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f / tanHalfFov, 0.0f, 0.0f,
					0.0f, 0.0f, (-zNear - zFar) / zRange, 2.0f * zFar * zNear / zRange,
					0.0f, 0.0f, 1.0f, 0.0f);
			}

			static Mat4x4 Orthographics(F32 left, F32 right, F32 bottom, F32 top, F32 zNear, F32 zFar)
			{
				F32 width = right - left;
				F32 height = top - bottom;
				F32 depth = zFar - zNear;

				return Mat4x4(
					2.0f / width, 0.0f, 0.0f, -(right + left) / width,
					0.0f, 2.0f / height, 0.0f, -(top + bottom) / height,
					0.0f, 0.0f, -2.0f / depth, -(zFar + zNear) / depth,
					0.0f, 0.0f, 0.0f, 1.0f);
			}

		};

		
		template<Size M, Size N, typename ValueTypeT>
		XLUX_FORCE_INLINE std::ostream& operator<<(std::ostream& os, const Mat<M, N, ValueTypeT>& m)
		{
			os << "Mat" << M << "x" << N << ": {\n";
			for (Size i = 0; i < M; ++i)
			{
				os << "  ";
				for (Size j = 0; j < N; ++j)
				{
					os << m.At(i, j) << " ";
				}
				os << std::endl;
			}
			os << "}";
			return os;
		}


	}

}