//File: ZFXDI.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#pragma once

#include <ZFXInputDevice.h>
#include <ZFX.h>
#include <dinput.h>


class ZFXDIDevice
{
public:
	ZFXDIDevice() {}
	virtual ~ZFXDIDevice() {}

	//base functions
	virtual void Create(LPDIRECTINPUT8, HWND, FILE*);
	virtual void Release();
	virtual HRESULT StartDevice(REFGUID rguid, LPCDIDATAFORMAT lpdf);

	//accessor functions
	virtual void GetPosition(POINT *pPoint){ (*pPoint).x = m_lX; (*pPoint).y = m_lY; }

	//pure virtual functions
	virtual HRESULT Init()=0;
	virtual HRESULT Update()=0;

protected:
	virtual HRESULT GetData(ZFXINPUTDEV type, void *pData, DWORD *dwNum);

	LPDIRECTINPUTDEVICE8	m_pDevice;
	LPDIRECTINPUT8			m_pDI;
	HWND					m_hWnd;
	long					m_lX;
	long					m_lY;
	FILE					*m_pLog;
};