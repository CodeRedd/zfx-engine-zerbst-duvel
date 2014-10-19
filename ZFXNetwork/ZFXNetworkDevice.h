//File: ZFXNetworkDevice.h
//Created by Stefan Zerbst and Oliver Duvel

#pragma once

#include <Windows.h>
#include <ZFX.h>
#include <ZFX3D.h>

struct ZFXPACKET
{
	UINT	nLength;	//length of pData in bytes
	UCHAR	nType;		//message type ID
	UINT	nSender;	//sender ID where 0 is the server
	void	*pData;		//packet bytes
};
/*	Reserved Values For nType:
0 - First message client receives from server after connection is established. pData should contain client ID as unsigned int
1 - Message from server to all clients that new client has been added to network. pData contains new client's ID
2 - Message from server to all clients that one client has been disconnected. pData contains disconnected client's ID
*/

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

