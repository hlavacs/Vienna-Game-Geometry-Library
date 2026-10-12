//
// This file is part of the Terathon Math Library, by Eric Lengyel.
// Copyright 1999-2025, Terathon Software LLC
//
// This software is distributed under the MIT License.
// Separate proprietary licenses are available from Terathon Software.
//


#include "TSMatrix4D.h"


using namespace Terathon;


alignas(64) const ConstMatrix4D Matrix4D::identity = {{{1.0F, 0.0F, 0.0F, 0.0F}, {0.0F, 1.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 1.0F, 0.0F}, {0.0F, 0.0F, 0.0F, 1.0F}}};
alignas(64) const ConstTransform3D Transform3D::identity = {{{1.0F, 0.0F, 0.0F, 0.0F}, {0.0F, 1.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 1.0F, 0.0F}, {0.0F, 0.0F, 0.0F, 1.0F}}};


Matrix4D::Matrix4D(real n00, real n01, real n02, real n03, real n10, real n11, real n12, real n13, real n20, real n21, real n22, real n23, real n30, real n31, real n32, real n33) : Mat4D<TypeMatrix4D>(n00, n01, n02, n03, n10, n11, n12, n13, n20, n21, n22, n23, n30, n31, n32, n33)
{
}

Matrix4D::Matrix4D(const Vector4D& a, const Vector4D& b, const Vector4D& c, const Vector4D& d)
{
	col0 = a;
	col1 = b;
	col2 = c;
	col3 = d;
}

Matrix4D::Matrix4D(const Bivector3D& r0, real n03, const Bivector3D& r1, real n13, const Bivector3D& r2, real n23, const Bivector3D& r3, real n33)
{
	row0.Set(r0.x, r0.y, r0.z, n03);
	row1.Set(r1.x, r1.y, r1.z, n13);
	row2.Set(r2.x, r2.y, r2.z, n23);
	row3.Set(r3.x, r3.y, r3.z, n33);
}

Matrix4D& Matrix4D::Set(real n00, real n01, real n02, real n03, real n10, real n11, real n12, real n13, real n20, real n21, real n22, real n23, real n30, real n31, real n32, real n33)
{
	matrix.Set(n00, n01, n02, n03, n10, n11, n12, n13, n20, n21, n22, n23, n30, n31, n32, n33);
	return (*this);
}

Matrix4D& Matrix4D::Set(const Vector4D& a, const Vector4D& b, const Vector4D& c, const Vector4D& d)
{
	col0 = a;
	col1 = b;
	col2 = c;
	col3 = d;
	return (*this);
}

Matrix4D& Matrix4D::operator *=(const Matrix4D& m)
{
	real x = m00;
	real y = m01;
	real z = m02;
	real w = m03;
	m00 = x * m.m00 + y * m.m10 + z * m.m20 + w * m.m30;
	m01 = x * m.m01 + y * m.m11 + z * m.m21 + w * m.m31;
	m02 = x * m.m02 + y * m.m12 + z * m.m22 + w * m.m32;
	m03 = x * m.m03 + y * m.m13 + z * m.m23 + w * m.m33;

	x = m10;
	y = m11;
	z = m12;
	w = m13;
	m10 = x * m.m00 + y * m.m10 + z * m.m20 + w * m.m30;
	m11 = x * m.m01 + y * m.m11 + z * m.m21 + w * m.m31;
	m12 = x * m.m02 + y * m.m12 + z * m.m22 + w * m.m32;
	m13 = x * m.m03 + y * m.m13 + z * m.m23 + w * m.m33;

	x = m20;
	y = m21;
	z = m22;
	w = m23;
	m20 = x * m.m00 + y * m.m10 + z * m.m20 + w * m.m30;
	m21 = x * m.m01 + y * m.m11 + z * m.m21 + w * m.m31;
	m22 = x * m.m02 + y * m.m12 + z * m.m22 + w * m.m32;
	m23 = x * m.m03 + y * m.m13 + z * m.m23 + w * m.m33;

	x = m30;
	y = m31;
	z = m32;
	w = m33;
	m30 = x * m.m00 + y * m.m10 + z * m.m20 + w * m.m30;
	m31 = x * m.m01 + y * m.m11 + z * m.m21 + w * m.m31;
	m32 = x * m.m02 + y * m.m12 + z * m.m22 + w * m.m32;
	m33 = x * m.m03 + y * m.m13 + z * m.m23 + w * m.m33;

	return (*this);
}

Matrix4D& Matrix4D::operator *=(const Matrix3D& m)
{
	real x = m00;
	real y = m01;
	real z = m02;
	m00 = x * m.m00 + y * m.m10 + z * m.m20;
	m01 = x * m.m01 + y * m.m11 + z * m.m21;
	m02 = x * m.m02 + y * m.m12 + z * m.m22;

	x = m10;
	y = m11;
	z = m12;
	m10 = x * m.m00 + y * m.m10 + z * m.m20;
	m11 = x * m.m01 + y * m.m11 + z * m.m21;
	m12 = x * m.m02 + y * m.m12 + z * m.m22;

	x = m20;
	y = m21;
	z = m22;
	m20 = x * m.m00 + y * m.m10 + z * m.m20;
	m21 = x * m.m01 + y * m.m11 + z * m.m21;
	m22 = x * m.m02 + y * m.m12 + z * m.m22;

	x = m30;
	y = m31;
	z = m32;
	m30 = x * m.m00 + y * m.m10 + z * m.m20;
	m31 = x * m.m01 + y * m.m11 + z * m.m21;
	m32 = x * m.m02 + y * m.m12 + z * m.m22;

	return (*this);
}

Matrix4D& Matrix4D::SetIdentity(void)
{
	m00 = m11 = m22 = m33 = 1.0F;
	m01 = m02 = m03 = m10 = m12 = m13 = m20 = m21 = m23 = m30 = m31 = m32 = 0.0F;
	return (*this);
}


