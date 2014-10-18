//File: ZFXAudioDevice.h
//Created by Stefan Zerbst and Oliver Duvel

#pragma once

#include <Windows.h>
#include <stdio.h>
#include <ZFX.h>
#include <ZFX3D.h>

class ZFXAudioDevice
{
protected:
	HWND		m_hWndMain;		//main window
	HINSTANCE	m_hDLL;			// DLL handle
	bool		m_bRunning;		// init done?

public:
	ZFXAudioDevice() {}
	virtual ~ZFXAudioDevice() {}

	virtual HRESULT Init(HWND, const TCHAR*, bool)=0;
	virtual void Release()=0;
	virtual bool IsRunning()=0;

	//stop all audio output
	virtual void StopAll()=0;

	//load sound file from disk
	virtual HRESULT LoadSound(const TCHAR*, UINT_PTR)=0;

	//play specific sound
	virtual void PlaySound(UINT, bool bLoop)=0;

	//stop specific sound
	virtual void StopSound(UINT)=0;

	//listener params for 3D sounds
	virtual void SetListener(ZFXVector vPos, ZFXVector vDir, ZFXVector vUp, ZFXVector vV)=0;

	//parameters of 3D sound source
	virtual void SetSoundPosition(ZFXVector, UINT)=0;
	virtual void SetSoundDirection(ZFXVector, ZFXVector vV, UINT)=0;
	virtual void SetSoundMaxDist(float, UINT)=0;
};
typedef class ZFXAudioDevice *LPZFXAUDIODEVICE;

/*----------------------------------------------------------------*/

extern "C"
{
	HRESULT CreateAudioDevice(HINSTANCE hDLL, ZFXAudioDevice **pInterface);
	typedef HRESULT(*CREATEAUDIODEVICE)(HINSTANCE hDLL, ZFXAudioDevice **pInterface);

	HRESULT ReleaseAudioDevice(ZFXAudioDevice **pInterface);
	typedef HRESULT(*RELEASEAUDIODEVICE)(ZFXAudioDevice **pInterface);
}
