//File: ZFXD3D_vcache.cpp 
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFXD3D_vcache.h"    // class definition

extern bool g_bLF;

//////////////////
//VERTEX MANAGER
//////////////////
ZFXD3DVCManager::ZFXD3DVCManager(ZFXD3DSkinManager *pSkinMan, LPDIRECT3DDEVICE9 pDevice,
	ZFXD3D *pZFXD3D, UINT nMaxVerts, UINT nMaxIndic, FILE *pLog)
{
	DWORD dwID  = 1;
	int	  i     = 0;

	m_pSB			= NULL;
	m_pIB			= NULL; 
	m_nNumSB		= 0;
	m_nNumIB		= 0;

	m_pLog			= pLog;
	m_pDevice		= pDevice;
	m_pZFXD3D		= pZFXD3D;
	m_pSkinMan		= pSkinMan;
	m_dwActiveCache = MAX_ID;
	m_dwActiveSB	= MAX_ID;
	m_dwActiveIB	= MAX_ID;

	for (i = 0; i < NUM_CACHES; i++)
	{
		m_CachePS[i] = new ZFXD3DVCache(nMaxVerts, nMaxIndic, sizeof(PVERTEX), pSkinMan, pDevice, this,	dwID++, FVF_PVERTEX, pLog);
		m_CacheUU[i] = new ZFXD3DVCache(nMaxVerts, nMaxIndic, sizeof(VERTEX), pSkinMan, pDevice, this, dwID++, FVF_VERTEX, pLog);
		m_CacheUL[i] = new ZFXD3DVCache(nMaxVerts, nMaxIndic, sizeof(LVERTEX), pSkinMan, pDevice, this, dwID++, FVF_LVERTEX, pLog);
		m_CacheCA[i] = new ZFXD3DVCache(nMaxVerts, nMaxIndic, sizeof(CVERTEX), pSkinMan, pDevice, this,	dwID++, FVF_CVERTEX, pLog);
		m_Cache3T[i] = new ZFXD3DVCache(nMaxVerts, nMaxIndic, sizeof(VERTEX3T), pSkinMan, pDevice, this, dwID++, FVF_T3VERTEX, pLog);
		m_CacheTV[i] = new ZFXD3DVCache(nMaxVerts, nMaxIndic, sizeof(TVERTEX), pSkinMan, pDevice, this, dwID++, FVF_TVERTEX, pLog);
	}
}

ZFXD3DVCManager::~ZFXD3DVCManager()
{
	UINT n=0;
	int  i=0;

	//release memory in the static and index buffers
	if (m_pSB)
	{
		for (n = 0; n < m_nNumSB; n++)
		{
			if (m_pSB[n].pVB)
			{
				m_pSB[n].pVB->Release();
				m_pSB[n].pVB = NULL;
			}
			if (m_pSB[n].pIB)
			{
				m_pSB[n].pIB->Release();
				m_pSB[n].pIB = NULL;
			}
		}
		free( m_pSB );
		m_pSB = NULL;
	}

	if (m_pIB)
	{
		for (n = 0; n < m_nNumIB; n++)
		{
			if (m_pIB[n].pIB)
			{
				m_pIB[n].pIB->Release();
				m_pIB[n].pIB = NULL;
			}
		}
		free(m_pIB);
		m_pIB = NULL;
	}

	//release the vertex cache objects
	for (i = 0; i < NUM_CACHES; i++)
	{
		if (m_CachePS[i])
		{
			delete m_CachePS[i];
			m_CachePS[i] = NULL;
		}
		if (m_CacheUU[i])
		{
			delete m_CacheUU[i];
			m_CacheUU[i] = NULL;
		}
		if (m_CacheUL[i])
		{
			delete m_CacheUL[i];
			m_CacheUL[i] = NULL;
		}
		if (m_CacheCA[i])
		{
			delete m_CacheCA[i];
			m_CacheCA[i] = NULL;
		}		
		if (m_Cache3T[i])
		{
			delete m_Cache3T[i];
			m_Cache3T[i] = NULL;
		}		
		if (m_CacheTV[i])
		{
			delete m_CacheTV[i];
			m_CacheTV[i] = NULL;
		}
	}
}

