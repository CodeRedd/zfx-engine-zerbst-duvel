//File: ZFXQueue.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFXQueue.h"
#include <stddef.h>        // #def NULL
#include <memory.h>        // memcpy()

ZFXQueue::ZFXQueue()
{
   m_pTail = NULL;
   m_pHead = NULL;
   m_nCount = 0;
}

ZFXQueue::~ZFXQueue()
{
   while ( m_pHead )
   {
      Dequeue();
   }
   m_pTail = NULL;
   m_pHead = NULL;
   m_nCount = 0;
}

void ZFXQueue::Enqueue( const void *pData, unsigned int nSize )
{
   ZFXQueueElem *pNew = new ZFXQueueElem((const char*)pData, nSize);

   //is this the first element?
   if ( m_nCount == 0 )
   {
      m_pHead = pNew;
      m_pTail = pNew;
   }
   else if (m_nCount == 1) //or the second?
   {
      m_pHead->m_pNext = pNew;
      m_pTail = pNew;
   }
   else
   {
      m_pTail->m_pNext = pNew;
      m_pTail = pNew;
   }
   m_nCount++;
}

void ZFXQueue::Dequeue()
{
   ZFXQueueElem *pTemp;

   //are we already empty?
   if ( m_nCount == 0 )
   {
      return;
   }
   else if ( m_nCount == 1 )//is this the last element?
   {
      delete m_pHead;
      m_pHead = NULL;
      m_pTail = NULL;
   }
   else
   {
      pTemp = m_pHead;
      m_pHead = m_pHead->m_pNext;
      delete pTemp;
   }
   m_nCount--;
}

void ZFXQueue::Front( void *pData, bool bDequeue )
{
   if ( pData )
   {
      if ( m_pHead )
      {
         memcpy(pData, m_pHead->m_pData, m_pHead->m_nSize);
      }
   }
   if ( bDequeue )
   {
      Dequeue();
   }
}

ZFXQueueElem::ZFXQueueElem( const char *pData, unsigned int nSize )
{
   m_pData = NULL;
   m_pNext = NULL;
   m_pData = new char[nSize];
   m_nSize = nSize;
   memcpy(m_pData, pData, nSize);
}

ZFXQueueElem::~ZFXQueueElem()
{
   if ( m_pData )
   {
      delete [] m_pData;
      m_pData = NULL;
   }

   m_pNext = NULL;
}