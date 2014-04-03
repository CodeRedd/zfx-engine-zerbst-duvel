//File: ZFX3D_polygon.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFX3D.h"

extern bool g_bSSE;

//constructor and destructor

ZFXPolygon::ZFXPolygon()
{
	m_pPoints	= NULL;
	m_pIndic	= NULL;
	m_NumP		= 0;
	m_NumI		= 0;
	m_Flag		= 0;
	memset(&m_Aabb, 0, sizeof(ZFXAABB));
}

ZFXPolygon::~ZFXPolygon()
{
	if (m_pPoints)
	{
		delete [] m_pPoints;
		m_pPoints = NULL;
	}
	if (m_pIndic)
	{
		delete [] m_pIndic;
		m_pIndic = NULL;
	}
}


void ZFXPolygon::Set(const ZFXVector *pPoints, int nNumP, const unsigned int *pIndic, int nNumI)
{
	ZFXVector vcEdge0, vcEdge1;
	bool bPlaneFound = false;

	if (m_pPoints)
	{
		delete [] m_pPoints;
	}
	if (m_pIndic)
	{
		delete [] m_pIndic; 
	}

	m_pPoints = new ZFXVector[nNumP];
	m_pIndic = new unsigned int[nNumI];

	m_NumP = nNumP;
	m_NumI = nNumI;

	memcpy(m_pPoints, pPoints, sizeof(ZFXVector)*nNumP);
	memcpy(m_pIndic, pIndic, sizeof(unsigned int)*nNumI);
	

	vcEdge0 = m_pPoints[m_pIndic[1]] - m_pPoints[m_pIndic[0]];
	vcEdge0.Normalize();

	//Calculate the function's normal vector by creating edges from three points and crossing those edge vectors
	for (int i = 2; bPlaneFound == false; i++)
	{
		if (i + 1 > m_NumI)
		{
			break;
		}

		vcEdge1 = m_pPoints[m_pIndic[i]] - m_pPoints[m_pIndic[0]];
		vcEdge1.Normalize();

		//NOTE: We cannot simply take the first three points because we need to avoid crossing parallel edge vectors
		if (vcEdge0.AngleBetween(vcEdge1) != 0.0)
		{
			bPlaneFound = true;
		}
	}

	m_Plane.m_vcN.Cross(vcEdge0, vcEdge1);
	m_Plane.m_vcN.Normalize();
	m_Plane.m_fD = -(m_Plane.m_vcN * m_pPoints[0]);
	m_Plane.m_vcPoint = m_pPoints[0];

	CalcBoundingBox();
}

//utility functions

void ZFXPolygon::CalcBoundingBox()
{
	ZFXVector vcMax, vcMin;
	vcMax = vcMin = m_pPoints[0];

	for (int i = 0; i < m_NumP; i++)
	{
		if (m_pPoints[i].x > vcMax.x)
		{
			vcMax.x = m_pPoints[i].x;
		}
		else if (m_pPoints[i].x < vcMin.x)
		{
			vcMin.x = m_pPoints[i].x;
		}

		if (m_pPoints[i].y > vcMax.y)
		{
			vcMax.y = m_pPoints[i].y;
		}
		else if (m_pPoints[i].y < vcMin.y)
		{
			vcMin.y = m_pPoints[i].y;
		}

		if (m_pPoints[i].z > vcMax.z)
		{
			vcMax.z = m_pPoints[i].z;
		}
		else if (m_pPoints[i].z < vcMin.z)
		{
			vcMin.z = m_pPoints[i].z;
		}
	}

	m_Aabb.vcMax	= vcMax;
	m_Aabb.vcMin	= vcMin;
	m_Aabb.vcCenter = (vcMax + vcMin) / 2.0f;
}

//invert the normal vector and reverse the list of indices
void ZFXPolygon::SwapFaces()
{
	unsigned int *pIndic = new unsigned int[m_NumI];

	for (int i = 0; i < m_NumI; i++)
	{
		pIndic[m_NumI - i - 1] = m_pIndic[i];
	}

	m_Plane.m_vcN *= -1.0f;
	m_Plane.m_fD  *= -1.0f; //don't forget to invert the plane equation too!

	delete [] m_pIndic;
	m_pIndic = pIndic;
}

//Clipping functions