HRESULT ZFXD3DVCManager::CreateStaticBuffer(ZFXVERTEXID VertexID, UINT nSkinID, UINT nVerts, UINT nIndic, const void *pVerts,
	const WORD *pIndic, UINT *pnID)
{
	HRESULT hr;
	DWORD	dwActualFVF;
	void	*pData;

	if (m_nNumSB >= (MAX_ID - 1))
	{
		return ZFX_OUTOFMEMORY;
	}

	//allocate memory if needed
	if ((m_nNumSB % ARRAY_ALLOCATION_SIZE) == 0)
	{
		int n = (m_nNumSB + ARRAY_ALLOCATION_SIZE)*sizeof(ZFXSTATICBUFFER);
		m_pSB = (ZFXSTATICBUFFER*)realloc(m_pSB, n);
		if (!m_pSB)
		{
			return ZFX_OUTOFMEMORY;
		}
	}

	m_pSB[m_nNumSB].nNumVerts = nVerts;
	m_pSB[m_nNumSB].nNumIndic = nIndic;
	m_pSB[m_nNumSB].nSkinID   = nSkinID;

	//size and format of the vertices
	switch (VertexID)
	{
	case VID_PS:
		m_pSB[m_nNumSB].nStride = sizeof(PVERTEX);
		m_pSB[m_nNumSB].dwFVF = FVF_PVERTEX;
		break;
	case VID_UU:
		m_pSB[m_nNumSB].nStride = sizeof(VERTEX);
		m_pSB[m_nNumSB].dwFVF = FVF_VERTEX;
		break;
	case VID_UL:
		m_pSB[m_nNumSB].nStride = sizeof(LVERTEX);
		m_pSB[m_nNumSB].dwFVF = FVF_LVERTEX;
		break;
	case VID_CA:
		m_pSB[m_nNumSB].nStride = sizeof(CVERTEX);
		m_pSB[m_nNumSB].dwFVF = FVF_CVERTEX;
		break;
	case VID_3T:
		m_pSB[m_nNumSB].nStride = sizeof(VERTEX3T);
		m_pSB[m_nNumSB].dwFVF = FVF_T3VERTEX;
		break;
	case VID_TV:
		m_pSB[m_nNumSB].nStride = sizeof(TVERTEX);
		m_pSB[m_nNumSB].dwFVF = FVF_TVERTEX;
		break;
	default:
		return ZFX_INVALIDID;
	}

	//create index buffer if needed
	if (nIndic > 0)
	{
		m_pSB[m_nNumSB].bIndic = true;
		m_pSB[m_nNumSB].nNumTris = int(nIndic / 3.0f);

		hr = m_pDevice->CreateIndexBuffer(nIndic*sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pSB[m_nNumSB].pIB,
										  NULL);

		if (FAILED(hr))
		{
			return ZFX_CREATEBUFFER;
		}

		//fill the index buffer
		if (SUCCEEDED(m_pSB[m_nNumSB].pIB->Lock(0, 0, (void**)(&pData), 0)))
		{
			memcpy(pData, pIndic, nIndic*sizeof(WORD));
			m_pSB[m_nNumSB].pIB->Unlock();
		}
		else
		{
			return ZFX_BUFFERLOCK;
		}
	}
	else
	{
		m_pSB[m_nNumSB].bIndic = true;
		m_pSB[m_nNumSB].nNumTris = int(nVerts / 3.0f);
		m_pSB[m_nNumSB].pIB = NULL;
	}

	//no need for FVF if we use shaders
	if (m_pZFXD3D->UsesShaders())
	{
		dwActualFVF = 0;
	}
	else
	{
		dwActualFVF = m_pSB[m_nNumSB].dwFVF;
	}

	//create vertex buffer
	hr = m_pDevice->CreateVertexBuffer(nVerts*m_pSB[m_nNumSB].nStride, D3DUSAGE_WRITEONLY, dwActualFVF, D3DPOOL_DEFAULT,
									   &m_pSB[m_nNumSB].pVB, NULL);

	if (FAILED(hr))
	{
		return ZFX_CREATEBUFFER;
	}

	//fill vertex buffer
	if (SUCCEEDED(m_pSB[m_nNumSB].pVB->Lock(0, 0, (void**)(&pData), 0)))
	{
		memcpy(pData, pVerts, nVerts*m_pSB[m_nNumSB].nStride);
		m_pSB[m_nNumSB].pVB->Unlock();
	}
	else
	{
		return ZFX_BUFFERLOCK;
	}

	(*pnID) = m_nNumSB;
	m_nNumSB++;
	return ZFX_OK;
}

