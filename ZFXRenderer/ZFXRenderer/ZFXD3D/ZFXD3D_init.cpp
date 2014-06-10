//File: ZFXD3D_init.cpp 
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "resource.h"
#include "ZFXD3D_vcache.h"

//variables for callbacks to dialog
ZFXDEVICEINFO	g_xDevice;
D3DDISPLAYMODE	g_Dspmd;
D3DFORMAT		g_fmtA;
D3DFORMAT		g_fmtB;

//initialization global variables
ZFXD3D *g_ZFXD3D = NULL;

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

//Constructor--basic variable initialization
ZFXD3D::ZFXD3D(HINSTANCE hDLL)
{
	m_hDLL				= hDLL;
	m_pEnum				= NULL;
	m_pD3D				= NULL;
	m_pDevice			= NULL;
	m_pLog				= NULL;
	m_ClearColor		= D3DCOLOR_COLORVALUE(0.0f, 0.0f, 0.0f, 1.0f);
	m_bRunning			= false;
	m_bIsSceneRunning	= false;
	m_bCanDoShaders		= false;
	m_bUseShaders		= false;
	m_nActivehWnd		= 0;
	
	//Authors note that having a global var for a singleton is quick and dirty style
	g_ZFXD3D = this;
}

//Destructor
ZFXD3D::~ZFXD3D()
{
	Release();
}

//Does the actual destruction work
void ZFXD3D::Release()
{
	if (m_pEnum)
	{
		delete m_pEnum;
		m_pEnum = NULL;
	}
	if (m_pDevice)
	{
		m_pDevice->Release();
		m_pDevice = NULL;
	}
	if (m_pD3D)
	{
		m_pD3D->Release();
		m_pD3D = NULL;
	}
	fclose(m_pLog);
}

