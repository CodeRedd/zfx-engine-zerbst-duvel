//File: ZFX3D.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include <Windows.h>
#include <math.h>

//CPU Info Stuff
typedef struct CPUINFO_TYPE {
	bool bSSE;			//Streaming SIMD Extensions
	bool bSSE2;			//Streaming SIMD Extensions 2
	bool b3DNOW;		//3DNow! (vendor independent)
	bool bMMX;			//MMX support
	TCHAR name[48];		//cpu name
	bool bEXT;			//extended features available
	bool bMMXEX;		//MMX (AMD specific extensions)
	bool b3DNOWEX;		//3DNow! (AMD specific extensions)
	TCHAR vendor[13];	//vendor name
}	CPUINFO;

CPUINFO GetCPUInfo();
void GetCPUName(TCHAR *chName, int n, const TCHAR *vendor);
bool OSSupportsSSE();
bool ZFX3DInitCPU();

//Forward Declarations
class ZFXVector;
class ZFXMatrix;
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