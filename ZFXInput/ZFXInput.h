//File: ZFXInput.h
//Created by Stefan Zerbst and Oliver Duvel

#include "ZFXInputDevice.h"

#pragma once

class ZFXInput 
{
public:
	ZFXInput(HINSTANCE hInst);
	~ZFXInput();

	HRESULT          CreateDevice();
	LPZFXINPUTDEVICE GetDevice() { return m_pDevice; }
	HINSTANCE        GetModule() { return m_hDLL; }
	void             Release();

private:
	ZFXInputDevice  *m_pDevice;
	HINSTANCE        m_hInst;
	HMODULE          m_hDLL;
}; // class

typedef class ZFXInput *LPZFXINPUT;