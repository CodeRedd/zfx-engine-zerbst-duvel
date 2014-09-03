//File: ZFXDI.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFXDI.h"

ZFXDI::ZFXDI(HINSTANCE hDLL)
{
	m_hDLL		= hDLL;
	m_pDI		= NULL;
	m_pLog		= NULL;
	m_bRunning	= false;
	m_pKB		= NULL;
	m_pMouse	= NULL;
	m_pGamepad	= NULL;
}

ZFXDI::~ZFXDI()
{
	Release();
}

void ZFXDI::Release()
{
	if (m_pKB)
	{
		delete m_pKB;
		m_pKB = NULL;
	}

	if (m_pMouse)
	{
		delete m_pMouse;
		m_pMouse = NULL;
	}
	
	if (m_pGamepad)
	{
		delete m_pGamepad;
		m_pGamepad = NULL;
	}
	if (m_pDI)
	{
		m_pDI->Release();
		m_pDI = NULL;
	}
}

HRESULT ZFXDI::Init(HWND hWnd, const RECT *rcMouseCage, LONG lMaxScroll, LONG lMinScroll, bool bSaveLog)
{
	m_hWndMain = hWnd;

	//create DirectInput main object
	if (FAILED(DirectInput8Create(m_hDLL, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pDI, NULL)))
	{
		return ZFX_FAIL;
	}

	//create all input device objects
	m_pKB =			new ZFXKeyboard(m_pDI, hWnd, m_pLog);
	m_pMouse =		new ZFXMouse(m_pDI, hWnd, m_pLog);
	m_pGamepad =	new ZFXGamepad(m_pDI, hWnd, m_pLog);

	//initialize all input device objects
	if (FAILED(m_pKB->Init()))
	{
		if (m_pKB)
		{
			delete m_pKB;
		}
		m_pKB = NULL;
		return ZFX_FAIL;
	}

	if (FAILED(m_pMouse->Init()))
	{
		if (m_pMouse)
		{
			delete m_pMouse;
		}
		m_pMouse = NULL;
		return ZFX_FAIL;
	}

	if (rcMouseCage)
	{
		m_pMouse->SetBounds(*rcMouseCage, lMaxScroll, lMinScroll);
	}

	if (FAILED(m_pGamepad->Init()))
	{
		if (m_pGamepad)
		{
			delete m_pGamepad;
		}
		m_pGamepad = NULL;
	}
	m_bRunning = true;
	return ZFX_OK;
}

HRESULT ZFXDI::Update()
{
	HRESULT hr;

	if (!IsRunning())
	{
		return ZFX_FAIL;
	}

	if (m_pKB)
	{
		if (FAILED(hr = m_pKB->Update()))
		{
			return hr;
		}
	}

	if (m_pMouse)
	{
		if (FAILED(hr = m_pMouse->Update()))
		{
			return hr;
		}
	}

	if (m_pGamepad)
	{
		if (FAILED(hr = m_pGamepad->Update()))
		{
			return hr;
		}
	}
	return ZFX_OK;
}

bool ZFXDI::HasGamepad(TCHAR *pGamepadName)
{
	if (m_pGamepad)
	{
		if (pGamepadName)
		{
			m_pGamepad->GetName(pGamepadName);
		}
		return true;
	}
	return false;
}

HRESULT ZFXDI::GetPosition(ZFXINPUTDEV idType, POINT *pPt)
{
	if (idType == IDV_MOUSE)
	{
		m_pMouse->GetPosition(pPt);
		return ZFX_OK;
	}
	else if (idType == IDV_GAMEPAD)
	{
		if (m_pGamepad)
		{
			m_pGamepad->GetPosition(pPt);
		}
		else
		{
			(*pPt).x = 0;
			(*pPt).y = 0;
		}
		return ZFX_OK;
	}
	else
	{
		return ZFX_INVALIDPARAM;
	}
}

