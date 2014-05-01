//File: ZFXD3D.h 
//Created by Stefan Zerbst and Oliver Duvel
//Partially Reimplemented by Culver Redd
//Enumeration code originally found only on CD-ROM accompanying book, which I am missing.
//Found copy of enum code on Google Code here: https://code.google.com/p/3d-zfxengine/source/browse/ZFXEngine/ZFXD3D/ZFXD3D.h?r=3
//Google Code implementation by contealucard
//I am distributing via GNU GPLv3--see License.txt for details.

#include <Windows.h>
#include <D3D9.h>
#include "..\ZFXRenderDevice.h"

#define MAX_3DHWND 8
#define MAX_SHADER 20


//vertex type definitions
#define FVF_VERTEX  ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 )
#define FVF_PVERTEX ( D3DFVF_XYZ )
#define FVF_LVERTEX ( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 )
#define FVF_CVERTEX ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 )
#define FVF_T3VERTEX ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX3 )
#define FVF_TVERTEX ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0) )


//creates some macros for utility functions that should exist outside the RenderDevice interface
//These allow us to get a pointer to something implementing ZFXRenderDevice (in the DLL) from outside the library. So this is what actually lets us use the stuff we're writing!
//"Extern C" exports the functions as C code to reduce overhead of C++. Exporting as C++ would also force modification of names and parameter lists that we don't want to deal with.
//__declspec(dllexport) is what marks these declarations as exported functions

extern "C"				__declspec(dllexport) HRESULT CreateRenderDevice(HINSTANCE hDLL, ZFXRenderDevice **pInterface);
extern "C"				__declspec(dllexport) HRESULT ReleaseRenderDevice(ZFXRenderDevice **pInterface);

// one for each AdapterFormat-BackbufferFormat-WindowMode
// (windowed or fullscreen) combination that is valid
struct ZFXCOMBOINFO {
	UINT       nAdapter;             // belongs to
	D3DDEVTYPE d3dDevType;           // HAL, SW, REF
	bool       bWindowed;            // windowed mode

	D3DFORMAT  fmtAdapter;           // pixelbuffer
	D3DFORMAT  fmtBackBuffer;        // backbuffer
	D3DFORMAT  fmtDepthStencil;      // z/stencil format

	DWORD      dwBehavior;           // vertex processing
	D3DMULTISAMPLE_TYPE msType;      // multisample type
};
/*----------------------------------------------------------------*/

// up to three for each adapter
struct ZFXDEVICEINFO {
	UINT         nAdapter;        // belongs to
	D3DDEVTYPE   d3dDevType;      // HAL, SW, REF
	D3DCAPS9     d3dCaps;         // capabilites
	ZFXCOMBOINFO d3dCombo[80];    // combo
	UINT         nNumCombo;       // number of combos
};
/*----------------------------------------------------------------*/

struct ZFXADAPTERINFO {
	D3DADAPTER_IDENTIFIER9 d3dAdapterIdentifier;
	UINT                   nAdapter;         // which one
	D3DDISPLAYMODE         d3ddspmd[150];    // display modes
	UINT                   nNumModes;        // number of modes
	ZFXDEVICEINFO          d3dDevs[3];       // list of devices
	UINT                   nNumDevs;         // number of devices
};
/*----------------------------------------------------------------*/

// enumeration stuff
class ZFXD3DEnum {
public:
	ZFXD3DEnum(int nMinDepth, int nMinStencil)
	{
		m_nMinDepth = nMinDepth;  m_nMinStencil = nMinStencil;
	}
	~ZFXD3DEnum(void) { ;/* nothing to do */ }

	// enumerate all the stuff
	HRESULT Enum(HWND, HWND, HWND, HWND, HWND, HWND, HWND, FILE*);

	// combobox selection changed
	void ChangedAdapter(void);
	void ChangedDevice(void);
	void ChangedAdapterFmt(void);
	void ChangedWindowMode(void);

	// get final settings to crank up
	void GetSelections(ZFXDEVICEINFO *pD, D3DDISPLAYMODE *dspmd,
		D3DFORMAT *fmtA, D3DFORMAT *fmtB);


	LPDIRECT3D9    m_pD3D;
	ZFXADAPTERINFO m_xAdapterInfo[10];
	DWORD          m_dwNumAdapters;


private:
	D3DDISPLAYMODE m_dspmd;    // current desktop display mode
	D3DFORMAT m_fmtAdapter[5]; // list of possible adapter formats
	UINT      m_nNumFmt;       // adapter formats possible
	UINT      m_nMinWidth;     // minimum screen with
	UINT      m_nMinHeight;    // minimum screen height
	UINT      m_nMinBits;      // minimum backbuffer bits
	UINT      m_nMinDepth;     // minimum depth bits
	UINT      m_nMinStencil;   // minimum stencil bits
	FILE     *m_pLog;          // log file opened by zfxd3d class

	// handle to GUI items given from zfxd3d dialog
	HWND m_hADAPTER;           // adapter combobox 
	HWND m_hMODE;              // mode combobox
	HWND m_hDEVICE;            // device combobox
	HWND m_hADAPTERFMT;        // adapter format combobox
	HWND m_hBACKFMT;           // backbuffer fmt combobox
	HWND m_hWND;               // radiobtn windowed
	HWND m_hFULL;              // radiobtn fullscreen

