//File: ZFX3D_obb.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFX3D.h"

extern bool g_bSSE;

inline void ZFXOBB::DeTransform(const ZFXOBB &obb, const ZFXMatrix &m)
{
	ZFXMatrix mat = m;
	ZFXVector vcT;

	//apply translation and delete from matrix
	vcT.Set(mat._41, mat._42, mat._43);
	mat._41 = mat._42 = mat._43 = 0.0f;

	//rotate vectors
	this->vcCenter	= mat * obb.vcCenter;
	this->vcA0		= mat * obb.vcA0;
	this->vcA1		= mat * obb.vcA1;
	this->vcA2		= mat * obb.vcA2;

	//translate center point
	this->vcCenter += vcT;

	fA0 = obb.fA0;
	fA1 = obb.fA1;
	fA2 = obb.fA2;
}

//assumes the frustum normals point outward
int ZFXOBB::Cull(const ZFXPlane *pPlanes, int nNumPlanes)
{
	ZFXVector	vN;
	int			nResult = ZFXVISIBLE;
	float		fRadius, fTest;

	//for all planes
	for (int i = 0; i < nNumPlanes; i++)
	{
		//bend normals inward
		vN = pPlanes[i].m_vcN * -1.0f;

		//calculate box radius
		fRadius = _fabs(fA0 * (vN * vcA0))
				 +_fabs(fA1 * (vN * vcA1))
				 +_fabs(fA2 * (vN * vcA2));

		//reference value: (N*C - d) (#)
		fTest = vN * this->vcCenter - pPlanes[i].m_fD;

		//obb on far side of plane: (#) < -r
		if (fTest < -fRadius)
		{
			return ZFXCULLED;
		}
		//or intersecting plane
		else if (!(fTest < fRadius))
		{
			nResult = ZFXCLIPPED;
		}
	}

	return nResult;
}

//Triangle Intersection and Helpers
//These two functions follow the separation axis algorithm as described by David Eberly

bool ZFXOBB::Intersects(const ZFXVector &vc0, const ZFXVector &vc1, const ZFXVector &vc2)
{
	float		fMin0, fMax0, fMin1, fMax1;
	float		fD_C;
	ZFXVector	vcV, vcTriEdge[3], vcA[3];

	//enable looping through vcA's
	vcA[0] = this->vcA0;
	vcA[1] = this->vcA1;
	vcA[2] = this->vcA2;

	//direction of tri normals
	vcTriEdge[0] = vc1 - vc0;
	vcTriEdge[1] = vc2 - vc0;

	vcV.Cross(vcTriEdge[0], vcTriEdge[1]);

	fMin0 = vcV * vc0;
	fMax0 = fMin0;

	this->ObbProj((*this), vcV, &fMin1, &fMax1);
	if (fMax1 < fMin0 || fMax0 < fMin1)
	{
		return false;
	}

	//direction of obb planes
	//axis 1:
	vcV = this->vcA0;
	this->TriProj(vc0, vc1, vc2, vcV, &fMin0, &fMax0);
	fD_C = vcV * this->vcCenter;
	fMin1 = fD_C - this->fA0;
	fMax1 = fD_C + this->fA0;
	if (fMax1 < fMin0 || fMax0 < fMin1)
	{
		return false;
	}
	
	//axis 2:
	vcV = this->vcA1;
	this->TriProj(vc0, vc1, vc2, vcV, &fMin0, &fMax0);
	fD_C = vcV * this->vcCenter;
	fMin1 = fD_C - this->fA1;
	fMax1 = fD_C + this->fA1;
	if (fMax1 < fMin0 || fMax0 < fMin1)
	{
		return false;
	}

	//axis 3:
	vcV = this->vcA2;
	this->TriProj(vc0, vc1, vc2, vcV, &fMin0, &fMax0);
	fD_C = vcV * this->vcCenter;
	fMin1 = fD_C - this->fA2;
	fMax1 = fD_C + this->fA2;
	if (fMax1 < fMin0 || fMax0 < fMin1)
	{
		return false;
	}

	//direction of tri-obb-edge cross products
	vcTriEdge[2] = vcTriEdge[1] - vcTriEdge[0];
	for (int j = 0; j < 3; j++)
	{
		for (int k = 0; k < 3; k++)
		{
			vcV.Cross(vcTriEdge[j], vcA[k]);
			this->TriProj(vc0, vc1, vc2, vcV, &fMin0, &fMax0);
			this->ObbProj((*this), vcV, &fMin1, &fMax1);

			if (fMax1 < fMin0 || fMax0 < fMin1)
			{
				return false;
			}
		}
	}
	return true;
}