Matrix4D Terathon::operator *(const Matrix4D& m1, const Matrix3D& m2)
{
	return (Matrix4D(m1(0,0) * m2(0,0) + m1(0,1) * m2(1,0) + m1(0,2) * m2(2,0),
	                 m1(0,0) * m2(0,1) + m1(0,1) * m2(1,1) + m1(0,2) * m2(2,1),
	                 m1(0,0) * m2(0,2) + m1(0,1) * m2(1,2) + m1(0,2) * m2(2,2),
	                 m1(0,3),
	                 m1(1,0) * m2(0,0) + m1(1,1) * m2(1,0) + m1(1,2) * m2(2,0),
	                 m1(1,0) * m2(0,1) + m1(1,1) * m2(1,1) + m1(1,2) * m2(2,1),
	                 m1(1,0) * m2(0,2) + m1(1,1) * m2(1,2) + m1(1,2) * m2(2,2),
	                 m1(1,3),
	                 m1(2,0) * m2(0,0) + m1(2,1) * m2(1,0) + m1(2,2) * m2(2,0),
	                 m1(2,0) * m2(0,1) + m1(2,1) * m2(1,1) + m1(2,2) * m2(2,1),
	                 m1(2,0) * m2(0,2) + m1(2,1) * m2(1,2) + m1(2,2) * m2(2,2),
	                 m1(2,3),
	                 m1(3,0) * m2(0,0) + m1(3,1) * m2(1,0) + m1(3,2) * m2(2,0),
	                 m1(3,0) * m2(0,1) + m1(3,1) * m2(1,1) + m1(3,2) * m2(2,1),
	                 m1(3,0) * m2(0,2) + m1(3,1) * m2(1,2) + m1(3,2) * m2(2,2),
	                 m1(3,3)));
}

Vector4D Terathon::operator *(const Matrix4D& m, const Vector3D& v)
{
	return (Vector4D(m(0,0) * v.x + m(0,1) * v.y + m(0,2) * v.z,
	                 m(1,0) * v.x + m(1,1) * v.y + m(1,2) * v.z,
	                 m(2,0) * v.x + m(2,1) * v.y + m(2,2) * v.z,
	                 m(3,0) * v.x + m(3,1) * v.y + m(3,2) * v.z));
}

Vector4D Terathon::operator *(const Vector3D& v, const Matrix4D& m)
{
	return (Vector4D(m(0,0) * v.x + m(1,0) * v.y + m(2,0) * v.z,
	                 m(0,1) * v.x + m(1,1) * v.y + m(2,1) * v.z,
	                 m(0,2) * v.x + m(1,2) * v.y + m(2,2) * v.z,
	                 m(0,3) * v.x + m(1,3) * v.y + m(2,3) * v.z));
}

Vector4D Terathon::operator *(const Matrix4D& m, const Point3D& p)
{
	return (Vector4D(m(0,0) * p.x + m(0,1) * p.y + m(0,2) * p.z + m(0,3),
	                 m(1,0) * p.x + m(1,1) * p.y + m(1,2) * p.z + m(1,3),
	                 m(2,0) * p.x + m(2,1) * p.y + m(2,2) * p.z + m(2,3),
	                 m(3,0) * p.x + m(3,1) * p.y + m(3,2) * p.z + m(3,3)));
}

Vector4D Terathon::operator *(const Point3D& p, const Matrix4D& m)
{
	return (Vector4D(m(0,0) * p.x + m(1,0) * p.y + m(2,0) * p.z + m(3,0),
	                 m(0,1) * p.x + m(1,1) * p.y + m(2,1) * p.z + m(3,1),
	                 m(0,2) * p.x + m(1,2) * p.y + m(2,2) * p.z + m(3,2),
	                 m(0,3) * p.x + m(1,3) * p.y + m(2,3) * p.z + m(3,3)));
}

Vector4D Terathon::operator *(const Matrix4D& m, const Vector2D& v)
{
	return (Vector4D(m(0,0) * v.x + m(0,1) * v.y,
	                 m(1,0) * v.x + m(1,1) * v.y,
	                 m(2,0) * v.x + m(2,1) * v.y,
	                 m(3,0) * v.x + m(3,1) * v.y));
}

Vector4D Terathon::operator *(const Vector2D& v, const Matrix4D& m)
{
	return (Vector4D(m(0,0) * v.x + m(1,0) * v.y,
	                 m(0,1) * v.x + m(1,1) * v.y,
	                 m(0,2) * v.x + m(1,2) * v.y,
	                 m(0,3) * v.x + m(1,3) * v.y));
}

Vector4D Terathon::operator *(const Matrix4D& m, const Point2D& p)
{
	return (Vector4D(m(0,0) * p.x + m(0,1) * p.y + m(0,3),
	                 m(1,0) * p.x + m(1,1) * p.y + m(1,3),
	                 m(2,0) * p.x + m(2,1) * p.y + m(2,3),
	                 m(3,0) * p.x + m(3,1) * p.y + m(3,3)));
}

Vector4D Terathon::operator *(const Point2D& p, const Matrix4D& m)
{
	return (Vector4D(m(0,0) * p.x + m(1,0) * p.y + m(3,0),
	                 m(0,1) * p.x + m(1,1) * p.y + m(3,1),
	                 m(0,2) * p.x + m(1,2) * p.y + m(3,2),
	                 m(0,3) * p.x + m(1,3) * p.y + m(3,3)));
}

real Terathon::Determinant(const Matrix4D& m)
{
	const Vector3D& a = reinterpret_cast<const Vector3D&>(m[0]);
	const Vector3D& b = reinterpret_cast<const Vector3D&>(m[1]);
	const Vector3D& c = reinterpret_cast<const Vector3D&>(m[2]);
	const Vector3D& d = reinterpret_cast<const Vector3D&>(m[3]);

	const real& x = m(3,0);
	const real& y = m(3,1);
	const real& z = m(3,2);
	const real& w = m(3,3);

	Bivector3D s = a ^ b;
	Bivector3D t = c ^ d;
	Vector3D u = a * y - b * x;
	Vector3D v = c * w - d * z;

	return ((s ^ v) + (t ^ u));
}

