//File: ZFXNetworkDevice.h
//Created by Stefan Zerbst and Oliver Duvel

#pragma once

#include <Windows.h>
#include <ZFX.h>
#include <ZFX3D.h>

class ZFXNetworkDevice
{
protected:
	HWND		m_hWndMain;		//main window
	HINSTANCE	m_hDLL;			// DLL handle
	bool		m_bRunning;		// init done?

public:
	ZFXNetworkDevice() {}
	virtual ~ZFXNetworkDevice() {}

};
typedef class ZFXNetworkDevice *LPZFXNETWORKDEVICE;

/*----------------------------------------------------------------*/

extern "C"
{
	HRESULT CreateNetworkDevice(HINSTANCE hDLL, ZFXNetworkDevice **pInterface);
	typedef HRESULT(*CREATENETWORKDEVICE)(HINSTANCE hDLL, ZFXNetworkDevice **pInterface);

	HRESULT ReleaseNetworkDevice(ZFXNetworkDevice **pInterface);
	typedef HRESULT(*RELEASENETWORKDEVICE)(ZFXNetworkDevice **pInterface);
}

