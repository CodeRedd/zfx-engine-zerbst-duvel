//File: ZFXD3D_main.cpp 
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFXD3D.h"

HRESULT ZFXD3D::UseWindow(UINT nHwnd)
{
	LPDIRECT3DSURFACE9 pBack = NULL;

	if (!m_d3dpp.Windowed)
	{
		return ZFX_OK;
	}
	else if (nHwnd >= m_nNumWnd)
	{
		return ZFX_FAIL;
	}

	//try to get appropriate back buffer
	//Does directX 10/11 support stereo rendering? Might be a good thing to look at for later upgrades
	if (FAILED(m_pChain[nHwnd]->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBack)))
	{
		return ZFX_FAIL;
	}

	//activate it for the device
	//NOTE: The depth stencil surface attached to the device must not be smaller than render target dims!
	//Default code uses desktop size for implicit D3D swap chain/depth stencil surface, but if you want to create
	//other depth stencil surfaces, each render target will need one and you'll need to swap them out here.
	m_pDevice->SetRenderTarget(0, pBack);
	pBack->Release();
	m_nActivehWnd = nHwnd;
	return ZFX_OK;
}

//Clears out the buffers and starts rendering a new scene
HRESULT ZFXD3D::BeginRendering(bool bClearPixel, bool bClearDepth, bool bClearStencil)
{
	DWORD dw = 0;

	//anything to be cleared?
	if (bClearPixel || bClearDepth || bClearStencil)
	{
		if (bClearPixel)
		{
			dw |= D3DCLEAR_TARGET;
		}

		if (bClearDepth)
		{
			dw |= D3DCLEAR_ZBUFFER;
		}

		if (bClearStencil && m_bStencil)
		{
			dw |= D3DCLEAR_STENCIL;
		}

		if (FAILED(m_pDevice->Clear(0, NULL, dw, m_ClearColor, 1.0f, 0)))
		{
			return ZFX_FAIL;
		}
	}

	if (FAILED(m_pDevice->BeginScene()))
	{
		return ZFX_FAIL;
	}

	m_bIsSceneRunning = true;
	return ZFX_OK;
}

//Clears the buffers for the scene without starting a new one (possibly while it's running)
HRESULT ZFXD3D::Clear(bool bClearPixel, bool bClearDepth, bool bClearStencil)
{
	DWORD dw = 0;

	//we don't necessarily want to spend time clearing the pixel buffer every frame if every pixel is going to be overwritten next frame anyway
	if (bClearPixel)
	{
		dw |= D3DCLEAR_TARGET;
	}

	if (bClearDepth)
	{
		dw |= D3DCLEAR_ZBUFFER;
	}

	if (bClearStencil && m_bStencil)
	{
		dw |= D3DCLEAR_STENCIL;
	}

	if (m_bIsSceneRunning)
	{
		m_pDevice->EndScene();
	}

	if (FAILED(m_pDevice->Clear(0, NULL, dw, m_ClearColor, 1.0f, 0)))
	{
		return ZFX_FAIL;
	}

	if (m_bIsSceneRunning)
	{
		m_pDevice->BeginScene();
	}

	return ZFX_OK;
}

//stops rendering the current scene
void ZFXD3D::EndRendering()
{
	m_pDevice->EndScene();
	m_pDevice->Present(NULL, NULL, NULL, NULL); // DO NOT CALL THIS MORE THAN ONCE PER SWAP CHAIN PER FRAME
	m_bIsSceneRunning = false;
}

//Sets the default color for the renderer that will be shown when the scene is cleared
void ZFXD3D::SetClearColor(float fRed, float fGreen, float fBlue)
{
	m_ClearColor = D3DCOLOR_COLORVALUE(fRed, fGreen, fBlue, 1.0f);
}

//Creates a Direct3D Font. Note: Underlines and Strikethroughs currently are unused. Why the book gave code without using these I do not know...
HRESULT ZFXD3D::CreateFont(const wchar_t *chType, int nWeight, bool bItalic, bool bUnderline, bool bStrike, DWORD dwSize, UINT *pID)
{
	HRESULT hr;
	HDC hDC;
	int nHeight;

	if (!pID)
	{
		return ZFX_INVALIDPARAM;
	}

	hDC = GetDC(NULL);
	nHeight = -MulDiv(dwSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
	ReleaseDC(NULL, hDC);

	m_pFont = (LPD3DXFONT*)realloc(m_pFont, sizeof(LPD3DXFONT)*(m_nNumFonts+1));

	//build D3DX font from GDI font
	hr = D3DXCreateFont(m_pDevice,
						nHeight, nHeight/2, //height and width
						nWeight, // thickness: 0=default, 700=bold
						1, //mipmaps
						bItalic,
						DEFAULT_CHARSET, //char set
						OUT_DEFAULT_PRECIS, //precision
						DEFAULT_QUALITY, //quality
						DEFAULT_PITCH | FF_DONTCARE, //pitch and font family
						L"Arial", //font typeface
						&m_pFont[m_nNumFonts]); //address for new font

	if (SUCCEEDED(hr))
	{
		(*pID) = m_nNumFonts;
		m_nNumFonts++;
		return ZFX_OK;
	}
	else
	{
		return ZFX_FAIL;
	}
}

//renders text with a given font
HRESULT ZFXD3D::DrawText(UINT nID, int x, int y, UCHAR r, UCHAR g, UCHAR b, wchar_t *ch, ...)
{
	RECT rc = { x, y, 0, 0 };
	wchar_t cch[1024];
	va_list pArgs;

	//put variables into the args string
	va_start(pArgs, ch);
	vswprintf_s(cch, 256, ch, pArgs);

	if (nID >= m_nNumFonts)
	{
		return ZFX_INVALIDPARAM;
	}

	//calculate size of the text
	m_pFont[nID]->DrawText(NULL, cch, -1, &rc, DT_SINGLELINE | DT_CALCRECT, 0);

	//now draw the text
	m_pFont[nID]->DrawText(NULL, cch, -1, &rc, DT_SINGLELINE, D3DCOLOR_ARGB(255, r, g, b));

	return ZFX_OK;
}

void ZFXD3D::SetAmbientLight(float fRed, float fGreen, float fBlue)
{
	// last chance check
	m_pVertexMan->ForcedFlushAll();

	int nRed = (int)(fRed * 255.0f);
	int nGreen = (int)(fGreen * 255.0f);
	int nBlue = (int)(fBlue * 255.0f);

	if (m_bCanDoShaders)
	{
		// default setting to use as diffuse vertex color
		float fCol[4] = { fRed, fGreen, fBlue, 1.0f };
		m_pDevice->SetVertexShaderConstantF(4, fCol, 1);
	}

	m_pDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(nRed, nGreen, nBlue));
}