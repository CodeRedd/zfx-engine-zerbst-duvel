//File: ZFX3D_vector.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFX3D.h"

extern bool g_bSSE;

//Absolute value operation for floats
float _fabs(float f)
{
	if (f < 0.0f)
	{
		return -f;
	}
	return f;
}

//C++ Functions
inline void ZFXVector::Set(float _x, float _y, float _z, float _w /*=1.0f*/)
{
	x = _x;
	y = _y;
	z = _z;
	w = _w;
}

inline float ZFXVector::GetSqrLength()
{
	return (x*x + y*y + z*z);
}

inline void ZFXVector::Negate()
{
	x = -x;
	y = -y;
	z = -z;
}

inline void ZFXVector::Difference(const ZFXVector &u, const ZFXVector &v)
{
	x = v.x - u.x;
	y = v.y - u.y;
	z = v.z - u.z; 
	w = 1.0f;
}

inline float ZFXVector::AngleBetween(ZFXVector &v)
{
	return (float)acos(((*this) * v) / (this->GetLength()*v.GetLength()));
}

inline void ZFXVector::RotateWith(const ZFXMatrix &m) {
	// applying rotational part of matrix only
	float _x = x * m._11 + y * m._21 + z * m._31;
	float _y = x * m._12 + y * m._22 + z * m._32;
	float _z = x * m._13 + y * m._23 + z * m._33;
	x = _x;   y = _y;   z = _z;
}

inline void ZFXVector::InvRotateWith(const ZFXMatrix &m) {
	// using transposed matrix
	float _x = x * m._11 + y * m._12 + z * m._13;
	float _y = x * m._21 + y * m._22 + z * m._23;
	float _z = x * m._31 + y * m._32 + z * m._33;
	x = _x;   y = _y;   z = _z;
}

//Assembly Functions
inline float ZFXVector::GetLength()
{
	float f;

	if (!g_bSSE)
	{
		f = (float)sqrt(GetSqrLength());
	}
	else
	{
		float *pf = &f;
		w = 0.0f; //zero this out temporarily so that it doesn't get added as a 4th dimension to the length of the vector
		__asm
		{
			mov		ecx,   pf		; point to result register
			mov		esi,   this		; vector U
			movups  xmm0,  [esi]	; vector U in XMM0
			mulps	xmm0,  xmm0		; square U
			movaps	xmm1,  xmm0		; copy result
			shufps	xmm1,  xmm1, 4Eh; shuffle: z^2, w^2, x^2, y^2
			addps	xmm0,  xmm1		; add: x^2 + z^2, y^2 + w^2, z^2 + x^2, w^2 + y^2
			movaps	xmm1,  xmm0		; copy result
			shufps	xmm1,  xmm1, 11h; shuffle: w^2 + y^2, z^2 + x^2, y^2 + w^2, x^2 + z^2
			addps	xmm0,  xmm1		; add: x^2 + y^2 + z^2 + w^2, x^2 + y^2 + z^2 + w^2, x^2 + y^2 + z^2 + w^2, x^2 + y^2 + z^2 + w^2
			sqrtss	xmm0,  xmm0		; square root on the first of the four values
			movss	[ecx], xmm0		; move result to ECX which is the pointer to f
		}
		w = 1.0f;
	}

	return f;
}

inline void ZFXVector::Normalize()
{
	if (!g_bSSE)
	{
		float f = (float)sqrt(GetSqrLength());
		if (f != 0.0f)
		{
			x /= f;
			y /= f;
			z /= f;
		}
	}
	else
	{
		w = 0.0f; //zero this out temporarily so that it doesn't get added as a 4th dimension to the length of the vector
		__asm
		{
			mov		esi, this
			movups  xmm0, [esi]
			movaps	xmm2, xmm0
			mulps	xmm0, xmm0
			movaps  xmm1, xmm0
			shufps	xmm1, xmm1, 4Eh
			addps	xmm0, xmm1
			movaps	xmm1, xmm0
			shufps	xmm1, xmm1, 11h
			addps	xmm0, xmm1

			rsqrtps xmm0, xmm0 ; reciprocal of the square root
			mulps	xmm2, xmm0 ; multiply by original vector
			movups	[esi], xmm0
		}
		w = 1.0f;
	}
}

