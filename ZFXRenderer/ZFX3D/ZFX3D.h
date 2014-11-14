//File: ZFX3D.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#pragma once

#include <Windows.h>
#include <math.h>

//math constants
const double ZFXPI = 3.14159265;
const double ZFXPI2 = 1.5707963;
const double ZFX2PI = 6.2831853;
const float  ZFXG = -32.174f; // ft/s^2
const float  ZFXEPSILON = 0.00001f;

//definitions
#define ZFXFRONT	0
#define ZFXBACK		1
#define ZFXPLANAR	2
#define ZFXCLIPPED  3
#define ZFXCULLED   4
#define ZFXVISIBLE  5

//Utility method
float _fabs(float f);

//CPU Info Stuff
struct CPUINFO {
	bool bSSE;			//Streaming SIMD Extensions
	bool bSSE2;			//Streaming SIMD Extensions 2
	bool b3DNOW;		//3DNow! (vendor independent)
	bool bMMX;			//MMX support
	wchar_t name[48];		//cpu name
	bool bEXT;			//extended features available
	bool bMMXEX;		//MMX (AMD specific extensions)
	bool b3DNOWEX;		//3DNow! (AMD specific extensions)
	wchar_t vendor[13];	//vendor name
};

CPUINFO GetCPUInfo();
void GetCPUName(wchar_t *chName, int n, const wchar_t *vendor);
bool OSSupportsSSE();
bool ZFX3DInitCPU();

//Forward Declarations
class ZFXVector;
class ZFXMatrix;
class ZFXRay;
class ZFXPlane;
class ZFXAABB;
class ZFXOBB;
class ZFXPolygon;
class ZFXQuat;

//4 Dimensional Vector Class
//4th dimension (w) exists mostly so that the vector can be multiplied with 4x4 matrices
class __declspec(dllexport) ZFXVector
{
public:
	float x, y, z, w;

	ZFXVector() { x = 0, y = 0, z = 0, w = 1.0f; }
	ZFXVector(float _x, float _y, float _z) { x = _x, y = _y, z = _z, w = 1.0f; }

	inline void		Set(float _x, float _y, float _z, float _w = 1.0f);
	inline float	GetLength();
	inline float	GetSqrLength();
	inline void		Negate();
	inline void		Normalize();
	inline void		RotateWith(const ZFXMatrix &m);
	inline void		InvRotateWith(const ZFXMatrix &m);
	inline float	AngleBetween(ZFXVector &v);
	inline void		Difference(const ZFXVector &u, const ZFXVector &v);
	inline void		Cross(const ZFXVector &u, const ZFXVector &v);


	void operator += (const ZFXVector &v);          
	void operator -= (const ZFXVector &v);          
	void operator *= (float f);                     
	void operator /= (float f);                     
	void operator += (float f);                     
	void operator -= (float f);                     
	float     operator * (const ZFXVector &v)const; 
	ZFXVector operator * (float f)const;            
	ZFXVector operator / (float f)const;            
	ZFXVector operator + (float f)const;            
	ZFXVector operator - (float f)const;            
	ZFXQuat   operator * (const ZFXQuat   &q)const; 
	ZFXVector operator * (const ZFXMatrix &m)const;
	ZFXVector operator + (const ZFXVector &v)const; 
	ZFXVector operator - (const ZFXVector &v)const; 

};

//4 dimensional matrix
class __declspec(dllexport) ZFXMatrix
{
public:
	float _11, _12, _13, _14;
	float _21, _22, _23, _24;
	float _31, _32, _33, _34;
	float _41, _42, _43, _44;

	ZFXMatrix() { ; }

	inline void Identity();			//make identity matrix
	inline void RotaX(float a);		//Rotation matrix around X Axis
	inline void RotaY(float a);		//Rotation matrix around Y Axis
	inline void RotaZ(float a);		//Rotation matrix around Z Axis
	inline void Rota(const ZFXVector &vc);            // x, y and z
	inline void Rota(float x, float y, float z);      // x, y and z
	inline void RotaArbi(ZFXVector vcAxis, float a);
	inline void ApplyInverseRota(ZFXVector *pvc);
	inline void Translate(float dx, float dy, float dz);
	inline void SetTranslation(ZFXVector vc, bool EraseContent = false);
	inline ZFXVector GetTranslation(void);

	inline void Billboard(ZFXVector vcPos, ZFXVector vcDir,
		ZFXVector vcWorldUp = ZFXVector(0, 1, 0));
	inline void LookAt(ZFXVector vcPos, ZFXVector vcLookAt,
		ZFXVector vcWorldUp = ZFXVector(0, 1, 0));

