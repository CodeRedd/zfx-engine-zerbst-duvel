//File: ZFXDA.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

//NOTE: The original implementation calls for using components of DirectMusic that haven't been shipped with DirectX since 2004.
//I haven't been able to find a proper copy of the necessary .lib file to include, so for now I'm going to just have to
//leave the DirectMusic bits commented out. Unfortunately, this means that this code will be non-functional, but I'm choosing not to
//worry about this, since I care more about learning how to handle the audio inside the engine than how to interface with Windows.
//MS interfaces are well-documented enough that I could work that part out on my own with a less-deprecated sound technology. 
//#include <dmusici.h>
#include <dsound.h>
#include <ZFXAudioDevice.h>

BOOL WINAPI DllEntryPoint(HINSTANCE hDll, DWORD fdwReason, LPVOID lpvRserved);

struct ZFXSOUND
{
	TCHAR					*chName;
	bool					bChanged;
	//IDirectMusicSegment8		*pSegment;
	//IDirectMusicAudioPath8	*p3DPath;
	IDirectSound3DBuffer8	*p3DBuffer;
};

class ZFXDA : public ZFXAudioDevice
{
public:
	ZFXDA(HINSTANCE hDll);
	~ZFXDA();

	HRESULT Init(HWND, const TCHAR*, bool);

	//interface functions
	void	Release();
	bool	IsRunning() { return m_bRunning; }

	void	SetListener(ZFXVector, ZFXVector, ZFXVector, ZFXVector);

	HRESULT LoadSound(const TCHAR*, UINT_PTR);
	void	PlaySound(UINT nID, bool);
	void	StopSound(UINT nID);
	void	StopAll() { /*if (m_pPerformance)	{ m_pPerformance->Stop(NULL, NULL, 0, 0); }*/ }
	void	SetSoundPosition(ZFXVector, UINT);
	void	SetSoundMaxDist(float, UINT);
	void	SetSoundDirection(ZFXVector, ZFXVector, UINT);

private:
	//IDirectMusicLoader8		*m_pLoader;
	//IDirectMusicPerformance8	*m_pPerformance;
	IDirectSound3DListener8		*m_pListener;
	//IDirectMusicAudioPath8	*m_pDAPath;
	DS3DLISTENER				m_dsListener;
	DS3DBUFFER					m_dsBuffer;
	ZFXSOUND					*m_pSounds;
	UINT						m_nNumSounds;
	FILE*						m_pLog;
};