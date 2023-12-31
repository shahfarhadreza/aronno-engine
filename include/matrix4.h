

struct Vec3
{
    float x, y, z;

    inline Vec3 operator + ( const Vec3& rkVector ) const
    {
        return Vec3(
            x + rkVector.x,
            y + rkVector.y,
            z + rkVector.z);
    }

    inline Vec3 operator - ( const Vec3& rkVector ) const
    {
        return Vec3(
            x - rkVector.x,
            y - rkVector.y,
            z - rkVector.z);
    }
};

// -- Use this for Vector normalisation!!!
inline float asm_rsq( float r )
{
	return 1. / sqrt( r );
}

inline Vec3 T3DVec3CrossProduct(const Vec3* pV1, const Vec3* pV2) {
	Vec3 out;
	out.x = pV1->y * pV2->z - pV1->z * pV2->y;
	out.y = pV1->z * pV2->x - pV1->x * pV2->z;
	out.z = pV1->x * pV2->y - pV1->y * pV2->x;
	return out;
}

inline void T3DVec3CrossProduct(Vec3* pOut, const Vec3* pV1, const Vec3* pV2) {
	pOut->x = pV1->y * pV2->z - pV1->z * pV2->y;
	pOut->y = pV1->z * pV2->x - pV1->x * pV2->z;
	pOut->z = pV1->x * pV2->y - pV1->y * pV2->x;
}

inline float T3DVec3DotProduct(const Vec3* pV1, const Vec3* pV2) {
	return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
}

inline float T3DVec3LengthSQ(const Vec3* pV1) {
	return pV1->x*pV1->x + pV1->y*pV1->y + pV1->z*pV1->z;
}

inline float T3DVec3Length(const Vec3* pV1) {
	return sqrt(pV1->x*pV1->x + pV1->y*pV1->y + pV1->z*pV1->z);
}

inline float T3DVec3Normalize(Vec3* v) {
	float length = v->x*v->x + v->y*v->y + v->z*v->z;
	if (length == 0 ) // this check isn't an optimization but prevents getting NAN in the sqrt.
		return 0.0;
	length = asm_rsq(length);

	v->x = (float)(v->x * length);
	v->y = (float)(v->y * length);
	v->z = (float)(v->z * length);

	return length;
}

#define _MATRIX_COLUMN_ 1

