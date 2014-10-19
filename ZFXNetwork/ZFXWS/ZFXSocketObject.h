//File: ZFXSocketObject.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#pragma once

#include <WinSock2.h>
#include "ZFXWS.h"

class ZFXSocketObject
{
public:
   ZFXSocketObject();
   ~ZFXSocketObject();

   HRESULT  CreateSocket();
   void     Disconnect();

   bool IsRunning() { return m_bRunning; }
   SOCKET   GetSocket() {return m_skSocket; }

   //Server functions
   HRESULT  Bind(int nPort);
   HRESULT  Listen();
   HRESULT  Accept(SOCKET *skToNewClient);

   //Client functions
   HRESULT  Connect(char *chServer, int nPort);
   

   //Send and receive
   int      Send(const char*, UINT);
   int      Send(const char*, UINT, SOCKET);
   HRESULT  Receive(SOCKET sk);

   //queue manipulation functions
   bool     IsPkgWaiting()    { return ( m_pInbox->GetCount() > 0 ); }
   UINT     GetNextPktize()  { return ( m_pInbox->GetFrontSize()); }
   HRESULT  GetNextPkt(ZFXPACKET*);

private:
   ZFXQueue    *m_pInbox;
   SOCKET      m_skSocket;
   char        *m_Buffer;
   bool        m_bRunning;
};