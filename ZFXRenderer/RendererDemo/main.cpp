//File: main.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#define WIN32_MEAN_AND_LEAN

#include "main.h"
#include "s3d_loader.h"
#include <ZFX.h>
#include <ZFX3D.h>
#include <ZFXRenderer.h>

//Windows global vars
HWND		g_hWnd	= NULL;
HINSTANCE	g_hInst = NULL;
wchar_t		g_szAppClass[] = TEXT("FrameWorkTest");

//application global vars
BOOL g_bIsActive	= FALSE;
bool g_bDone		= false;
FILE *g_pLog		= NULL;

//zfx global vars
LPZFXRENDERER		g_pRenderer = NULL;
LPZFXRENDERDEVICE	g_pDevice	= NULL;

//model mocks
ZFXModel *g_pG3 = NULL,
		 *g_pLeopard2 = NULL,
		 *g_pMarder = NULL;

//WinMain entry point
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX		wndclass;
	HRESULT			hr;
	HWND			hWnd;
	MSG				msg;

	//initialize the window
	wndclass.hIconSm		= LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wndclass.cbSize			= sizeof(wndclass);
	wndclass.lpfnWndProc	= MsgProc;
	wndclass.cbClsExtra		= 0;
	wndclass.cbWndExtra		= 0;
	wndclass.hInstance		= hInst;
	wndclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
	wndclass.lpszMenuName	= NULL;
	wndclass.lpszClassName	= g_szAppClass;
	wndclass.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;

	if (RegisterClassEx(&wndclass) == 0)
	{
		return 0;
	}

	if (!(hWnd = CreateWindowEx(NULL, g_szAppClass, L"Cranking up ZFXEngine", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		GetSystemMetrics(SM_CXSCREEN) / 2 - 190, GetSystemMetrics(SM_CYSCREEN) / 2 - 140, 380, 280, NULL, NULL, hInst, NULL) ) )
	{
		return 0;
	}

	g_hWnd = hWnd;
	g_hInst = hInst;

	FILE** pLog = &g_pLog;
	fopen_s(pLog, "log_main.txt", "w");

	if (!pLog)
	{
		return 0;
	}
	g_pLog = *pLog;

	//start the engine
	if (FAILED(hr = ProgramStartup(L"Direct3D")))
	{
		fprintf(g_pLog, "error: ProgramStartup() failed\n");
		g_bDone = true;
	}
	else if (hr == ZFX_CANCELED) 
	{
		fprintf(g_pLog, "ProgramStartup() canceled\n");
		g_bDone = true;
		PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
	}
	else
	{
		g_pDevice->SetClearColor(0.1f, 0.3f, 0.1f);

		g_pLeopard2 = new ZFXModel("models\\leo2.s3d", g_pDevice, g_pLog);
		g_pG3 = new ZFXModel("models\\G3.s3d", g_pDevice, g_pLog);
		g_pMarder = new ZFXModel("models\\ma3.s3d", g_pDevice, g_pLog);

		if (FAILED(BuildAndSetShader()))
		{
			g_bDone = true;
		}
	}

	ZFXVector vR(1, 0, 0), vU(0, 1, 0), vD(0, 0, 1), vP(0, 0, 0);

	while (!g_bDone)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//do one frame on each window
		if (g_bIsActive)
		{ //BUG: For some reason, only window 0 is being used
			g_pDevice->UseWindow(0);
			g_pDevice->SetView3D(vR, vU, vD, vP);
			ProgramTick();

			g_pDevice->UseWindow(1);
			g_pDevice->SetView3D(vU*-1.0f, vR, vD, vP);
			ProgramTick();

			g_pDevice->UseWindow(2);
			g_pDevice->SetView3D(vR*-1.0f, vU*-1.0f, vD, vP);
			ProgramTick();

			g_pDevice->UseWindow(3);
			g_pDevice->SetView3D(vU, vR*-1.0f, vD, vP);
			ProgramTick();
		}
	}

	//cleanup
	ProgramCleanup();
	UnregisterClass(g_szAppClass, hInst);
	return (int)msg.wParam;
}