class Matrix4
{
public:
// Variables:
	union {
		float m[4][4];
		float M[16];
	};
	static const Matrix4 ZERO;
	static const Matrix4 IDENTITY;
// Methods:
	inline Matrix4() {
	}
	inline Matrix4(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33 )
    {
        m[0][0] = m00;
        m[0][1] = m01;
        m[0][2] = m02;
        m[0][3] = m03;
        m[1][0] = m10;
        m[1][1] = m11;
        m[1][2] = m12;
        m[1][3] = m13;
        m[2][0] = m20;
        m[2][1] = m21;
        m[2][2] = m22;
        m[2][3] = m23;
        m[3][0] = m30;
        m[3][1] = m31;
        m[3][2] = m32;
        m[3][3] = m33;
    }
	float& operator()(const int row, const int col)
	{
		return m[row][col];
	}
	const float& operator()(const int row, const int col) const
	{
		return m[row][col];
	}
	float* operator[](unsigned int row)
	{
		return m[row];
	}
	const float* operator[](unsigned int row) const
	{
		return m[row];
	}
	const float* pointer() const
	{
		return &M[0];
	}
	float* pointer()
	{
		return &M[0];
	}
	inline bool operator == ( const Matrix4& m2 ) const
	{
		if(
            m[0][0] != m2.m[0][0] || m[0][1] != m2.m[0][1] || m[0][2] != m2.m[0][2] || m[0][3] != m2.m[0][3] ||
            m[1][0] != m2.m[1][0] || m[1][1] != m2.m[1][1] || m[1][2] != m2.m[1][2] || m[1][3] != m2.m[1][3] ||
            m[2][0] != m2.m[2][0] || m[2][1] != m2.m[2][1] || m[2][2] != m2.m[2][2] || m[2][3] != m2.m[2][3] ||
            m[3][0] != m2.m[3][0] || m[3][1] != m2.m[3][1] || m[3][2] != m2.m[3][2] || m[3][3] != m2.m[3][3] )
            return false;
		return true;
	}
	inline bool operator != ( const Matrix4& m2 ) const
    {
        if(
            m[0][0] != m2.m[0][0] || m[0][1] != m2.m[0][1] || m[0][2] != m2.m[0][2] || m[0][3] != m2.m[0][3] ||
            m[1][0] != m2.m[1][0] || m[1][1] != m2.m[1][1] || m[1][2] != m2.m[1][2] || m[1][3] != m2.m[1][3] ||
            m[2][0] != m2.m[2][0] || m[2][1] != m2.m[2][1] || m[2][2] != m2.m[2][2] || m[2][3] != m2.m[2][3] ||
            m[3][0] != m2.m[3][0] || m[3][1] != m2.m[3][1] || m[3][2] != m2.m[3][2] || m[3][3] != m2.m[3][3] )
            return true;
        return false;
    }
	inline Matrix4& MakeIdentity()
	{
		memset(M, 0, 16*sizeof(float));

		m[0][0] = 1.0f;
		m[1][1] = 1.0f;
		m[2][2] = 1.0f;
		m[3][3] = 1.0f;

		return *this;
	}
	/*
	inline bool IsIdentity() const
	{
		if (!T3DEquals( M[12], 0 ) || !T3DEquals( M[13], 0 ) || !T3DEquals( M[14], 0 ) || !T3DEquals( M[15], 1 ))
			return false;

		if (!T3DEquals( m[0][0], 1 ) || !T3DEquals( M[ 1], 0 ) || !T3DEquals( M[ 2], 0 ) || !T3DEquals( M[ 3], 0 ))
			return false;

		if (!T3DEquals( M[ 4], 0 ) || !T3DEquals( M[ 5], 1 ) || !T3DEquals( M[ 6], 0 ) || !T3DEquals( M[ 7], 0 ))
			return false;

		if (!T3DEquals( M[ 8], 0 ) || !T3DEquals( M[ 9], 0 ) || !T3DEquals( M[10], 1 ) || !T3DEquals( M[11], 0 ))
			return false;

		return true;
	}
	*/
	inline static Matrix4 GetScale( float s_x, float s_y, float s_z )
    {
        Matrix4 r;
        r.m[0][0] = s_x; r.m[0][1] = 0.0; r.m[0][2] = 0.0; r.m[0][3] = 0.0;
        r.m[1][0] = 0.0; r.m[1][1] = s_y; r.m[1][2] = 0.0; r.m[1][3] = 0.0;
        r.m[2][0] = 0.0; r.m[2][1] = 0.0; r.m[2][2] = s_z; r.m[2][3] = 0.0;
        r.m[3][0] = 0.0; r.m[3][1] = 0.0; r.m[3][2] = 0.0; r.m[3][3] = 1.0;

        return r;
    }

	//-----------------------------------------------------------------------
    Matrix4 Inverse() const
    {
        float m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
        float m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
        float m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
        float m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];

        float v0 = m20 * m31 - m21 * m30;
        float v1 = m20 * m32 - m22 * m30;
        float v2 = m20 * m33 - m23 * m30;
        float v3 = m21 * m32 - m22 * m31;
        float v4 = m21 * m33 - m23 * m31;
        float v5 = m22 * m33 - m23 * m32;

        float t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
        float t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
        float t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
        float t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

        float invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

        float d00 = t00 * invDet;
        float d10 = t10 * invDet;
        float d20 = t20 * invDet;
        float d30 = t30 * invDet;

        float d01 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
        float d11 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
        float d21 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
        float d31 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

        v0 = m10 * m31 - m11 * m30;
        v1 = m10 * m32 - m12 * m30;
        v2 = m10 * m33 - m13 * m30;
        v3 = m11 * m32 - m12 * m31;
        v4 = m11 * m33 - m13 * m31;
        v5 = m12 * m33 - m13 * m32;

