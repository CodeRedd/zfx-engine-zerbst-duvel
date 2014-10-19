//File: ZFXNetwork.h
//Created by Stefan Zerbst and Oliver Duvel

#pragma once

#include "ZFXNetworkDevice.h"

class ZFXNetwork
{
public:
	ZFXNetwork(HINSTANCE hInst);
	~ZFXNetwork(void);

	HRESULT				CreateDevice(void);
	LPZFXNETWORKDEVICE	GetDevice(void) { return m_pDevice; }
	HINSTANCE			GetModule(void) { return m_hDLL; }
	void				Release(void);

private:
	ZFXNetworkDevice  *m_pDevice;
	HINSTANCE        m_hInst;
	HMODULE          m_hDLL;
}; // class

typedef class ZFXNetwork *LPZFXNETWORK;