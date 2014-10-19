//File: ZFXSocketObject.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd


#include <Ws2tcpip.h> //for Inet_Pton
#include "ZFXSocketObject.h"

int g_PktSize = sizeof(ZFXPACKET);

ZFXSocketObject::ZFXSocketObject()
{
   m_skSocket     = INVALID_SOCKET;
   m_bRunning     = false;
   m_pInbox       = NULL;
   m_Buffer      = NULL;
}

ZFXSocketObject::~ZFXSocketObject()
{
   if ( IsRunning() )
   {
      Disconnect();
      m_bRunning = false;
   }

   if ( m_pInbox )
   {
      delete m_pInbox;
      m_pInbox = NULL;
   }

   if ( m_Buffer )
   {
      delete[] m_Buffer;
      m_Buffer = NULL;
   }

   m_skSocket = INVALID_SOCKET;
}

HRESULT ZFXSocketObject::CreateSocket()
{
   if ( m_skSocket != INVALID_SOCKET )
   {
      Disconnect();
   }

   m_skSocket = socket(AF_INET, SOCK_STREAM, 0);
   if ( m_skSocket == INVALID_SOCKET )
   {
      return ZFX_FAIL;
   }

   m_pInbox = new ZFXQueue();
   m_Buffer = new char[65536];
   memset(m_Buffer, 0, 65536);
   return ZFX_OK;
}

HRESULT ZFXSocketObject::Bind( int nPort )
{
   sockaddr_in saServerAddress;

   memset(&saServerAddress, 0, sizeof(sockaddr_in));
   saServerAddress.sin_family       = AF_INET;
   saServerAddress.sin_addr.s_addr  = htonl(INADDR_ANY);
   saServerAddress.sin_port         = htons(nPort);

   if ( bind( m_skSocket, (sockaddr*) &saServerAddress, sizeof( sockaddr ) ) == SOCKET_ERROR )
   {
      Disconnect();
      return ZFX_FAIL;
   }
   return ZFX_OK;
}

HRESULT ZFXSocketObject::Listen()
{
   if ( listen( m_skSocket, 32 ) != 0 )
   {
      return ZFX_FAIL;
   }

   m_bRunning = true;
   return ZFX_OK;
}

HRESULT ZFXSocketObject::Accept( SOCKET *skToNewClient )
{
   sockaddr_in saClientAddress;
   int         nClientSize = sizeof(sockaddr_in);

   ( *skToNewClient ) = accept(m_skSocket, (sockaddr* )&saClientAddress, &nClientSize);

   if ( ( *skToNewClient ) == INVALID_SOCKET )
   {
      return ZFX_FAIL;
   }
   return ZFX_OK;
}

HRESULT ZFXSocketObject::Connect( char *chServer, int nPort )
{
   sockaddr_in saServerAddress;
   ADDRINFO*   pAddrInfo = NULL;

   //try to find the server
   memset(&saServerAddress, 0, sizeof(sockaddr_in));
   saServerAddress.sin_port         = htons(nPort);
   saServerAddress.sin_family       = AF_INET;
   inet_pton(AF_INET, chServer, &(saServerAddress.sin_addr.s_addr));

   if ( saServerAddress.sin_addr.s_addr == INADDR_NONE )
   {
      getaddrinfo( chServer, NULL, NULL, &pAddrInfo );

      // I know this part isn't quite right -- original use of gethostname() was deprecated and I don't quite understand how to
      // pick the right address out of the addrinfo linked list. So I'm just grabbing the first IPv4 address I can find.
      if ( pAddrInfo != NULL )
      {
         while ( pAddrInfo->ai_family != AF_INET )
         {
            if ( pAddrInfo->ai_next == NULL )
            {
               return ZFX_FAIL;
            }
            pAddrInfo = pAddrInfo->ai_next;
         }

         saServerAddress.sin_addr = ( (sockaddr_in*) pAddrInfo->ai_addr )->sin_addr;
      }
      else
      {
         return ZFX_FAIL;
      }
   }

   //connect to server address
   if ( connect( m_skSocket, (sockaddr*) &saServerAddress, sizeof( sockaddr ) ) == SOCKET_ERROR )
   {
      Disconnect();
      return ZFX_FAIL;
   }

   m_bRunning = true;
   return ZFX_OK;
}