        float d02 = + (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
        float d12 = - (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
        float d22 = + (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
        float d32 = - (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

        v0 = m21 * m10 - m20 * m11;
        v1 = m22 * m10 - m20 * m12;
        v2 = m23 * m10 - m20 * m13;
        v3 = m22 * m11 - m21 * m12;
        v4 = m23 * m11 - m21 * m13;
        v5 = m23 * m12 - m22 * m13;

        float d03 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
        float d13 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
        float d23 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
        float d33 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

        return Matrix4(
            d00, d01, d02, d03,
            d10, d11, d12, d13,
            d20, d21, d22, d23,
            d30, d31, d32, d33);
    }

	//-----------------------------------------------------------------------
    Matrix4 InverseAffine(void) const
    {
        assert(IsAffine());

        float m10 = m[1][0], m11 = m[1][1], m12 = m[1][2];
        float m20 = m[2][0], m21 = m[2][1], m22 = m[2][2];

        float t00 = m22 * m11 - m21 * m12;
        float t10 = m20 * m12 - m22 * m10;
        float t20 = m21 * m10 - m20 * m11;

        float m00 = m[0][0], m01 = m[0][1], m02 = m[0][2];

        float invDet = 1 / (m00 * t00 + m01 * t10 + m02 * t20);

        t00 *= invDet; t10 *= invDet; t20 *= invDet;

        m00 *= invDet; m01 *= invDet; m02 *= invDet;

        float r00 = t00;
        float r01 = m02 * m21 - m01 * m22;
        float r02 = m01 * m12 - m02 * m11;

        float r10 = t10;
        float r11 = m00 * m22 - m02 * m20;
        float r12 = m02 * m10 - m00 * m12;

        float r20 = t20;
        float r21 = m01 * m20 - m00 * m21;
        float r22 = m00 * m11 - m01 * m10;

        float m03 = m[0][3], m13 = m[1][3], m23 = m[2][3];

        float r03 = - (r00 * m03 + r01 * m13 + r02 * m23);
        float r13 = - (r10 * m03 + r11 * m13 + r12 * m23);
        float r23 = - (r20 * m03 + r21 * m13 + r22 * m23);

        return Matrix4(
            r00, r01, r02, r03,
            r10, r11, r12, r13,
            r20, r21, r22, r23,
              0,   0,   0,   1);
    }
	inline Matrix4 operator+(const Matrix4 &m2) const
	{
		Matrix4 r;

        r.m[0][0] = m[0][0] + m2.m[0][0];
        r.m[0][1] = m[0][1] + m2.m[0][1];
        r.m[0][2] = m[0][2] + m2.m[0][2];
        r.m[0][3] = m[0][3] + m2.m[0][3];

        r.m[1][0] = m[1][0] + m2.m[1][0];
        r.m[1][1] = m[1][1] + m2.m[1][1];
        r.m[1][2] = m[1][2] + m2.m[1][2];
        r.m[1][3] = m[1][3] + m2.m[1][3];

        r.m[2][0] = m[2][0] + m2.m[2][0];
        r.m[2][1] = m[2][1] + m2.m[2][1];
        r.m[2][2] = m[2][2] + m2.m[2][2];
        r.m[2][3] = m[2][3] + m2.m[2][3];

        r.m[3][0] = m[3][0] + m2.m[3][0];
        r.m[3][1] = m[3][1] + m2.m[3][1];
        r.m[3][2] = m[3][2] + m2.m[3][2];
        r.m[3][3] = m[3][3] + m2.m[3][3];

        return r;
	}
	inline Matrix4 operator-(const Matrix4& m2) const
	{
		Matrix4 r;

        r.m[0][0] = m[0][0] - m2.m[0][0];
        r.m[0][1] = m[0][1] - m2.m[0][1];
        r.m[0][2] = m[0][2] - m2.m[0][2];
        r.m[0][3] = m[0][3] - m2.m[0][3];

        r.m[1][0] = m[1][0] - m2.m[1][0];
        r.m[1][1] = m[1][1] - m2.m[1][1];
        r.m[1][2] = m[1][2] - m2.m[1][2];
        r.m[1][3] = m[1][3] - m2.m[1][3];

        r.m[2][0] = m[2][0] - m2.m[2][0];
        r.m[2][1] = m[2][1] - m2.m[2][1];
        r.m[2][2] = m[2][2] - m2.m[2][2];
        r.m[2][3] = m[2][3] - m2.m[2][3];

        r.m[3][0] = m[3][0] - m2.m[3][0];
        r.m[3][1] = m[3][1] - m2.m[3][1];
        r.m[3][2] = m[3][2] - m2.m[3][2];
        r.m[3][3] = m[3][3] - m2.m[3][3];

        return r;
	}
	/* Vector mTransformation using '*' operator
	 */
	inline Vec3 operator * ( const Vec3 &v ) const
	{
		Vec3 r;

		float fInvW = 1.0f / ( m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] );

        r.x = ( m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] ) * fInvW;
        r.y = ( m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] ) * fInvW;
        r.z = ( m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] ) * fInvW;