//MsgProc to proceed Windows messages
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		//application focus
		case WM_ACTIVATE:
		{
			g_bIsActive = (BOOL)wParam;
		} break;
		//key events
		case WM_KEYDOWN:
		{
			switch (wParam)
			{
				case VK_ESCAPE:
				{
					g_bDone = true;
					PostMessage(hWnd, WM_CLOSE, 0, 0);
					return 0;
				} break;
			}
		}break;
		//destroy window
		case WM_DESTROY:
		{
			g_bDone = true;
			PostQuitMessage(0);
			return 1;
		} break;
		default:
		{
			break;
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

//Create the render device object
HRESULT ProgramStartup(wchar_t *chAPI)
{
	HWND hWnd3D[4];
	RECT rcWnd;
	int x = 0, y = 0;

	//we don't have OpenGL support, but if we did we'd activate it here
	if (wcscmp(chAPI, L"OpenGL") == 0)
	{
		return S_OK;
	}

	//create the renderer object
	g_pRenderer = new ZFXRenderer(g_hInst);

	//create the render device
	if (FAILED( g_pRenderer->CreateDevice(chAPI) ) )
	{
		return E_FAIL;
	}

	//save pointer to the render device
	g_pDevice = g_pRenderer->GetDevice();
	if (g_pDevice == NULL)
	{
		return E_FAIL;
	}

	//query client area size
	GetClientRect(g_hWnd, &rcWnd);

	for (int i = 0; i < 4; i++)
	{
		if (i == 0 || i == 2)
		{
			x = 10;
		}
		else
		{
			x = rcWnd.right / 2 + 10;
		}

		if (i == 0 || i == 1)
		{
			y = 10;
		}
		else
		{
			y = rcWnd.bottom / 2 + 10;
		}

		hWnd3D[i] = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("static"), NULL, WS_CHILD | SS_BLACKRECT | WS_VISIBLE, x, y,
			rcWnd.right / 2 - 20, rcWnd.bottom / 2 - 20, g_hWnd, NULL, g_hInst, NULL);
	}
	
	//initialize render device (show dialog box)
	if (FAILED(g_pDevice->Init(g_hWnd, hWnd3D, 4, 16, 0, false)))
	{
		return E_FAIL;
	}

	//new bugfixes
	POINT ptRes;
	g_pDevice->GetResolution(&ptRes);
	LONG lx = 0, ldx = 0, ldy = 0, fs = 0;

	// viewport size / position
	ldx = ptRes.x / 2.666666f;
	ldy = ldx / 1.333333f;
	lx = ptRes.x - ldx - 10;

	// font size
	fs = ptRes.x / 20;

	//prepare viewport
	ZFXVIEWPORT rc = { 750, 50, 480, 360 };
	g_pDevice->InitStage(0.8f, NULL, 0);
	g_pDevice->InitStage(0.8f, &rc, 1);
}

//Free allocated resources
HRESULT ProgramCleanup()
{
	if (g_pG3) {
		delete g_pG3;
		g_pG3 = NULL;
	}
	if (g_pLeopard2) {
		delete g_pLeopard2;
		g_pLeopard2 = NULL;
	}
	if (g_pMarder) {
		delete g_pMarder;
		g_pMarder = NULL;
	}

	if (g_pRenderer)
	{
		delete g_pRenderer;
		g_pRenderer = NULL;
	}

	if (g_pLog)
	{
		fclose(g_pLog);
		g_pLog = NULL;
	}

	return S_OK;
}

//call once each frame to render some objects
HRESULT ProgramTick()
{
	ZFXMatrix mWorld;
	mWorld.Identity();

	HRESULT hr = ZFX_FAIL;

	//activate first viewport
	g_pDevice->SetMode(EMD_PERSPECTIVE, 0);
	g_pDevice->SetClearColor(0.7f, 0.7f, 1.0f);

	//clear buffer and start scene
	g_pDevice->BeginRendering(true, true, true);

	//RENDER CALLS

	mWorld.Translate(-2.0f, 1.0f, 13.0f);
	g_pDevice->SetWorldTransform(&mWorld);
	hr = g_pMarder->Render(true, false);

	mWorld.Translate(-1.0f, -5.0f, 15.0f);
	g_pDevice->SetWorldTransform(&mWorld);
	hr = g_pLeopard2->Render(true, false);

	mWorld.Translate(1.1f, -0.6f, 4.5f);
	g_pDevice->SetWorldTransform(&mWorld);
	hr = g_pG3->Render(true, false);

	//activate second viewport
	g_pDevice->SetMode(EMD_PERSPECTIVE, 1);
	g_pDevice->SetClearColor(1.0f, 0.2f, 0.2f);
	g_pDevice->Clear(true, true, true);

	// MORE RENDER CALLS
	mWorld.Translate(-2.0f, 1.0f, 13.0f);
	g_pDevice->SetWorldTransform(&mWorld);
	g_pMarder->Render(true, true);

	mWorld.Translate(-1.0f, -5.0f, 15.0f);
	g_pDevice->SetWorldTransform(&mWorld);
	g_pLeopard2->Render(true, true);

	mWorld.Translate(1.1f, -0.6f, 4.5f);
	g_pDevice->SetWorldTransform(&mWorld);
	g_pG3->Render(true, true);

	g_pDevice->EndRendering();
	return ZFX_OK;
}

//loads some mock data
HRESULT BuildAndSetShader(void) {
	UINT nID = 0;

	if (!g_pDevice->CanDoShaders()) return S_OK;

	if (FAILED(g_pDevice->CreateVShader(L"Shaders\\test.vsh", 0, true, false, &nID))) 
	{

		return ZFX_FAIL;
	}

	if (FAILED(g_pDevice->ActivateVShader(nID, VID_UU))) 
	{
		return ZFX_FAIL;
	}

	if (FAILED(g_pDevice->CreatePShader(L"Shaders\\test.psh", 0, true, false, &nID)))
	{
		return ZFX_FAIL;
	}

	if (FAILED(g_pDevice->ActivatePShader(nID))) 
	{
		return ZFX_FAIL;
	}

	return ZFX_OK;
}