ZFXRENDERSTATE ZFXD3DVCManager::GetShadeMode(void)
{
	return m_pZFXD3D->GetShadeMode();
}

HRESULT ZFXD3DVCManager::Render(ZFXVERTEXID VertexID, UINT nVerts, UINT nIndic, const void *pVerts, const WORD *pIndic, UINT SkinID)
{
	ZFXD3DVCache **pCache = NULL,
				  *pCacheEmpty = NULL,
				  *pCacheFullest = NULL;
	int nEmptyVC = -1;
	int nFullestVC = 0;

	bool bShaders = m_pZFXD3D->UsesShaders();

	//which vertex type?
	switch (VertexID)
	{
	case VID_PS:
		pCache = m_CachePS;
		break;
	case VID_UU:
		pCache = m_CacheUU;
		break;
	case VID_UL:
		pCache = m_CacheUL;
		break;
	case VID_CA:
		pCache = m_CacheCA;
		break;
	case VID_3T:
		pCache = m_Cache3T;
		break;
	case VID_TV:
		pCache = m_CacheTV;
		break;
	default:
		return ZFX_INVALIDID;
	}

	pCacheFullest = pCache[0];

	//active buffer is set invalid for now
	m_dwActiveSB = MAX_ID;

	//SEARCH FOR THE MOST APPROPRIATE BUFFER
	
	//is there a cache with this skin?
	for (int i = 0; i < NUM_CACHES; i++)
	{
		//yes, so add the data!
		if (pCache[i]->UsesSkin(SkinID))
		{
			return pCache[i]->Add(nVerts, nIndic, pVerts, pIndic, bShaders);
		}

		//save the last-found empty cache
		if (pCache[i]->IsEmpty())
		{
			pCacheEmpty = pCache[i];
		}

		//save the fullest cache
		if (pCache[i]->NumVerts() > pCacheFullest->NumVerts())
		{
			pCacheFullest = pCache[i];
		}
	}

	//if no cache is already using this skin, can we use an empty one?
	if (pCacheEmpty)
	{
		pCacheEmpty->SetSkin(SkinID, bShaders);
		return pCacheEmpty->Add(nVerts, nIndic, pVerts, pIndic, bShaders);
	}

	//if there are no empty caches, flush the fullest cache and use that
	pCacheFullest->Flush(bShaders);
	pCacheFullest->SetSkin(SkinID, bShaders);
	return pCacheFullest->Add(nVerts, nIndic, pVerts, pIndic, bShaders);
}

