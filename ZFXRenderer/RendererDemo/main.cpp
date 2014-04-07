//File: main.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#define WIN32_MEAN_AND_LEAN

#include "ZFXRenderer.h"
#include "ZFX.h"
#include "main.h"

//link the static library
#pragma comment(lib, "ZFXRenderer.lib")

//Windows global vars
HWND		g_hWnd	= NULL;
HINSTANCE	g_hInst = NULL;
TCHAR		g_szAppClass[] = TEXT("FrameWorkTest");

//application global vars
BOOL g_bIsActive	= FALSE;
bool g_bDone		= false;
FILE *g_pLog		= NULL;

//zfx global vars
LPZFXRENDERER		g_pRenderer = NULL;
LPZFXRENDERDEVICE	g_pDevice	= NULL;


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
	}

	while (!g_bDone)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (g_bIsActive)
		{
			if (g_pDevice->IsRunning())
			{
				g_pDevice->BeginRendering(true, true, true);
				g_pDevice->EndRendering();
			}
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
HRESULT ProgramStartup(TCHAR *chAPI)
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
	return g_pDevice->Init(g_hWnd, hWnd3D, 4, 16, 0, false);
}

//Free allocated resources
HRESULT ProgramCleanup()
{
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