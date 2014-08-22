//File: ZFXMouse.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFXDI.h"

#define BUFFER_SIZE 16

ZFXMouse::ZFXMouse(LPDIRECTINPUT8 pDI, HWND hWnd, FILE* pLog)
{
	Create(pDI, hWnd, pLog);
}

ZFXMouse::~ZFXMouse()
{
	Release();
}

HRESULT ZFXMouse::Init()
{
	//clear memory
	memset(m_bPressed, 0, sizeof(bool)* MOUSE_BUTTON_LIMIT);
	memset(m_bReleased, 0, sizeof(bool)*MOUSE_BUTTON_LIMIT);
	m_lX = m_lY = m_lZ = 0;

	if (FAILED(StartDevice(GUID_SysMouse, &c_dfDIMouse)))
	{
		return ZFX_FAIL;
	}

	//event notification
	if (!(m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL)))
	{
		return ZFX_FAIL;
	}

	if (FAILED(m_pDevice->SetEventNotification(m_hEvent)))
	{
		return ZFX_FAIL;
	}

	//build mouse buffer
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = BUFFER_SIZE;

	if (FAILED(m_pDevice->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
	{
		return ZFX_FAIL;
	}

	m_pDevice->Acquire();
	return ZFX_OK;
}

HRESULT ZFXMouse::Update()
{
	DIDEVICEOBJECTDATA od[BUFFER_SIZE];
	DWORD dwNumElem = BUFFER_SIZE;

	//read data from the buffer
	if (FAILED(GetData(IDV_MOUSE, &od[0], &dwNumElem)))
	{
		return ZFX_FAIL;
	}

	m_bReleased[0] = m_bReleased[1] = m_bReleased[2] = false;

	//now we have dwNumElem mouse events to process
	for (DWORD i = 0; i < dwNumElem; i++)
	{
		switch (od[i].dwOfs)
		{
		//MOVEMENT
		case DIMOFS_X:
			m_lX += od[i].dwData;
			if (m_lX < m_rcCage.left)
			{
				m_lX = m_rcCage.left;
			}
			if (m_lX > m_rcCage.right)
			{
				m_lX = m_rcCage.right;
			}
			break;

		case DIMOFS_Y:
			m_lY += od[i].dwData;
			if (m_lY < m_rcCage.top)
			{
				m_lY = m_rcCage.top;
			}
			if (m_lY > m_rcCage.bottom)
			{
				m_lY = m_rcCage.bottom;
			}
			break;

		case DIMOFS_Z:
			m_lZ += od[i].dwData;
			if (m_lZ < m_lMinScroll)
			{
				m_lZ = m_lMinScroll;
			}
			if (m_lZ > m_lMaxScroll)
			{
				m_lZ = m_lMaxScroll;
			}
			break;

		//MOUSE BUTTONS
		case DIMOFS_BUTTON0:
			if (od[i].dwData & 0x80) 
			{
				m_bPressed[0] = true;
			}
			else
			{
				if (m_bPressed[0])
				{
					m_bReleased[0] = true;
				}
				m_bPressed[0] = false;
			}
			break;

		case DIMOFS_BUTTON1:
			if (od[i].dwData & 0x80)
			{
				m_bPressed[1] = true;
			}
			else
			{
				if (m_bPressed[1])
				{
					m_bReleased[1] = true;
				}
				m_bPressed[1] = false;
			}
			break;

		case DIMOFS_BUTTON2:
			if (od[i].dwData & 0x80)
			{
				m_bPressed[2] = true;
			}
			else
			{
				if (m_bPressed[2])
				{
					m_bReleased[2] = true;
				}
				m_bPressed[2] = false;
			}
			break;

		case DIMOFS_BUTTON3:
			if (od[i].dwData & 0x80)
			{
				m_bPressed[3] = true;
			}
			else
			{
				if (m_bPressed[3])
				{
					m_bReleased[3] = true;
				}
				m_bPressed[3] = false;
			}
			break;

		case DIMOFS_BUTTON4:
			if (od[i].dwData & 0x80)
			{
				m_bPressed[4] = true;
			}
			else
			{
				if (m_bPressed[4])
				{
					m_bReleased[4] = true;
				}
				m_bPressed[4] = false;
			}
			break;

		case DIMOFS_BUTTON5:
			if (od[i].dwData & 0x80)
			{
				m_bPressed[5] = true;
			}
			else
			{
				if (m_bPressed[5])
				{
					m_bReleased[5] = true;
				}
				m_bPressed[5] = false;
			}
			break;

		case DIMOFS_BUTTON6:
			if (od[i].dwData & 0x80)
			{
				m_bPressed[6] = true;
			}
			else
			{
				if (m_bPressed[6])
				{
					m_bReleased[6] = true;
				}
				m_bPressed[6] = false;
			}
			break;
		};
	}
	return ZFX_OK;
}