Matrix4D Terathon::Inverse(const Matrix4D& m)
{
	// See FGED1, Section 1.7.5.

	#if defined(TERATHON_SSE) && !defined(VGEO_DOUBLE_PRECISION)

		Transform3D		result;

		vec_float a = VecLoad(&m(0,0));
		vec_float b = VecLoad(&m(0,1));
		vec_float c = VecLoad(&m(0,2));
		vec_float d = VecLoad(&m(0,3));

		vec_float x = VecSmearW(a);
		vec_float y = VecSmearW(b);
		vec_float z = VecSmearW(c);
		vec_float w = VecSmearW(d);

		vec_float s = VecCross3D(a, b);
		vec_float t = VecCross3D(c, d);
		vec_float u = a * y - b * x;
		vec_float v = c * w - d * z;

		vec_float invDet = VecSmearX(VecDivScalar(VecLoadScalarConstant<0x3F800000>(), VecDot3D(s, v) + VecDot3D(t, u)));

		s = s * invDet;
		t = t * invDet;
		u = u * invDet;
		v = v * invDet;

		vec_float r0 = VecCross3D(b, v) + t * y;
		vec_float r1 = VecCross3D(v, a) - t * x;
		vec_float r2 = VecCross3D(d, u) + s * w;
		vec_float r3 = VecCross3D(u, c) - s * z;

		vec_float h0 = VecShuffle<1,0,1,0>(r0, r1);
		vec_float h1 = VecShuffle<1,0,1,0>(r2, r3);
		vec_float h2 = VecShuffle<3,2,3,2>(r0, r1);
		vec_float h3 = VecShuffle<3,2,3,2>(r2, r3);

		VecStore(VecShuffle<2,0,2,0>(h0, h1), &result(0,0));
		VecStore(VecShuffle<3,1,3,1>(h0, h1), &result(0,1));
		VecStore(VecShuffle<2,0,2,0>(h2, h3), &result(0,2));

		VecStoreX(-VecDot3D(b, t), &result(0,3));
		VecStoreX(VecDot3D(a, t), &result(1,3));
		VecStoreX(-VecDot3D(d, s), &result(2,3));
		VecStoreX(VecDot3D(c, s), &result(3,3));

		return (result);

	#else

		const Vector3D& a = reinterpret_cast<const Vector3D&>(m[0]);
		const Vector3D& b = reinterpret_cast<const Vector3D&>(m[1]);
		const Vector3D& c = reinterpret_cast<const Vector3D&>(m[2]);
		const Vector3D& d = reinterpret_cast<const Vector3D&>(m[3]);

		const real& x = m(3,0);
		const real& y = m(3,1);
		const real& z = m(3,2);
		const real& w = m(3,3);

		Bivector3D s = a ^ b;
		Bivector3D t = c ^ d;
		Vector3D u = a * y - b * x;
		Vector3D v = c * w - d * z;

		real invDet = 1.0F / ((s ^ v) + (t ^ u));

		s *= invDet;
		t *= invDet;
		u *= invDet;
		v *= invDet;

		return (Matrix4D((b ^ v) + t * y, -(b ^ t),
		                 (v ^ a) - t * x,  (a ^ t),
		                 (d ^ u) + s * w, -(d ^ s),
		                 (u ^ c) - s * z,  (c ^ s)));

	#endif
}

Matrix4D Terathon::Adjugate(const Matrix4D& m)
{
	const Vector3D& a = reinterpret_cast<const Vector3D&>(m[0]);
	const Vector3D& b = reinterpret_cast<const Vector3D&>(m[1]);
	const Vector3D& c = reinterpret_cast<const Vector3D&>(m[2]);
	const Vector3D& d = reinterpret_cast<const Vector3D&>(m[3]);

	const real& x = m(3,0);
	const real& y = m(3,1);
	const real& z = m(3,2);
	const real& w = m(3,3);

	Bivector3D s = a ^ b;
	Bivector3D t = c ^ d;
	Vector3D u = a * y - b * x;
	Vector3D v = c * w - d * z;

	return (Matrix4D((b ^ v) + t * y, -(b ^ t),
	                 (v ^ a) - t * x,  (a ^ t),
	                 (d ^ u) + s * w, -(d ^ s),
	                 (u ^ c) - s * z,  (c ^ s)));
}


Transform3D::Transform3D(real n00, real n01, real n02, real n03, real n10, real n11, real n12, real n13, real n20, real n21, real n22, real n23)
{
	m00 = n00;
	m01 = n01;
	m02 = n02;
	m03 = n03;
	m10 = n10;
	m11 = n11;
	m12 = n12;
	m13 = n13;
	m20 = n20;
	m21 = n21;
	m22 = n22;
	m23 = n23;

	row3.Set(0.0F, 0.0F, 0.0F, 1.0F);
}

Transform3D::Transform3D(const Vector3D& a, const Vector3D& b, const Vector3D& c, const Point3D& p)
{
	m00 = a.x;
	m10 = a.y;
	m20 = a.z;
	m01 = b.x;
	m11 = b.y;
	m21 = b.z;
	m02 = c.x;
	m12 = c.y;
	m22 = c.z;
	m03 = p.x;
	m13 = p.y;
	m23 = p.z;

	row3.Set(0.0F, 0.0F, 0.0F, 1.0F);
}

Transform3D::Transform3D(const Bivector3D& r0, real n03, const Bivector3D& r1, real n13, const Bivector3D& r2, real n23)
{
	row0.Set(r0.x, r0.y, r0.z, n03);
	row1.Set(r1.x, r1.y, r1.z, n13);
	row2.Set(r2.x, r2.y, r2.z, n23);
	row3.Set(0.0F, 0.0F, 0.0F, 1.0F);
}

Transform3D::Transform3D(const Matrix3D& m)
{
	m00 = m.m00;
	m10 = m.m10;
	m20 = m.m20;
	m01 = m.m01;
	m11 = m.m11;
	m21 = m.m21;
	m02 = m.m02;
	m12 = m.m12;
	m22 = m.m22;

	m03 = m13 = m23 = 0.0F;
	row3.Set(0.0F, 0.0F, 0.0F, 1.0F);
}