//render static buffer
HRESULT ZFXD3DVCManager::Render(UINT nID)
{
	HRESULT hr = ZFX_OK;

	ZFXRENDERSTATE sm = m_pZFXD3D->GetShadeMode();

	//active vertex cache is invalidated
	m_dwActiveCache = MAX_ID;

	//activate static buffer if not active yet
	if (m_dwActiveSB != nID)
	{
		//using indices?
		if (m_pSB[nID].bIndic)
		{
			m_pDevice->SetIndices(m_pSB[nID].pIB);
		}

		m_pDevice->SetStreamSource(0, m_pSB[nID].pVB, 0, m_pSB[nID].nStride);
		m_dwActiveSB = nID;
	}

	//is the skin active?
	if (m_pZFXD3D->GetActiveSkinID() != m_pSB[nID].nSkinID)
	{
		//set it active now
		ZFXSKIN *pSkin = &m_pSkinMan->GetSkin(m_pSB[nID].nSkinID);

		//Are we using wireframe mode?
		if (sm != RS_SHADE_SOLID)
		{
			ZFXMATERIAL *pMat = &m_pSkinMan->GetMaterial(pSkin->nMaterial);
			D3DMATERIAL9 mat = {
				pMat->cDiffuse.fR, pMat->cDiffuse.fG,
				pMat->cDiffuse.fB, pMat->cDiffuse.fA,
				pMat->cAmbient.fR, pMat->cAmbient.fG,
				pMat->cAmbient.fB, pMat->cAmbient.fA,
				pMat->cSpecular.fR, pMat->cSpecular.fG,
				pMat->cSpecular.fB, pMat->cSpecular.fA,
				pMat->cEmissive.fR, pMat->cEmissive.fG,
				pMat->cEmissive.fB, pMat->cEmissive.fA,
				pMat->fPower };
			m_pDevice->SetMaterial(&mat);

			//set texture
			for (int i = 0; i < 8; i++)
			{
				if (pSkin->nTexture[i] != MAX_ID)
				{
					m_pDevice->SetTexture(i, (LPDIRECT3DTEXTURE9)m_pSkinMan->GetTexture(pSkin->nTexture[i]).pData);
				}
			}
		}
		else
		{
			ZFXCOLOR clrWire = m_pZFXD3D->GetWireColor();

			//set material
			D3DMATERIAL9 matW = {
				clrWire.fR, clrWire.fG, clrWire.fB, clrWire.fA,
				clrWire.fR, clrWire.fG, clrWire.fB, clrWire.fA,
				0.0f,	    0.0f,		0.0f,		1.0f,
				0.0f,		0.0f,		0.0f,		1.0f,
				1.0f };
			m_pDevice->SetMaterial(&matW);

			//don't use a texture
			m_pDevice->SetTexture(0, NULL);
		}

		//set alpha states if necessary
		if (pSkin->bAlpha)
		{
			m_pDevice->SetRenderState(D3DRS_ALPHAREF, 50);
			m_pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
			m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
			m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		}
		else
		{
			m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		}

		//active skin has changed
		m_pZFXD3D->SetActiveSkinID(m_pSB[nID].nSkinID);
	}

	//if no shader is used, activate FVF
	if (!m_pZFXD3D->UsesShaders())
	{
		m_pDevice->SetFVF(m_pSB[nID].dwFVF);
	}

	//indexed primitives
	if (m_pSB[nID].bIndic)
	{
		if (sm == RS_SHADE_POINTS)
		{
			hr = m_pDevice->DrawPrimitive(D3DPT_POINTLIST, 0, m_pSB[nID].nNumVerts);
		}
		else if (sm == RS_SHADE_HULLWIRE)
		{
			hr = m_pDevice->DrawIndexedPrimitive(D3DPT_LINESTRIP, 0, 0, m_pSB[nID].nNumVerts, 0, m_pSB[nID].nNumVerts);
		}
		else
		{ //RS_SHADE_SOLID || RS_SHADE_TRIWIRE
			hr = m_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_pSB[nID].nNumVerts, 0, m_pSB[nID].nNumTris);
		}
	}
	else
	{
		if (sm == RS_SHADE_POINTS)
		{
			hr = m_pDevice->DrawPrimitive(D3DPT_POINTLIST, 0, m_pSB[nID].nNumVerts);
		}
		else if (sm == RS_SHADE_HULLWIRE)
		{
			hr = m_pDevice->DrawPrimitive(D3DPT_LINESTRIP, m_pSB[nID].nNumVerts, m_pSB[nID].nNumVerts);
		}
		else
		{ //RS_SHADE_SOLID || RS_SHADE_TRIWIRE
			hr = m_pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, m_pSB[nID].nNumVerts, m_pSB[nID].nNumTris);
		}
	}

	return hr;
}

