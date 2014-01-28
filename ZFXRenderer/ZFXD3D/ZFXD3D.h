//File: ZFXD3D.h 
//Created by Stefan Zerbst and Oliver Duvel
//Partially Reimplemented by Culver Redd
//Enumeration code originally found only on CD-ROM accompanying book, which I am missing.
//Found copy of enum code on Google Code here: https://code.google.com/p/3d-zfxengine/source/browse/ZFXEngine/ZFXD3D/ZFXD3D.h?r=3
//Google Code implementation by contealucard
//I am distributing via GNU GPLv3--see License.txt for details.

#include <Windows.h>
#include <d3d9.h>

#include "../ZFXRenderer/ZFXRenderDevice.h"


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

};

#define MAX_3DHWND 8

class ZFXD3D : public ZFXRenderDevice
{
public:

	//construct and destruct
	ZFXD3D(HINSTANCE hDLL);
	~ZFXD3D();

	//Initialize
	HRESULT Init(HWND, const HWND*, int, int, int, bool);

	//Pathway for graphics options UI to access DLL functions 
	BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

	//Interface functions
	void	Release();
	bool	IsRunning() { return m_bIsRunning; }
	HRESULT BeginRendering(bool, bool, bool);
	HRESULT Clear(bool, bool, bool);
	void	EndRendering();
	void	SetClearColor(float, float, float);
	HRESULT	UseWindow(UINT nHwnd);

private:
	ZFXD3DEnum				*m_pEnum;
	LPDIRECT3D9				m_pD3D;
	LPDIRECT3DDEVICE9		m_pDevice;
	LPDIRECT3DSWAPCHAIN9	m_pChain[MAX_3DHWND]; // Using Direct3D "swap chains" which take one rendered image each, you can render out multiple views for use in a 
												  // level editor, etc.
	D3DPRESENT_PARAMETERS	m_d3dpp;
	D3DCOLOR				m_ClearColor;
	bool					m_bIsRunning;
	bool					m_bStencil;

	//Start the API
	HRESULT Go();

	//System logging function
	void Log(char *, ...);
};


