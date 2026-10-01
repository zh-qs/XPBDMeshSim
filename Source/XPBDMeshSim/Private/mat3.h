// inspired by https://gist.github.com/mattatz/1f3234e3f978706b46eb32493b3f9fa9
#pragma once

struct mat3 {
    float m[3][3];

    inline mat3() {
        m[0][0] = 1.0; m[1][0] = 0.0; m[2][0] = 0.0; 
        m[0][1] = 0.0; m[1][1] = 1.0; m[2][1] = 0.0; 
        m[0][2] = 0.0; m[1][2] = 0.0; m[2][2] = 1.0; 
    }

    inline mat3(
        const float m11, const float m12, const float m13,
        const float m21, const float m22, const float m23,
        const float m31, const float m32, const float m33
    ) {
        m[0][0] = m11; m[1][0] = m12; m[2][0] = m13; 
        m[0][1] = m21; m[1][1] = m22; m[2][1] = m23; 
        m[0][2] = m31; m[1][2] = m32; m[2][2] = m33; 
    }

    inline mat3(
        const FVector3f& m1, const FVector3f& m2, const FVector3f& m3
    ) {
        m[0][0] = m1[0]; m[1][0] = m2[0]; m[2][0] = m3[0];
        m[0][1] = m1[1]; m[1][1] = m2[1]; m[2][1] = m3[1];
        m[0][2] = m1[2]; m[1][2] = m2[2]; m[2][2] = m3[2];
    }

    inline float* operator[] (const size_t idx) {
        return m[idx];
    }

