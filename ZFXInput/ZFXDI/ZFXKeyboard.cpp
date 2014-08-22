//File: ZFXKeyboard.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFXDI.h"

ZFXKeyboard::ZFXKeyboard(LPDIRECTINPUT8 pDI, HWND hWnd, FILE* pLog)
{
	Create(pDI, hWnd, pLog);
}

ZFXKeyboard::~ZFXKeyboard()
{
	Release();
}


HRESULT ZFXKeyboard::Init()
{
	//given that this is a GUID for a specific keyboard layout, I'm guessing this isn't very localization-friendly
	if (FAILED(StartDevice(GUID_SysKeyboard, &c_dfDIKeyboard)))
	{
		return ZFX_FAIL;
	}

	//clear memory
	memset(m_Keys, 0, sizeof(m_Keys));
	memset(m_KeysOld, 0, sizeof(m_KeysOld));

	//acquire the device
	m_pDevice->Acquire();
	return ZFX_OK;
}

//call this each frame
HRESULT ZFXKeyboard::Update()
{
	memcpy(m_KeysOld, m_Keys, sizeof(m_Keys));

	//query status
	if (FAILED(GetData(IDV_KEYBOARD, &m_Keys[0], NULL)))
	{
		return ZFX_FAIL;
	}
	return ZFX_OK;
}

//the ID we send should be one of the key IDs as defined in ZFXInputDevice.h
bool ZFXKeyboard::IsPressed(UINT nID)
{
	return (m_Keys[nID] & 0x80);
}

//the ID we send should be one of the key IDs as defined in ZFXInputDevice.h
bool ZFXKeyboard::IsReleased(UINT nID)
{
	return ((m_KeysOld[nID] & 0x80) && !(m_Keys[nID] & 0x80));
}