void ZFXSocketObject::Disconnect()
{
   if ( m_skSocket != INVALID_SOCKET )
   {
      shutdown(m_skSocket, SD_BOTH);
      closesocket(m_skSocket);
      m_skSocket = INVALID_SOCKET;
   }
}

int ZFXSocketObject::Send( const char *pPkt, UINT nSize )
{
   UINT nSent = 0;
   UINT n = 0;

   while ( nSent < nSize )
   {
      n = send(m_skSocket, pPkt + nSent, nSize-nSent, 0);
      if ( n == SOCKET_ERROR )
      {
         return n;
      }
      else
      {
         nSent += n;
      }
   }
   return nSent;
}

int ZFXSocketObject::Send( const char *pPkt, UINT nSize, SOCKET skReceiver )
{
   UINT nSent = 0;
   UINT n = 0;

   while ( nSent < nSize )
   {
      n = send( skReceiver, pPkt + nSent, nSize - nSent, 0 );
      if ( n == SOCKET_ERROR )
      {
         return n;
      }
      else
      {
         nSent += n;
      }
   }
   return nSent;
}

HRESULT ZFXSocketObject::Receive( SOCKET sk )
{
   HRESULT  hr          = ZFX_OK;
   UINT     nSize       = 65536;    // max bytes for each read call
   UINT     nBytesRead  = 0;        // read bytes in one call
   UINT     nReadHead   = 0;        // position in m_Buffer
   UINT     n           = 0;        // size of leftover data from the last call
   bool     bDone       = false;    // finished?

   ZFXPACKET   *pPkt          = NULL;
   UINT        nPktSizeTotal  = 0;

   //read up to 65,536 bytes in each call
   //loop until no more data is waiting
   while ( !bDone )
   {
      nBytesRead = recv(sk, &m_Buffer[n], nSize-n,0);

      if ( nBytesRead == SOCKET_ERROR )
      {
         int WSAError = WSAGetLastError();

         //ignore harmless messages
         if ( WSAError != WSAEMSGSIZE && WSAError != WSAEWOULDBLOCK )
         {
            hr = ZFX_FAIL;
            bDone = true;
            break;
         }
      }

      //now we have nBytesRead bytes in m_Buffer
      if ( nBytesRead <= 0 )
      {
         bDone = true; //no data read
      }
      else
      {
         //take care of old data in the buffer
         nBytesRead += n;

         //loop until we find a complete header
         while ( ( nBytesRead - nReadHead ) > g_PktSize )
         {
            //next chunk of data
            pPkt = (ZFXPACKET*) &m_Buffer[nReadHead];
            pPkt->pData = &m_Buffer[nReadHead] + g_PktSize;

            nPktSizeTotal = g_PktSize + pPkt->nLength;

            //did we get the whole packet?
            if ( ( nBytesRead - nReadHead ) >= nPktSizeTotal )
            {
               m_pInbox->Enqueue(pPkt, nPktSizeTotal);
               nReadHead += nPktSizeTotal;
            }
            else //if not, back to recv() for more data
            {
               //copy incomplete packet to start of buffer
               memcpy(m_Buffer, &m_Buffer[nReadHead], nBytesRead-nReadHead);
               n = nBytesRead - nReadHead;
               break;
            }
         }

         //all data waiting has been read
         if ( nBytesRead < nSize )
         {
            bDone = true;
         }
      }
   }
   return hr;
}

HRESULT ZFXSocketObject::GetNextPkt( ZFXPACKET *pPkt )
{
   //only continue if we have anything in the inbox
   if ( m_pInbox->GetCount() > 0 )
   {
      //draw data into our buffer
      m_pInbox->Front(m_Buffer, true);

      //fill data structure
      memcpy(pPkt, m_Buffer, g_PktSize);
      memcpy(pPkt->pData, m_Buffer+g_PktSize, pPkt->nLength);
      return ZFX_OK;
   }
   return ZFX_FAIL;
}