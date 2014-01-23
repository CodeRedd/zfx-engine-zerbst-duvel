//File: ZFXRenderDevice.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include <Windows.h> //needed for many functions and macros
#include <cstdio>	//needed for the FILE struct

#define MAX_3DHWND 8

//Interface for the ZFX Engine 2.0 renderer. Makes the Renderer itself graphics-API-agnostic (Zerbst and Duvel use Direct3D, though, and I will as well)
class ZFXRenderDevice
	{
	//Zerbst notes that it is bad form for "hardcore" C++ to define attributes in abstract classes, but that he is less strict because he feels it's more important that
	//the derived classes don't have a boatload of attributes
	protected:
		HWND		m_hWndMain;			//main window
		HWND		m_hWnd[MAX_3DHWND];	//render window(s)
		UINT		m_nNumWnd;			//number of render windows
		UINT		m_nActivehWnd;		//active window
		HINSTANCE	m_hDLL;				//DLL Module
		DWORD		m_dwWidth;			//screen width
		DWORD		m_dwHeight;			//screen height
		bool		m_bWindowed;		//windowed mode?
		char		m_chAdapter[256];	//graphics adapter name
		FILE	*m_pLog;			//log file
		bool		m_bRunning;			//Is the renderer running?

	public:
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
	};
typedef struct ZFXRenderDevice *LPZFXRENDERDEVICE;

//create some macros for utility functions that should exist outside the RenderDevice interface
//These allow us to get a pointer to something implementing ZFXRenderDevice (in the DLL) from outside the library. So this is what actually lets us use the stuff we're writing!
//"Extern C" exports the functions as C code to reduce overhead of C++. Exporting as C++ would also force modification of names and parameter lists that we don't want to deal with.
extern "C"
{
	HRESULT CreateRenderDevice(HINSTANCE hDLL, ZFXRenderDevice **pInterface);

	typedef HRESULT(*CREATERENDERDEVICE) (HINSTANCE hDLL, ZFXRenderDevice**pInterface);

	HRESULT ReleaseRenderDevice(ZFXRenderDevice **pInterface);

	typedef HRESULT(*RELEASERENDERDEVICE) (ZFXRenderDevice **pInterface);
}