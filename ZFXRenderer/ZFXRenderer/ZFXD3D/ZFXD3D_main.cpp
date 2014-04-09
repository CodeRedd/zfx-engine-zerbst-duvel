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