Transform3D::Transform3D(const Matrix3D& m, const Vector3D& v)
{
	m00 = m.m00;
	m10 = m.m10;
	m20 = m.m20;
	m01 = m.m01;
	m11 = m.m11;
	m21 = m.m21;
	m02 = m.m02;
	m12 = m.m12;
	m22 = m.m22;
	m03 = v.x;
	m13 = v.y;
	m23 = v.z;

	row3.Set(0.0F, 0.0F, 0.0F, 1.0F);
}

Transform3D& Transform3D::Set(const Matrix3D& m, const Vector3D& v)
{
	m00 = m.m00;
	m10 = m.m10;
	m20 = m.m20;
	m01 = m.m01;
	m11 = m.m11;
	m21 = m.m21;
	m02 = m.m02;
	m12 = m.m12;
	m22 = m.m22;
	m03 = v.x;
	m13 = v.y;
	m23 = v.z;

	row3.Set(0.0F, 0.0F, 0.0F, 1.0F);
	return (*this);
}

Transform3D& Transform3D::Set(real n00, real n01, real n02, real n03, real n10, real n11, real n12, real n13, real n20, real n21, real n22, real n23)
{
	m00 = n00;
	m01 = n01;
	m02 = n02;
	m03 = n03;
	m10 = n10;
	m11 = n11;
	m12 = n12;
	m13 = n13;
	m20 = n20;
	m21 = n21;
	m22 = n22;
	m23 = n23;

	row3.Set(0.0F, 0.0F, 0.0F, 1.0F);
	return (*this);
}

Transform3D& Transform3D::Set(const Vector3D& a, const Vector3D& b, const Vector3D& c, const Point3D& p)
{
	m00 = a.x;
	m10 = a.y;
	m20 = a.z;
	m01 = b.x;
	m11 = b.y;
	m21 = b.z;
	m02 = c.x;
	m12 = c.y;
	m22 = c.z;
	m03 = p.x;
	m13 = p.y;
	m23 = p.z;

	row3.Set(0.0F, 0.0F, 0.0F, 1.0F);
	return (*this);
}

Transform3D& Transform3D::operator *=(const Transform3D& m)
{
	real x = m00;
	real y = m01;
	real z = m02;
	m00 = x * m.m00 + y * m.m10 + z * m.m20;
	m01 = x * m.m01 + y * m.m11 + z * m.m21;
	m02 = x * m.m02 + y * m.m12 + z * m.m22;
	m03 = x * m.m03 + y * m.m13 + z * m.m23 + m03;

	x = m10;
	y = m11;
	z = m12;
	m10 = x * m.m00 + y * m.m10 + z * m.m20;
	m11 = x * m.m01 + y * m.m11 + z * m.m21;
	m12 = x * m.m02 + y * m.m12 + z * m.m22;
	m13 = x * m.m03 + y * m.m13 + z * m.m23 + m13;

	x = m20;
	y = m21;
	z = m22;
	m20 = x * m.m00 + y * m.m10 + z * m.m20;
	m21 = x * m.m01 + y * m.m11 + z * m.m21;
	m22 = x * m.m02 + y * m.m12 + z * m.m22;
	m23 = x * m.m03 + y * m.m13 + z * m.m23 + m23;

	return (*this);
}

Transform3D& Transform3D::operator *=(const Matrix3D& m)
{
	real x = m00;
	real y = m01;
	real z = m02;
	m00 = x * m.m00 + y * m.m10 + z * m.m20;
	m01 = x * m.m01 + y * m.m11 + z * m.m21;
	m02 = x * m.m02 + y * m.m12 + z * m.m22;

	x = m10;
	y = m11;
	z = m12;
	m10 = x * m.m00 + y * m.m10 + z * m.m20;
	m11 = x * m.m01 + y * m.m11 + z * m.m21;
	m12 = x * m.m02 + y * m.m12 + z * m.m22;

	x = m20;
	y = m21;
	z = m22;
	m20 = x * m.m00 + y * m.m10 + z * m.m20;
	m21 = x * m.m01 + y * m.m11 + z * m.m21;
	m22 = x * m.m02 + y * m.m12 + z * m.m22;

	return (*this);
}

Transform3D& Transform3D::SetMatrix3D(const Vector3D& a, const Vector3D& b, const Vector3D& c)
{
	m00 = a.x;
	m10 = a.y;
	m20 = a.z;
	m01 = b.x;
	m11 = b.y;
	m21 = b.z;
	m02 = c.x;
	m12 = c.y;
	m22 = c.z;

	return (*this);
}

Transform3D& Transform3D::Orthogonalize(int32 column)
{
	Vector3D& x = (*this)[column];
	Vector3D& y = (*this)[(column < 2) ? column + 1 : 0];
	Vector3D& z = (*this)[(column > 0) ? column - 1 : 2];

	x.Normalize();
	y = Normalize(y - x * Dot(x, y));
	z = Normalize(z - x * Dot(x, z) - y * Dot(y, z));

	return (*this);
}

void Transform3D::GetEulerAngles(real *x, real *y, real *z) const
{
	real sy = m02;
	if (sy < 1.0F)
	{
		if (sy > -1.0F)
		{
			*x = -Arctan(m12, m22);
			*y = Arcsin(sy);
			*z = -Arctan(m01, m00);
		}
		else
		{
			*x = 0.0F;
			*y = -Math::tau_over_4;
			*z = Arctan(m10, m11);
		}
	}
	else
	{
		*x = 0.0F;
		*y = Math::tau_over_4;
		*z = Arctan(m10, m11);
	}
}

Transform3D& Transform3D::SetEulerAngles(real x, real y, real z)
{
	Vector2D xrot = CosSin(x);
	Vector2D yrot = CosSin(y);
	Vector2D zrot = CosSin(z);

	m00 = yrot.x * zrot.x;
	m10 = xrot.x * zrot.y + xrot.y * yrot.y * zrot.x;
	m20 = xrot.y * zrot.y - xrot.x * yrot.y * zrot.x;
	m01 = -yrot.x * zrot.y;
	m11 = xrot.x * zrot.x - xrot.y * yrot.y * zrot.y;
	m21 = xrot.y * zrot.x + xrot.x * yrot.y * zrot.y;
	m02 = yrot.y;
	m12 = -xrot.y * yrot.x;
	m22 = xrot.x * yrot.x;

	m03 = m13 = m23 = 0.0F;
	row3.Set(0.0F, 0.0F, 0.0F, 1.0F);
	return (*this);
}