        return r;
	}
	inline Matrix4 operator*(float scalar) const
	{
		return Matrix4(
                scalar*m[0][0], scalar*m[0][1], scalar*m[0][2], scalar*m[0][3],
                scalar*m[1][0], scalar*m[1][1], scalar*m[1][2], scalar*m[1][3],
                scalar*m[2][0], scalar*m[2][1], scalar*m[2][2], scalar*m[2][3],
                scalar*m[3][0], scalar*m[3][1], scalar*m[3][2], scalar*m[3][3]);
	}

	inline Matrix4 Concatenate(const Matrix4 &m2) const
    {
        Matrix4 r;

        r.m[0][0] = m[0][0] * m2.m[0][0] + m[0][1] * m2.m[1][0] + m[0][2] * m2.m[2][0] + m[0][3] * m2.m[3][0];
        r.m[0][1] = m[0][0] * m2.m[0][1] + m[0][1] * m2.m[1][1] + m[0][2] * m2.m[2][1] + m[0][3] * m2.m[3][1];
        r.m[0][2] = m[0][0] * m2.m[0][2] + m[0][1] * m2.m[1][2] + m[0][2] * m2.m[2][2] + m[0][3] * m2.m[3][2];
        r.m[0][3] = m[0][0] * m2.m[0][3] + m[0][1] * m2.m[1][3] + m[0][2] * m2.m[2][3] + m[0][3] * m2.m[3][3];

        r.m[1][0] = m[1][0] * m2.m[0][0] + m[1][1] * m2.m[1][0] + m[1][2] * m2.m[2][0] + m[1][3] * m2.m[3][0];
        r.m[1][1] = m[1][0] * m2.m[0][1] + m[1][1] * m2.m[1][1] + m[1][2] * m2.m[2][1] + m[1][3] * m2.m[3][1];
        r.m[1][2] = m[1][0] * m2.m[0][2] + m[1][1] * m2.m[1][2] + m[1][2] * m2.m[2][2] + m[1][3] * m2.m[3][2];
        r.m[1][3] = m[1][0] * m2.m[0][3] + m[1][1] * m2.m[1][3] + m[1][2] * m2.m[2][3] + m[1][3] * m2.m[3][3];

        r.m[2][0] = m[2][0] * m2.m[0][0] + m[2][1] * m2.m[1][0] + m[2][2] * m2.m[2][0] + m[2][3] * m2.m[3][0];
        r.m[2][1] = m[2][0] * m2.m[0][1] + m[2][1] * m2.m[1][1] + m[2][2] * m2.m[2][1] + m[2][3] * m2.m[3][1];
        r.m[2][2] = m[2][0] * m2.m[0][2] + m[2][1] * m2.m[1][2] + m[2][2] * m2.m[2][2] + m[2][3] * m2.m[3][2];
        r.m[2][3] = m[2][0] * m2.m[0][3] + m[2][1] * m2.m[1][3] + m[2][2] * m2.m[2][3] + m[2][3] * m2.m[3][3];

        r.m[3][0] = m[3][0] * m2.m[0][0] + m[3][1] * m2.m[1][0] + m[3][2] * m2.m[2][0] + m[3][3] * m2.m[3][0];
        r.m[3][1] = m[3][0] * m2.m[0][1] + m[3][1] * m2.m[1][1] + m[3][2] * m2.m[2][1] + m[3][3] * m2.m[3][1];
        r.m[3][2] = m[3][0] * m2.m[0][2] + m[3][1] * m2.m[1][2] + m[3][2] * m2.m[2][2] + m[3][3] * m2.m[3][2];
        r.m[3][3] = m[3][0] * m2.m[0][3] + m[3][1] * m2.m[1][3] + m[3][2] * m2.m[2][3] + m[3][3] * m2.m[3][3];

        return r;
    }

	//! multiply by another matrix
	inline Matrix4 operator*(const Matrix4& m2) const
	{
		return Concatenate( m2 );
	}
	inline Matrix4 operator/(const float& scalar) const
	{
		Matrix4 temp;

		temp.M[0] = M[0]/scalar;
		temp.M[1] = M[1]/scalar;
		temp.M[2] = M[2]/scalar;
		temp.M[3] = M[3]/scalar;
		temp.M[4] = M[4]/scalar;
		temp.M[5] = M[5]/scalar;
		temp.M[6] = M[6]/scalar;
		temp.M[7] = M[7]/scalar;
		temp.M[8] = M[8]/scalar;
		temp.M[9] = M[9]/scalar;
		temp.M[10] = M[10]/scalar;
		temp.M[11] = M[11]/scalar;
		temp.M[12] = M[12]/scalar;
		temp.M[13] = M[13]/scalar;
		temp.M[14] = M[14]/scalar;
		temp.M[15] = M[15]/scalar;

		return temp;
	}
	inline Vec3 GetTranslation() const
	{
		// column major
		return Vec3(m[0][3], m[1][3], m[2][3]);
	}
	inline Matrix4& SetTranslation( float x, float y, float z )
	{
		// column major
		m[0][3] = x;
        m[1][3] = y;
        m[2][3] = z;
		return *this;
	}
	inline Matrix4& SetTranslation( const Vec3& v )
	{
		return SetTranslation(v.x, v.y, v.z);
	}
	inline Matrix4& SetScale( const Vec3& scale )
	{
		// row/column major
		m[0][0] = scale.x;
		m[1][1] = scale.y;
		m[2][2] = scale.z;
		return *this;
	}
	//! An alternate transform vector method, writing into a second vector
	inline void RotateVect(Vec3& out, const Vec3& in) const
	{
		// column major
		out.x = m[0][0] * in.x + m[0][1] * in.y + m[0][2] * in.z;
		out.y = m[1][0] * in.x + m[1][1] * in.y + m[1][2] * in.z;
		out.z = m[2][0] * in.x + m[2][1] * in.y + m[2][2] * in.z;
	}
	inline void TransformVect( Vec3& vect) const
	{
		float vector[3];

		vector[0] = m[0][0] * vect.x + m[0][1] * vect.y + m[0][2] * vect.z + m[0][3];
		vector[1] = m[1][0] * vect.x + m[1][1] * vect.y + m[1][2] * vect.z + m[1][3];
		vector[2] = m[2][0] * vect.x + m[2][1] * vect.y + m[2][2] * vect.z + m[2][3];

		vect.x = vector[0];
		vect.y = vector[1];
		vect.z = vector[2];
	}
	inline void TransformVect( Vec3& out, const Vec3& in) const
	{
		out.x = m[0][0] * in.x + m[0][1] * in.y + m[0][2] * in.z + m[0][3];
		out.y = m[1][0] * in.x + m[1][1] * in.y + m[1][2] * in.z + m[1][3];
		out.z = m[2][0] * in.x + m[2][1] * in.y + m[2][2] * in.z + m[2][3];
	}
	inline Vec3 TransformAffine(const Vec3& v) const
    {
        assert(IsAffine());

        return Vec3(
                m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3],
                m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3],
                m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3]);
    }
	void CalcProjectionParameters(float fov, float aspect, float zNear, float zFar,
		float& left, float& right, float& bottom, float& top) const
	{

		float thetaY (fov * 0.5f);
		float tanThetaY = tan(thetaY);
		float tanThetaX = tanThetaY * aspect;

		float mFocalLength = 1.0f;
		float mFrustumOffset_x = 0.0f;
		float mFrustumOffset_y = 0.0f;

		float nearFocal = zNear / mFocalLength;
		float nearOffsetX = mFrustumOffset_x * nearFocal;
		float nearOffsetY = mFrustumOffset_y * nearFocal;
		float half_w = tanThetaX * zNear;
		float half_h = tanThetaY * zNear;

		left   = - half_w + nearOffsetX;
		right  = + half_w + nearOffsetX;
		bottom = - half_h + nearOffsetY;
		top    = + half_h + nearOffsetY;
	}