//clip to bounding box - extract all planes from the BB and clip to each of those individually
void ZFXPolygon::Clip(const ZFXAABB &aabb)
{
	ZFXPolygon	BackPoly, ClippedPoly;
	ZFXPlane	Planes[6];
	bool		bClipped = false;

	//get rid of const
	ZFXAABB *pAabb = ((ZFXAABB*)&aabb);

	//get planes from aabb, normals pointing outward
	pAabb->GetPlanes(Planes);

	//copy the poly
	ClippedPoly.CopyOf(*this);

	//apply clipping
	for (int i = 0; i < 6; i++)
	{
		if (Planes[i].Classify(ClippedPoly) == ZFXCLIPPED)
		{
			ClippedPoly.Clip(Planes[i], NULL, &BackPoly);
			ClippedPoly.CopyOf(BackPoly);
			bClipped = true;
		}
	}

	if (bClipped)
	{
		CopyOf(ClippedPoly);
	}
}

//clip to specific side of a plane
//creates new polygon pFront on the front side of the plane, and polygon pBack on the back side
//will create new points as needed where edges are split
//NOTE: Assumes that intersection between plane and polygon exists
void ZFXPolygon::Clip(const ZFXPlane &plane, ZFXPolygon *pFront, ZFXPolygon *pBack)
{
	if (!pFront && !pBack)
	{
		return;
	}

	ZFXVector	vcHit, vcA, vcB;
	ZFXRay		Ray;

	//get rid of const
	ZFXPlane	*pPlane = ((ZFXPlane*) &plane);

	unsigned int nNumFront = 0;	//num of points on front side
	unsigned int nNumBack = 0;	//num of points on back side
	unsigned int nLoop = 0, nCurrent = 0;
	
	ZFXVector *pvcFront = new ZFXVector[m_NumP * 3];
	ZFXVector *pvcBack	= new ZFXVector[m_NumP * 3];

	//classify the first point
	switch (pPlane->Classify(m_pPoints[0]))
	{
		case ZFXFRONT:
			pvcFront[nNumFront++] = m_pPoints[0];
			break;
		case ZFXBACK:
			pvcFront[nNumBack++] = m_pPoints[0];
			break;
		case ZFXPLANAR:
			pvcFront[nNumFront++] = m_pPoints[0];
			pvcFront[nNumBack++] = m_pPoints[0];
			break;
		default:
			return;
	}

	//loop through all points in the polygon
	for (nLoop = 1; nLoop < (m_NumP + 1); nLoop++)
	{
		if (nLoop == m_NumP)
		{
			nCurrent = 0;
		}
		else
		{
			nCurrent = nLoop;
		}

		//take two neighboring points from the polygon
		vcA = m_pPoints[nLoop-1];
		vcB = m_pPoints[nCurrent];

		//classify them wrt the plane
		int nClass = pPlane->Classify(vcB);
		int nClassA = pPlane->Classify(vcA);

		//if B is planar then add to both sides
		if (nClass == ZFXPLANAR)
		{
			pvcFront[nNumFront++] = m_pPoints[nCurrent];
			pvcFront[nNumBack++] = m_pPoints[nCurrent];
		}
		//test if an edge intersects the plane
		else
		{
			Ray.m_vcOrig = vcA;
			Ray.m_vcDir	 = vcB - vcA;

			float fLength = Ray.m_vcDir.GetLength();
			if (fLength != 0.0f)
			{
				Ray.m_vcDir /= fLength;
			}

			if (Ray.Intersects(plane, false, 0, &vcHit, fLength) && nClassA != ZFXPLANAR)
			{
				//insert intersection point as new point in both lists
				pvcFront[nNumFront++] = vcHit;
				pvcFront[nNumBack++] = vcHit;
			}
			//sort the current point
			if (nCurrent = 0)
			{
				continue;
			}

			if (nClass == ZFXFRONT)
			{
				pvcFront[nNumFront++] = m_pPoints[nCurrent];
			}
			else if (nClass == ZFXBACK)
			{
				pvcFront[nNumBack++] = m_pPoints[nCurrent];
			}
		}
	}

	//now that we have the vertices for the two polygons, create indices
	unsigned int I0, I1, I2;
	unsigned int *pnFront = NULL;
	unsigned int *pnBack  = NULL;

	if (nNumFront > 2)
	{
		pnFront = new unsigned int[(nNumFront-2)*3];

		for (nLoop = 0; nLoop < (nNumFront - 2); nLoop++)
		{
			if (nLoop == 0)
			{
				I0 = 0;
				I1 = 1;
				I2 = 2;
			}
			else
			{
				I1 = I2;
				I2++;
			}

			pnFront[(nLoop * 3)	   ] = I0;
			pnFront[(nLoop * 3) + 1] = I1;
			pnFront[(nLoop * 3) + 2] = I2;
		}
	}

	if (nNumBack > 2)
	{
		pnBack = new unsigned int[(nNumBack - 2) * 3];

		for (nLoop = 0; nLoop < (nNumBack - 2); nLoop++)
		{
			if (nLoop == 0)
			{
				I0 = 0;
				I1 = 1;
				I2 = 2;
			}
			else
			{
				I1 = I2;
				I2++;
			}

			pnBack[(nLoop * 3)	  ] = I0;
			pnBack[(nLoop * 3) + 1] = I1;
			pnBack[(nLoop * 3) + 2] = I2;
		}
	}

	//generate new polygons
	if (pFront && pnFront)
	{
		pFront->Set(pvcFront, nNumFront, pnFront, (nNumFront-2)*3);

		//maintain same orientation as original poly
		if (pFront->GetPlane().m_vcN*m_Plane.m_vcN < 0.0f)
		{
			pFront->SwapFaces();
		}
	}

	if (pBack && pnBack)
	{
		pBack->Set(pvcFront, nNumBack, pnBack, (nNumBack - 2) * 3);

		//maintain same orientation as original poly
		if (pBack->GetPlane().m_vcN*m_Plane.m_vcN < 0.0f)
		{
			pBack->SwapFaces();
		}
	}

	//cleanup
	if (pvcFront)
	{
		delete [] pvcFront;
	}
	if (pvcBack)
	{
		delete [] pvcBack;
	}
	if (pnFront)
	{
		delete [] pnFront;
	}
	if (pnBack)
	{
		delete [] pnBack;
	}
}