Transform3D Transform3D::MakeRotationX(real angle)
{
	Vector2D v = CosSin(angle);

	return (Transform3D(1.0F, 0.0F, 0.0F, 0.0F,
	                    0.0F,  v.x, -v.y, 0.0F,
	                    0.0F,  v.y,  v.x, 0.0F));
}

Transform3D Transform3D::MakeRotationY(real angle)
{
	Vector2D v = CosSin(angle);

	return (Transform3D( v.x, 0.0F,  v.y, 0.0F,
	                    0.0F, 1.0F, 0.0F, 0.0F,
	                    -v.y, 0.0F,  v.x, 0.0F));
}

Transform3D Transform3D::MakeRotationZ(real angle)
{
	Vector2D v = CosSin(angle);

	return (Transform3D( v.x, -v.y, 0.0F, 0.0F,
	                     v.y,  v.x, 0.0F, 0.0F,
	                    0.0F, 0.0F, 1.0F, 0.0F));
}

Transform3D Transform3D::MakeRotation(real angle, const Bivector3D& axis)
{
	Vector2D v = CosSin(angle);
	const real& c = v.x;
	const real& s = v.y;
	real d = 1.0F - c;

	real x = axis.x * d;
	real y = axis.y * d;
	real z = axis.z * d;
	real axay = x * axis.y;
	real axaz = x * axis.z;
	real ayaz = y * axis.z;

	return (Transform3D(c + x * axis.x, axay - s * axis.z, axaz + s * axis.y, 0.0F,
	                    axay + s * axis.z, c + y * axis.y, ayaz - s * axis.x, 0.0F,
	                    axaz - s * axis.y, ayaz + s * axis.x, c + z * axis.z, 0.0F));
}

Transform3D Transform3D::MakeReflection(const Vector3D& a)
{
	real x = a.x * -2.0F;
	real y = a.y * -2.0F;
	real z = a.z * -2.0F;
	real axay = x * a.y;
	real axaz = x * a.z;
	real ayaz = y * a.z;

	return (Transform3D(x * a.x + 1.0F, axay, axaz, 0.0F,
	                    axay, y * a.y + 1.0F, ayaz, 0.0F,
	                    axaz, ayaz, z * a.z + 1.0F, 0.0F));
}

Transform3D Transform3D::MakeInvolution(const Vector3D& a)
{
	real x = a.x * 2.0F;
	real y = a.y * 2.0F;
	real z = a.z * 2.0F;
	real axay = x * a.y;
	real axaz = x * a.z;
	real ayaz = y * a.z;

	return (Transform3D(x * a.x - 1.0F, axay, axaz, 0.0F,
	                    axay, y * a.y - 1.0F, ayaz, 0.0F,
	                    axaz, ayaz, z * a.z - 1.0F, 0.0F));
}

Transform3D Transform3D::MakeReflection(const Plane3D& plane)
{
	real x = plane.x * -2.0F;
	real y = plane.y * -2.0F;
	real z = plane.z * -2.0F;
	real nxny = x * plane.y;
	real nxnz = x * plane.z;
	real nynz = y * plane.z;

	return (Transform3D(x * plane.x + 1.0F, nxny, nxnz, x * plane.w,
	                    nxny, y * plane.y + 1.0F, nynz, y * plane.w,
	                    nxnz, nynz, z * plane.z + 1.0F, z * plane.w));
}

Transform3D Transform3D::MakeScale(real scale)
{
	return (Transform3D(scale, 0.0F, 0.0F, 0.0F, 0.0F, scale, 0.0F, 0.0F, 0.0F, 0.0F, scale, 0.0F));
}

Transform3D Transform3D::MakeScale(real sx, real sy, real sz)
{
	return (Transform3D(sx, 0.0F, 0.0F, 0.0F, 0.0F, sy, 0.0F, 0.0F, 0.0F, 0.0F, sz, 0.0F));
}

Transform3D Transform3D::MakeScaleX(real sx)
{
	return (Transform3D(sx, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F));
}

Transform3D Transform3D::MakeScaleY(real sy)
{
	return (Transform3D(1.0F, 0.0F, 0.0F, 0.0F, 0.0F, sy, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F));
}

Transform3D Transform3D::MakeScaleZ(real sz)
{
	return (Transform3D(1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F, 0.0F, sz, 0.0F));
}

Transform3D Transform3D::MakeScale(real scale, const Vector3D& a)
{
	scale -= 1.0F;
	real x = a.x * scale;
	real y = a.y * scale;
	real z = a.z * scale;
	real axay = x * a.y;
	real axaz = x * a.z;
	real ayaz = y * a.z;

	return (Transform3D(x * a.x + 1.0F, axay, axaz, 0.0F,
	                    axay, y * a.y + 1.0F, ayaz, 0.0F,
	                    axaz, ayaz, z * a.z + 1.0F, 0.0F));
}

Transform3D Transform3D::MakeSkew(real angle, const Vector3D& a, const Vector3D& b)
{
	real t = Tan(angle);
	real x = a.x * t;
	real y = a.y * t;
	real z = a.z * t;

	return (Transform3D(x * b.x + 1.0F, x * b.y, x * b.z, 0.0F,
	                    y * b.x, y * b.y + 1.0F, y * b.z, 0.0F,
	                    z * b.x, z * b.y, z * b.z + 1.0F, 0.0F));
}

Transform3D Transform3D::MakeTranslation(const Vector3D& dv)
{
	return (Transform3D(1.0F, 0.0F, 0.0F, dv.x, 0.0F, 1.0F, 0.0F, dv.y, 0.0F, 0.0F, 1.0F, dv.z));
}


