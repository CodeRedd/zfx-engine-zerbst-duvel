//File: ZFXWS.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#pragma once

#include "ZFXSocketObject.h"
#include <ZFXNetworkDevice.h>

struct ZFXCLIENT
{
   SOCKET skToClient;
   UINT   nID;
};

class ZFXWS : public ZFXNetworkDevice
{
public:
   ZFXWS(HINSTANCE hDLL);
   ~ZFXWS();

   //interface functions
   HRESULT  Init(HWND, ZFXNETMODE, int, char*, UINT, bool);
   void     Release();
   bool     IsRunning() { return m_bRunning; }
   HRESULT  MsgProc(WPARAM wp, LPARAM lp);
   HRESULT  SendToServer(const ZFXPACKET*);
   HRESULT  SendToClients(const ZFXPACKET*);
   HRESULT  ServerUpdate();
   bool     IsPktWaiting()                { return m_pSockObj->IsPktWaiting(); }
   UINT     GetNextPktSize()              { return m_pSockObj->GetNextPktSize(); }
   HRESULT  GetNextPkt(ZFXPACKET *pPkt)   { return m_pSockObj->GetNextPkt(pPkt); }

private:
   ZFXSocketObject   *m_pSockObj;
   ZFXNETMODE        m_Mode;
   ZFXCLIENT         m_Clients[256];
   char              m_ClCount;
   UINT              m_ClID;
   char              *m_Buffer;

   //initialize socket objects
   HRESULT CreateServer(ZFXSocketObject **ppSkObject);
   HRESULT CreateClient(ZFXSocketObject **ppSkObject);

   //message processing
   HRESULT OnAccept();
   HRESULT OnReceive(SOCKET skReceiving);
   HRESULT OnDisconnect(SOCKET skDisconnecting);
};