void ZFXOBB::ObbProj(const ZFXOBB &obb, const ZFXVector &vcV, float *pfMin, float *pfMax)
{
	float fDP = vcV * obb.vcCenter;
	float fR = obb.fA0 * _fabs(vcV * obb.vcA0)
			  +obb.fA0 * _fabs(vcV * obb.vcA1)
			  +obb.fA1 * _fabs(vcV * obb.vcA2);
	*pfMin = fDP - fR;
	*pfMax = fDP + fR;
}

void ZFXOBB::TriProj(const ZFXVector &vc0, const ZFXVector &vc1, const ZFXVector &vc2, const ZFXVector &vcV, float *pfMin, float *pfMax)
{
	*pfMin = vcV * vc0;
	*pfMax = *pfMin;

	float fDP = vcV * vc1;
	if (fDP < *pfMin)
	{
		*pfMin = fDP;
	}
	else if (fDP > *pfMax)
	{
		*pfMax = fDP;
	}

	fDP = vcV * vc2;
	if (fDP < *pfMin)
	{
		*pfMin = fDP;
	}
	else if (fDP > *pfMax)
	{
		*pfMax = fDP;
	}
}

// intersects ray at certain length (line segment), slaps method
bool ZFXOBB::Intersects(const ZFXRay &Ray, float *t, float fL /*= -1*/) {
	float e, f, t1, t2, temp;
	float tmin = -99999.9f,
		tmax = +99999.9f;

	ZFXVector vcP = this->vcCenter - Ray.m_vcOrig;

	// 1st slap
	e = this->vcA0 * vcP;
	f = this->vcA0 * Ray.m_vcDir;
	if (_fabs(f) > 0.00001f) 
	{
		t1 = (e + this->fA0) / f;
		t2 = (e - this->fA0) / f;

		if (t1 > t2) 
		{ 
			temp = t1;
			t1 = t2;
			t2 = temp;
		}
		if (t1 > tmin)
		{
			tmin = t1;
		}
		if (t2 < tmax)
		{
			tmax = t2;
		}
		if (tmin > tmax || tmax < 0.0f)
		{
			return false;
		}
	}
	else if (((-e - this->fA0) > 0.0f) || ((-e + this->fA0) < 0.0f))
	{
		return false;
	}

	// 2nd slap
	e = this->vcA1 * vcP;
	f = this->vcA1 * Ray.m_vcDir;
	if (_fabs(f) > 0.00001f) {

		t1 = (e + this->fA1) / f;
		t2 = (e - this->fA1) / f;

		if (t1 > t2) 
		{ 
			temp = t1; 
			t1 = t2; 
			t2 = temp;
		}
		if (t1 > tmin)
		{
			tmin = t1;
		}
		if (t2 < tmax)
		{
			tmax = t2;
		}
		if (tmin > tmax || tmax < 0.0f)
		{
			return false;
		}
	}
	else if (((-e - this->fA1) > 0.0f) || ((-e + this->fA1) < 0.0f))
	{
		return false;
	}

	// 3rd slap
	e = this->vcA2 * vcP;
	f = this->vcA2 * Ray.m_vcDir;
	if (_fabs(f) > 0.00001f) {

		t1 = (e + this->fA2) / f;
		t2 = (e - this->fA2) / f;

		if (t1 > t2) 
		{ 
			temp = t1; 
			t1 = t2;
			t2 = temp;
		}
		if (t1 > tmin) 
		{
			tmin = t1;
		}
		if (t2 < tmax)
		{
			tmax = t2;
		}
		if (tmin > tmax || tmax < 0.0f)
		{
			return false;
		}
	}
	else if (((-e - this->fA2) > 0.0f) || ((-e + this->fA2) < 0.0f))
	{
		return false;
	}

	if ((tmin > 0.0f))
	{
		if( (fL >= 0 && tmin <= fL) || fL < 0)
		{
			if (t)
			{
				*t = tmin;
			}
			return true;
		}
	}

	// intersection on line but not on segment
	if (fL >= 0 && tmax > fL)
	{
		return false;
	}

	if (t)
	{
		*t = tmax;
	}

	return true;
}