Transform3D Terathon::operator *(const Transform3D& m1, const Transform3D& m2)
{
	#ifndef TERATHON_NO_SIMD

		Transform3D		result;

		vec_float a = VecLoad(&m1(0,0));
		vec_float b = VecLoad(&m1(0,1));
		vec_float c = VecLoad(&m1(0,2));
		VecStore(VecTransformVector3D(a, b, c, VecLoad(&m2(0,0))), &result(0,0));
		VecStore(VecTransformVector3D(a, b, c, VecLoad(&m2(0,1))), &result(0,1));
		VecStore(VecTransformVector3D(a, b, c, VecLoad(&m2(0,2))), &result(0,2));
		VecStore(VecTransformPoint3D(a, b, c, VecLoad(&m1(0,3)), VecLoad(&m2(0,3))), &result(0,3));

		return (result);

	#else

		return (Transform3D(m1(0,0) * m2(0,0) + m1(0,1) * m2(1,0) + m1(0,2) * m2(2,0),
		                    m1(0,0) * m2(0,1) + m1(0,1) * m2(1,1) + m1(0,2) * m2(2,1),
		                    m1(0,0) * m2(0,2) + m1(0,1) * m2(1,2) + m1(0,2) * m2(2,2),
		                    m1(0,0) * m2(0,3) + m1(0,1) * m2(1,3) + m1(0,2) * m2(2,3) + m1(0,3),
		                    m1(1,0) * m2(0,0) + m1(1,1) * m2(1,0) + m1(1,2) * m2(2,0),
		                    m1(1,0) * m2(0,1) + m1(1,1) * m2(1,1) + m1(1,2) * m2(2,1),
		                    m1(1,0) * m2(0,2) + m1(1,1) * m2(1,2) + m1(1,2) * m2(2,2),
		                    m1(1,0) * m2(0,3) + m1(1,1) * m2(1,3) + m1(1,2) * m2(2,3) + m1(1,3),
		                    m1(2,0) * m2(0,0) + m1(2,1) * m2(1,0) + m1(2,2) * m2(2,0),
		                    m1(2,0) * m2(0,1) + m1(2,1) * m2(1,1) + m1(2,2) * m2(2,1),
		                    m1(2,0) * m2(0,2) + m1(2,1) * m2(1,2) + m1(2,2) * m2(2,2),
		                    m1(2,0) * m2(0,3) + m1(2,1) * m2(1,3) + m1(2,2) * m2(2,3) + m1(2,3)));

	#endif
}

Matrix4D Terathon::operator *(const Matrix4D& m1, const Transform3D& m2)
{
	#ifndef TERATHON_NO_SIMD

		Matrix4D	result;

		vec_float a = VecLoad(&m1(0,0));
		vec_float b = VecLoad(&m1(0,1));
		vec_float c = VecLoad(&m1(0,2));
		VecStore(VecTransformVector3D(a, b, c, VecLoad(&m2(0,0))), &result(0,0));
		VecStore(VecTransformVector3D(a, b, c, VecLoad(&m2(0,1))), &result(0,1));
		VecStore(VecTransformVector3D(a, b, c, VecLoad(&m2(0,2))), &result(0,2));
		VecStore(VecTransformPoint3D(a, b, c, VecLoad(&m1(0,3)), VecLoad(&m2(0,3))), &result(0,3));

		return (result);

	#else

		return (Matrix4D(m1(0,0) * m2(0,0) + m1(0,1) * m2(1,0) + m1(0,2) * m2(2,0),
		                 m1(0,0) * m2(0,1) + m1(0,1) * m2(1,1) + m1(0,2) * m2(2,1),
		                 m1(0,0) * m2(0,2) + m1(0,1) * m2(1,2) + m1(0,2) * m2(2,2),
		                 m1(0,0) * m2(0,3) + m1(0,1) * m2(1,3) + m1(0,2) * m2(2,3) + m1(0,3),
		                 m1(1,0) * m2(0,0) + m1(1,1) * m2(1,0) + m1(1,2) * m2(2,0),
		                 m1(1,0) * m2(0,1) + m1(1,1) * m2(1,1) + m1(1,2) * m2(2,1),
		                 m1(1,0) * m2(0,2) + m1(1,1) * m2(1,2) + m1(1,2) * m2(2,2),
		                 m1(1,0) * m2(0,3) + m1(1,1) * m2(1,3) + m1(1,2) * m2(2,3) + m1(1,3),
		                 m1(2,0) * m2(0,0) + m1(2,1) * m2(1,0) + m1(2,2) * m2(2,0),
		                 m1(2,0) * m2(0,1) + m1(2,1) * m2(1,1) + m1(2,2) * m2(2,1),
		                 m1(2,0) * m2(0,2) + m1(2,1) * m2(1,2) + m1(2,2) * m2(2,2),
		                 m1(2,0) * m2(0,3) + m1(2,1) * m2(1,3) + m1(2,2) * m2(2,3) + m1(2,3),
		                 m1(3,0) * m2(0,0) + m1(3,1) * m2(1,0) + m1(3,2) * m2(2,0),
		                 m1(3,0) * m2(0,1) + m1(3,1) * m2(1,1) + m1(3,2) * m2(2,1),
		                 m1(3,0) * m2(0,2) + m1(3,1) * m2(1,2) + m1(3,2) * m2(2,2),
		                 m1(3,0) * m2(0,3) + m1(3,1) * m2(1,3) + m1(3,2) * m2(2,3) + m1(3,3)));

	#endif
}

