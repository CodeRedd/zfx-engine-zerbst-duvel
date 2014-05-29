//File: ZFX.h 
//Created by Stefan Zerbst and Oliver Duvel
//Enumeration code originally found only on CD-ROM accompanying book, which I am missing.
//Found copy of macro code on Google Code here: https://code.google.com/p/3d-zfxengine/source/browse/ZFXEngine/ZFXD3D/ZFX.h?r=3&spec=svn3
//Google Code implementation by contealucard
//I am distributing via GNU GPLv3--see License.txt for details.

#pragma once 

#include <Windows.h>

// everything went smooth
#define ZFX_OK                        S_OK

// just reports no errors
#define ZFX_CANCELED                  0x02000001

// generell error message
#define ZFX_FAIL                      0x82000001

// specific error messages
#define ZFX_CREATEAPI                 0x82000002
#define ZFX_CREATEDEVICE              0x82000003
#define ZFX_CREATEBUFFER              0x82000004
#define ZFX_INVALIDPARAM              0x82000005
#define ZFX_INVALIDID                 0x82000006
#define ZFX_BUFFERSIZE                0x82000007
#define ZFX_BUFFERLOCK                0x82000008
#define ZFX_NOTCOMPATIBLE             0x82000009
#define ZFX_OUTOFMEMORY               0x8200000a
#define ZFX_FILENOTFOUND              0x8200000b
#define ZFX_INVALIDFILE               0x8200000c
#define ZFX_NOSHADERSUPPORT           0x8200000d
#define ZFX_FAILa               0x8200000c
#define ZFX_FAILb               0x8200000d
#define ZFX_FAILc               0x8200000e
#define ZFX_FAILd               0x8200000f

//ENUMS
typedef enum ZFXENGINEMODE
{
	EMD_PERSPECTIVE, //perspective projection
	EMD_TWOD,		 //world equals screen coords
	EMD_ORTHOGONAL	 //orthogonal projection
};

typedef enum ZFXVERTEXID
{
	VID_PS,       // untransformed position only
	VID_UU,       // untransformed and unlit vertex
	VID_UL,       // untransformed and lit vertex vertex
	VID_CA,       // used for character animation
	VID_3T,       // three texture coord pairs
	VID_TV        // like UU but with tangent vector
};

typedef enum ZFXRENDERSTATE {
	RS_NONE,             // just nothing
	RS_CULL_CW,          // cull clockwise ordered triangles
	RS_CULL_CCW,         // cull counter cw ordered triangles
	RS_CULL_NONE,        // render front- and backsides
	RS_DEPTH_READWRITE,  // read and write depth buffer
	RS_DEPTH_READONLY,   // read but don't write depth buffer
	RS_DEPTH_NONE,       // no read or write with depth buffer
	RS_SHADE_POINTS,     // render just vertices
	RS_SHADE_LINES,      // render two verts as one line
	RS_SHADE_TRIWIRE,    // render triangulated wire
	RS_SHADE_HULLWIRE,   // render poly hull as polyline
	RS_SHADE_SOLID,      // render solid polygons
	RS_TEX_ADDSIGNED,    // texture stage operation
	RS_TEX_MODULATE,     // texture stage operation
	RS_STENCIL_DISABLE,        // stencilbuffer off
	RS_STENCIL_ENABLE,         // stencilbuffer off
	RS_STENCIL_FUNC_ALWAYS,    // stencil pass mode
	RS_STENCIL_FUNC_LESSEQUAL, // stencil pass mode
	RS_STENCIL_MASK,           // stencil mask
	RS_STENCIL_WRITEMASK,      // stencil write mask
	RS_STENCIL_REF,            // reference value
	RS_STENCIL_FAIL_DECR,      // stencil fail decrements
	RS_STENCIL_FAIL_INCR,      // stencil fail increments
	RS_STENCIL_FAIL_KEEP,      // stencil fail keeps
	RS_STENCIL_ZFAIL_DECR,     // stencil pass but z fail decrements
	RS_STENCIL_ZFAIL_INCR,     // stencil pass but z fail increments
	RS_STENCIL_ZFAIL_KEEP,     // stencil pass but z fail keeps
	RS_STENCIL_PASS_DECR,      // stencil and z pass decrements
	RS_STENCIL_PASS_INCR,      // stencil and z pass increments
	RS_STENCIL_PASS_KEEP,      // stencil and z pass keeps
	RS_DEPTHBIAS               // bias value to add to depth value
};

//STRUCTS
struct ZFXVIEWPORT
{
	DWORD X; //position of upper ...
	DWORD Y; // ...left corner
	DWORD Width;
	DWORD Height;
};


struct ZFXCOLOR
{
	union
	{
		struct
		{
			float fR;
			float fG;
			float fB;
			float fA;
		};
		float c[4];
	};
}; 

//this is similar to D3D9 material, but we want to stay API-independent
struct ZFXMATERIAL
{
	ZFXCOLOR cDiffuse;	//RGBA diffuse light
	ZFXCOLOR cAmbient;	//RGBA ambient light
	ZFXCOLOR cSpecular;	//RGBA specular light
	ZFXCOLOR cEmissive; //RGBA emissive light
	float	 fPower;	//specular exponent
};

//also similar to D3D9 but again we want to stay independent of API
struct ZFXTEXTURE
{
	float		fAlpha;		//transparency
	TCHAR		*chName;	//texture filename--also used as ID for texture so that we don't load the same file more than once
	void		*pData;		//texture data
	ZFXCOLOR	*pClrKeys;	//color key array -- adjust an RGB color to a specific alpha value
	DWORD		dwNum;		//number of color keys
};

struct ZFXSKIN
{
	bool	bAlpha;		//do we use non-unity alpha values?
	UINT	nMaterial;
	UINT	nTexture[8];//can hold up to 8 textures to support multi-pass rendering
};

// VERTEX TYPES:

struct PVERTEX {
	float	 x, y, z;
};

struct VERTEX {
	float	 x, y, z;
	float  vcN[3];
	float  tu, tv;
};

struct LVERTEX {
	float	 x, y, z;
	DWORD  Color;
	float  tu, tv;
};

struct CVERTEX {
	float	 x, y, z;
	float  vcN[3];
	float  tu, tv;
	float  fBone1, fWeight1;
	float  fBone2, fWeight2;
};

struct VERTEX3T {
	float	 x, y, z;
	float  vcN[3];
	float  tu0, tv0;
	float  tu1, tv1;
	float  tu2, tv2;
};

struct TVERTEX {
	float	 x, y, z;
	float  vcN[3];
	float  tu, tv;
	float  vcU[3];
};