//File: ZFXWS.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include <stdio.h>
#include "ZFXWS.h"

int g_PktSize = sizeof( ZFXPACKET );

ZFXWS::ZFXWS( HINSTANCE hDLL )
{
   m_hDLL         = hDLL;
   m_pSockObj     = NULL;
   m_Event        = NULL;
   m_Buffer       = NULL;
   m_nPort        = 0;
   m_ClID         = 1;     //0 reserved for server
   m_ClCount      = 0;
   m_bRunning     = false;
}

ZFXWS::~ZFXWS()
{
   Release();
}

HRESULT ZFXWS::Init( HWND hWnd, ZFXNETMODE nmd, int nPort, char *pIP, UINT nMaxPktSize, bool bSaveLog )
{
   WSADATA  wsaData;
   UINT     nEvents = 0;
   WORD     wVersion;
   int      nRes;

   m_nMaxSize  = nMaxPktSize;
   m_Event     = WSACreateEvent();
   m_Buffer    = new char[m_nMaxSize];
   m_hWndMain  = hWnd;
   m_nPort     = nPort;
   m_Mode      = nmd;

   if ( pIP )
   {
      sprintf_s(m_pIP, "%s", pIP);
   }

   wVersion = MAKEWORD(2,0);

   if ( ( nRes = WSAStartup( wVersion, &wsaData ) ) != 0 )
   {
      return ZFX_FAIL;
   }

   //create master socket object and start it listening
   if ( m_Mode == NMD_SERVER )
   {
      if ( FAILED( CreateServer( &m_pSockObj ) ) )
      {
         return ZFX_FAIL;
      }
   }
   else if ( m_Mode == NMD_CLIENT )
   {
      if ( strcmp( m_pIP, "" ) == 0 )
      {
         sprintf_s(m_pIP, "LOCALHOST");
      }

      if ( FAILED( CreateClient( &m_pSockObj ) ) )
      {
         return ZFX_FAIL;
      }
   }
   else
   {
      return ZFX_INVALIDPARAM;
   }

   m_bRunning = true;
   return ZFX_OK;
}

void ZFXWS::Release()
{
   if (m_Mode == NMD_SERVER )
   { 
      for ( int i = 0; i < m_ClCount; i++ )
      {
         shutdown(m_Clients[i].skToClient,0x02);
         closesocket(m_Clients[i].skToClient);
         m_Clients[i].skToClient = INVALID_SOCKET;
      }
   }
   
   if ( m_pSockObj )
   {
      delete m_pSockObj;
      m_pSockObj = NULL;
   }

   if ( m_Event )
   {
      WSACloseEvent(m_Event);
      delete m_Event;
      m_Event = NULL;
   }

   if ( m_Buffer )
   {
      delete[] m_Buffer;
      m_Buffer = NULL;
   }
   WSACleanup();
   m_bRunning = false;
}

HRESULT ZFXWS::CreateServer( ZFXSocketObject **ppSkObject )
{
   UINT nEvents = 0;

   ( *ppSkObject ) = new ZFXSocketObject();

   if ( !( *ppSkObject ) )
   {
      return ZFX_FAIL;
   }

   //1. create a socket object
   if ( FAILED( ( *ppSkObject )->CreateSocket() ) )
   {
      return ZFX_FAIL;
   }

   //2. Bind to port
   if ( FAILED( ( *ppSkObject )->Bind(m_nPort) ) )
   {
      return ZFX_FAIL;
   }

   //3. start listening
   if ( FAILED( ( *ppSkObject )->Listen() ) )
   {
      return ZFX_FAIL;
   }

   nEvents |= FD_READ | FD_WRITE | FD_CONNECT | FD_ACCEPT | FD_CLOSE;

   //4. set windows notifications
   if ( WSAEventSelect( ( *ppSkObject )->GetSocket(), m_Event , nEvents ) == SOCKET_ERROR )
   {
      m_pSockObj->Disconnect();
      return ZFX_FAIL;
   }

   //set all clients invalid
   for ( int i = 0; i < 256; i++ )
   {
      m_Clients[i].skToClient = INVALID_SOCKET;
      m_Clients[i].nID = 0;
   }
   return ZFX_OK;
}

HRESULT ZFXWS::CreateClient( ZFXSocketObject **ppSkObject )
{
   UINT nEvents = 0;

   ( *ppSkObject ) = new ZFXSocketObject();
   if ( !( *ppSkObject ) )
   {
      return ZFX_FAIL;
   }

   //1. create socket object
   if ( FAILED( ( *ppSkObject )->CreateSocket() ) )
   {
      return ZFX_FAIL;
   }

   if ( m_pIP == NULL )
   {
      gethostname(m_pIP, 10);
   }

   //2. try to connect
   if ( FAILED( ( *ppSkObject )->Connect(m_pIP, m_nPort) ) )
   {
      return ZFX_FAIL;
   }

   nEvents |= FD_READ | FD_CLOSE;

   //3. set up windows notifications
   if ( WSAEventSelect( ( *ppSkObject )->GetSocket(), m_Event, nEvents ) == SOCKET_ERROR )
   {
      m_pSockObj->Disconnect();
      return ZFX_FAIL;
   }
   return ZFX_OK;
}