HRESULT ZFXD3DVCManager::RenderPoints(ZFXVERTEXID VertexID, UINT nVerts, const void *pVerts, const ZFXCOLOR *pClr)
{
	D3DMATERIAL9	mtrl;
	DWORD			dwFVF;
	int				nStride;

	//invalidate active settings
	InvalidateStates();

	memset(&mtrl, 0, sizeof(D3DMATERIAL9));
	mtrl.Diffuse.r = mtrl.Ambient.r = pClr->fR;
	mtrl.Diffuse.g = mtrl.Ambient.g = pClr->fG;
	mtrl.Diffuse.b = mtrl.Ambient.b = pClr->fB;
	mtrl.Diffuse.a = mtrl.Ambient.a = pClr->fA;

	m_pDevice->SetMaterial(&mtrl);
	m_pDevice->SetTexture(0, NULL);

	//which vertex type?
	switch (VertexID)
	{
	case VID_PS:
		nStride = sizeof(PVERTEX);
		dwFVF = FVF_PVERTEX;
		break;
	case VID_UU:
		nStride = sizeof(VERTEX);
		dwFVF = FVF_VERTEX;
		break;
	case VID_UL:
		nStride = sizeof(LVERTEX);
		dwFVF = FVF_LVERTEX;
		break;
	case VID_CA:
		nStride = sizeof(CVERTEX);
		dwFVF = FVF_CVERTEX;
		break;
	case VID_3T:
		nStride = sizeof(VERTEX3T);
		dwFVF = FVF_T3VERTEX;
		break;
	case VID_TV:
		nStride = sizeof(TVERTEX);
		dwFVF = FVF_TVERTEX;
		break;
	default:
		return ZFX_INVALIDID;
	}

	//shader or FVF?
	if (m_pZFXD3D->UsesShaders())
	{
		m_pZFXD3D->ActivateVShader(0, VertexID);
		m_pZFXD3D->ActivatePShader(0);
	}
	else
	{
		m_pDevice->SetFVF(dwFVF);
	}

	//render list of points
	if (FAILED(m_pDevice->DrawPrimitiveUP(D3DPT_POINTLIST, nVerts, pVerts, nStride)))
	{
		return ZFX_FAIL;
	}
	return ZFX_OK;
}

HRESULT ZFXD3DVCManager::RenderLines(ZFXVERTEXID VertexID, UINT nVerts, const void *pVerts, const ZFXCOLOR *pClr, bool bStrip)
{
	D3DMATERIAL9	mtrl;
	DWORD			dwFVF;
	int				nStride;

	//invalidate active settings
	InvalidateStates();

	memset(&mtrl, 0, sizeof(D3DMATERIAL9));
	mtrl.Diffuse.r = mtrl.Ambient.r = pClr->fR;
	mtrl.Diffuse.g = mtrl.Ambient.g = pClr->fG;
	mtrl.Diffuse.b = mtrl.Ambient.b = pClr->fB;
	mtrl.Diffuse.a = mtrl.Ambient.a = pClr->fA;

	m_pDevice->SetMaterial(&mtrl);
	m_pDevice->SetTexture(0, NULL);

	//which vertex type?
	switch (VertexID)
	{
	case VID_PS:
		nStride = sizeof(PVERTEX);
		dwFVF = FVF_PVERTEX;
		break;
	case VID_UU:
		nStride = sizeof(VERTEX);
		dwFVF = FVF_VERTEX;
		break;
	case VID_UL:
		nStride = sizeof(LVERTEX);
		dwFVF = FVF_LVERTEX;
		break;
	case VID_CA:
		nStride = sizeof(CVERTEX);
		dwFVF = FVF_CVERTEX;
		break;
	case VID_3T:
		nStride = sizeof(VERTEX3T);
		dwFVF = FVF_T3VERTEX;
		break;
	case VID_TV:
		nStride = sizeof(TVERTEX);
		dwFVF = FVF_TVERTEX;
		break;
	default:
		return ZFX_INVALIDID;
	}

	//shader or FVF?
	if (m_pZFXD3D->UsesShaders())
	{
		m_pZFXD3D->ActivateVShader(0, VertexID);
		m_pZFXD3D->ActivatePShader(0);
	}
	else
	{
		m_pDevice->SetFVF(dwFVF);
	}

	//render list of points
	if (FAILED(m_pDevice->DrawPrimitiveUP(D3DPT_LINELIST, nVerts/2, pVerts, nStride)))
	{
		return ZFX_FAIL;
	}
	return ZFX_OK;
}

