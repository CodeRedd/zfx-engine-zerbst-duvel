//File: ZFXD3D_init.cpp 
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "resource.h"

#include "ZFX.h"
#include "ZFXD3D.h"

//variables for callbacks to dialog
ZFXDEVICEINFO	g_xDevice;
D3DDISPLAYMODE	g_Dspmd;
D3DFORMAT		g_fmtA;
D3DFORMAT		g_fmtB;

//Creates a new ZFXD3D instance
extern "C"				__declspec(dllexport) HRESULT CreateRenderDevice(HINSTANCE hDLL, ZFXRenderDevice **pDevice)
{
	if( !*pDevice )
	{
		*pDevice = new ZFXD3D( hDLL );
		return ZFX_OK;
	}
	return ZFX_FAIL;
}

//Clears the ZFXD3D instance
extern "C"				__declspec(dllexport) HRESULT ReleaseRenderDevice(ZFXRenderDevice **pDevice)
{
	if (!*pDevice)
	{
		return ZFX_FAIL;
	}
	delete *pDevice;
	*pDevice = NULL;
	return ZFX_OK;
}

//Callback procedure for dialog
BOOL CALLBACK ZFXD3D::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	DIBSECTION	dibSection;
	BOOL		bWnd = FALSE;

	//get control handles
	HWND hFULL		 = GetDlgItem(hDlg, IDC_FULL);
	HWND hWND		 = GetDlgItem(hDlg, IDC_WND);
	HWND hADAPTER	 = GetDlgItem(hDlg, IDC_ADAPTER);
	HWND hMODE		 = GetDlgItem(hDlg, IDC_MODE);
	HWND hADAPTERFMT = GetDlgItem(hDlg, IDC_ADAPTERFMT);
	HWND hBACKFMT	 = GetDlgItem(hDlg, IDC_BACKFMT);
	HWND hDEVICE	 = GetDlgItem(hDlg, IDC_DEVICE);

	switch (message)
		{
		//preselect windowed mode when we initialize the dialog
		case WM_INITDIALOG:
		{
			SendMessage(hWND, BM_SETCHECK, BST_CHECKED, 0);
			m_pEnum->Enum(hADAPTER, hMODE, hDEVICE, hADAPTERFMT, hBACKFMT, hWND, hFULL, m_pLog);
			return TRUE;
		}
		
		// Zerbst and Duvel also specify a case for rendering a bitmap logo inside the dialog, but we're not going to worry about that here

		// a control reports a message
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
				{
					//OK Button
				case IDOK:
				{
					m_bWindowed = SendMessage(hFULL, BM_GETCHECK, 0, 0) != BST_CHECKED;
					m_pEnum->GetSelections(&g_xDevice, &g_Dspmd, &g_fmtA, &g_fmtB);
					GetWindowTextW(hADAPTER,m_chAdapter, 256);
					EndDialog(hDlg, 1);
					return TRUE;
				} break;

				//Cancel button
				case IDCANCEL:
				{
					EndDialog(hDlg, 0);
					return TRUE;
				} break;

				case IDC_ADAPTER:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						m_pEnum->ChangedAdapter();
					}
				} break;

				case IDC_DEVICE:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						m_pEnum->ChangedDevice();
					}
				}break;

				case IDC_ADAPTERFMT:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						m_pEnum->ChangedAdapterFmt();
					}
				}break;

				case IDC_FULL: case IDC_WND:
				{
					m_pEnum->ChangedWindowMode();
				}break;
			} //inner switch
		}break;//last case
	}//outer switch
	return FALSE;
}