HRESULT ZFXWS::SendToServer( const ZFXPACKET *pPkt )
{
   int nBytes = 0;
   int nSize = g_PktSize + pPkt->nLength;

   if ( m_Mode != NMD_CLIENT )
   {
      return ZFX_FAIL;
   }
   if ( nSize > m_nMaxSize )
   {
      return ZFX_OUTOFMEMORY;
   }

   //serialize data
   memcpy(m_Buffer, pPkt, g_PktSize);
   memcpy(m_Buffer+g_PktSize, pPkt->pData, pPkt->nLength);

   nBytes = m_pSockObj->Send(m_Buffer, nSize);
   if ( nBytes == SOCKET_ERROR || nBytes < nSize )
   {
      return ZFX_FAIL;
   }
   return ZFX_OK;
}

HRESULT ZFXWS::SendToClients( const ZFXPACKET *pPkt )
{
   HRESULT hr = ZFX_OK;
   int nBytes = 0;
   int nSize = g_PktSize + pPkt->nLength;

   if ( m_Mode != NMD_SERVER )
   {
      return ZFX_FAIL;
   }
   if ( nSize > m_nMaxSize )
   {
      return ZFX_OUTOFMEMORY;
   }

   //serialize data
   memcpy( m_Buffer, pPkt, g_PktSize );
   memcpy( m_Buffer + g_PktSize, pPkt->pData, pPkt->nLength );

   for ( UINT i = 0; i < m_ClCount; i++ )
   {
      if ( m_Clients[i].skToClient != INVALID_SOCKET )
      {
         nBytes = m_pSockObj->Send( m_Buffer, nSize, m_Clients[i].skToClient );

         if ( nBytes == SOCKET_ERROR || nBytes < nSize )
         {
            hr = ZFX_FAIL;
         }
      }
   }
   return hr;
}

// NOTE: Since the old WSAAsync functionality has been exchanged in the favor of WSAEvent, I'm not sure if MsgProc will fire correctly
HRESULT ZFXWS::MsgProc( WPARAM wp, LPARAM lp )
{
   WORD  wEvent, wError;

   wError = HIWORD(lp);
   wEvent = LOWORD(lp);

   //evaluate what event occurred
   switch ( wEvent )
   {
      //new client accepted
      case FD_CONNECT:
         break;
      
      //new client is knocking
      case FD_ACCEPT:
         return OnAccept();
         break;

      //there is data to be received
      case FD_READ:
         return OnReceive(wp);
         break;

      //a socket is closing
      case FD_CLOSE:
         return OnDisconnect(wp);
         break;

      //after sending data
      case FD_WRITE:
         break;
   }
   return ZFX_OK;
}

HRESULT ZFXWS::OnAccept()
{
   int nSize   = 0;
   int nBytes  = 0;
   int i       = m_ClCount;

   if ( m_ClCount >= 255 )
   {
      return ZFX_OUTOFMEMORY;
   }

   if ( FAILED( m_pSockObj->Accept( &( m_Clients[i].skToClient ) ) ) )
   {
      return ZFX_FAIL;
   }

   //Send ID to new client
   ZFXPACKET *pPkt = (ZFXPACKET*)m_Buffer;
   pPkt->pData = &m_Buffer[g_PktSize];
   pPkt->nLength = sizeof(UINT);
   pPkt->nType = 0;
   pPkt->nSender = 0;
   memcpy(pPkt->pData, &m_ClID, sizeof(UINT));

   //increase counter
   m_Clients[i].nID = m_ClID;
   m_ClCount++;
   m_ClID++;

   nSize = g_PktSize + pPkt->nLength;
   nBytes = m_pSockObj->Send(m_Buffer, nSize, m_Clients[i].skToClient);
   if ( nBytes == SOCKET_ERROR || nBytes < nSize )
   {
      return ZFX_FAIL;
   }

   //inform other clients about the new client
   pPkt->nType = 1;
   SendToClients(pPkt);
   return ZFX_OK;
}

HRESULT ZFXWS::OnDisconnect( SOCKET skDisconnecting )
{
   ZFXPACKET   Pkt;
   UCHAR       i = 0;

   if ( skDisconnecting == INVALID_SOCKET )
   {
      return ZFX_FAIL;
   }

   if ( m_Mode == NMD_SERVER )
   {
      for ( i = 0; i < m_ClCount; i++ )
      {
         if ( m_Clients[i].skToClient == skDisconnecting )
         {
            break;
         }
      }
      if ( i > m_ClCount )
      {
         return ZFX_FAIL;
      }

      //close the socket
      shutdown(m_Clients[i].skToClient, 0x02);
      closesocket(m_Clients[i].skToClient);
      m_Clients[i].skToClient = INVALID_SOCKET;

      //inform the other clients
      Pkt.pData   = &m_Buffer[g_PktSize];
      Pkt.nLength = sizeof(UINT);
      Pkt.nType   = 2;
      Pkt.nSender = 0;
      memcpy(Pkt.pData, &m_Clients[i].nID, sizeof(UINT));

      SendToClients(&Pkt);

      //copy last client to free position
      memcpy(&m_Clients[i], &m_Clients[m_ClCount - 1], sizeof(ZFXCLIENT));
      m_ClCount--;
   }
   else
   {
      shutdown(m_pSockObj->GetSocket(),0x02);
      closesocket(m_pSockObj->GetSocket());
   }
   return ZFX_OK;
}

HRESULT ZFXWS::OnReceive( SOCKET skReceiving )
{
   if ( m_bRunning )
   {
      return m_pSockObj->Receive(skReceiving);
   }
   else
   {
      return ZFX_FAIL;
   }
}