HRESULT ZFXD3DVCManager::ForcedFlush(ZFXVERTEXID VertexID)
{
	ZFXD3DVCache **pCache = NULL;
	HRESULT hr = ZFX_OK;
	int i = 0;

	bool bShaders = m_pZFXD3D->UsesShaders();

	switch (VertexID)
	{
	case VID_PS:
		pCache = m_CachePS;
		break;
	case VID_UU:
		pCache = m_CacheUU;
		break;
	case VID_UL:
		pCache = m_CacheUL;
		break;
	case VID_CA:
		pCache = m_CacheCA;
		break;
	case VID_3T:
		pCache = m_Cache3T;
		break;
	case VID_TV:
		pCache = m_CacheTV;
		break;
	default:
		return ZFX_INVALIDID;
	}

	for (i = 0; i < NUM_CACHES; i++)
	{
		if (FAILED(pCache[i]->Flush(bShaders)))
		{
			hr = ZFX_FAIL;
		}
	}

	return hr;
}

HRESULT ZFXD3DVCManager::ForcedFlushAll()
{
	HRESULT hr = ZFX_OK;
	bool bShaders = m_pZFXD3D->UsesShaders();
	int i;

	for (i = 0; i < NUM_CACHES; i++)
	{
		if (!m_CachePS[i]->IsEmpty())
		{
			if (FAILED(m_CachePS[i]->Flush(bShaders)))
			{
				hr = ZFX_FAIL;
			}
		}
	}

	for (i = 0; i < NUM_CACHES; i++)
	{
		if (!m_CacheUU[i]->IsEmpty())
		{
			if (FAILED(m_CacheUU[i]->Flush(bShaders)))
			{
				hr = ZFX_FAIL;
			}
		}
	}

	for (i = 0; i < NUM_CACHES; i++)
	{
		if (!m_CacheUL[i]->IsEmpty())
		{
			if (FAILED(m_CacheUL[i]->Flush(bShaders)))
			{
				hr = ZFX_FAIL;
			}
		}
	}

	for (i = 0; i < NUM_CACHES; i++)
	{
		if (!m_CacheCA[i]->IsEmpty())
		{
			if (FAILED(m_CacheCA[i]->Flush(bShaders)))
			{
				hr = ZFX_FAIL;
			}
		}
	}

	for (i = 0; i < NUM_CACHES; i++)
	{
		if (!m_Cache3T[i]->IsEmpty())
		{
			if (FAILED(m_Cache3T[i]->Flush(bShaders)))
			{
				hr = ZFX_FAIL;
			}
		}
	}

	for (i = 0; i < NUM_CACHES; i++)
	{
		if (!m_CacheTV[i]->IsEmpty())
		{
			if (FAILED(m_CacheTV[i]->Flush(bShaders)))
			{
				hr = ZFX_FAIL;
			}
		}
	}

	return hr;
}

//////////////////
//VERTEX CACHE
//////////////////
ZFXD3DVCache::ZFXD3DVCache(UINT nVertsMax, UINT nIndicMax, UINT nStride, ZFXD3DSkinManager *pSkinMan,
	LPDIRECT3DDEVICE9 pDevice, ZFXD3DVCManager *pManager, DWORD dwID, DWORD dwFVF, FILE *pLog)
{
	HRESULT hr;

	m_pDevice		= pDevice;
	m_pSkinMan		= pSkinMan;
	m_pManager		= pManager;
	m_nNumVertsMax	= nVertsMax;
	m_nNumIndicMax	= nIndicMax;
	m_nNumVerts		= 0;
	m_nNumIndic		= 0;
	m_dwID			= dwID;
	m_dwFVF			= dwFVF;
	m_nStride		= nStride;
	m_pLog			= pLog;

	memset(&m_Skin, MAX_ID, sizeof(ZFXSKIN));
	m_SkinID = MAX_ID;

	//create the buffer
	m_pVB = NULL;
	m_pIB = NULL;

	hr = pDevice->CreateVertexBuffer(nVertsMax * nStride, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pVB, NULL);

	if (FAILED(hr))
	{
		m_pVB = NULL;
	}

	hr = pDevice->CreateIndexBuffer(nIndicMax * nStride, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIB, NULL);

	if (FAILED(hr))
	{
		m_pIB = NULL;
	}
}

