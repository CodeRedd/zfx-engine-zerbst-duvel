//File: ZFXGamepad.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFXDI.h"

ZFXGamepad *g_pThis = NULL;


ZFXGamepad::ZFXGamepad(LPDIRECTINPUT8 pDI, HWND hWnd, FILE* pLog)
{
	Create(pDI, hWnd, pLog);
}

ZFXGamepad::~ZFXGamepad()
{
	Release();
}

BOOL CALLBACK gEnumPadCallback(const DIDEVICEINSTANCE* pInst, void* pUserData)
{
	return g_pThis->EnumPadCallback(pInst);
}

BOOL ZFXGamepad::EnumPadCallback(const DIDEVICEINSTANCE *pInst)
{
	//try to start up this one
	if (SUCCEEDED(StartDevice(pInst->guidInstance, &c_dfDIJoystick)))
	{
		m_bPadFound = true;
		wcscpy_s((TCHAR*)pInst->tszProductName, sizeof(m_Name), m_Name);
		return DIENUM_STOP;
	}

	return DIENUM_CONTINUE;
}

HRESULT ZFXGamepad::Init()
{
	DIPROPRANGE diprg;
	DIDEVCAPS	diCaps;

	//some initialization
	memset(m_bPressed, 0, sizeof(m_bPressed));
	memset(m_bReleased, 0, sizeof(m_bReleased));
	m_bPadFound = false;
	m_lX = m_lY = m_lZ = 0;
	g_pThis = this;

	//enumerate attached joysticks
	m_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, (LPDIENUMDEVICESCALLBACK) gEnumPadCallback, &m_guid, DIEDFL_ATTACHEDONLY);

	//none found?
	if (!m_bPadFound)
	{
		return ZFX_FAIL;
	}

	//final settings
	diprg.diph.dwSize = sizeof(DIPROPRANGE);
	diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	diprg.diph.dwHow = DIPH_BYOFFSET;
	diprg.lMin = -1000;
	diprg.lMax = +1000;
	
	diprg.diph.dwObj = DIJOFS_X;
	m_pDevice->SetProperty(DIPROP_RANGE, &diprg.diph);

	diprg.diph.dwObj = DIJOFS_Y;
	m_pDevice->SetProperty(DIPROP_RANGE, &diprg.diph);

	diprg.diph.dwObj = DIJOFS_Z;
	m_pDevice->SetProperty(DIPROP_RANGE, &diprg.diph);

	//number of buttons
	if (SUCCEEDED(m_pDevice->GetCapabilities(&diCaps)))
	{
		m_dwNumBtns = diCaps.dwButtons;
	}
	else
	{
		m_dwNumBtns = 4;
	}

	m_pDevice->Acquire();
	return ZFX_OK;
}

HRESULT ZFXGamepad::Update()
{
	DIJOYSTATE js;

	//poll the gamepad
	m_pDevice->Poll();

	//get the data from the gamepad
	if (FAILED(GetData(IDV_GAMEPAD, &js, NULL)))
	{
		return ZFX_FAIL;
	}

	//gamepad buttons
	for (DWORD i = 0; i < m_dwNumBtns; i++)
	{
		m_bReleased[i] = false;

		if (js.rgbButtons[i] & 0x80)
		{
			m_bPressed[i] = true;
		}
		else
		{
			if (m_bPressed[i])
			{
				m_bReleased[i] = true;
			}
			m_bPressed[i] = false;
		}
	}

	//position of the stick
	m_lX = js.lX;
	m_lY = js.lY;
	m_lZ = js.lZ;
	m_lRx = js.lRx;
	m_lRy = js.lRy;
	m_lRz = js.lRz;

	return ZFX_OK;
}