	inline void TransposeOf(const ZFXMatrix &m);
	inline void InverseOf(const ZFXMatrix &m);

	ZFXMatrix operator * (const ZFXMatrix &m)  const;
	ZFXVector operator * (const ZFXVector &vc) const;
};

//Infinite-length ray and finite-length line segment
class __declspec(dllexport) ZFXRay
{
public:
	ZFXVector m_vcOrig, //origin
			  m_vcDir;  //direction

	ZFXRay() { ; }

	inline void Set(ZFXVector vcOrig, ZFXVector vcDir);
	inline void DeTransform(const ZFXMatrix &_m);

	//intersecting triangles
	bool Intersects(const ZFXVector &vc0, const ZFXVector &vc1, const ZFXVector &vc2, bool bCull, float *t, float fL = -1);

	//intersecting planes
	bool Intersects(const ZFXPlane &plane, bool bCull, float *t, ZFXVector *vcHit, float fL = -1);

	//intersecting AA Bounding Box
	bool Intersects(const ZFXAABB &aabb, ZFXVector *vcHit);

	//intersecting O Bounding Box
	bool Intersects(const ZFXOBB &obb, float *t, float fL = -1);
};

//Infinite-extent plane
class __declspec(dllexport) ZFXPlane
{
public:
	ZFXVector m_vcN,	//normal vector
			  m_vcPoint;//point on the plane
	float	  m_fD;		//distance from origin

	ZFXPlane() { ; }

	inline void Set(const ZFXVector &vcN, const ZFXVector &vcPoint);
	inline void Set(const ZFXVector &vcN, const ZFXVector &vcPoint, float fD);
	inline void Set(const ZFXVector &vc0, const ZFXVector &vc1, const ZFXVector &vc2);

	//distance of a point to the plane
	inline float Distance(const ZFXVector &vcPoint);

	//classify a point with respect to plane's front or back
	inline int Classify(const ZFXVector &vcPoint);
	inline int Classify(const ZFXPolygon &Poly);

	//Clips a ray into two segments if it intersects the plane
	bool Clip(const ZFXRay*, float, ZFXRay*, ZFXRay*);

	//intersection with a triangle
	bool Intersects(const ZFXVector &vc0, const ZFXVector &vc1, const ZFXVector &vc2);

	//intersection line of two planes
	bool Intersects(ZFXPlane &plane, ZFXRay *pIntersection);

	//intersection with bounding boxes
	bool Intersects(const ZFXAABB &aabb);
	bool Intersects(const ZFXOBB &obb);

};

//Axis-Aligned Bounding Box (AABB)
class __declspec(dllexport) ZFXAABB
{
public:
	ZFXVector vcMin, vcMax; //extreme points
	ZFXVector vcCenter;		//center point

	ZFXAABB() { ; }

	void Construct(const ZFXOBB &obb);
	int	 Cull(const ZFXPlane *pPlanes, int nNumPlanes);

	void GetPlanes(ZFXPlane *pPlanes);
	bool Contains(const ZFXRay &ray, float fL);
	bool Intersects(const ZFXRay &ray, float *t, float fL = -1);
	bool Intersects(const ZFXAABB &aabb);
	bool Intersects(const ZFXVector &vc);
};

//Oriented Bounding Box (OBB)
class __declspec(dllexport) ZFXOBB
{
public:
	float	 	 fA0,  fA1,  fA2;	//half-extent on each axis
	ZFXVector	vcA0, vcA1, vcA2;	//axis vectors
	ZFXVector	vcCenter;			// center point

	ZFXOBB() { ; }

	inline void DeTransform(const ZFXOBB &obb, const ZFXMatrix &m);

	bool Intersects(const ZFXRay &Ray, float *t, float fL = -1);
	bool Intersects(const ZFXOBB &obb);
	bool Intersects(const ZFXVector &vc0, const ZFXVector &vc1, const ZFXVector &vc2);

	int  Cull(const ZFXPlane *pPlanes, int nNumPlanes);

private:
	void ObbProj(const ZFXOBB &obb, const ZFXVector &vcV, float *pfMin, float *pfMax);
	void TriProj(const ZFXVector &vc0, const ZFXVector &vc1, const ZFXVector &vc2, const ZFXVector &vcV, float*pfMin, float *pfMax);
};

//Polygon
//NOTE: DO NOT USE THIS CLASS FOR RENDERING. IT IS ONLY MEANT FOR COLLISION DETECTION
class __declspec(dllexport) ZFXPolygon
{
	friend class ZFXPlane;

private:
	ZFXPlane		m_Plane;	//plane of polygon
	int				m_NumP;		//number of points
	int				m_NumI;		//number of indices
	ZFXAABB			m_Aabb;		//bounding box
	unsigned int	m_Flag;		//flag for free use
	ZFXVector		*m_pPoints;	//points defining the polygon
	unsigned int	*m_pIndic;	//indices for triangulation of the polygon

