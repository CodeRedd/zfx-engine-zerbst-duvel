//File: ZFXRenderDevice.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include <Windows.h> //needed for many functions and macros
#include <cstdio>	//needed for the FILE struct

#include "ZFXD3D\ZFX.h"
#include <ZFX3D.h>

#define ARRAY_ALLOCATION_SIZE 50
#define MAX_3DHWND			  8


//Interface for the ZFX Engine 2.0 skin manager
class ZFXSkinManager
{
protected:
	UINT		m_nNumSkins;		//number of skins
	UINT		m_nNumMaterials;	//number of materials
	UINT		m_nNumTextures;		//number of textures
	ZFXSKIN		*m_pSkins;
	ZFXMATERIAL	*m_pMaterials;
	ZFXTEXTURE	*m_pTextures;

public:

	ZFXSkinManager() {};
	virtual ~ZFXSkinManager() {};

	virtual HRESULT AddSkin(const ZFXCOLOR *pcAmbient, const ZFXCOLOR *pcDiffuse, const ZFXCOLOR *pcEmissive,
		const ZFXCOLOR *pcSpecular, float fSpecPower, UINT *nSkinID) = 0;

	virtual HRESULT AddTexture(UINT nSkinID, const TCHAR *chName, bool bAlpha, float fAlpha,
		ZFXCOLOR *cColorKeys, DWORD dwNumColorKeys) = 0;

	virtual bool MaterialEqual(const ZFXMATERIAL *pMat0, const ZFXMATERIAL *pMat1) = 0;

	virtual void LogCurrentStatus(TCHAR *chLog, bool bDetail) = 0;
};

//Interface for the ZFX Engine 2.0 renderer. Makes the Renderer itself graphics-API-agnostic (Zerbst and Duvel use Direct3D, though, and I will as well)
class ZFXRenderDevice
	{	
	//Zerbst notes that it is bad form for "hardcore" C++ to define attributes in abstract classes, but that he is less strict because he feels it's more important that
	//the derived classes don't have a boatload of attributes
	protected:

		//init
		HWND		m_hWndMain;				//main window
		HWND		m_hWnd[MAX_3DHWND];		//render window(s)
		UINT		m_nNumWnd;				//number of render windows
		UINT		m_nActivehWnd;			//active window
		HINSTANCE	m_hDLL;					//DLL Module
		DWORD		m_dwWidth;				//screen width
		DWORD		m_dwHeight;				//screen height
		bool		m_bWindowed;			//windowed mode?
		TCHAR		m_chAdapter[256];		//graphics adapter name
		FILE	*m_pLog;					//log file
		bool		m_bRunning;				//Is the renderer running?

		//managers
		ZFXSkinManager *m_pSkinMan;			// material and textures

		//view stuff
		float			m_fNear;			// near plane
		float			m_fFar;				// far plane
		ZFXENGINEMODE	m_Mode;				// 2D v 3D
		int				m_nStage;			// stage type (perspective vs orthogonal)
		ZFXVIEWPORT		m_VP[4];			// viewports

		//rendering
		bool            m_bUseShaders;       // shaders or fixed function pipeline
		ZFXRENDERSTATE  m_ShadeMode;         // wireframe rendering?
		ZFXCOLOR        m_clrWire;           // color for wireframe rendering


	public:
				
		/////////////////////
		// INIT STUFF
		/////////////////////
		ZFXRenderDevice() {};
		//note that virtual destructor means that the correct destructor is always called regardless of what pointer is pointing here
		virtual ~ZFXRenderDevice() {};

		//Init and Release Functions
		virtual HRESULT Init(HWND, const HWND*, int, int, int, bool)=0;
		virtual void Release()=0;
		virtual bool IsRunning()=0;


		//Renderer functions
		virtual HRESULT UseWindow(UINT nHwnd)=0;
		virtual HRESULT BeginRendering(bool bClearPixel, bool bClearDepth, bool bClearStencil)=0;
		virtual void EndRendering()=0;
		virtual HRESULT Clear(bool bClearPixel, bool bClearDepth, bool bClearStencil)=0;
		virtual void SetClearColor(float fRed, float fGreen, float fBlue)=0;

		//Manager getters
		virtual ZFXSkinManager* GetSkinManager()=0;

		/////////////////////
		// VIEW STUFF
		/////////////////////

		//view matrix from vRight, vUp, vDir, vPos
		virtual HRESULT SetView3D(const ZFXVector&, const ZFXVector&, const ZFXVector&, const ZFXVector&)=0;

		//view matrix from position, fix point, World Up
		virtual HRESULT SetViewLookAt(const ZFXVector&, const ZFXVector&, const ZFXVector&)=0;

		//near and far clipping planes
		virtual void SetClippingPlanes(float, float)=0;

		//stage mode, 0=perspective, 1=orthogonal
		virtual HRESULT SetMode(ZFXENGINEMODE, int n)=0;

		//FOV and viewport for stage n
		virtual HRESULT InitStage(float, ZFXVIEWPORT*, int n)=0;

		//plane of viewing frustum
		virtual HRESULT GetFrustum(ZFXPlane*)=0;

		//screen coords to world ray
		virtual void Transform2DTo3D(const POINT &pt, ZFXVector *vc0, ZFXVector *vcD)=0;

		//world coords to screen coords
		virtual POINT Transform3DTo2D(const ZFXVector &vcP)=0;

		// set world transformation matrix or NULL
		virtual void SetWorldTransform(const ZFXMatrix*) = 0;

		/////////////////////
		// RENDERING STUFF
		/////////////////////

		virtual void SetBackfaceCulling(ZFXRENDERSTATE)=0;
		virtual void SetDepthBufferMode(ZFXRENDERSTATE)=0;

		virtual void SetShadeMode(ZFXRENDERSTATE, float, const ZFXCOLOR*)=0;
		virtual ZFXRENDERSTATE GetShadeMode()=0;
	};
typedef struct ZFXRenderDevice *LPZFXRENDERDEVICE;

//Normally, the utility functions allowing us to get a pointer to the implementation of ZFXRenderDevice would have a declaration here as well, but
//the .def file used to accomplish this is deprecated as of VC++ 7.0+, so they have been moved to ZFXD3D_export.h in the ZFXD3D project
//"Extern C" exports the functions as C code to reduce overhead of C++. Exporting as C++ would also force modification of names and parameter lists that we don't want to deal with.
extern "C"
{
	typedef HRESULT(*CREATERENDERDEVICE) (HINSTANCE hDLL, ZFXRenderDevice**pInterface);

	typedef HRESULT(*RELEASERENDERDEVICE) (ZFXRenderDevice **pInterface);
}