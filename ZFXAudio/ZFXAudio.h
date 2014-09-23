//File: ZFXAudio.h
//Created by Stefan Zerbst and Oliver Duvel

#pragma once

#include "ZFXAudioDevice.h"



class ZFXAudio 
{
public:
	ZFXAudio(HINSTANCE hInst);
	~ZFXAudio(void);

	HRESULT          CreateDevice(void);
	LPZFXAUDIODEVICE GetDevice(void) { return m_pDevice; }
	HINSTANCE        GetModule(void) { return m_hDLL; }
	void             Release(void);

private:
	ZFXAudioDevice  *m_pDevice;
	HINSTANCE        m_hInst;
	HMODULE          m_hDLL;
}; // class

typedef class ZFXAudio *LPZFXAUDIO;