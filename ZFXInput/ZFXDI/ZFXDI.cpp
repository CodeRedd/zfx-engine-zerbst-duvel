//File: ZFXDI.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFXDI.h"

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