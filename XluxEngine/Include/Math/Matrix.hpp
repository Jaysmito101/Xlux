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

			XLUX_FORCE_INLINE Mat(
				F32 m00, F32 m01, F32 m02, F32 m03,
				F32 m10, F32 m11, F32 m12, F32 m13,
				F32 m20, F32 m21, F32 m22, F32 m23,
				F32 m30, F32 m31, F32 m32, F32 m33) requires (M == 4 && N == 4)
			{
				this->At(0, 0) = m00; this->At(0, 1) = m01; this->At(0, 2) = m02; this->At(0, 3) = m03;
				this->At(1, 0) = m10; this->At(1, 1) = m11; this->At(1, 2) = m12; this->At(1, 3) = m13;
				this->At(2, 0) = m20; this->At(2, 1) = m21; this->At(2, 2) = m22; this->At(2, 3) = m23;
				this->At(3, 0) = m30; this->At(3, 1) = m31; this->At(3, 2) = m32; this->At(3, 3) = m33;
			}

			XLUX_FORCE_INLINE Mat(
				F32 m00, F32 m01, F32 m02,
				F32 m10, F32 m11, F32 m12,
				F32 m20, F32 m21, F32 m22) requires (M == 3 && N == 3)
			{
				this->At(0, 0) = m00; this->At(0, 1) = m01; this->At(0, 2) = m02;
				this->At(1, 0) = m10; this->At(1, 1) = m11; this->At(1, 2) = m12;
				this->At(2, 0) = m20; this->At(2, 1) = m21; this->At(2, 2) = m22;
			}

			XLUX_FORCE_INLINE Mat(
				const Vec<4, ValueType>& v0,
				const Vec<4, ValueType>& v1,
				const Vec<4, ValueType>& v2,
				const Vec<4, ValueType>& v3) requires (M == 4 && N == 4)
			{
				this->At(0, 0) = v0[0]; this->At(0, 1) = v0[1]; this->At(0, 2) = v0[2]; this->At(0, 3) = v0[3];
				this->At(1, 0) = v1[0]; this->At(1, 1) = v1[1]; this->At(1, 2) = v1[2]; this->At(1, 3) = v1[3];
				this->At(2, 0) = v2[0]; this->At(2, 1) = v2[1]; this->At(2, 2) = v2[2]; this->At(2, 3) = v2[3];
				this->At(3, 0) = v3[0]; this->At(3, 1) = v3[1]; this->At(3, 2) = v3[2]; this->At(3, 3) = v3[3];
			}

			XLUX_FORCE_INLINE Mat(
				const Vec<3, ValueType>& v0,
				const Vec<3, ValueType>& v1,
				const Vec<3, ValueType>& v2,
				const Vec<3, ValueType>& v3) requires (M == 4 && N == 4)
			{
				this->At(0, 0) = v0[0]; this->At(0, 1) = v0[1]; this->At(0, 2) = v0[2]; this->At(0, 3) = 0.0f;
				this->At(1, 0) = v1[0]; this->At(1, 1) = v1[1]; this->At(1, 2) = v1[2]; this->At(1, 3) = 0.0f;
				this->At(2, 0) = v2[0]; this->At(2, 1) = v2[1]; this->At(2, 2) = v2[2]; this->At(2, 3) = 0.0f;
				this->At(3, 0) = v3[0]; this->At(3, 1) = v3[1]; this->At(3, 2) = v3[2]; this->At(3, 3) = 1.0f;
			}

			XLUX_FORCE_INLINE auto& operator=(const Mat<M, N, ValueTypeT>& other)
			{
				std::memcpy(m_Data, other.m_Data, sizeof(m_Data));
				return *this;
			}

			XLUX_FORCE_INLINE auto& operator+=(const Mat<M, N, ValueTypeT>& other)
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

			XLUX_FORCE_INLINE auto& operator-=(const Mat<M, N, ValueTypeT>& other)
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

			XLUX_FORCE_INLINE auto& operator*=(const ValueTypeT& value)
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

			XLUX_FORCE_INLINE auto& operator/=(const ValueTypeT& value)
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

			XLUX_FORCE_INLINE auto operator+(const Mat<M, N, ValueTypeT>& other) const
			{
				Mat<M, N, ValueTypeT> result = *this;
				result += other;
				return result;
			}

			XLUX_FORCE_INLINE auto operator-(const Mat<M, N, ValueTypeT>& other) const
			{
				Mat<M, N, ValueTypeT> result = *this;
				result -= other;
				return result;
			}

			XLUX_FORCE_INLINE auto operator*(const ValueTypeT& value) const
			{
				Mat<M, N, ValueTypeT> result = *this;
				result *= value;
				return result;
			}

			XLUX_FORCE_INLINE auto operator/(const ValueTypeT& value) const
			{
				Mat<M, N, ValueTypeT> result = *this;
				result /= value;
				return result;
			}

			XLUX_FORCE_INLINE auto Transpose() const
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

			XLUX_FORCE_INLINE auto FastInverse() const requires (M == 4 && N == 4)
			{
				const auto& A = *this; // Use A as a short-hand for this->

				// This implementation is based on the Cramer's rule for 4x4 matrices,
				// which computes the adjugate matrix and divides by the determinant.
				// It's structured to reuse calculations for efficiency.

				// Calculate cofactors of 2x2 submatrices for the first two rows
				auto s0 = A.At(0, 0) * A.At(1, 1) - A.At(1, 0) * A.At(0, 1);
				auto s1 = A.At(0, 0) * A.At(1, 2) - A.At(1, 0) * A.At(0, 2);
				auto s2 = A.At(0, 0) * A.At(1, 3) - A.At(1, 0) * A.At(0, 3);
				auto s3 = A.At(0, 1) * A.At(1, 2) - A.At(1, 1) * A.At(0, 2);
				auto s4 = A.At(0, 1) * A.At(1, 3) - A.At(1, 1) * A.At(0, 3);
				auto s5 = A.At(0, 2) * A.At(1, 3) - A.At(1, 2) * A.At(0, 3);

				// Calculate cofactors of 2x2 submatrices for the last two rows
				auto c5 = A.At(2, 2) * A.At(3, 3) - A.At(3, 2) * A.At(2, 3);
				auto c4 = A.At(2, 1) * A.At(3, 3) - A.At(3, 1) * A.At(2, 3);
				auto c3 = A.At(2, 1) * A.At(3, 2) - A.At(3, 1) * A.At(2, 2);
				auto c2 = A.At(2, 0) * A.At(3, 3) - A.At(3, 0) * A.At(2, 3);
				auto c1 = A.At(2, 0) * A.At(3, 2) - A.At(3, 0) * A.At(2, 2);
				auto c0 = A.At(2, 0) * A.At(3, 1) - A.At(3, 0) * A.At(2, 1);

				// Calculate determinant using cofactor expansion
				auto det = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;

				// Check if the matrix is singular (non-invertible)
				constexpr auto epsilon = 1e-100;
				if (std::abs(det) < epsilon)
				{
					xlux::log::Warn("Matrix is singular, cannot be inverted. Returning identity matrix.");
					return Mat<M, N, ValueTypeT>::Identity();
				}

				auto invDet = ValueTypeT(1) / det;
				Mat<M, N, ValueTypeT> inv;

				inv.At(0, 0) = (A.At(1, 1) * c5 - A.At(1, 2) * c4 + A.At(1, 3) * c3) * invDet;
				inv.At(0, 1) = (-A.At(0, 1) * c5 + A.At(0, 2) * c4 - A.At(0, 3) * c3) * invDet;
				inv.At(0, 2) = (A.At(3, 1) * s5 - A.At(3, 2) * s4 + A.At(3, 3) * s3) * invDet;
				inv.At(0, 3) = (-A.At(2, 1) * s5 + A.At(2, 2) * s4 - A.At(2, 3) * s3) * invDet;

				inv.At(1, 0) = (-A.At(1, 0) * c5 + A.At(1, 2) * c2 - A.At(1, 3) * c1) * invDet;
				inv.At(1, 1) = (A.At(0, 0) * c5 - A.At(0, 2) * c2 + A.At(0, 3) * c1) * invDet;
				inv.At(1, 2) = (-A.At(3, 0) * s5 + A.At(3, 2) * s2 - A.At(3, 3) * s1) * invDet;
				inv.At(1, 3) = (A.At(2, 0) * s5 - A.At(2, 2) * s2 + A.At(2, 3) * s1) * invDet;

				inv.At(2, 0) = (A.At(1, 0) * c4 - A.At(1, 1) * c2 + A.At(1, 3) * c0) * invDet;
				inv.At(2, 1) = (-A.At(0, 0) * c4 + A.At(0, 1) * c2 - A.At(0, 3) * c0) * invDet;
				inv.At(2, 2) = (A.At(3, 0) * s4 - A.At(3, 1) * s2 + A.At(3, 3) * s0) * invDet;
				inv.At(2, 3) = (-A.At(2, 0) * s4 + A.At(2, 1) * s2 - A.At(2, 3) * s0) * invDet;

				inv.At(3, 0) = (-A.At(1, 0) * c3 + A.At(1, 1) * c1 - A.At(1, 2) * c0) * invDet;
				inv.At(3, 1) = (A.At(0, 0) * c3 - A.At(0, 1) * c1 + A.At(0, 2) * c0) * invDet;
				inv.At(3, 2) = (-A.At(3, 0) * s3 + A.At(3, 1) * s1 - A.At(3, 2) * s0) * invDet;
				inv.At(3, 3) = (A.At(2, 0) * s3 - A.At(2, 1) * s1 + A.At(2, 2) * s0) * invDet;

				return inv;
			}

			XLUX_FORCE_INLINE void ResetTranslation() requires (M == 4 && N == 4)
			{
				this->At(0, 3) = 0.0f;
				this->At(1, 3) = 0.0f;
				this->At(2, 3) = 0.0f;
			}


			XLUX_FORCE_INLINE auto Mul(const Mat<M, N, ValueType>& other) const requires (M == N && M == 4 && N == 4)
			{
				Mat<M, N, ValueTypeT> result;
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

			XLUX_FORCE_INLINE auto Mul(const Vec<4, ValueType>& v) const requires (M == 4 && N == 4)
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

			static auto Identity() requires (M == N)
			{
				Mat<M, N, ValueTypeT> result;
				for (Size i = 0; i < M; ++i)
				{
					result.At(i, i) = ValueTypeT(1);
				}
				return result;
			}

			static auto Translate(const Vec<3, ValueType>& v) requires (M == 4 && N == 4)
			{
				return Mat<M, N, ValueTypeT>(
					1.0f, 0.0f, 0.0f, v[0],
					0.0f, 1.0f, 0.0f, v[1],
					0.0f, 0.0f, 1.0f, v[2],
					0.0f, 0.0f, 0.0f, 1.0f);
			}

			static auto Scale(const Vec<3, ValueType>& v) requires (M == 4 && N == 4)
			{
				return Mat<M, N, ValueTypeT>(
					v[0], 0.0f, 0.0f, 0.0f,
					0.0f, v[1], 0.0f, 0.0f,
					0.0f, 0.0f, v[2], 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f);
			}

			static auto RotateX(F32 angle) requires (M == 4 && N == 4)
			{
				F32 c = std::cos(angle);
				F32 s = std::sin(angle);
				return Mat<M, N, ValueTypeT>(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, c, -s, 0.0f,
					0.0f, s, c, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f);
			}

			static auto RotateY(F32 angle) requires (M == 4 && N == 4)
			{
				F32 c = std::cos(angle);
				F32 s = std::sin(angle);
				return Mat<M, N, ValueTypeT>(
					c, 0.0f, s, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					-s, 0.0f, c, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f);
			}

			static auto RotateZ(F32 angle) requires (M == 4 && N == 4)
			{
				F32 c = std::cos(angle);
				F32 s = std::sin(angle);
				return Mat<M, N, ValueTypeT>(
					c, -s, 0.0f, 0.0f,
					s, c, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f);
			}

			static auto LookAt(const Vec<3, ValueType>& eye, const Vec<3, ValueType>& center, const Vec<3, ValueType>& up) requires (M == 4 && N == 4)
			{
				Vec<3, ValueType> z = (eye - center).Normalized();
				Vec<3, ValueType> x = up.Cross(z).Normalized();
				Vec<3, ValueType> y = z.Cross(x).Normalized();

				return Mat<M, N, ValueTypeT>(
					x[0], x[1], x[2], -x.Dot(eye),
					y[0], y[1], y[2], -y.Dot(eye),
					z[0], z[1], z[2], -z.Dot(eye),
					0.0f, 0.0f, 0.0f, 1.0f);
			}

			static auto Perspective(F32 fov, F32 aspect, F32 zNear, F32 zFar) requires (M == 4 && N == 4)
			{
				F32 tanHalfFov = std::tan(fov / 2.0f);
				F32 zRange = zNear - zFar;
				return Mat<M, N, ValueTypeT>(
					1.0f / (aspect * tanHalfFov), 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f / tanHalfFov, 0.0f, 0.0f,
					0.0f, 0.0f, -(-zNear - zFar) / zRange, 2.0f * zFar * zNear / zRange,
					0.0f, 0.0f, -1.0f, 0.0f);
			}

			static auto Orthographic(F32 left, F32 right, F32 bottom, F32 top, F32 zNear, F32 zFar) requires (M == 4 && N == 4)
			{
				F32 width = right - left;
				F32 height = top - bottom;
				F32 depth = zFar - zNear;

				return Mat<M, N, ValueTypeT>(
					2.0f / width, 0.0f, 0.0f, -(right + left) / width,
					0.0f, 2.0f / height, 0.0f, -(top + bottom) / height,
					0.0f, 0.0f, -2.0f / depth, -(zFar + zNear) / depth,
					0.0f, 0.0f, 0.0f, 1.0f);
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

		using Mat4x4 = Mat<4, 4>;
		
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