//Wrapper for the dialog callback
//We have to create this because the compiler wants to confirm existence of a callback function at compile time,
//but the DlgProc function (apparently?) requires the existence of a ZFXD3D object to work. So we write a less OO wrapper for it instead
BOOL CALLBACK DlgProcWrap(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return g_ZFXD3D->DlgProc(hDlg, message, wParam, lParam);
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

//Starts the API up
HRESULT ZFXD3D::Go()
{
	ZFXCOMBOINFO	xCombo;
	HRESULT			hr;
	HWND			hwnd;

	//create Direct3D main object
	if (m_pD3D)
	{
		m_pD3D->Release();
		m_pD3D = NULL;
	}
	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	if (!m_pD3D)
	{
		return ZFX_CREATEAPI;
	}

	//get correct combo
	//clarification: combo, in D3D terms, is a combination of front/back buffer format, adapter, device type, vertex processing type, 
	//and depth-stencil format
	for (UINT i = 0; g_xDevice.nNumCombo; i++)
	{
		if ((g_xDevice.d3dCombo[i].bWindowed == m_bWindowed) && (g_xDevice.d3dCombo[i].d3dDevType == g_xDevice.d3dDevType) &&
			(g_xDevice.d3dCombo[i].fmtAdapter == g_fmtA) && (g_xDevice.d3dCombo[i].fmtBackBuffer == g_fmtB))
		{
			xCombo = g_xDevice.d3dCombo[i];
			break;
		}
	}

	//fill in present parameter structure
	SecureZeroMemory(&m_d3dpp, sizeof(m_d3dpp));
	m_d3dpp.Windowed				= m_bWindowed;
	m_d3dpp.BackBufferCount			= 1;
	m_d3dpp.BackBufferFormat		= g_Dspmd.Format;
	m_d3dpp.EnableAutoDepthStencil	= TRUE;
	m_d3dpp.MultiSampleType			= xCombo.msType;
	m_d3dpp.AutoDepthStencilFormat	= xCombo.fmtDepthStencil;
	m_d3dpp.SwapEffect				= D3DSWAPEFFECT_DISCARD;

	//stencil buffer active?
	m_bStencil = xCombo.fmtDepthStencil == D3DFMT_D24S8 || xCombo.fmtDepthStencil == D3DFMT_D24X4S4 || xCombo.fmtDepthStencil == D3DFMT_D15S1;

	//fullscreen mode
	if (!m_bWindowed)
	{
		m_d3dpp.hDeviceWindow		= hwnd = m_hWndMain;
		m_d3dpp.BackBufferWidth		= g_Dspmd.Width;
		m_d3dpp.BackBufferHeight	= g_Dspmd.Height;
		ShowCursor(FALSE);
	}
	//windowed mode
	else
	{
		m_d3dpp.hDeviceWindow = hwnd = m_hWnd[0];
		m_d3dpp.BackBufferWidth = GetSystemMetrics(SM_CXSCREEN);
		m_d3dpp.BackBufferHeight = GetSystemMetrics(SM_CYSCREEN);
	}

	//create direct3d device
	hr = m_pD3D->CreateDevice(g_xDevice.nAdapter, g_xDevice.d3dDevType, hwnd, xCombo.dwBehavior, &m_d3dpp, &m_pDevice);

	//create swap chains if needed
	//explanation from the book: a swap chain is a D3D tool that lets us render to more than one window. Used implicitly when changing between
	//front and back buffer with Present() function
	if (m_nNumWnd > 0 && m_bWindowed)
	{
		for (UINT i = 0; i < m_nNumWnd; i++)
		{
			m_d3dpp.hDeviceWindow - m_hWnd[i];
			m_pDevice->CreateAdditionalSwapChain(&m_d3dpp, &m_pChain[i]);
		}
	}

	delete m_pEnum;
	m_pEnum = NULL;

	if (FAILED(hr))
	{
		return ZFX_CREATEDEVICE;
	}

	m_bRunning = true;
	m_bIsSceneRunning = false;
	m_dwWidth = m_d3dpp.BackBufferWidth;
	m_dwHeight = m_d3dpp.BackBufferHeight;

	return OneTimeInit();
}

//Initializes the renderer and opens/processes results from the settings dialog
HRESULT ZFXD3D::Init(HWND hWnd, const HWND *hWnd3D, int nNumhWnd, int nMinDepth, int nMinStencil, bool bSaveLog)
{
	int nResult;

	FILE** pLog = &m_pLog;
	fopen_s(pLog, "log_renderdevice.txt", "w");
	if (!pLog)
	{
		return ZFX_FAIL;
	}
	m_pLog = *pLog;


	//should I use child windows??
	if (nNumhWnd > 0)
	{
		if (nNumhWnd > MAX_3DHWND)
		{
			nNumhWnd = MAX_3DHWND;
		}
		memcpy(&m_hWnd[0], hWnd3D, sizeof(HWND)*nNumhWnd);
	}
	//else use main window handle
	else
	{
		m_hWnd[0] = hWnd;
		m_nNumWnd = 0;
	}
	m_hWndMain = hWnd;;

	if (nMinStencil > 0)
	{
		m_bStencil = true;
	}

	//generate an enum object
	m_pEnum = new ZFXD3DEnum(nMinDepth, nMinStencil);

	//Here, we would load the ZFX logo image if we cared to do so and save it to a global variable
	//but since we don't have the .bmp from the CD we won't

	//open dialog
	nResult = DialogBox(m_hDLL, L"dlgChangeDevice", hWnd, DlgProcWrap);

	//we would free the bitmap's global var here since the dialog is closed now, but again we don't have a bitmap to load so we won't

	//is there an error from the dialog?
	if (nResult == -1)
	{
		return ZFX_FAIL;
	}
	//User cancelled the dialog
	else if (nResult == 0)
	{
		return ZFX_CANCELED;
	}
	//Dialog ok
	else
	{
		return Go();
	}


}

//from the CD:
//an alternative to the Init()->Go() combination that will fire up the renderer in windowed mode without prompting the user w/ a dialog
HRESULT ZFXD3D::InitWindowed(HWND hWnd, const HWND *hWnd3D,
	int nNumhWnd, bool bSaveLog) {
	HRESULT hr;
	HWND    hwnd;

	// are there any child windows to use?
	if (nNumhWnd > 0) {
		if (nNumhWnd > MAX_3DHWND) nNumhWnd = MAX_3DHWND;
		memcpy(&m_hWnd[0], hWnd3D, sizeof(HWND)*nNumhWnd);
		m_nNumWnd = nNumhWnd;
	}
	// else store handle to main window at least
	else {
		m_hWnd[0] = hWnd;
		m_nNumWnd = 0;
	}

	m_hWndMain = hWnd;
	m_bWindowed = true;

	// build main direct3d object
	if (m_pD3D) {
		m_pD3D->Release();
		m_pD3D = NULL;
	}
	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!m_pD3D) {
		Log(L"error: Direct3DCreate8()");
		return ZFX_CREATEAPI;
	}

	// prepare present parameters structure
	ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));
	m_d3dpp.Windowed = m_bWindowed;
	m_d3dpp.BackBufferCount = 1;
	m_d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	m_d3dpp.EnableAutoDepthStencil = TRUE;
	m_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	m_d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_bStencil = false;

	// windowed mode
	m_d3dpp.hDeviceWindow = hwnd = m_hWnd[0];
	m_d3dpp.BackBufferWidth = GetSystemMetrics(SM_CXSCREEN);
	m_d3dpp.BackBufferHeight = GetSystemMetrics(SM_CYSCREEN);

	// create direct3d device
	hr = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hwnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&m_d3dpp,
		&m_pDevice);

	// create additional swap chains if wished and possible
	if ((m_nNumWnd > 0) && m_bWindowed) {
		for (UINT i = 0; i<m_nNumWnd; i++) {
			m_d3dpp.hDeviceWindow = m_hWnd[i];
			m_pDevice->CreateAdditionalSwapChain(&m_d3dpp, &m_pChain[i]);
		}
	}

	if (FAILED(hr)) {
		Log(L"error: IDirect3D::CreateDevice()");
		return ZFX_CREATEDEVICE;
	}

	Log(L"initialized (online and ready)");
	m_pDevice->GetDeviceCaps(&g_xDevice.d3dCaps);

	m_bRunning = true;
	m_bIsSceneRunning = false;
	m_dwWidth = m_d3dpp.BackBufferWidth;
	m_dwHeight = m_d3dpp.BackBufferHeight;

	return OneTimeInit();
}