Matrix4D Terathon::operator *(const Transform3D& m1, const Matrix4D& m2)
{
	return (Matrix4D(m1(0,0) * m2(0,0) + m1(0,1) * m2(1,0) + m1(0,2) * m2(2,0) + m1(0,3) * m2(3,0),
	                 m1(0,0) * m2(0,1) + m1(0,1) * m2(1,1) + m1(0,2) * m2(2,1) + m1(0,3) * m2(3,1),
	                 m1(0,0) * m2(0,2) + m1(0,1) * m2(1,2) + m1(0,2) * m2(2,2) + m1(0,3) * m2(3,2),
	                 m1(0,0) * m2(0,3) + m1(0,1) * m2(1,3) + m1(0,2) * m2(2,3) + m1(0,3) * m2(3,3),
	                 m1(1,0) * m2(0,0) + m1(1,1) * m2(1,0) + m1(1,2) * m2(2,0) + m1(1,3) * m2(3,0),
	                 m1(1,0) * m2(0,1) + m1(1,1) * m2(1,1) + m1(1,2) * m2(2,1) + m1(1,3) * m2(3,1),
	                 m1(1,0) * m2(0,2) + m1(1,1) * m2(1,2) + m1(1,2) * m2(2,2) + m1(1,3) * m2(3,2),
	                 m1(1,0) * m2(0,3) + m1(1,1) * m2(1,3) + m1(1,2) * m2(2,3) + m1(1,3) * m2(3,3),
	                 m1(2,0) * m2(0,0) + m1(2,1) * m2(1,0) + m1(2,2) * m2(2,0) + m1(2,3) * m2(3,0),
	                 m1(2,0) * m2(0,1) + m1(2,1) * m2(1,1) + m1(2,2) * m2(2,1) + m1(2,3) * m2(3,1),
	                 m1(2,0) * m2(0,2) + m1(2,1) * m2(1,2) + m1(2,2) * m2(2,2) + m1(2,3) * m2(3,2),
	                 m1(2,0) * m2(0,3) + m1(2,1) * m2(1,3) + m1(2,2) * m2(2,3) + m1(2,3) * m2(3,3),
	                 m2(3,0), m2(3,1), m2(3,2), m2(3,3)));
}

Vector3D Terathon::operator *(const Transform3D& m, const Vector3D& v)
{
	#ifndef TERATHON_NO_SIMD

		Vector3D	result;

		VecStore3D(VecTransformVector3D(VecLoad(&m(0,0)), VecLoad(&m(0,1)), VecLoad(&m(0,2)), VecLoadUnaligned(&v.x)), &result.x);
		return (result);

	#else

		return (m * v.xyz);

	#endif
}

Bivector3D Terathon::operator *(const Bivector3D& v, const Transform3D& m)
{
	return (v.xyz * m);
}

Point3D Terathon::operator *(const Transform3D& m, const Point3D& p)
{
	#ifndef TERATHON_NO_SIMD

		Point3D		result;

		VecStore3D(VecTransformPoint3D(VecLoad(&m(0,0)), VecLoad(&m(0,1)), VecLoad(&m(0,2)), VecLoad(&m(0,3)), VecLoadUnaligned(&p.x)), &result.x);
		return (result);

	#else

		return (Point3D(m(0,0) * p.x + m(0,1) * p.y + m(0,2) * p.z + m(0,3),
		                m(1,0) * p.x + m(1,1) * p.y + m(1,2) * p.z + m(1,3),
		                m(2,0) * p.x + m(2,1) * p.y + m(2,2) * p.z + m(2,3)));

	#endif
}

Vector2D Terathon::operator *(const Transform3D& m, const Vector2D& v)
{
	return (Vector2D(m(0,0) * v.x + m(0,1) * v.y,
	                 m(1,0) * v.x + m(1,1) * v.y));
}

Point2D Terathon::operator *(const Transform3D& m, const Point2D& p)
{
	return (Point2D(m(0,0) * p.x + m(0,1) * p.y + m(0,3),
	                m(1,0) * p.x + m(1,1) * p.y + m(1,3)));
}

Transform3D Terathon::Scale(const Transform3D& m, const Vector3D& v)
{
	return (Transform3D(m(0,0) * v.x, m(0,1) * v.x, m(0,2) * v.x, m(0,3) * v.x,
	                    m(1,0) * v.y, m(1,1) * v.y, m(1,2) * v.y, m(1,3) * v.y,
	                    m(2,0) * v.z, m(2,1) * v.z, m(2,2) * v.z, m(2,3) * v.z));
}

Matrix3D Terathon::Transform(const Transform3D& m1, const Matrix3D& m2)
{
	return (Matrix3D(m1(0,0) * m2(0,0) + m1(0,1) * m2(1,0) + m1(0,2) * m2(2,0),
	                 m1(0,0) * m2(0,1) + m1(0,1) * m2(1,1) + m1(0,2) * m2(2,1),
	                 m1(0,0) * m2(0,2) + m1(0,1) * m2(1,2) + m1(0,2) * m2(2,2),
	                 m1(1,0) * m2(0,0) + m1(1,1) * m2(1,0) + m1(1,2) * m2(2,0),
	                 m1(1,0) * m2(0,1) + m1(1,1) * m2(1,1) + m1(1,2) * m2(2,1),
	                 m1(1,0) * m2(0,2) + m1(1,1) * m2(1,2) + m1(1,2) * m2(2,2),
	                 m1(2,0) * m2(0,0) + m1(2,1) * m2(1,0) + m1(2,2) * m2(2,0),
	                 m1(2,0) * m2(0,1) + m1(2,1) * m2(1,1) + m1(2,2) * m2(2,1),
	                 m1(2,0) * m2(0,2) + m1(2,1) * m2(1,2) + m1(2,2) * m2(2,2)));
}

#ifndef TERATHON_NO_SIMD

	vec_float Terathon::TransformVector3D(const Transform3D& m, vec_float v)
	{
		return (VecTransformVector3D(VecLoad(&m(0,0)), VecLoad(&m(0,1)), VecLoad(&m(0,2)), v));
	}

	vec_float Terathon::TransformPoint3D(const Transform3D& m, vec_float p)
	{
		return (VecTransformPoint3D(VecLoad(&m(0,0)), VecLoad(&m(0,1)), VecLoad(&m(0,2)), VecLoad(&m(0,3)), p));
	}

#endif


real Terathon::Determinant(const Transform3D& m)
{
	return (m(0,0) * (m(1,1) * m(2,2) - m(1,2) * m(2,1)) - m(0,1) * (m(1,0) * m(2,2) - m(1,2) * m(2,0)) + m(0,2) * (m(1,0) * m(2,1) - m(1,1) * m(2,0)));
}