    inline FVector3f operator*(const FVector3f& v) const {
        FVector3f ret;
        ret[0] = m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2];
        ret[1] = m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2];
        ret[2] = m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2];
        return ret;
    }

    inline mat3 operator*(const float f) const {
        mat3 ret;
        ret[0][0] = m[0][0] * f; ret[1][0] = m[1][0] * f; ret[2][0] = m[2][0] * f;
        ret[0][1] = m[0][1] * f; ret[1][1] = m[1][1] * f; ret[2][1] = m[2][1] * f;
        ret[0][2] = m[0][2] * f; ret[1][2] = m[1][2] * f; ret[2][2] = m[2][2] * f;
        return ret;
    }

    inline mat3 operator/(const float f) const {
        mat3 ret;
        ret[0][0] = m[0][0] / f; ret[1][0] = m[1][0] / f; ret[2][0] = m[2][0] / f;
        ret[0][1] = m[0][1] / f; ret[1][1] = m[1][1] / f; ret[2][1] = m[2][1] / f;
        ret[0][2] = m[0][2] / f; ret[1][2] = m[1][2] / f; ret[2][2] = m[2][2] / f;
        return ret;
    }

    inline mat3 operator+(const mat3& other) const {
        mat3 ret;
        ret[0][0] = m[0][0] + other.m[0][0]; ret[1][0] = m[1][0] + other.m[1][0]; ret[2][0] = m[2][0] + other.m[2][0];
        ret[0][1] = m[0][1] + other.m[0][1]; ret[1][1] = m[1][1] + other.m[1][1]; ret[2][1] = m[2][1] + other.m[2][1];
        ret[0][2] = m[0][2] + other.m[0][2]; ret[1][2] = m[1][2] + other.m[1][2]; ret[2][2] = m[2][2] + other.m[2][2];
        return ret;
    }

    inline mat3 operator-(const mat3& other) const {
        mat3 ret;
        ret[0][0] = m[0][0] - other.m[0][0]; ret[1][0] = m[1][0] - other.m[1][0]; ret[2][0] = m[2][0] - other.m[2][0];
        ret[0][1] = m[0][1] - other.m[0][1]; ret[1][1] = m[1][1] - other.m[1][1]; ret[2][1] = m[2][1] - other.m[2][1];
        ret[0][2] = m[0][2] - other.m[0][2]; ret[1][2] = m[1][2] - other.m[1][2]; ret[2][2] = m[2][2] - other.m[2][2];
        return ret;
    }

    inline mat3 operator*(const mat3& other) const {
        const auto& a11 = m[0][0], &a12 = m[1][0], &a13 = m[2][0];
        const auto& a21 = m[0][1], &a22 = m[1][1], &a23 = m[2][1];
        const auto& a31 = m[0][2], &a32 = m[1][2], &a33 = m[2][2];

        const auto& b11 = other.m[0][0], &b12 = other.m[1][0], &b13 = other.m[2][0];
        const auto& b21 = other.m[0][1], &b22 = other.m[1][1], &b23 = other.m[2][1];
        const auto& b31 = other.m[0][2], &b32 = other.m[1][2], &b33 = other.m[2][2];

        mat3 ret;
        ret[0][0] = a11 * b11 + a12 * b21 + a13 * b31;
        ret[0][1] = a21 * b11 + a22 * b21 + a23 * b31;
        ret[0][2] = a31 * b11 + a32 * b21 + a33 * b31;

        ret[1][0] = a11 * b12 + a12 * b22 + a13 * b32;
        ret[1][1] = a21 * b12 + a22 * b22 + a23 * b32;
        ret[1][2] = a31 * b12 + a32 * b22 + a33 * b32;

        ret[2][0] = a11 * b13 + a12 * b23 + a13 * b33;
        ret[2][1] = a21 * b13 + a22 * b23 + a23 * b33;
        ret[2][2] = a31 * b13 + a32 * b23 + a33 * b33;
        return ret;
    }

    inline mat3 transpose() const {
        mat3 ret;
        ret[0][0] = m[0][0]; ret[0][1] = m[1][0]; ret[0][2] = m[2][0];
        ret[1][0] = m[0][1]; ret[1][1] = m[1][1]; ret[1][2] = m[2][1];
        ret[2][0] = m[0][2]; ret[2][1] = m[1][2]; ret[2][2] = m[2][2];
        return ret;
    }

    inline float det() const {
        const auto& n11 = m[0][0], &n12 = m[1][0], &n13 = m[2][0];
        const auto& n21 = m[0][1], &n22 = m[1][1], &n23 = m[2][1];
        const auto& n31 = m[0][2], &n32 = m[1][2], &n33 = m[2][2];

        return (
              n11 * (n22 * n33 - n23 * n32)
            - n12 * (n21 * n33 - n23 * n31)
            + n13 * (n21 * n32 - n22 * n31)
            );
    }

    inline float trace() const {
        return m[0][0] + m[1][1] + m[2][2];
    }

    inline float secondInvariant() const {
        const auto& n11 = m[0][0], & n12 = m[1][0], & n13 = m[2][0];
        const auto& n21 = m[0][1], & n22 = m[1][1], & n23 = m[2][1];
        const auto& n31 = m[0][2], & n32 = m[1][2], & n33 = m[2][2];

        return n11 * (n22 + n33) + n22 * n33 - n12 * n21 - n23 * n32 - n13 * n31;
    }

    inline float frobeniusSqNorm() const {
        const auto& n11 = m[0][0], & n12 = m[1][0], & n13 = m[2][0];
        const auto& n21 = m[0][1], & n22 = m[1][1], & n23 = m[2][1];
        const auto& n31 = m[0][2], & n32 = m[1][2], & n33 = m[2][2];

        return n11 * n11 + n12 * n12 + n13 * n13
            + n21 * n21 + n22 * n22 + n23 * n23
            + n31 * n31 + n32 * n32 + n33 * n33;
    }

    inline float frobeniusNorm() const {
        return FMath::Sqrt(frobeniusSqNorm());
    }

    inline mat3 adjoint() const {
        const auto& n11 = m[0][0], & n12 = m[1][0], & n13 = m[2][0];
        const auto& n21 = m[0][1], & n22 = m[1][1], & n23 = m[2][1];
        const auto& n31 = m[0][2], & n32 = m[1][2], & n33 = m[2][2];

        const auto t11 = n22 * n33 - n23 * n32;
        const auto t12 = -n21 * n33 + n23 * n31;
        const auto t13 = n21 * n32 - n22 * n31;

        mat3 ret;

        ret[0][0] = t11;
        ret[1][0] = (-n12 * n33 + n13 * n32);
        ret[2][0] = (n12 * n23 - n13 * n22);

        ret[0][1] = t12;
        ret[1][1] = (n11 * n33 - n13 * n31);
        ret[2][1] = (-n11 * n23 + n13 * n21);

        ret[0][2] = t13;
        ret[1][2] = (-n11 * n32 + n12 * n31);
        ret[2][2] = (n11 * n22 - n12 * n21);

        return ret;
    }

    inline mat3 inverse() const {
        const auto& n11 = m[0][0], & n12 = m[1][0], & n13 = m[2][0];
        const auto& n21 = m[0][1], & n22 = m[1][1], & n23 = m[2][1];
        const auto& n31 = m[0][2], & n32 = m[1][2], & n33 = m[2][2];

        const auto t11 = n22 * n33 - n23 * n32;
        const auto t12 = -n21 * n33 + n23 * n31;
        const auto t13 = n21 * n32 - n22 * n31;

        const auto det = n11 * t11 + n12 * t12 + n13 * t13;
        const auto idet = 1.0f / det;

        mat3 ret;

        ret[0][0] = t11 * idet;
        ret[1][0] = (-n12 * n33 + n13 * n32) * idet;
        ret[2][0] = (n12 * n23 - n13 * n22) * idet;

        ret[0][1] = t12 * idet;
        ret[1][1] = (n11 * n33 - n13 * n31) * idet;
        ret[2][1] = (-n11 * n23 + n13 * n21) * idet;

        ret[0][2] = t13 * idet;
        ret[1][2] = (-n11 * n32 + n12 * n31) * idet;
        ret[2][2] = (n11 * n22 - n12 * n21) * idet;

        return ret;
    }

    inline void zero() {
        m[0][0] = 0.0; m[1][0] = 0.0; m[2][0] = 0.0;
        m[0][1] = 0.0; m[1][1] = 0.0; m[2][1] = 0.0;
        m[0][2] = 0.0; m[1][2] = 0.0; m[2][2] = 0.0;
    }

    inline void identity() {
        m[0][0] = 1.0; m[1][0] = 0.0; m[2][0] = 0.0;
        m[0][1] = 0.0; m[1][1] = 1.0; m[2][1] = 0.0;
        m[0][2] = 0.0; m[1][2] = 0.0; m[2][2] = 1.0;
    }

    inline mat3& operator=(const mat3& other) {
        m[0][0] = other.m[0][0]; m[1][0] = other.m[1][0]; m[2][0] = other.m[2][0];
        m[0][1] = other.m[0][1]; m[1][1] = other.m[1][1]; m[2][1] = other.m[2][1];
        m[0][2] = other.m[0][2]; m[1][2] = other.m[1][2]; m[2][2] = other.m[2][2];
		return *this;
    }
	
	inline void normalizeColumns() {
		const float n0 = FMath::Sqrt(columnSqNorm(0)),
			n1 = FMath::Sqrt(columnSqNorm(1)),
			n2 = FMath::Sqrt(columnSqNorm(2));
		m[0][0] /= n0; m[1][0] /= n1; m[2][0] /= n2;
        m[0][1] /= n0; m[1][1] /= n1; m[2][1] /= n2;
        m[0][2] /= n0; m[1][2] /= n1; m[2][2] /= n2;
		
	}

    inline float columnSqNorm(const size_t idx) const {
        return m[idx][0] * m[idx][0] + m[idx][1] * m[idx][1] + m[idx][2] * m[idx][2];
    }

    inline float colSumNorm() const {
        const float x1 = m[0][0] + m[1][0] + m[2][0],
            x2 = m[0][1] + m[1][1] + m[2][1],
            x3 = m[0][2] + m[1][2] + m[2][2];
        return x1 * x1 + x2 * x2 + x3 * x3;
    }

    inline mat3& operator*=(const float f) { return *this = *this * f; }
    inline mat3& operator/=(const float f) { return *this = *this / f; }
    inline mat3& operator+=(const mat3& mat) { return *this = *this + mat; }
    inline mat3& operator-=(const mat3& mat) { return *this = *this - mat; }
    inline mat3& operator*=(const mat3& mat) { return *this = *this * mat; }
};

inline mat3 operator*(const float& f, const mat3& m) {
    return m * f;
}