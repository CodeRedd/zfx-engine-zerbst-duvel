//File: ZFXDA.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFXDA.h"

bool g_bLF = false;
size_t g_szoff3 = sizeof(float) * 3; //shortcut size of three floats for copying vectors

ZFXDA::ZFXDA(HINSTANCE hDLL)
{
	m_hDLL			= hDLL;
	//m_pLoader		= NULL;
	m_pListener		= NULL;
	//m_pPerformance	= NULL;
	m_pSounds		= NULL;
	m_pLog			= NULL;
	m_bRunning		= false;

	//init structures
	m_dsListener.dwSize = sizeof(DS3DLISTENER);
	m_dsBuffer.dwSize = sizeof(DS3DBUFFER);
}

ZFXDA::~ZFXDA()
{
	Release();
}

HRESULT ZFXDA::Init(HWND hWnd, const TCHAR *chPath, bool bSaveLog)
{
	HRESULT hr = E_FAIL;

	m_hWndMain	= hWnd;
	g_bLF		= bSaveLog;

	//COM Initialize
	CoInitialize(NULL);

	//create an instance of the DirectMusic loader
	//hr = CoCreateInstance(CLSID_DirectMusicLoader, NULL, CLSCTX_INPROC, IID_IDirectMusicLoader8, (void**)&m_pLoader);
	if (FAILED(hr))
	{
		return ZFX_FAIL;
	}

	//create an instance of the DirectMusic performance
	//hr = CoCreateInstance(CLSID_DirectMusicPerformance, NULL, CLSCTX_INPROC, IID_IDirectMusicPerformance8, (void**)&m_pPerformance);
	if (FAILED(hr))
	{
		return ZFX_FAIL;
	}

	//default path for sound files
	//if(FAILED(hr = m_pLoader->SetSearchDirectory(GUID_DirectMusicAllTypes, chPath, false)))
	//{
	//	return ZFX_FAIL;
	//}

	//initialize the performance object
	//if (FAILED(hr = m_pPerformance->InitAudio(NULL, NULL, hWnd, DMUS_APATH_SHARED_STEREOPLUSREVERB, 64, DMUS_AUDIOF_ALL, NULL)))
	//{
	//	return ZFX_FAIL;
	//}

	//pointer to default audio path
	//if (FAILED(m_pPerformance->GetDefaultAudioPath(&m_pDAPath)))
	//{
	//	return ZFX_FAIL;
	//}

	//get pointer to listener in path
	//if (FAILED(m_pDAPath->GetObjectInPath(0, DMUS_PATH_PRIMARY_BUFFER, 0, GUID_NULL, 0,
	//										IID_IDirectSound3DListener8, (void**)&m_pListener)))
	//{
	//	return ZFX_FAIL;
	//}

	m_bRunning = true;
	return ZFX_OK;
}

void ZFXDA::Release()
{
	if (m_pSounds)
	{
		for (UINT i = 0; i < m_nNumSounds; i++)
		{
			if (true)//m_pSounds[i].pSegment)
			{
				//m_pSounds[i].pSegment->Unload(m_pPerformance);
				//m_pSounds[i].pSegment->Release();
				//m_pSounds[i].pSegment = NULL;
				delete[] m_pSounds[i].chName;
				m_pSounds[i].chName = NULL;
			}
		}
		free(m_pSounds);
	}

	//if (m_pLoader)
	//{
	//	m_pLoader->Release();
	//	m_pLoader = NULL;
	//}
	if (m_pListener)
	{
		m_pListener->Release();
		m_pListener = NULL;
	}
	//if (m_pPerformance)
	//{
	//	m_pPerformance->Stop(NULL, NULL, 0, 0);
	//	m_pPerformance->CloseDown();
	//	m_pPerformance->Release();
	//	m_pPerformance = NULL;
	//}

	//Shut down COM
	if (m_bRunning)
	{
		CoUninitialize();
	}
	m_bRunning = false;
}

