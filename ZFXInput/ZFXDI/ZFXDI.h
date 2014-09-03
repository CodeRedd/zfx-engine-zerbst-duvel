//File: ZFXDI.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#pragma once

#include <ZFXInputDevice.h>
#include <ZFX.h>
#include <dinput.h>

class ZFXKeyboard;
class ZFXMouse;
class ZFXGamepad;

class ZFXDI : public ZFXInputDevice
{
public:
	ZFXDI(HINSTANCE hDLL);
	~ZFXDI();

	HRESULT Init(HWND, const RECT *, LONG, LONG, bool);

	void Release();
	bool IsRunning() { return m_bRunning; }
	bool HasGamepad(TCHAR *pGamepadName);

	HRESULT Update();

	bool IsPressed(ZFXINPUTDEV idType, UINT nID);
	bool IsReleased(ZFXINPUTDEV idType, UINT nID);
	HRESULT GetPosition(ZFXINPUTDEV idType, POINT *pPt);

private:
	LPDIRECTINPUT8	m_pDI;
	ZFXKeyboard		*m_pKB;
	ZFXMouse		*m_pMouse;
	ZFXGamepad		*m_pGamepad;
};

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
	long					m_lZ;
	long					m_lRx;	//joystick rotation values
	long					m_lRy;
	long					m_lRz;
	FILE					*m_pLog;
};

class ZFXKeyboard : public ZFXDIDevice
{
public:
	ZFXKeyboard(LPDIRECTINPUT8, HWND, FILE*);
	~ZFXKeyboard();

	HRESULT Init();
	HRESULT Update();

	bool	IsPressed(UINT nID);
	bool	IsReleased(UINT nID);

private:
	TCHAR m_Keys[256];
	TCHAR m_KeysOld[256];
};

#define MOUSE_BUTTON_LIMIT 7

class ZFXMouse : public ZFXDIDevice
{
public:
	ZFXMouse(LPDIRECTINPUT8, HWND, FILE*);
	~ZFXMouse();

	HRESULT Init();
	HRESULT Update();

	void	SetBounds(RECT rcCage, LONG lMaxScroll, LONG lMinScroll ) { m_rcCage = rcCage; m_lMaxScroll = lMaxScroll; m_lMinScroll = lMinScroll; }

	bool	IsPressed(UINT nBtn) { if (nBtn < MOUSE_BUTTON_LIMIT){ return m_bPressed[nBtn]; } return false; }

	bool	IsReleased(UINT nBtn) { if (nBtn < MOUSE_BUTTON_LIMIT){ return m_bReleased[nBtn]; } return false; }

private:
	HANDLE	m_hEvent;
	RECT	m_rcCage;
	LONG	m_lMaxScroll;
	LONG	m_lMinScroll;
	bool	m_bPressed[MOUSE_BUTTON_LIMIT];
	bool	m_bReleased[MOUSE_BUTTON_LIMIT];
	POINT	m_Delta;

};

class ZFXGamepad : public ZFXDIDevice
{
public:
	ZFXGamepad(LPDIRECTINPUT8, HWND, FILE*);
	~ZFXGamepad();

	HRESULT Init();
	HRESULT Update();

	bool GamepadFound() { return m_bPadFound; }
	BOOL EnumPadCallback(const DIDEVICEINSTANCE *pI);
	void GetName(TCHAR *pPadName) { wcscpy_s(m_Name, sizeof(TCHAR) * 256, pPadName); }

	bool IsPressed(UINT nBtn) { if (nBtn <m_dwNumBtns){ return m_bPressed[nBtn]; } return false; }
	bool IsReleased(UINT nBtn) { if (nBtn < m_dwNumBtns){ return m_bReleased[nBtn]; } return false; }

private:
	GUID	m_guid;
	TCHAR	m_Name[256];
	bool	m_bPadFound;
	bool	m_bPressed[12];
	bool	m_bReleased[12];
	DWORD	m_dwNumBtns;
};