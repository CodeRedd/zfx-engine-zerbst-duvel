//File: ZFXRenderer.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd
#include "ZFXRenderer.h"

//C++ Learning note: all of the strings used for WinAPI functions have an L in front of them because these functions take wchar_t (Unicode-supporting version of wide character) arrays rather than standard char*

//Construct and destruct
ZFXRenderer::ZFXRenderer(HINSTANCE hInst)
{
	m_hInst		= hInst;
	m_hDLL		= NULL;
	m_pDevice	= NULL;
}

ZFXRenderer::~ZFXRenderer()
{
	Release();
}

//Create an object implementing the RenderDevice interface
//@param *chAPI - the name identifying the DLL to be referenced
HRESULT ZFXRenderer::CreateDevice(wchar_t *chAPI)
{
	wchar_t buffer[300];

	//covers case where we want to load Direct3D Library
	if (wcscmp(chAPI, L"Direct3D") == 0)
	{
		//this function does the actual DLL load (making sure it isn't loaded already) and sends back the handle for the DLL
		m_hDLL = LoadLibrary(L"ZFXD3D.dll");
		if (!m_hDLL)
		{
			MessageBox(NULL, L"Loading ZFXD3D.dll failed.", L"ZFXEngine - error", MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
	}
	//Right now we have an error message for other cases, but we could also have an OpenGL option in here.
	else
	{
		_snwprintf_s(buffer, 300, L"API '%s' not supported.", chAPI);
		MessageBox(NULL, buffer, L"ZFXEngine - error", MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	CREATERENDERDEVICE _CreateRenderDevice = 0;
	HRESULT hr;

	//pointer to DLL function 'CreateRenderDevice'--we have to get a pointer this way because the linker cannot find the function definition from 
	//here since it doesn't have the compiled DLL.
	//Get Proc Address will "sniff" inside the DLL to check for the address of functions that are exported at runtime
	_CreateRenderDevice = (CREATERENDERDEVICE)GetProcAddress(m_hDLL, "CreateRenderDevice");
	if (!_CreateRenderDevice)
	{
		return E_FAIL;
	}

	//call DLL function to create the device
	hr = _CreateRenderDevice(m_hDLL, &m_pDevice);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"CreateRenderDevice() from lib failed.", L"ZFXEngine - error", MB_OK | MB_ICONERROR);
		m_pDevice = NULL;
		return E_FAIL;
	}

	return S_OK;
}

//Releases the object that implements the RenderDevice interface
void ZFXRenderer::Release()
{
	RELEASERENDERDEVICE _ReleaseRenderDevice = 0;
	HRESULT hr;

	if (m_hDLL)
	{
		//pointer to DLL function 'ReleaseRenderDevice'
		_ReleaseRenderDevice = (RELEASERENDERDEVICE)GetProcAddress(m_hDLL, "ReleaseRenderDevice");
	}

	//call the DLL's release function
	if (m_pDevice)
	{
		hr = _ReleaseRenderDevice(&m_pDevice);
		if (FAILED(hr))
		{
			m_pDevice = NULL;
		}
	}
}