	void CalcBoundingBox();

public:
	ZFXPolygon();
	~ZFXPolygon();

	void			Set(const ZFXVector*, int, const unsigned int*, int);
	void			Clip(const ZFXPlane &plane, ZFXPolygon *pFront, ZFXPolygon *pBack );
	void			Clip(const ZFXAABB &aabb);
	int				Cull(const ZFXAABB &aabb); 
	void			CopyOf(const ZFXPolygon &poly);

	void			SwapFaces();
	bool			Intersects(const ZFXRay&, bool, float* t,float fL = -1);

	int				GetNumPoints()	{ return m_NumP; }
	int				GetNumIndic()	{ return m_NumI; }
	ZFXVector*		GetPoints()		{ return m_pPoints; }
	unsigned int*	GetIndices()	{ return m_pIndic; }
	ZFXPlane		GetPlane()		{ return m_Plane; }
	ZFXAABB			GetAabb()		{ return m_Aabb; }
	unsigned int	GetFlag()		{ return m_Flag; }
	void			SetFlag(unsigned int n) { m_Flag = n; }

};

//Quaternion
class __declspec(dllexport) ZFXQuat
{
public:
	float x, y, z, w;

	ZFXQuat() { x = 0.0f, y = 0.0f, z = 0.0f, w = 1.0f; }
	ZFXQuat(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }

	void  CreateFromEuler(float fPitch, float fYaw, float fRoll);
	void  Normalize();
	void  Conjugate(ZFXQuat q);
	void  GetEulers(float *fPitch, float *fYaw, float *fRoll);
	void  GetMatrix(ZFXMatrix *m);
	float GetMagnitude();

	void    operator /= (float f);
	ZFXQuat operator /  (float f);

	void    operator *= (float f);
	ZFXQuat operator *  (float f);

	ZFXQuat operator *  (const ZFXVector &v) const;

	ZFXQuat operator *	(const ZFXQuat &q)const;
	void	operator *= (const ZFXQuat &q);

	void    operator += (const ZFXQuat &q);
	ZFXQuat operator +  (const ZFXQuat &q) const;

	ZFXQuat operator~(void) const { return ZFXQuat(-x, -y, -z, w); }

	void Rotate(const ZFXQuat &q1, const ZFXQuat &q2);

	ZFXVector Rotate(const ZFXVector &v);
};

//Polygon List - Half stack
class ZFXPolyList
{
public:
   ZFXPolyList();
   ~ZFXPolyList();

   bool           AddPolygon(const ZFXPolygon&);
   void           Reset();
   ZFXPolygon*    GetPolylist() { return m_pPolys; }
   unsigned int   GetNum()      { return m_Num; }

private:
   ZFXPolygon     *m_pPolys;
   unsigned int   m_Num;
   unsigned int   m_Max;

   bool           CheckMem();
};

//Binary Space Partitioning Tree -- leafy variety
class ZFXBSPTree
{
public:
   ZFXBSPTree();
   virtual ~ZFXBSPTree();

   void     BuildTree(const ZFXPolygon*, UINT);
   void     TraverseBtF(ZFXPolyList*, ZFXVector, const ZFXPlane*);
   void     TraverseFtB(ZFXPolyList*, ZFXVector, const ZFXPlane*);
   
   ZFXAABB  GetAABB() { return m_AABB;}
   
   bool LineOfSight(const ZFXVector&, const ZFXVector&);

   bool TestCollision(const ZFXRay&, float, float*, ZFXVector*);
private:
   ZFXAABB        m_AABB;     // bounding box
   ZFXPlane       m_Plane;    // splitting plane
   ZFXBSPTree     *m_pBack;   // backlist
   ZFXBSPTree     *m_pFront;  // frontlist
   ZFXBSPTree     *m_pRoot;   // root node
   ZFXBSPTree     *m_pParent; // parent node
   ZFXPolygon     *m_pPolys;  // if leaf node
   UINT           m_NumPolys; // if leaf node

   static UINT m_sNum;        // final poly count

   void CreateChildren();
   bool FindBestSplitter();
   void AddPolygon(const ZFXPolygon&);
   void CalcBoundingBox(const ZFXPolygon*, UINT);
   void SetRelationship(ZFXBSPTree *R, ZFXBSPTree *D) { m_pParent = D; m_pRoot = R;}

   bool IsLeaf() { return (m_pFront == NULL && m_pBack == NULL ); }
};