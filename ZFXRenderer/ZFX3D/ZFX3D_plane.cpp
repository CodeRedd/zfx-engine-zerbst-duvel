//File: ZFX3D_plane.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFX3D.h"

extern bool g_bSSE;

inline void ZFXPlane::Set(const ZFXVector &vcN, const ZFXVector &vcPoint)
{
	m_fD		= -(vcN * vcPoint);
	m_vcN		= vcN;
	m_vcPoint	= vcPoint;
}

inline void ZFXPlane::Set(const ZFXVector &vcN, const ZFXVector &vcPoint, float fD)
{
	m_fD		= fD;
	m_vcN		= vcN;
	m_vcPoint	= vcPoint;
}

inline void ZFXPlane::Set(const ZFXVector &vc0, const ZFXVector &vc1, const ZFXVector &vc2)
{
	ZFXVector vcEdge1 = vc1 - vc0;
	ZFXVector vcEdge2 = vc2 - vc0;

	m_vcN.Cross(vcEdge1, vcEdge2);
	m_fD = -(m_vcN * vc0);
	m_vcPoint = vc0;	
}

inline float ZFXPlane::Distance(const ZFXVector &vcPoint)
{
	return _fabs((m_vcN * vcPoint) - m_fD);
}

inline int ZFXPlane::Classify(const ZFXVector &vcPoint)
{
	float f = (vcPoint * m_vcN) + m_fD;

	if (f > 0.00001)
	{
		return ZFXFRONT;
	}
	if (f < -0.00001)
	{
		return ZFXBACK;
	}

	return ZFXPLANAR;
}

// Classify polygon with respect to this plane
int ZFXPlane::Classify(const ZFXPolygon &Poly) {
	int NumFront = 0, NumBack = 0, NumPlanar = 0;
	int nClass;

	ZFXPolygon *pPoly = ((ZFXPolygon*)&Poly);

	int NumPoints = pPoly->GetNumPoints();

	// loop through all points
	for (int i = 0; i < NumPoints; i++) {
		nClass = Classify(pPoly->m_pPoints[i]);

		if (nClass == ZFXFRONT)     
		{
			NumFront++;
		}
		else if (nClass == ZFXBACK) 
		{
			NumBack++;
		}
		else {
			NumFront++;
			NumBack++;
			NumPlanar++;
		}
	}

	// all points are planar
	if (NumPlanar == NumPoints)
	{
		return ZFXPLANAR;
	}
	// all points are in front of plane
	else if (NumFront == NumPoints)
	{
		return ZFXFRONT;
	}
	// all points are on backside of plane
	else if (NumBack == NumPoints)
	{
		return ZFXBACK;
	}
	// poly is intersecting the plane
	else
	{
		return ZFXCLIPPED;
	}
}

bool ZFXPlane::Clip(const ZFXRay *_pRay, float fL, ZFXRay *pF, ZFXRay *pB) {
	ZFXVector vcHit(0.0f, 0.0f, 0.0f);

	ZFXRay *pRay = (ZFXRay*)_pRay;

	// ray intersects plane at all?
	if (!pRay->Intersects(*this, false, NULL, &vcHit, fL))
	{
		return false;
	}

	int n = Classify(_pRay->m_vcOrig);

	// ray comes fron planes backside
	if (n == ZFXBACK) {
		if (pB)
		{
			pB->Set(pRay->m_vcOrig, pRay->m_vcDir);
		}
		if (pF) 
		{
			pF->Set(vcHit, pRay->m_vcDir);
		}
	}
	// ray comes from planes front side
	else if (n == ZFXFRONT) {
		if (pF)
		{
			pF->Set(pRay->m_vcOrig, pRay->m_vcDir);
		}
		if (pB)
		{
			pB->Set(vcHit, pRay->m_vcDir);
		}
	}

	return true;
}

//Intersection with a plane defined by three points
//returns true if one point is one a different side of the plane from the other two or if any point is planar
bool ZFXPlane::Intersects(const ZFXVector &vc0, const ZFXVector &vc1, const ZFXVector &vc2)
{
	int n0 = this->Classify(vc0);
	int n1 = this->Classify(vc1);
	int n2 = this->Classify(vc2);
	
	if (n0 == ZFXPLANAR || n1 == ZFXPLANAR || n2 == ZFXPLANAR)
	{
		return true;
	}

	return !(n0 == n1 && n0 == n2);
}

//Intersection with another plane
//Based on an implementation by David Eberly
bool ZFXPlane::Intersects(ZFXPlane &plane, ZFXRay *pIntersection)
{
	ZFXVector vcCross;
	float	  fSqrLength;

	//if cross product is zero planes are parallel
	vcCross.Cross(this->m_vcN, plane.m_vcN);
	fSqrLength = vcCross.GetSqrLength();

	if (fSqrLength < 1e-08f)
	{
		return false;
	}

	//calculate the intersection line if needed
	if (pIntersection)
	{
		float fN00 = this->m_vcN.GetSqrLength();
		float fN01 = this-> m_vcN * plane.m_vcN;
		float fN11 = plane.m_vcN.GetSqrLength();
		float fDet = fN00 * fN11 - fN01 * fN01;

		if (_fabs(fDet) < 1e-08f)
		{
			return false;
		}

		float fInvDet = 1.0f/fDet;
		float fC0 = (fN11*this->m_fD - fN01*plane.m_fD) * fInvDet;
		float fC1 = (fN00*plane.m_fD - fN01*this->m_fD) * fInvDet;

		(*pIntersection).m_vcDir = vcCross;
		(*pIntersection).m_vcOrig = this->m_vcN * fC0 + plane.m_vcN * fC1;
	}

	return true;
}

//Intersection with AABB
//Algorithm from the book Realtime Rendering by Moller and Haines
bool ZFXPlane::Intersects(const ZFXAABB &aabb)
{
	ZFXVector Vmin, Vmax;

	//x-coordinate
	if (m_vcN.x >= 0.0f)
	{
		Vmin.x = aabb.vcMin.x;
		Vmax.x = aabb.vcMax.x;
	}
	else
	{
		Vmin.x = aabb.vcMax.x;
		Vmax.x = aabb.vcMin.x;
	}

	//y-coordinate
	if (m_vcN.y >= 0.0f)
	{
		Vmin.y = aabb.vcMin.y;
		Vmax.y = aabb.vcMax.y;
	}
	else
	{
		Vmin.y = aabb.vcMax.y;
		Vmax.y = aabb.vcMin.y;
	}

	//z-coordinate
	if (m_vcN.z >= 0.0f)
	{
		Vmin.z = aabb.vcMin.z;
		Vmax.z = aabb.vcMax.z;
	}
	else
	{
		Vmin.z = aabb.vcMax.z;
		Vmax.z = aabb.vcMin.z;
	}

	if ((m_vcN*Vmin) + m_fD > 0.0f)
	{
		return false;
	}

	if ((m_vcN*Vmax) + m_fD >= 0.0f)
	{
		return true;
	}

	return false;
}

//Intersection with OBB
bool ZFXPlane::Intersects(const ZFXOBB &obb)
{
	float fRadius = _fabs(obb.fA0 * (m_vcN*obb.vcA0))
					+ _fabs(obb.fA1 * (m_vcN*obb.vcA1))
					+ _fabs(obb.fA2 * (m_vcN*obb.vcA2));

	float fDistance = this->Distance(obb.vcCenter);
	return fDistance <= fRadius;
}