//culling and intersection functions

int ZFXPolygon::Cull(const ZFXAABB &aabb)
{
	ZFXPlane Planes[6];
	int		 nClass = 0;
	int		 nInside = 0, nCurrent = 0;
	bool	 bFirst = true;
	ZFXRay	 Ray;

	//get rid of const
	ZFXAABB  *pAabb = ((ZFXAABB*)&aabb);

	//grab planes from AABB, normals pointing outward
	pAabb->GetPlanes(Planes);

	//do AABB's intersect at all?
	if (!m_Aabb.Intersects(aabb))
	{
		return ZFXCULLED;
	}

	//loop through planes of the box
	for (int p = 0; p < 6; p++)
	{
		//one time test to see what points are inside the AABB
		if (bFirst)
		{
			for (int i = 0; i < m_NumP; i++)
			{
				if (pAabb->Intersects(m_pPoints[i]))
				{
					nInside++;
				}
			}
			bFirst = false;

			//if all points are inside?
			if (nInside == m_NumP)
			{
				return ZFXVISIBLE;
			}
		}

		//test intersection of plane with polygon edges
		for (int nLoop = 1; nLoop < (m_NumP + 1); nLoop++)
		{
			if (nLoop == m_NumP)
			{
				nCurrent = 0;
			}
			else
			{
				nCurrent = nLoop;
			}

			//edge from two neighboring points
			Ray.m_vcOrig = m_pPoints[nLoop - 1];
			Ray.m_vcDir  = m_pPoints[nCurrent] - m_pPoints[nLoop - 1];

			float fLength = Ray.m_vcDir.GetLength();
			if (fLength != 0.0f)
			{
				Ray.m_vcDir /= fLength;
			}

			//if AABB and polygon intersect
			if (Ray.Intersects(Planes[p], false, 0, NULL, fLength))
			{
				return ZFXCLIPPED;
			}
		}
	}

	//polygon is not inside the AABB but does not intersect
	return ZFXCULLED;
}

void ZFXPolygon::CopyOf(const ZFXPolygon &poly) {

	Set(poly.m_pPoints, poly.m_NumP, poly.m_pIndic, poly.m_NumI);

}

bool ZFXPolygon::Intersects(const ZFXRay &Ray, bool bCull, float *t, float fL /*=-1*/) {
	//get rid of const
	ZFXRay *pRay = (ZFXRay*)&Ray;

	for (int i = 0; i<m_NumI; i += 3) {
		if (pRay->Intersects(m_pPoints[m_pIndic[i]], m_pPoints[m_pIndic[i + 1]], m_pPoints[m_pIndic[i + 2]], false, t, fL))
		{
			return true;
		}
		if (!bCull) {
			if (pRay->Intersects(m_pPoints[m_pIndic[i + 2]], m_pPoints[m_pIndic[i + 1]], m_pPoints[m_pIndic[i]], false, t, fL))
			{
				return true;
			}
		}
	}
	return false;
}