//intersects another obb
bool ZFXOBB::Intersects(const ZFXOBB &obb) {
	float T[3];

	// difference vector between both obb
	ZFXVector vcD = obb.vcCenter - this->vcCenter;

	float matM[3][3];   // B's axis in relation to A
	float ra,           // radius A
		rb,           // radius B
		t;            // absolute values from T[]

	// Obb A's axis as separation axis?
	// ================================
	// first axis vcA0
	matM[0][0] = this->vcA0 * obb.vcA0;
	matM[0][1] = this->vcA0 * obb.vcA1;
	matM[0][2] = this->vcA0 * obb.vcA2;
	ra = this->fA0;
	rb = obb.fA0 * _fabs(matM[0][0]) +
		obb.fA1 * _fabs(matM[0][1]) +
		obb.fA2 * _fabs(matM[0][2]);

	T[0] = vcD * this->vcA0;
	t = _fabs(T[0]);
	if (t > (ra + rb))
	{
		return false;
	}

	// second axis vcA1
	matM[1][0] = this->vcA1 * obb.vcA0;
	matM[1][1] = this->vcA1 * obb.vcA1;
	matM[1][2] = this->vcA1 * obb.vcA2;
	ra = this->fA1;
	rb = obb.fA0 * _fabs(matM[1][0]) +
		obb.fA1 * _fabs(matM[1][1]) +
		obb.fA2 * _fabs(matM[1][2]);
	T[1] = vcD * this->vcA1;
	t = _fabs(T[1]);
	if (t > (ra + rb))
	{ 
		return false;
	}

	// third axis vcA2
	matM[2][0] = this->vcA2 * obb.vcA0;
	matM[2][1] = this->vcA2 * obb.vcA1;
	matM[2][2] = this->vcA2 * obb.vcA2;
	ra = this->fA2;
	rb = obb.fA0 * _fabs(matM[2][0]) +
		obb.fA1 * _fabs(matM[2][1]) +
		obb.fA2 * _fabs(matM[2][2]);
	T[2] = vcD * this->vcA2;
	t = _fabs(T[2]);
	if (t > (ra + rb))
	{
		return false;
	}

	// Obb B's axis as separation axis?
	// ================================
	// first axis vcA0
	ra = this->fA0 * _fabs(matM[0][0]) +
		this->fA1 * _fabs(matM[1][0]) +
		this->fA2 * _fabs(matM[2][0]);
	rb = obb.fA0;
	t = _fabs(T[0] * matM[0][0] + T[1] * matM[1][0] + T[2] * matM[2][0]);
	if (t > (ra + rb))
	{
		return false;
	}

	// second axis vcA1
	ra = this->fA0 * _fabs(matM[0][1]) +
		this->fA1 * _fabs(matM[1][1]) +
		this->fA2 * _fabs(matM[2][1]);
	rb = obb.fA1;
	t = _fabs(T[0] * matM[0][1] + T[1] * matM[1][1] + T[2] * matM[2][1]);
	if (t > (ra + rb))
	{
		return false;
	}

	// third axis vcA2
	ra = this->fA0 * _fabs(matM[0][2]) +
		this->fA1 * _fabs(matM[1][2]) +
		this->fA2 * _fabs(matM[2][2]);
	rb = obb.fA2;
	t = _fabs(T[0] * matM[0][2] + T[1] * matM[1][2] + T[2] * matM[2][2]);
	if (t > (ra + rb))
	{
		return false;
	}

	// other candidates: cross products of axis:
	// =========================================
	// axis A0xB0
	ra = this->fA1*_fabs(matM[2][0]) + this->fA2*_fabs(matM[1][0]);
	rb = obb.fA1*_fabs(matM[0][2]) + obb.fA2*_fabs(matM[0][1]);
	t = _fabs(T[2] * matM[1][0] - T[1] * matM[2][0]);
	if (t > ra + rb)
	{
		return false;
	}

	// axis A0xB1
	ra = this->fA1*_fabs(matM[2][1]) + this->fA2*_fabs(matM[1][1]);
	rb = obb.fA0*_fabs(matM[0][2]) + obb.fA2*_fabs(matM[0][0]);
	t = _fabs(T[2] * matM[1][1] - T[1] * matM[2][1]);
	if (t > ra + rb)
	{
		return false;
	}

	// axis A0xB2
	ra = this->fA1*_fabs(matM[2][2]) + this->fA2*_fabs(matM[1][2]);
	rb = obb.fA0*_fabs(matM[0][1]) + obb.fA1*_fabs(matM[0][0]);
	t = _fabs(T[2] * matM[1][2] - T[1] * matM[2][2]);
	if (t > ra + rb)
	{
		return false;
	}

	// axis A1xB0
	ra = this->fA0*_fabs(matM[2][0]) + this->fA2*_fabs(matM[0][0]);
	rb = obb.fA1*_fabs(matM[1][2]) + obb.fA2*_fabs(matM[1][1]);
	t = _fabs(T[0] * matM[2][0] - T[2] * matM[0][0]);
	if (t > ra + rb)
	{
		return false;
	}

	// axis A1xB1
	ra = this->fA0*_fabs(matM[2][1]) + this->fA2*_fabs(matM[0][1]);
	rb = obb.fA0*_fabs(matM[1][2]) + obb.fA2*_fabs(matM[1][0]);
	t = _fabs(T[0] * matM[2][1] - T[2] * matM[0][1]);
	if (t > ra + rb)
	{
		return false;
	}

	// axis A1xB2
	ra = this->fA0*_fabs(matM[2][2]) + this->fA2*_fabs(matM[0][2]);
	rb = obb.fA0*_fabs(matM[1][1]) + obb.fA1*_fabs(matM[1][0]);
	t = _fabs(T[0] * matM[2][2] - T[2] * matM[0][2]);
	if (t > ra + rb)
	{ 
		return false;
	}

	// axis A2xB0
	ra = this->fA0*_fabs(matM[1][0]) + this->fA1*_fabs(matM[0][0]);
	rb = obb.fA1*_fabs(matM[2][2]) + obb.fA2*_fabs(matM[2][1]);
	t = _fabs(T[1] * matM[0][0] - T[0] * matM[1][0]);
	if (t > ra + rb)
	{
		return false;
	}

	// axis A2xB1
	ra = this->fA0*_fabs(matM[1][1]) + this->fA1*_fabs(matM[0][1]);
	rb = obb.fA0 *_fabs(matM[2][2]) + obb.fA2*_fabs(matM[2][0]);
	t = _fabs(T[1] * matM[0][1] - T[0] * matM[1][1]);
	if (t > ra + rb)
	{
		return false;
	}

	// axis A2xB2
	ra = this->fA0*_fabs(matM[1][2]) + this->fA1*_fabs(matM[0][2]);
	rb = obb.fA0*_fabs(matM[2][1]) + obb.fA1*_fabs(matM[2][0]);
	t = _fabs(T[1] * matM[0][2] - T[0] * matM[1][2]);
	if (t > ra + rb)
	{
		return false;
	}

	// no separation axis found => intersection
	return true;
}