Transform3D Terathon::Inverse(const Transform3D& m)
{
	// See FGED1, Section 2.6.

	#if defined(TERATHON_SSE) && !defined(VGEO_DOUBLE_PRECISION)

		Transform3D		result;

		vec_float a = VecLoad(&m(0,0));
		vec_float b = VecLoad(&m(0,1));
		vec_float c = VecLoad(&m(0,2));
		vec_float d = VecLoad(&m(0,3));

		vec_float t0 = VecShuffle<1,0,1,0>(a, b);
		vec_float t1 = VecShuffle<1,0,1,0>(c, d);
		vec_float t2 = VecShuffle<3,2,3,2>(a, b);
		vec_float t3 = VecShuffle<3,2,3,2>(c, d);

		a = VecShuffle<2,0,2,0>(t0, t1);
		b = VecShuffle<3,1,3,1>(t0, t1);
		c = VecShuffle<2,0,2,0>(t2, t3);

		vec_float x = VecSmearW(a);
		vec_float y = VecSmearW(b);
		vec_float z = VecSmearW(c);

		vec_float s = VecCross3D(a, b);
		vec_float u = a * y - b * x;

		vec_float invDet = VecSmearX(VecDivScalar(VecLoadScalarConstant<0x3F800000>(), VecDot3D(s, c)));

		s = s * invDet;
		u = u * invDet;
		vec_float v = c * invDet;

		VecStore(VecCross3D(b, v), &result(0,0));
		VecStore(VecCross3D(v, a), &result(0,1));
		VecStore(s, &result(0,2));
		VecStore(VecCross3D(u, c) - s * z, &result(0,3));

		result(3,0) = result(3,1) = result(3,2) = 0.0F;
		result(3,3) = 1.0F;
		return (result);

	#else

		const Vector3D& a = m[0];
		const Vector3D& b = m[1];
		const Vector3D& c = m[2];
		const Vector3D& d = m[3];

		Bivector3D s = a ^ b;
		Bivector3D t = c ^ d;

		real invDet = 1.0F / (s ^ c);

		s *= invDet;
		t *= invDet;
		Vector3D v = c * invDet;

		return (Transform3D(b ^ v, -(b ^ t), v ^ a, a ^ t, s, -(d ^ s)));

	#endif
}

Transform3D Terathon::InverseUnitDet(const Transform3D& m)
{
	#if defined(TERATHON_SSE) && !defined(VGEO_DOUBLE_PRECISION)

		Transform3D		result;

		vec_float a = VecLoad(&m(0,0));
		vec_float b = VecLoad(&m(0,1));
		vec_float c = VecLoad(&m(0,2));
		vec_float d = VecLoad(&m(0,3));

		vec_float t0 = VecShuffle<1,0,1,0>(a, b);
		vec_float t1 = VecShuffle<1,0,1,0>(c, d);
		vec_float t2 = VecShuffle<3,2,3,2>(a, b);
		vec_float t3 = VecShuffle<3,2,3,2>(c, d);

		vec_float r0 = VecShuffle<2,0,2,0>(t0, t1);
		vec_float r1 = VecShuffle<3,1,3,1>(t0, t1);
		vec_float r2 = VecShuffle<2,0,2,0>(t2, t3);

		vec_float s = VecCross3D(r0, r1);
		vec_float h = r0 * VecSmearW(r1) - r1 * VecSmearW(r0);

		VecStore(VecCross3D(r1, r2), &result(0,0));
		VecStore(VecCross3D(r2, r0), &result(0,1));
		VecStore(s, &result(0,2));
		VecStore(VecCross3D(h, r2) - s * VecSmearW(r2), &result(0,3));

		result(3,0) = result(3,1) = result(3,2) = 0.0F;
		result(3,3) = 1.0F;
		return (result);

	#else

		const Vector3D& a = m[0];
		const Vector3D& b = m[1];
		const Vector3D& c = m[2];
		const Vector3D& d = m[3];

		Bivector3D s = a ^ b;
		Bivector3D t = c ^ d;

		return (Transform3D(b ^ c, -(b ^ t), c ^ a, a ^ t, s, -(d ^ s)));

	#endif
}

Matrix3D Terathon::InverseUnitDet3D(const Transform3D& m)
{
	const Vector3D& a = m[0];
	const Vector3D& b = m[1];
	const Vector3D& c = m[2];

	Bivector3D g0 = b ^ c;
	Bivector3D g1 = c ^ a;
	Bivector3D g2 = a ^ b;

	return (Matrix3D(g0.x, g0.y, g0.z, g1.x, g1.y, g1.z, g2.x, g2.y, g2.z));
}

Vector3D Terathon::InverseTransform(const Transform3D& m, const Vector3D& v)
{
	const Vector3D& a = m[0];
	const Vector3D& b = m[1];
	const Vector3D& c = m[2];

	Bivector3D s = a ^ b;
	real invDet = 1.0F / (s ^ c);

	return (Vector3D((b ^ c ^ v) * invDet, (c ^ a ^ v) * invDet, (s ^ v) * invDet));
}

Point3D Terathon::InverseTransform(const Transform3D& m, const Point3D& p)
{
	const Vector3D& a = m[0];
	const Vector3D& b = m[1];
	const Vector3D& c = m[2];
	const Vector3D& d = m[3];

	Bivector3D s = a ^ b;
	real invDet = 1.0F / (s ^ c);

	Vector3D q = p - d;
	return (Point3D((b ^ c ^ q) * invDet, (c ^ a ^ q) * invDet, (s ^ q) * invDet));
}

Vector3D Terathon::InverseUnitDetTransform(const Transform3D& m, const Vector3D& v)
{
	const Vector3D& a = m[0];
	const Vector3D& b = m[1];
	const Vector3D& c = m[2];

	return (Vector3D(b ^ c ^ v, c ^ a ^ v, a ^ b ^ v));
}

Point3D Terathon::InverseUnitDetTransform(const Transform3D& m, const Point3D& p)
{
	const Vector3D& a = m[0];
	const Vector3D& b = m[1];
	const Vector3D& c = m[2];
	const Vector3D& d = m[3];

	Vector3D q = p - d;
	return (Point3D(b ^ c ^ q, c ^ a ^ q, a ^ b ^ q));
}