	HRESULT EnumAdapters(void);
	HRESULT EnumDevices(ZFXADAPTERINFO &);
	HRESULT EnumCombos(ZFXDEVICEINFO &);

	UINT    GetBits(D3DFORMAT);
	HRESULT ConfirmDevice(D3DCAPS9*, DWORD, D3DFORMAT);
	bool    ConfirmDepthFmt(ZFXCOMBOINFO*);

	//helper functions
	void	AddItem(HWND hWnd, TCHAR *ch, void *pData);
	void*	GetSelectedItem(HWND hWnd);
	bool	ContainsString(HWND hWnd, TCHAR *ch);
	TCHAR*	D3DDevTypeToString(D3DDEVTYPE devType);
	TCHAR*	D3DFormatToString(D3DFORMAT format);
	TCHAR*	BehaviorTypeToString(DWORD vpt);



};

class ZFXD3D : public ZFXRenderDevice
{
public:

	//construct and destruct
	ZFXD3D(HINSTANCE hDLL);
	~ZFXD3D();

	//Initialize
	HRESULT Init(HWND, const HWND*, int, int, int, bool);
	HRESULT InitWindowed(HWND, const HWND*, int, bool);

	//Pathway for graphics options UI to access DLL functions 
	BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
	
	/////////////////////////
	// INTERFACE FUNCTIONS
	/////////////////////////

	//general
	void	Release();
	bool	IsRunning() { return m_bIsSceneRunning; }
	HRESULT BeginRendering(bool, bool, bool);
	HRESULT Clear(bool, bool, bool);
	void	EndRendering();
	void	SetClearColor(float, float, float);
	HRESULT	UseWindow(UINT nHwnd);
	ZFXSkinManager* GetSkinManager(){ return m_pSkinMan; }

	//view stuff
	HRESULT   SetView3D(const ZFXVector&, const ZFXVector&,	const ZFXVector&, const ZFXVector&);
	HRESULT   SetViewLookAt(const ZFXVector&, const ZFXVector&,	const ZFXVector&);
	void      SetClippingPlanes(float, float);
	HRESULT   SetMode(ZFXENGINEMODE, int n);
	HRESULT   InitStage(float, ZFXVIEWPORT*, int n);
	HRESULT   GetFrustum(ZFXPlane*);
	void      Transform2DTo3D(const POINT&, ZFXVector*, ZFXVector*);
	POINT     Transform3DTo2D(const ZFXVector&);
	void      SetWorldTransform(const ZFXMatrix*);

	/////////////////////////
	// SHADER FUNCTIONS
	/////////////////////////
	
	HRESULT CreateVShader(const void*, UINT, bool, bool, UINT*);
	HRESULT CreatePShader(const void*, UINT, bool, bool, UINT*);
	HRESULT ActivateVShader(UINT, ZFXVERTEXID);
	HRESULT ActivatePShader(UINT);
	bool	UsesShaders() { return m_bUseShaders; }

private:

	//general
	ZFXD3DEnum				*m_pEnum;
	LPDIRECT3D9				m_pD3D;
	LPDIRECT3DDEVICE9		m_pDevice;
	LPDIRECT3DSWAPCHAIN9	m_pChain[MAX_3DHWND]; // Using Direct3D "swap chains" which take one rendered image each, you can render out multiple views for use in a 
												  // level editor, etc.
	D3DPRESENT_PARAMETERS	m_d3dpp;
	D3DCOLOR				m_ClearColor;
	bool					m_bIsSceneRunning;
	bool					m_bStencil;

	//Start the API
	HRESULT Go();

	//System logging function
	void Log(TCHAR *, ...);

	//view stuff
	D3DMATRIX	m_mView2D,			//view matrix 2D
				m_mView3D,			//view matrix 3D
				m_mProj2D,			//orthogonal projection for 2D
				m_mProjP[4],		//perspective projections
				m_mProjO[4],		//orthogonal projections
				m_mWorld,			//world transformation
				m_mViewProj,		//combo-matrix for 3D
				m_mWorldViewProj;	//combo-matrix for 3D
	void    Prepare2D(void);
	void    CalcViewProjMatrix(void);
	void    CalcWorldViewProjMatrix(void);
	HRESULT CalcPerspProjMatrix(float, float, D3DMATRIX*);
	//void    CalcOrthoProjMatrix(float, float, float, float, float, float, int);

	//shader stuff
	LPDIRECT3DVERTEXDECLARATION9      m_pDeclVertex;   
	LPDIRECT3DVERTEXDECLARATION9      m_pDeclPVertex;  
	LPDIRECT3DVERTEXDECLARATION9      m_pDeclLVertex;  
	LPDIRECT3DVERTEXDECLARATION9      m_pDeclCVertex;  
	LPDIRECT3DVERTEXDECLARATION9      m_pDecl3TVertex; 
	LPDIRECT3DVERTEXDECLARATION9      m_pDeclTVertex;  
	
	LPDIRECT3DVERTEXSHADER9		m_pVShader[MAX_SHADER];
	LPDIRECT3DPIXELSHADER9		m_pPShader[MAX_SHADER];
	UINT								m_nNumVShaders;
	UINT								m_nNumPShaders;
	
	void    PrepareShaderStuff(void);
};