ZFXD3DVCache::~ZFXD3DVCache()
{
	if (m_pVB)
	{
		m_pVB->Release();
		m_pVB = NULL;
	}

	if (m_pIB)
	{
		m_pIB->Release();
		m_pIB = NULL;
	}
}

void ZFXD3DVCache::SetSkin(UINT SkinID, bool bUseShaders)
{
	if (!UsesSkin(SkinID))
	{
		ZFXSKIN *pSkin = &m_pSkinMan->GetSkin(SkinID);

		if (!IsEmpty())
		{
			Flush(bUseShaders);
		}

		memcpy(&m_Skin, pSkin, sizeof(ZFXSKIN));
		m_SkinID = SkinID;

		m_pManager->SetActiveCache(MAX_ID);
	}
}

HRESULT ZFXD3DVCache::Add(UINT nVerts, UINT nIndic, const void *pVerts, const WORD *pIndic, bool bUseShaders)
{
	BYTE  *tmp_pVerts = NULL;
	WORD  *tmp_pIndic = NULL;
	int	  nSizeV	= m_nStride * nVerts;
	int   nSizeI = sizeof(WORD) * nIndic;
	int   nPosV;
	int   nPosI;
	DWORD dwFlags;

	//is buffer size large enough?
	if (nVerts > m_nNumVertsMax | nIndic > m_nNumIndicMax)
	{
		return ZFX_BUFFERSIZE;
	}

	//cache is too full so empty it out
	if ((nVerts + m_nNumVerts) > m_nNumVertsMax | (nIndic + m_nNumIndic) > m_nNumIndicMax)
	{
		if (Flush(bUseShaders) != ZFX_OK)
		{
			return ZFX_FAIL;
		}
	}

	//set discard flag if buffer is empty
	if (m_nNumVerts == 0)
	{
		nPosV = nPosI = 0;
		dwFlags = D3DLOCK_DISCARD;
	}
	//else append with no-overwrite flag
	else
	{
		nPosV = m_nStride * m_nNumVerts;
		nPosI = sizeof(WORD) * m_nNumIndic;
		dwFlags = D3DLOCK_NOOVERWRITE;
	}

	//lock buffers
	if (FAILED(m_pVB->Lock(nPosV, nSizeV, (void**)&tmp_pVerts, dwFlags)))
	{
		return ZFX_BUFFERLOCK;
	}

	if (FAILED(m_pIB->Lock(nPosI, nSizeI, (void**)&tmp_pIndic, dwFlags)))
	{
		return ZFX_BUFFERLOCK;
	}

	//copy vertices
	memcpy(tmp_pVerts, pVerts, nSizeV);

	//copy indices
	int nBase = m_nNumVerts;
	if (!pIndic)
	{
		nIndic = nVerts;
	}

	for (UINT i = 0; i < nIndic; i++)
	{
		if (pIndic != NULL)
		{
			tmp_pIndic[i] = pIndic[i] + nBase;
		}
		else
		{
			tmp_pIndic[i] = i + nBase;
		}
		m_nNumIndic++;
	}

	//increment vertex counter
	m_nNumVerts += nVerts;

	m_pVB->Unlock();
	m_pIB->Unlock();
	return ZFX_OK;
}