bool ZFXDI::IsPressed(ZFXINPUTDEV idType, UINT nBtn)
{
	if (idType == IDV_MOUSE)
	{
		return m_pMouse->IsPressed(nBtn);
	}
	else if (idType == IDV_KEYBOARD)
	{
		return m_pKB->IsPressed(nBtn);
	}
	else if (idType == IDV_GAMEPAD && m_pGamepad)
	{
		return m_pGamepad->IsPressed(nBtn);
	}
	else
	{
		return false;
	}
}

bool ZFXDI::IsReleased(ZFXINPUTDEV idType, UINT nBtn)
{
	if (idType == IDV_MOUSE)
	{
		return m_pMouse->IsReleased(nBtn);
	}
	else if (idType == IDV_KEYBOARD)
	{
		return m_pKB->IsReleased(nBtn);
	}
	else if (idType == IDV_GAMEPAD && m_pGamepad)
	{
		return m_pGamepad->IsReleased(nBtn);
	}
	else
	{
		return false;
	}
}

void ZFXDIDevice::Create(LPDIRECTINPUT8 pDI, HWND hWnd, FILE* pLog)
{
	m_pLog = pLog;
	m_hWnd = hWnd;
	m_pDI = pDI;
	m_pDevice = NULL;
}

void ZFXDIDevice::Release()
{
	if (m_pDevice)
	{
		m_pDevice->Unacquire();
		m_pDevice->Release();
		m_pDevice = NULL;
	}
}

HRESULT ZFXDIDevice::StartDevice(REFGUID rguid, LPCDIDATAFORMAT lpdf)
{
	DWORD dwFlags = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;

	//if already existing destroy it
	if (m_pDevice)
	{
		m_pDevice->Unacquire();
		m_pDevice->Release();
		m_pDevice = NULL;
	}

	//create the device
	if (FAILED(m_pDI->CreateDevice(rguid, &m_pDevice, NULL)))
	{
		return ZFX_FAIL;
	}

	//define the right data format
	if (FAILED(m_pDevice->SetDataFormat(lpdf)))
	{
		return ZFX_FAIL;
	}

	//set the cooperative level
	if (FAILED(m_pDevice->SetCooperativeLevel(m_hWnd, dwFlags)))
	{
		return ZFX_FAIL;
	}

	return ZFX_OK;
}

HRESULT ZFXDIDevice::GetData(ZFXINPUTDEV type, void *pData, DWORD *pdwNum)
{
	HRESULT hr = ZFX_FAIL;
	size_t size = 0;

	//what device type is this?
	if (type == IDV_MOUSE)
	{
		size = sizeof(DIDEVICEOBJECTDATA);
		hr = m_pDevice->GetDeviceData(size, (DIDEVICEOBJECTDATA*)pData, pdwNum, 0);
	}
	else
	{
		if (type == IDV_KEYBOARD)
		{
			size = sizeof(TCHAR)*256;
		}
		else
		{
			size = sizeof(DIJOYSTATE);
		}

		hr = m_pDevice->GetDeviceState(size, pData);
	}

	//query failed?
	if (FAILED(hr))
	{
		//do we have the device acquired at all?
		if (hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST)
		{
			hr = m_pDevice->Acquire();
			while (hr == DIERR_INPUTLOST)
			{
				hr = m_pDevice->Acquire(); //seems like a hang risk if we unplug the mouse/joystick for too long
			}
		}

		//another app has priority?
		if (hr == DIERR_OTHERAPPHASPRIO)
		{
			return ZFX_OK;
		}

		//we got it back
		if (SUCCEEDED(hr))
		{
			if (type == IDV_MOUSE)
			{
				hr = m_pDevice->GetDeviceData(size, (DIDEVICEOBJECTDATA*)pData, pdwNum, 0);
			}
			else
			{
				hr = m_pDevice->GetDeviceState(size, pData);
			}

			//an error again?
			if (FAILED(hr))
			{
				return ZFX_FAIL;
			}
		}
		//an error again?
		else
		{
			return ZFX_FAIL;
		}
	}
	return ZFX_OK;
}