//File: ZFXNetworkDevice.h
//Created by Stefan Zerbst and Oliver Duvel

#pragma once

#include <Windows.h>
#include <ZFX.h>

enum ZFXNETMODE
{
   NMD_SERVER = 0,
   NMD_CLIENT = 1,
};

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
   HWND		   m_hWndMain;		//main window
   HINSTANCE	m_hDLL;			// DLL handle
   bool		   m_bRunning;		// init done?
   int         m_nPort;       // port number
   char        m_pIP[256];    // IP Address
   UINT        m_nMaxSize;    // buffer size

public:
   ZFXNetworkDevice() {}
   virtual ~ZFXNetworkDevice() {}

   //init and release
   virtual HRESULT   Init(HWND, ZFXNETMODE, int Port, char *IP, UINT Size, bool) = 0;
   virtual void      Release() = 0;
   virtual bool      IsRunning() = 0;

   //message procedure
   virtual HRESULT   MsgProc(WPARAM, LPARAM) = 0;

   //send and receive
   virtual HRESULT   SendToServer(const ZFXPACKET*) = 0;
   virtual HRESULT   SendToClients(const ZFXPACKET*) = 0;

   //information about inbox queue
   virtual bool      IsPktWaiting() = 0;
   virtual UINT      GetNextPktSize() = 0;
   virtual HRESULT   GetNextPkt(ZFXPACKET*) = 0;
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