HRESULT ZFXD3D::OneTimeInit()
{
	ZFX3DInitCPU();

	m_bUseShaders = true;

	m_pSkinMan = new ZFXD3DSkinManager(m_pDevice, m_pLog);

	m_pVertexMan = new ZFXD3DVCManager((ZFXD3DSkinManager*)m_pSkinMan, m_pDevice, this, 3000, 4500, m_pLog);

	//activate render states
	m_pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);

	//create standard material
	memset(&m_StdMtrl, 0, sizeof(D3DMATERIAL9));
	m_StdMtrl.Ambient.r = 1.0f;
	m_StdMtrl.Ambient.g = 1.0f;
	m_StdMtrl.Ambient.b = 1.0f;
	m_StdMtrl.Ambient.a = 1.0f;

	if (FAILED(m_pDevice->SetMaterial(&m_StdMtrl)))
	{
		Log(L"error: set material (OneTimeInit)");
		return ZFX_FAIL;
	}

	//activate texture filtering
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	ZFXVIEWPORT vpView = { 0, 0, m_dwWidth, m_dwHeight };
	m_Mode = EMD_PERSPECTIVE;
	m_nStage = -1;
	SetActiveSkinID(MAX_ID);

	//identity matrix for view matrix
	IDENTITY(m_mView3D);

	//clipping plane values
	SetClippingPlanes(0.1f, 1000.0f);

	//init shader stuff
	PrepareShaderStuff();

	//build default shader with ID = 0
	/*if (m_bUseShaders) BUG: Default shader fails to compile
	{
		const wchar_t BaseShader[] =
			L"vs.1.1				\n"\
			L"dcl_position  v0		\n"\
			L"dcl_normal	v3		\n"\
			L"dcl_texcoord  v6		\n"\
			L"dp4 oPos.x, v0, c0	\n"\
			L"dp4 oPos.y, v0, c1	\n"\
			L"dp4 oPos.z, v0, c2	\n"\
			L"dp4 oPos.w, v0, c3	\n"\
			L"mov oD0, c4			\n"\
			L"mov oT0, v6			\n";

		if (FAILED(CreateVShader((void*)BaseShader, sizeof(BaseShader), false, false, NULL)))
		{
			return ZFX_FAIL;
		}
		if (FAILED(ActivateVShader(0, VID_UU)))
		{
			return ZFX_FAIL;
		}
	}*/

	//set ambient light level
	SetAmbientLight(1.0f, 1.0f, 1.0f);

	//set perspective projection stage 0
	if (FAILED(InitStage(0.8f, &vpView, 0)))
	{
		return ZFX_FAIL;
	}

	//activate perspective projection stage 1
	if (FAILED(SetMode(EMD_PERSPECTIVE, 0)))
	{
		return ZFX_FAIL;
	}

	return ZFX_OK;
}

//logging function that writes to logfile
void ZFXD3D::Log(wchar_t *chString, ...) {

	wchar_t ch[256];
	va_list pArgs;

	va_start(pArgs, chString);
	vswprintf_s(ch, 256 , chString, pArgs);
	fwprintf(m_pLog, ch);

#ifdef _DEBUGFLUSH_
	fflush(m_pLog);
#endif
}