#define INFINITE_FAR_PLANE_ADJUST 0.00001f

	inline static Matrix4 MakeOrthoOffset(float minX, float maxX, float minY, float maxY, float cNear, float cFar) {
		Matrix4 LProj = Matrix4::IDENTITY;

		float l = minX;
		float r = maxX;

		float t = minY;
		float b = maxY;

		LProj.m[0][0] = 2.0f / (r - l);
		LProj.m[1][1] = 2.0f / (t - b);
		LProj.m[2][2] = 1.0f / (cFar - cNear);

		LProj.m[0][3] = -1.0f -2.0f *l / (r - l);
		LProj.m[1][3] = 1.0f + 2.0f * t / (b - t);
		LProj.m[2][3] = cNear / (cNear - cFar);

		return LProj;
	}

	// Builds a left-handed orthogonal projection matrix.
	inline Matrix4& MakeProjectionOrtho(
			float widthOfViewVolume, float heightOfViewVolume, float zNear, float zFar)
	{
		float inv_d = 1 / (zFar - zNear);

		float q, qn;

		if (zFar == 0)
		{
			// Can not do infinite far plane here, avoid divided zero only
			q = - INFINITE_FAR_PLANE_ADJUST / zNear;
			qn = - INFINITE_FAR_PLANE_ADJUST - 1;
		}
		else
		{
			q = - 2 * inv_d;
			qn = - (zFar + zNear)  * inv_d;
		}

		// column-major
		m[0][0] = (float)(2/widthOfViewVolume);
		m[1][0] = 0;
		m[2][0] = 0;
		m[3][0] = 0;

		m[0][1] = 0;
		m[1][1] = (float)(2/heightOfViewVolume);
		m[2][1] = 0;
		m[3][1] = 0;

		m[0][2] = 0;
		m[1][2] = 0;
		m[2][2] = q;
		m[3][2] = 0;

		m[0][3] = 0;
		m[1][3] = 0;
		m[2][3] = qn;
		m[3][3] = 1;

		// Convert depth range from [-1,+1] to [0,1]
		m[2][2] = (m[2][2] + m[3][2]) / 2;
		m[2][3] = (m[2][3] + m[3][3]) / 2;

		// Convert right-handed to left-handed
		m[2][2] = -m[2][2];

		return *this;
	}
	// Builds a left-handed perspective projection matrix based on a field of view
	inline void MakePerspectiveLH(float fov, float aspect, float zNear, float zFar)
	{
		const float h = 1.0f / /*asm_*/tanf(fov * 0.5f);
		const float w = h / aspect;
		// Make left-handed directly ( column-major )

		m[0][0] = w;
		m[1][0] = 0;
		m[2][0] = 0;
		m[3][0] = 0;

		m[0][1] = 0;
		m[1][1] = (float)h;
		m[2][1] = 0;
		m[3][1] = 0;

		m[0][2] = 0;
		m[1][2] = 0;
		m[2][2] = (float)(zFar/(zFar-zNear));
		m[3][2] = 1;

		m[0][3] = 0;
		m[1][3] = 0;
		m[2][3] = (float)(-zNear*zFar/(zFar-zNear));
		m[3][3] = 0;
	}

	// Builds a left-handed look-at matrix.
	inline void MakeLookatViewLH(
				const Vec3& position,
				const Vec3& target,
				const Vec3& upVector)
	{
		Vec3 zaxis = /*position - target;//*/target - position;
		::T3DVec3Normalize(&zaxis );

		Vec3 xaxis = ::T3DVec3CrossProduct(&upVector, &zaxis);
		::T3DVec3Normalize(&xaxis );

		Vec3 yaxis = ::T3DVec3CrossProduct(&zaxis, &xaxis);

		m[0][0] = xaxis.x;
		m[1][0] = yaxis.x;
		m[2][0] = zaxis.x;
		m[3][0] = 0;

		m[0][1] = xaxis.y;
		m[1][1] = yaxis.y;
		m[2][1] = zaxis.y;
		m[3][1] = 0;

		m[0][2] = xaxis.z;
		m[1][2] = yaxis.z;
		m[2][2] = zaxis.z;
		m[3][2] = 0;

		m[0][3] = -::T3DVec3DotProduct( &xaxis, &position );
		m[1][3] = -::T3DVec3DotProduct( &yaxis, &position );
		m[2][3] = -::T3DVec3DotProduct( &zaxis, &position );
		m[3][3] = 1;
	}

	inline Matrix4 Transpose(void) const
    {
        return Matrix4(m[0][0], m[1][0], m[2][0], m[3][0],
                       m[0][1], m[1][1], m[2][1], m[3][1],
                       m[0][2], m[1][2], m[2][2], m[3][2],
                       m[0][3], m[1][3], m[2][3], m[3][3]);
    }
    inline bool IsAffine(void) const
    {
        return m[3][0] == 0 && m[3][1] == 0 && m[3][2] == 0 && m[3][3] == 1;
    }
	inline Matrix4 ConcatenateAffine(const Matrix4 &m2) const
    {
        assert(IsAffine() && m2.IsAffine());

        return Matrix4(
            m[0][0] * m2.m[0][0] + m[0][1] * m2.m[1][0] + m[0][2] * m2.m[2][0],
            m[0][0] * m2.m[0][1] + m[0][1] * m2.m[1][1] + m[0][2] * m2.m[2][1],
            m[0][0] * m2.m[0][2] + m[0][1] * m2.m[1][2] + m[0][2] * m2.m[2][2],
            m[0][0] * m2.m[0][3] + m[0][1] * m2.m[1][3] + m[0][2] * m2.m[2][3] + m[0][3],

            m[1][0] * m2.m[0][0] + m[1][1] * m2.m[1][0] + m[1][2] * m2.m[2][0],
            m[1][0] * m2.m[0][1] + m[1][1] * m2.m[1][1] + m[1][2] * m2.m[2][1],
            m[1][0] * m2.m[0][2] + m[1][1] * m2.m[1][2] + m[1][2] * m2.m[2][2],
            m[1][0] * m2.m[0][3] + m[1][1] * m2.m[1][3] + m[1][2] * m2.m[2][3] + m[1][3],

            m[2][0] * m2.m[0][0] + m[2][1] * m2.m[1][0] + m[2][2] * m2.m[2][0],
            m[2][0] * m2.m[0][1] + m[2][1] * m2.m[1][1] + m[2][2] * m2.m[2][1],
            m[2][0] * m2.m[0][2] + m[2][1] * m2.m[1][2] + m[2][2] * m2.m[2][2],
            m[2][0] * m2.m[0][3] + m[2][1] * m2.m[1][3] + m[2][2] * m2.m[2][3] + m[2][3],

            0, 0, 0, 1);
    }
};

const Matrix4 Matrix4::ZERO(
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0 );

const Matrix4 Matrix4::IDENTITY(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1 );


