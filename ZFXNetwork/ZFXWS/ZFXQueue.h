//File: ZFXQueue.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd
#pragma once

class ZFXQueueElem
{
public:
	ZFXQueueElem(const char* pData, unsigned int nSize);
	~ZFXQueueElem();

	ZFXQueueElem *m_pNext;
	char		 *m_pData;
	unsigned int m_nSize;
};

class ZFXQueue
{
private:
	ZFXQueueElem *m_pHead;
	ZFXQueueElem *m_pTail;
	unsigned int m_nCount;

public:
	ZFXQueue();
	~ZFXQueue();

	void Dequeue();
	void Enqueue(const void* pData, unsigned int nSize);
	void Front(void *pData, bool bDequeue);

	unsigned int GetCount() { return m_nCount; }
	unsigned int GetFrontSize() { if (m_pHead){ return m_pHead->m_nSize; } else { return 0; } } 
};