HRESULT ZFXDA::LoadSound(const TCHAR* chName, UINT_PTR nID)
{
	HRESULT hr;

	//is this sound file already loaded?
	for (UINT i = 0; i < m_nNumSounds; i++)
	{
		if (wcscmp(chName, m_pSounds[i].chName) == 0)
		{
			nID = i;
			return ZFX_OK;
		}
	}

	//if we're full, create 50 new sound slots
	if (m_nNumSounds % 50 == 0)
	{
		int n = (m_nNumSounds + 50)*sizeof(ZFXSOUND);
		m_pSounds = (ZFXSOUND*)realloc(m_pSounds, n);
		if (!m_pSounds)
		{
			return ZFX_OUTOFMEMORY;
		}
	}

	m_pSounds[m_nNumSounds].chName = new TCHAR[wcslen(chName)+1];
	wcscpy_s(m_pSounds[m_nNumSounds].chName, wcslen(chName), chName);

	m_pSounds[m_nNumSounds].bChanged = false;

	//load the file
	//if (FAILED(hr = m_pLoader->LoadObjectFromFile(CLSID_DirectMusicSegment, IID_IDirectMusicSegment8,
	//											  chName, (void**)&m_pSounds[m_nNumSounds].pSegment)))
	//{
	//	if (hr == DMUS_E_LOADER_FAILEDOPEN || hr == DMUS_E_LOADER_FAILEDCREATE)
	//	{
	//		return ZFX_FILENOTFOUND;
	//	}
	//	else if (hr == DMUS_E_LOADER_FORMATNOTSUPPORTED)
	//	{
	//		return ZFX_INVALIDPARAM;
	//	}
	//	else if (hr == E_OUTOFMEMORY)
	//	{
	//		return ZFX_OUTOFMEMORY;
	//	}
	//	return ZFX_FAIL;
	//}

	// IDirectMusicSegment8 *pSeg = m_pSounds[m_nNumSounds].pSegment;
	//download instruments
	//if (FAILED(pSeg->Download(m_pPerformance))
	//{
	//	pSeg->Release();
	//	pSeg = NULL;
	//	return ZFX_FAIL;
	//}

	//create an audio path
	//m_pPerformance->CreateStandardAudioPath(DMUS_APATH_DYNAMIC_3D, 64, TRUE, &m_pSounds[m_nNumSounds].p3DPath);

	//m_pSounds[m_nNumSounds].p3DPath->GetObjectInPath(DMUS_PCHANNEL_ALL, DMUS_PATH_BUFFER, 0, GUID_NULL, 0
	//												 IID_IDirectSound3DBuffer, (void**)&m_pSounds[m_nNumSounds].p3DBuffer);

	m_nNumSounds++;
	return ZFX_OK;
}

void ZFXDA::PlaySound(UINT nID, bool bLoop)
{
	if (nID >= m_nNumSounds)
	{
		return;
	}

	//any changes?
	if (m_pSounds[nID].bChanged)
	{
		m_pListener->CommitDeferredSettings();
		m_pSounds[nID].bChanged = false;
	}

	if (bLoop)
	{
		//m_pSounds[nID].pSegment->SetRepeats(DMUS_SEG_REPEAT_INFINITE);
	}

	//play as secondary buffer
	//m_pPerformance->PlaySegment(m_pSounds[nID].pSegment, DMUS_SEGF_DEFAULT | DMUS_SEGF_SECONDARY, 0, 0);
}

void ZFXDA::StopSound(UINT nID)
{
	if (nID >= m_nNumSounds)
	{
		return;
	}

	//m_pPerformance->Stop(m_pSounds[nId].pSegment, 0, 0, 0);
}

void ZFXDA::SetListener(ZFXVector vPos, ZFXVector vDir, ZFXVector vUp, ZFXVector vSpeed)
{
	if (m_pListener)
	{
		m_pListener->GetAllParameters(&m_dsListener);

		memcpy(&m_dsListener.vPosition, &vPos, g_szoff3);
		memcpy(&m_dsListener.vOrientFront, &vDir, g_szoff3);
		memcpy(&m_dsListener.vOrientTop, &vUp, g_szoff3);
		memcpy(&m_dsListener.vVelocity, &vSpeed, g_szoff3);

		m_pListener->SetAllParameters(&m_dsListener, DS3D_IMMEDIATE);
	}
}

void ZFXDA::SetSoundPosition(ZFXVector vPos, UINT nID)
{
	IDirectSound3DBuffer8 *p3DBuffer;
	if (nID >= m_nNumSounds)
	{
		return;
	}

	p3DBuffer = m_pSounds[nID].p3DBuffer;
	m_pSounds[nID].bChanged = true;

	p3DBuffer->GetAllParameters(&m_dsBuffer);

	m_dsBuffer.dwMode = DS3DMODE_NORMAL;
	memcpy(&m_dsBuffer.vPosition, &vPos, g_szoff3);

	p3DBuffer->SetAllParameters(&m_dsBuffer, DS3D_DEFERRED);
}

void ZFXDA::SetSoundDirection(ZFXVector vDir, ZFXVector vV, UINT nID)
{
	IDirectSound3DBuffer8 *p3DBuffer;
	if (nID >= m_nNumSounds)
	{
		return;
	}

	p3DBuffer = m_pSounds[nID].p3DBuffer;
	m_pSounds[nID].bChanged = true;

	p3DBuffer->GetAllParameters(&m_dsBuffer);

	m_dsBuffer.dwMode = DS3DMODE_NORMAL;
	memcpy(&m_dsBuffer.vVelocity, &vV, g_szoff3);
	memcpy(&m_dsBuffer.vConeOrientation, &vDir, g_szoff3);

	p3DBuffer->SetAllParameters(&m_dsBuffer, DS3D_DEFERRED);
}

void ZFXDA::SetSoundMaxDist(float fDis, UINT nID)
{
	IDirectSound3DBuffer8 *p3DBuffer;
	if (nID >= m_nNumSounds)
	{
		return;
	}

	p3DBuffer = m_pSounds[nID].p3DBuffer;
	m_pSounds[nID].bChanged = true;

	p3DBuffer->GetAllParameters(&m_dsBuffer);

	m_dsBuffer.dwMode = DS3DMODE_NORMAL;
	m_dsBuffer.flMaxDistance = fDis;

	p3DBuffer->SetAllParameters(&m_dsBuffer, DS3D_DEFERRED);
}