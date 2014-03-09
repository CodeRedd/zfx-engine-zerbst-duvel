//File: ZFX3D_ray.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFX3D.h"

extern bool g_bSSE;

//NOTE: The vector parameters for this function should be normalized. Normalizing is expensive so we will not check for it or do it here.
inline void ZFXRay::Set(ZFXVector vcOrig, ZFXVector vcDir)
{
	m_vcOrig = vcOrig;
	m_vcDir = vcDir;
}

//Transforms the ray from world space to match the local coordinate space of a matrix
inline void ZFXRay::DeTransform(const ZFXMatrix &_m)
{
	ZFXMatrix mInv;
	ZFXMatrix m = _m;

	//inverse translation -- match up zero points
	m_vcOrig.x -= m._41;
	m_vcOrig.y -= m._42;
	m_vcOrig.z -= m._43;

	//delete translation in the matrix. we don't want to have a translation applied to our ray's direction!
	m._41 = 0.0f;
	m._42 = 0.0f;
	m._43 = 0.0f;

	//invert matrix
	mInv.InverseOf(m);

	//apply inverse matrix
	m_vcOrig = m_vcOrig * mInv;
	m_vcDir = m_vcDir * mInv;
}

//Intersection With Triangles--uses algorithm from Moller and Trumbore
//aligns ray with x axis
//normalizes tri, translates it to origin, and aligns it with yz plane
//if given a pointer for t, places the distance from ray origin to collision point inside it.
bool ZFXRay::Intersects(const ZFXVector &vc0, const ZFXVector &vc1, const ZFXVector &vc2, bool bCull, float *t, float fL /*-1*/)
{
	ZFXVector pvec, tvec, qvec;

	ZFXVector edge1 = vc1 - vc0;
	ZFXVector edge2 = vc2 - vc0;

	pvec.Cross(m_vcDir, edge2);

	//if cloase to 0, ray is parallel
	float det = edge1 * pvec;

	//to account for poor float precision
	//use epsilon value of 0.0001 instead
	//of comparing to zero exactly
	if (bCull && det < 0.0001f)
	{
		return false;
	}
	else if (det < 0.0001f && det > -0.0001f)
	{
		return false;
	}

	//distance to plane, <0 means ray is behind the plane
	tvec = m_vcOrig - vc0;
	float u = tvec * pvec;
	if (u < 0.0f || u > det)
	{
		return false;
	}

	qvec.Cross(tvec, edge1);
	float v = m_vcDir * qvec;
	if (v < 0.0f || u + v > det)
	{
		return false;
	}

	//at this point, we know that an infinite ray will collide with the triangle. calculate distance to collision
	float fLength;
	fLength = edge2 * qvec;
	float fInvDet = 1.0f / det;
	fLength *= fInvDet;

	//if we've given a max length, check to make sure our collision point isn't beyond it
	if (fL >= 0 && fLength > fL)
	{
		return false;
	}
	if (t)
	{
		*t = fLength;
	}

	return true;
}

//Intersection with Planes
bool ZFXRay::Intersects(const ZFXPlane &plane, bool bCull, float *t, ZFXVector *vcHit, float fL /*= -1*/)
{
	float Vd = plane.m_vcN * m_vcDir;

	//ray parallel to the plane
	if (_fabs(Vd) < 0.00001f)
	{
		return false;
	}

	//plane normal points away from the ray's direction => intersection with the back face if ome exists
	if (bCull && Vd > 0.0f)
	{
		return false;
	}

	float Vo = -((plane.m_vcN * m_vcOrig) + plane.m_fD);
	float _t = Vo / Vd;

	//intersection before ray origin
	if (_t < 0.0f || (fL >= 0 && _t > fL))
	{
		return false;
	}

	if (vcHit)
	{
		(*vcHit) = m_vcOrig + (m_vcDir * _t);
	}
	
	if (t)
	{
		(*t) = _t;
	}

	return true;
}