inline void ZFXVector::Cross(const ZFXVector &u, const ZFXVector &v)
{
	if (!g_bSSE)
	{
		x = u.y * v.z - u.z * v.y;
		y = u.z * v.x - u.x * v.z;
		z = u.x * v.y - u.y * v.x;
		w = 1.0f;
	}
	else
	{
		__asm
		{
			mov esi, u
			mov edi, v

			movups xmm0, [esi]
			movups xmm1, [edi]
			movaps xmm2, xmm0
			movaps xmm3, xmm1

			shufps xmm0, xmm0, 0xC9
			shufps xmm1, xmm1, 0xD2
			mulps  xmm0, xmm1

			shufps xmm2, xmm2, 0xD2
			shufps xmm3, xmm3, 0xC9
			mulps  xmm2, xmm3

			subps  xmm0, xmm2

			mov    esi, this
			movups [esi], xmm0
		}
		w = 1.0f;
	}
}

//Operator Overloads
void ZFXVector::operator += (const ZFXVector &v) {
	x += v.x;   y += v.y;   z += v.z;
}

ZFXVector ZFXVector::operator + (const ZFXVector &v) const {
	return ZFXVector(x + v.x, y + v.y, z + v.z);
}

void ZFXVector::operator -= (const ZFXVector &v) {
	x -= v.x;   y -= v.y;   z -= v.z;
}

ZFXVector ZFXVector::operator - (const ZFXVector &v) const {
	return ZFXVector(x - v.x, y - v.y, z - v.z);
}

void ZFXVector::operator *= (float f) {
	x *= f;   y *= f;   z *= f;
}

void ZFXVector::operator /= (float f) {
	x /= f;   y /= f;   z /= f;
}

ZFXVector ZFXVector::operator * (float f) const {
	return ZFXVector(x*f, y*f, z*f);
}

ZFXVector ZFXVector::operator / (float f) const {
	return ZFXVector(x / f, y / f, z / f);
}

void ZFXVector::operator += (float f) {
	x += f;   y += f;   z += f;
}

void ZFXVector::operator -= (float f) {
	x -= f;   y -= f;   z -= f;
}

ZFXVector ZFXVector::operator + (float f) const {
	return ZFXVector(x + f, y + f, z + f);
}

ZFXVector ZFXVector::operator - (float f) const {
	return ZFXVector(x - f, y - f, z - f);
}


float ZFXVector::operator * (const ZFXVector &v) const {
	return (v.x*x + v.y*y + v.z*z);
}

/*ZFXQuat ZFXVector::operator * (const ZFXQuat &q) const {
	return ZFXQuat(q.w*x + q.z*y - q.y*z,
		q.w*y + q.x*z - q.z*x,
		q.w*z + q.y*x - q.x*y,
		-(q.x*x + q.y*y + q.z*z));
}*/

ZFXVector ZFXVector::operator * (const ZFXMatrix &m) const
{
	ZFXVector vcResult;

	if (!g_bSSE)
	{
		vcResult.x = x*m._11 + y*m._21 + z*m._31 + m._41;
		vcResult.y = x*m._12 + y*m._22 + z*m._32 + m._42;
		vcResult.z = x*m._13 + y*m._23 + z*m._33 + m._43;
		vcResult.w = x*m._14 + y*m._24 + z*m._34 + m._44;

		vcResult.x = vcResult.x / vcResult.w;
		vcResult.y = vcResult.y / vcResult.w;
		vcResult.z = vcResult.z / vcResult.w;
		vcResult.w = 1.0f;
	}
	else
	{
		float *ptrRet = (float*)&vcResult;
		__asm
		{
			mov    ecx,  this		; vector
			mov    edx,   m			; matrix
			movss  xmm0,  [ecx]
			mov    eax,   ptrRet	; result vector
			shufps xmm0,  xmm0, 0
			movss  xmm1,  [ecx+4]
			mulps  xmm0,  [edx]
			shufps xmm1,  xmm1, 0
			movss  xmm2,  [ecx+8]
			mulps  xmm1,  [edx+16]
			shufps xmm2,  xmm2, 0
			movss  xmm3,  [ecx+12]
			mulps  xmm2,  [edx+32]
			shufps xmm3,  xmm3, 0
			addps  xmm0,  xmm1
			mulps  xmm3,  [edx+48]
			addps  xmm2,  xmm3
			addps  xmm0,  xmm2
			movups [eax], xmm0		; store as result 
			mov    [eax+3], 1		; w = 1
		}
	}

	return vcResult;
}