HRESULT ZFXD3DVCache::Flush(bool bUseShaders)
{
	ZFXRENDERSTATE sm;
	HRESULT hr = ZFX_FAIL;
	if (m_nNumVerts <= 0)
	{
		return ZFX_OK;
	}

	//if this cache is not active
	if (m_pManager->GetActiveCache() != m_dwID)
	{
		//use FVF if we don't use shaders
		if (!bUseShaders)
		{
			m_pDevice->SetFVF(m_dwFVF);
		}

		m_pDevice->SetIndices(m_pIB);
		m_pDevice->SetStreamSource(0, m_pVB, 0, m_nStride);
		m_pManager->SetActiveCache(m_dwID);
	}

	//if this skin is not yet active
	if (m_pManager->GetZFXD3D()->GetActiveSkinID() != m_SkinID)
	{
		LPDIRECT3DTEXTURE9 pTex = NULL;
		ZFXMATERIAL *pMat = &m_pSkinMan->m_pMaterials[m_Skin.nMaterial];

		if (m_pManager->GetZFXD3D()->GetShadeMode() == RS_SHADE_SOLID)
		{
			//set the material
			D3DMATERIAL9 mat = {
				pMat->cDiffuse.fR,	pMat->cDiffuse.fG,
				pMat->cDiffuse.fB,  pMat->cDiffuse.fA,
				pMat->cAmbient.fR,  pMat->cAmbient.fG,
				pMat->cAmbient.fB,  pMat->cAmbient.fA,
				pMat->cSpecular.fR, pMat->cSpecular.fG,
				pMat->cSpecular.fB, pMat->cSpecular.fA,
				pMat->cEmissive.fR, pMat->cEmissive.fG,
				pMat->cEmissive.fB, pMat->cEmissive.fA,
				pMat->fPower };
			m_pDevice->SetMaterial(&mat);

			//set the texture
			for (int i = 0; i < 8; i++)
			{
				if (m_Skin.nTexture[i] != MAX_ID)
				{
					pTex = (LPDIRECT3DTEXTURE9)m_pSkinMan->m_pTextures[m_Skin.nTexture[i]].pData;
					m_pDevice->SetTexture(i, pTex);
				}
				else
				{
					break;
				}
			}
		}
		//Wireframe mode special case
		else
		{
			ZFXCOLOR clrWire = m_pManager->GetZFXD3D()->GetWireColor();
			
			//set material
			D3DMATERIAL9 matW = {
				clrWire.fR, clrWire.fG, clrWire.fB, clrWire.fA,
				clrWire.fR, clrWire.fG, clrWire.fB, clrWire.fA,
				0.0f,		0.0f,		0.0f,		1.0f,
				0.0f,		0.0f,		0.0f,		1.0f,
				1.0f };
			m_pDevice->SetMaterial(&matW);

			//no texture for the device
			m_pDevice->SetTexture(0, NULL);
		}

		//activate alpha blending
		if (m_Skin.bAlpha)
		{
			m_pDevice->SetRenderState(D3DRS_ALPHAREF, 50);
			m_pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
			m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
			m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		}
		else
		{
			m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		}

		//mark skin as active
		m_pManager->GetZFXD3D()->SetActiveSkinID(m_SkinID);
	}

	//FINALLY RENDER
	sm = m_pManager->GetZFXD3D()->GetShadeMode();

	//POINT-SPRITES
	if (sm == RS_SHADE_POINTS)
	{
		hr = m_pDevice->DrawPrimitive(D3DPT_POINTLIST, 0, m_nNumVerts);
	}
	//LINE STRIP
	else if (sm == RS_SHADE_HULLWIRE)
	{
		hr = m_pDevice->DrawIndexedPrimitive(D3DPT_LINESTRIP, 0, 0, m_nNumVerts, 0, m_nNumIndic/3);
	}
	//POLYGON LIST
	else
	{ // RS_SHADE_SOLID || RS_SHADE_TRIWIRE
		hr = m_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_nNumVerts, 0, m_nNumIndic/3);
	}

	if (FAILED(hr))
	{
		return ZFX_FAIL;
	}

	//reset counters
	m_nNumVerts = 0;
	m_nNumIndic = 0;
	return ZFX_OK;
}

//Lets the manager know to recalculate because we've done something to invalidate the active skin, cache, and static buffer.
//used largely when rendering points or lines
void ZFXD3DVCManager::InvalidateStates()
{
	m_pZFXD3D->SetActiveSkinID(MAX_ID);
	m_dwActiveSB = MAX_ID;
	m_dwActiveIB = MAX_ID;
	m_dwActiveCache = MAX_ID;
}