//Intersection with axis-aligned bounding box
//Using algorithm by Andrew Woo
bool ZFXRay::Intersects(const ZFXAABB &aabb, ZFXVector *vcHit)
{
	bool bInside = true;
	ZFXVector MaxT;

	MaxT.Set(-1.0f, -1.0f, -1.0f);

	//find x component
	if (m_vcOrig.x < aabb.vcMin.x)
	{
		(*vcHit).x = aabb.vcMin.x;
		bInside = false;
		if (m_vcDir.x != 0.0f)
		{
			MaxT.x = (aabb.vcMin.x - m_vcOrig.x) / m_vcDir.x;
		}
	}
	else if (m_vcOrig.x > aabb.vcMax.x)
	{
		(*vcHit).x = aabb.vcMax.x;
		bInside = false;
		if (m_vcDir.x != 0.0f)
		{
			MaxT.x = (aabb.vcMax.x - m_vcOrig.x) / m_vcDir.x;
		}
	}

	//find y component
	if (m_vcOrig.y < aabb.vcMin.y)
	{
		(*vcHit).y = aabb.vcMin.y;
		bInside = false;
		if (m_vcDir.y != 0.0f)
		{
			MaxT.y = (aabb.vcMin.y - m_vcOrig.y) / m_vcDir.y;
		}
	}
	else if (m_vcOrig.y > aabb.vcMax.y)
	{
		(*vcHit).y = aabb.vcMax.y;
		bInside = false;
		if (m_vcDir.y != 0.0f)
		{
			MaxT.y = (aabb.vcMax.y - m_vcOrig.y) / m_vcDir.y;
		}
	}

	//find z component
	if (m_vcOrig.z < aabb.vcMin.z)
	{
		(*vcHit).z = aabb.vcMin.z;
		bInside = false;
		if (m_vcDir.z != 0.0f)
		{
			MaxT.z = (aabb.vcMin.z - m_vcOrig.z) / m_vcDir.z;
		}
	}
	else if (m_vcOrig.z > aabb.vcMax.z)
	{
		(*vcHit).z = aabb.vcMax.z;
		bInside = false;
		if (m_vcDir.z != 0.0f)
		{
			MaxT.z = (aabb.vcMax.z - m_vcOrig.z) / m_vcDir.z;
		}
	}

	//ray origin inside the box
	if (bInside)
	{
		(*vcHit) = m_vcOrig;
		return true;
	}

	//find max value for maxT
	int nPlane = 0;

	if (MaxT.y > ((float*)&MaxT)[nPlane])
	{
		nPlane = 1;
	}
	if (MaxT.z > ((float*)&MaxT)[nPlane])
	{
		nPlane = 2;
	}
	if (((float*)&MaxT)[nPlane] < 0.0f)
	{
		return false;
	}

	if (nPlane != 0)
	{
		(*vcHit).x = m_vcOrig.x + MaxT.x * m_vcDir.x;
		if ((*vcHit).x < aabb.vcMin.x - 0.00001f || (*vcHit).x < aabb.vcMax.x + 0.00001f)
		{
			return false;
		}
	}
	if (nPlane != 1)
	{
		(*vcHit).y = m_vcOrig.y + MaxT.y * m_vcDir.y;
		if ((*vcHit).y < aabb.vcMin.y - 0.00001f || (*vcHit).y < aabb.vcMax.y + 0.00001f)
		{
			return false;
		}
	}
	if (nPlane != 0)
	{
		(*vcHit).z = m_vcOrig.z + MaxT.z * m_vcDir.z;
		if ((*vcHit).z < aabb.vcMin.z - 0.00001f || (*vcHit).z < aabb.vcMax.z + 0.00001f)
		{
			return false;
		}
	}
	return true;
}

//Intersection with oriented bounding box
//using slabs method developed by Moller and Haines
bool ZFXRay::Intersects(const ZFXOBB &obb, float *t, float fL /*= -1*/)
{
	float e, f, t1, t2, temp;
	float tmin = -99999.9f, tmax = +99999.9f;

	ZFXVector vcP = obb.vcCenter - m_vcOrig;

	//1. Slap
	e = obb.vcA0 * vcP;
	f = obb.vcA0 * m_vcDir;

	if (_fabs(f) > 0.00001f)
	{
		t1 = (e + obb.fA0) / f;
		t2 = (e - obb.fA0) / f;

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
	//ray parallel to plane normal
	else if ((-e - obb.fA0) > 0.0f || (-e + obb.fA0 < 0.0f))
	{
		return false;
	}

	//2. Slap
	e = obb.vcA1 * vcP;
	f = obb.vcA1 * m_vcDir;

	if (_fabs(f) > 0.00001f)
	{
		t1 = (e + obb.fA1) / f;
		t2 = (e - obb.fA1) / f;

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
	//ray parallel to plane normal
	else if ((-e - obb.fA1) > 0.0f || (-e + obb.fA1 < 0.0f))
	{
		return false;
	}

	//3. Slap
	e = obb.vcA2 * vcP;
	f = obb.vcA2 * m_vcDir;

	if (_fabs(f) > 0.00001f)
	{
		t1 = (e + obb.fA2) / f;
		t2 = (e - obb.fA2) / f;

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
	//ray parallel to plane normal
	else if ((-e - obb.fA2) > 0.0f || (-e + obb.fA2 < 0.0f))
	{
		return false;
	}

	if (tmin > 0.0f)
	{
		if (tmin > fL)
		{
			return false;
		}
		if (t)
		{
			*t = tmin;
		}
		return true;
	}

	if (tmax > fL)
	{
		return false;
	}
	if (t)
	{
		*t = tmax;
	}
	return true;

}
