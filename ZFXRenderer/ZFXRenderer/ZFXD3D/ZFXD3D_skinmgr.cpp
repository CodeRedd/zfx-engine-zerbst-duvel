//File: ZFXD3D_skinmgr.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFXD3D_skinmgr.h"   // class definition

extern bool g_bLF; //determines whether to flush the log file for crash safety

//constructor
//NOTE: The arrays for textures, skins, and materials have entry memory allocated in chunks of size MAX_ARRAY_ALLOCATION to prevent
//excessive fragmentation
ZFXD3DSkinManager::ZFXD3DSkinManager(LPDIRECT3DDEVICE9 pDevice, FILE *pLog)
{
	m_nNumMaterials = 0;
	m_nNumTextures  = 0;
	m_nNumSkins		= 0;
	m_pMaterials	= NULL;
	m_pTextures		= NULL;
	m_pSkins		= NULL;
	m_pLog			= pLog;
	m_pDevice		= pDevice;
	Log(L"online");
}

//destructor
ZFXD3DSkinManager::~ZFXD3DSkinManager()
{
	//release D3D textures
	if (m_pTextures)
	{
		for (UINT i = 0; i < m_nNumTextures; i++)
		{
			if (m_pTextures[i].pData)
			{
				((LPDIRECT3DTEXTURE9)m_pTextures[i].pData)->Release();
				m_pTextures[i].pData = NULL;
			}
			if (m_pTextures[i].pClrKeys)
			{
				delete [] m_pTextures[i].pClrKeys;
				m_pTextures[i].pClrKeys = NULL;
			}
			if (m_pTextures[i].chName)
			{
				delete [] m_pTextures[i].chName;
				m_pTextures[i].chName = NULL;
			}
		}

		free( m_pTextures );
		m_pTextures = NULL;
	}

	//free memory
	if (m_pMaterials)
	{
		free(m_pMaterials);
		m_pMaterials = NULL;
	}
	if (m_pSkins)
	{
		free(m_pSkins);
		m_pSkins = NULL;
	}

	Log(L"offline (ok)");
}

HRESULT ZFXD3DSkinManager::AddSkin(const ZFXCOLOR *pcAmbient, const ZFXCOLOR *pcDiffuse, const ZFXCOLOR *pcEmissive,
								   const ZFXCOLOR *pcSpecular, float fSpecPower, UINT *nSkinID)
{
	UINT nMat, n;
	bool bMat = false;

	//allocate memory for new skins if needed
	if (m_nNumSkins % ARRAY_ALLOCATION_SIZE == 0)
	{
		n = (m_nNumSkins + ARRAY_ALLOCATION_SIZE)*sizeof(ZFXSKIN);
		m_pSkins = (ZFXSKIN*)realloc(m_pSkins, n);
		if (!m_pSkins)
		{
			return ZFX_OUTOFMEMORY;
		}
	}

	ZFXMATERIAL mat;
	mat.cAmbient = *pcAmbient;
	mat.cDiffuse = *pcDiffuse;
	mat.cEmissive = *pcEmissive;
	mat.cSpecular = *pcSpecular;
	mat.fPower = fSpecPower;

	//does this material already exist?
	for (nMat = 0; nMat < m_nNumMaterials; nMat++)
	{
		if (MaterialEqual(&mat, &m_pMaterials[nMat]))
		{
			bMat = true;
			break;
		}
	}

	//if so, save its id, otherwise add the new material and increment counter
	if (bMat)
	{
		m_pSkins[m_nNumSkins].nMaterial = nMat;
	}
	else
	{
		m_pSkins[m_nNumSkins].nMaterial = m_nNumMaterials;

		//allocate memory for more materials if needed
		if (m_nNumMaterials % ARRAY_ALLOCATION_SIZE == 0)
		{
			n = (m_nNumMaterials + ARRAY_ALLOCATION_SIZE)*sizeof(ZFXMATERIAL);
			m_pMaterials = (ZFXMATERIAL*)realloc(m_pMaterials, n);
			if (!m_pMaterials)
			{
				return ZFX_OUTOFMEMORY;
			}
		}

		memcpy(&m_pMaterials[m_nNumMaterials], &mat, sizeof(ZFXMATERIAL));
		m_nNumMaterials++;
	}

	//create the new skin and increment counter
	m_pSkins[m_nNumSkins].bAlpha = false;
	for (int i = 0; i < 8; i++)
	{
		m_pSkins[m_nNumSkins].nTexture[i] = MAX_ID; //MAX_ID is interpreted as an invalid texture ID in this engine
	}
	(*nSkinID) = m_nNumSkins;
	m_nNumSkins++;

	return ZFX_OK;
}

//add texture to a skin
HRESULT ZFXD3DSkinManager::AddTexture(UINT nSkinID, const TCHAR *chName, bool bAlpha, float fAlpha, ZFXCOLOR *cColorKeys, DWORD dwNumColorKeys)
{
	ZFXTEXTURE *pZFXTex = NULL;	//helper pointer
	HRESULT hr;
	UINT nTex, n;
	bool bTex = false;

	//is skin ID valid?
	if (nSkinID >= m_nNumSkins)
	{
		return ZFX_INVALIDID;
	}

	//do we have a free texture slot on the skin?
	if (m_pSkins[nSkinID].nTexture[7] != MAX_ID)
	{
		Log(L"error: AddTexture() failed, all 8 stages set");
		return ZFX_BUFFERSIZE;
	}

	//is this texture already loaded?
	for (nTex = 0; nTex < m_nNumTextures; nTex++)
	{
		if (wcscmp(chName, m_pTextures[nTex].chName) == 0)
		{
			bTex = true;
			break;
		}
	}

	//load new texture if necessary
	if (!bTex)
	{
		//allocate memory for more textures if needed
		if (m_nNumTextures % ARRAY_ALLOCATION_SIZE == 0)
		{
			n = (m_nNumTextures + ARRAY_ALLOCATION_SIZE)*sizeof(ZFXTEXTURE);
			m_pTextures = (ZFXTEXTURE*)realloc(m_pTextures, n);
			if (!m_pTextures)
			{
				return ZFX_OUTOFMEMORY;
			}
		}

		//do we need alpha blending?
		if (bAlpha)
		{
			m_pSkins[nSkinID].bAlpha = true;
		}
		else
		{
			m_pTextures[m_nNumTextures].fAlpha = 1.0f;
		}

		m_pTextures[m_nNumTextures].pClrKeys = NULL;

		//save texture name
		m_pTextures[m_nNumTextures].chName = new TCHAR[wcslen(chName)+1];
		memcpy(m_pTextures[m_nNumTextures].chName, chName, wcslen(chName)+1);

		//create new Direct3D texture object
		hr = CreateTexture(&m_pTextures[m_nNumTextures], bAlpha);
		if (FAILED(hr))
		{
			Log(L"error: CreateTexture() failed");
			return hr;
		}

		//add alpha channel if necessary
		if (bAlpha)
		{
			pZFXTex = &m_pTextures[m_nNumTextures];
			pZFXTex->dwNum = dwNumColorKeys;
			pZFXTex->pClrKeys = new ZFXCOLOR[dwNumColorKeys];
			memcpy(pZFXTex->pClrKeys, cColorKeys, sizeof(ZFXCOLOR)*pZFXTex->dwNum);
			LPDIRECT3DTEXTURE9 pTex = (LPDIRECT3DTEXTURE9) pZFXTex->pData;

			//color keys are added first--the SetAlphaKey function expects colors with an alpha value of unity
			for (DWORD dw = 0; dw < dwNumColorKeys; dw++)
			{
				hr = SetAlphaKey(&pTex, UCHAR(cColorKeys[dw].fR * 255), UCHAR(cColorKeys[dw].fG * 255),
								 UCHAR(cColorKeys[dw].fB * 255), UCHAR(cColorKeys[dw].fA * 255));
			}

			if (FAILED(hr))
			{
				Log(L"error: SetAlphaKey() failed");
				return hr;
			}

			//now add general transparency
			if (fAlpha < 1.0f)
			{
				m_pTextures[m_nNumTextures].fAlpha = fAlpha;
				hr = SetTransparency(&pTex, UCHAR(fAlpha*255));
				if (FAILED(hr))
				{
					Log(L"error: SetTransparency() failed");
					return hr;
				}
			}
		}

		//save id and increment counter
		nTex = m_nNumTextures;
		m_nNumTextures++;
	}
	
	//save id to first free texture slot in the skin
	for (int i = 0; i < 8; i++)
	{
		if (m_pSkins[nSkinID].nTexture[i] == MAX_ID)
		{
			m_pSkins[nSkinID].nTexture[i] = nTex;
			break;
		}
	}

	return ZFX_OK;
}

//add texture to a skin as a normal map
HRESULT ZFXD3DSkinManager::AddTextureHeightMapAsBump(UINT nSkinID, const TCHAR *chName)
{
	ZFXTEXTURE *pZFXTex = NULL;	//helper pointer
	HRESULT hr;
	UINT nTex, n;
	bool bTex = false;

	//is skin ID valid?
	if (nSkinID >= m_nNumSkins)
	{
		return ZFX_INVALIDID;
	}

	//do we have a free texture slot on the skin?
	if (m_pSkins[nSkinID].nTexture[7] != MAX_ID)
	{
		Log(L"error: AddTexture() failed, all 8 stages set");
		return ZFX_BUFFERSIZE;
	}

	//is this texture already loaded?
	for (nTex = 0; nTex < m_nNumTextures; nTex++)
	{
		if (wcscmp(chName, m_pTextures[nTex].chName) == 0)
		{
			bTex = true;
			break;
		}
	}

	//load new texture if necessary
	if (!bTex)
	{
		//allocate memory for more textures if needed
		if (m_nNumTextures % ARRAY_ALLOCATION_SIZE == 0)
		{
			n = (m_nNumTextures + ARRAY_ALLOCATION_SIZE)*sizeof(ZFXTEXTURE);
			m_pTextures = (ZFXTEXTURE*)realloc(m_pTextures, n);
			if (!m_pTextures)
			{
				return ZFX_OUTOFMEMORY;
			}
		}

		//no alpha needed
		m_pTextures[m_nNumTextures].fAlpha = 1.0f;
		m_pTextures[m_nNumTextures].pClrKeys = NULL;

		//save texture name
		m_pTextures[m_nNumTextures].chName = new TCHAR[wcslen(chName) + 1];
		memcpy(m_pTextures[m_nNumTextures].chName, chName, wcslen(chName) + 1);

		//create new Direct3D texture object
		hr = CreateTexture(&m_pTextures[m_nNumTextures], true);
		if (FAILED(hr))
		{
			Log(L"error: CreateTexture() failed");
			return hr;
		}

		//build normals from height values
		//hr = ConvertToNormalMap(&m_pTextures[m_nNumTextures]);
		if (FAILED(hr))
		{
			Log(L"error: ConvertToNormalMap() failed");
			return hr;
		}

		//save id and increment counter
		nTex = m_nNumTextures;
		m_nNumTextures++;
	}

	//save id to first free texture slot in the skin
	for (int i = 0; i < 8; i++)
	{
		if (m_pSkins[nSkinID].nTexture[i] == MAX_ID)
		{
			m_pSkins[nSkinID].nTexture[i] = nTex;
			break;
		}
	}

	return ZFX_OK;
}

//macro for downgrading from 24-bit color to 16-bit color
#define RGB16BIT(r, g, b) ((b%32) + (g%64) << 5) + ((r%32) << 11)
//loads in an image from file and maps it to a Direct3D texture and ZFXTEXTURE. Contains unused 16-bit color support as an exercise
HRESULT ZFXD3DSkinManager::CreateTexture(ZFXTEXTURE *pTexture, bool bAlpha)
{
	D3DLOCKED_RECT		d3dRect;
	D3DFORMAT			fmt;
	DIBSECTION			dibS;
	HRESULT				hr;
	int					LineWidth;
	void				*pMemory = NULL;

	//loads initial data from file to a bitmap handle
	HBITMAP hBMP = (HBITMAP)LoadImage(NULL, pTexture->chName, IMAGE_BITMAP,0,0,LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if (!hBMP)
	{
		return ZFX_FILENOTFOUND;
	}

	//gets the DIBSection struct
	GetObject(hBMP, sizeof(DIBSECTION), &dibS);

	//we support only 24-bit bitmaps
	if (dibS.dsBmih.biBitCount != 24)
	{
		DeleteObject(hBMP);
		return ZFX_INVALIDFILE;
	}

	//if(!bAlpha)
	//{
	fmt = D3DFMT_A8R8G8B8;
	/*}
	else
	{
		fmt = D3DFMT_R5G6B5;
	}
	*/

	long lWidth		= dibS.dsBmih.biWidth;
	long lHeight	= dibS.dsBmih.biHeight;
	BYTE *pBMPBits = (BYTE*)dibS.dsBm.bmBits;

	//create the shell of a Direct3D texture object
	hr = m_pDevice->CreateTexture(lWidth, lHeight, 1, 0, fmt, D3DPOOL_MANAGED, (LPDIRECT3DTEXTURE9*)(&(pTexture->pData)), NULL);

	//set dummy pointer
	LPDIRECT3DTEXTURE9 pTex = ((LPDIRECT3DTEXTURE9)pTexture->pData);

	if (FAILED(pTex->LockRect(0, &d3dRect, NULL, 0)))
	{
		return ZFX_BUFFERLOCK;
	}

	//if(bAlpha)
	//{
	LineWidth = d3dRect.Pitch >> 2; //32 bit = 4 byte
	pMemory = (DWORD*)d3dRect.pBits;
	/*}
	else
	{
		LineWidth = d3dRect.Pitch >> 1; //15 bit = 2 byte
		pMemory = (USHORT*)d3dRect.pBits;
	}
	*/

	//copy each pixel
	for (int cy = 0; cy < lHeight; cy++)
	{
		for (int cx = 0; cx < lWidth; cx++)
		{
			//if(bAlpha)
			//{
			DWORD Color = 0xff000000;
			int i = (cy*lWidth + cx)*3;
			memcpy(&Color, &pBMPBits[i], sizeof(BYTE)*3);

			((DWORD*)pMemory)[cx + (cy*LineWidth)] = Color;
			/*}
			else
			{
				//convert 24 bit to 16 bit
				UCHAR B = (pBMPBits[(xy*lWidth+cx)*3 + 0])>>3,
					  G = (pBMPBits[(xy*lWidth+cx)*3 + 1])>>3,
					  R = (pBMPBits[(xy*lWidth+cx)*3 + 2])>>3;

				//map values to 5,6,5 bit pattern and call macro
				USHORT Color = RGB16BIT( (int)(((float) R / 255.0f) * 32.0f),
										 (int)(((float) G / 255.0f) * 64.0f),
										 (int)(((float) B / 255.0f) * 32.0f) );
				
				//write pixel as 16-bit color
				((USHORT*)pMemory)[cx+(cy*LineWidth)] = Color;
			}
			*/
		}
	}

	pTex->UnlockRect(0);
	DeleteObject(hBMP);
	return ZFX_OK;
}


//D3D stores RGBA colors as a single 32-bit value. This function will combine 8-bit values to make a D3D-friendly DWORD
DWORD ZFXD3DSkinManager::MakeD3DColor(UCHAR R, UCHAR G, UCHAR B, UCHAR A)
{
	return (A << 24) | (R << 16) | (G << 8) | B;
}

//Applies single-color transparency effect (aka chroma key or green screen) to a texture
//NOTE: This function requires that color key not have any non-unity alpha values!
HRESULT ZFXD3DSkinManager::SetAlphaKey(LPDIRECT3DTEXTURE9 *ppTexture, UCHAR R, UCHAR G, UCHAR B, UCHAR A)
{
	D3DSURFACE_DESC	d3dDesc;
	D3DLOCKED_RECT	d3dRect;
	DWORD			dwKey, Color;

	//make sure we're using 32-bit ARGB format
	(*ppTexture)->GetLevelDesc(0, &d3dDesc);
	if (d3dDesc.Format != D3DFMT_A8R8G8B8)
	{
		return ZFX_INVALIDPARAM;
	}

	//color to be replaced
	dwKey = MakeD3DColor(R, G, B, 255);

	//color to replace old one with
	if (A > 0)
	{
		Color = MakeD3DColor(R, G, B, A);
	}
	else
	{
		Color = MakeD3DColor(0, 0, 0, A);
	}

	if (FAILED((*ppTexture)->LockRect(0, &d3dRect, NULL, 0)))
	{
		return ZFX_BUFFERLOCK;
	}

	//overwrite all pixels to be replaced
	for (DWORD y = 0; y < d3dDesc.Height; y++)
	{
		for (DWORD x = 0; x < d3dDesc.Width; x++)
		{
			if (((DWORD*)d3dRect.pBits)[d3dDesc.Width*y + x] == dwKey)
			{
				((DWORD*)d3dRect.pBits)[d3dDesc.Width*y+x] = Color;
			}
		}
	}

	(*ppTexture)->UnlockRect(0);
	return ZFX_OK;
}

//Sets the overall transparency of the texture
HRESULT ZFXD3DSkinManager::SetTransparency(LPDIRECT3DTEXTURE9 *ppTexture, UCHAR Alpha)
{
	D3DSURFACE_DESC	d3dDesc;
	D3DLOCKED_RECT  d3dRect;
	DWORD			Color;
	UCHAR			A, R, G, B;

	//make sure we're using 32-bit ARGB format
	(*ppTexture)->GetLevelDesc(0, &d3dDesc);
	if (d3dDesc.Format != D3DFMT_A8R8G8B8)
	{
		return ZFX_INVALIDPARAM;
	}

	if (FAILED((*ppTexture)->LockRect(0, &d3dRect, NULL, 0)))
	{
		return ZFX_BUFFERLOCK;
	}

	//loop through all pixels
	for (DWORD y = 0; y < d3dDesc.Height; y++)
	{
		for (DWORD x = 0; x < d3dDesc.Width; x++)
		{
			//get color from the pixel
			Color = ((DWORD*)d3dRect.pBits)[d3dDesc.Width*y+x];

			//calculate ARGB values
			A = (UCHAR)((Color & 0xff000000) >> 24);
			R = (UCHAR)((Color & 0x00ff0000) >> 16);
			G = (UCHAR)((Color & 0x0000ff00) >> 8);
			B = (UCHAR)((Color & 0x000000ff) >> 0);

			//set new alpha value only if new alpha is lower
			//if we're already at 100% transparency, we don't want to become less transparent by applying a new layer of 50% transparency
			if (A >= Alpha)
			{
				A = Alpha;
			}

			((DWORD*)d3dRect.pBits)[d3dDesc.Width*y + x] = MakeD3DColor(R, G, B, A);
		}
	}

	(*ppTexture)->UnlockRect(0);
	return ZFX_OK;
}


//compare two colors--are any of the RGBA values different?
inline bool ZFXD3DSkinManager::ColorEqual(const ZFXCOLOR *pCol0, const ZFXCOLOR *pCol1)
{
	if (pCol0->fA != pCol1->fA || pCol0->fR != pCol1->fR || pCol0->fG != pCol1->fG || pCol0->fB != pCol1->fB)
	{
		return false;
	}
	return true;
}

//compare two materials--are there different values for lighting colors or specular exponent?
bool ZFXD3DSkinManager::MaterialEqual(const ZFXMATERIAL *pMat0, const ZFXMATERIAL *pMat1)
{
	if (!ColorEqual(&pMat0->cAmbient, &pMat1->cAmbient) || !ColorEqual(&pMat0->cDiffuse, &pMat1->cDiffuse) ||
		!ColorEqual(&pMat0->cEmissive, &pMat1->cEmissive) || !ColorEqual(&pMat0->cSpecular, &pMat1->cSpecular) ||
		pMat0->fPower != pMat1->fPower)
	{
		return false;
	}
	return true;
}

//getter functions
ZFXSKIN ZFXD3DSkinManager::GetSkin(UINT nSkinID)
{
	if (nSkinID < m_nNumSkins)
	{
		return m_pSkins[nSkinID];
	}
	else
	{
		ZFXSKIN EmptySkin = { 0 };
		return EmptySkin;
	}
}

ZFXMATERIAL ZFXD3DSkinManager::GetMaterial(UINT nMatID)
{
	if (nMatID < m_nNumMaterials)
	{
		return m_pMaterials[nMatID];
	}
	else
	{
		ZFXMATERIAL EmptyMaterial = { 0 };
		return EmptyMaterial;
	}
}

//gets alpha and color key information as well as name
const TCHAR* ZFXD3DSkinManager::GetTextureName(UINT nTexID, float *pfAlpha, ZFXCOLOR *pAK, UCHAR *pNum)
{
	if (nTexID >= m_nNumTextures)
	{
		return NULL;
	}
	if (pfAlpha)
	{
		*pfAlpha = m_pTextures[nTexID].fAlpha;
	}
	if (pNum)
	{
		*pNum = m_pTextures[nTexID].dwNum;
	}

	if (m_pTextures[nTexID].pClrKeys && pAK)
	{
		memcpy(pAK, m_pTextures[nTexID].pClrKeys, sizeof(ZFXCOLOR)* m_pTextures[nTexID].dwNum);
	}

	return m_pTextures[nTexID].chName;
}

//write data out to log file
void ZFXD3DSkinManager::Log(TCHAR *chString, ...)
{
	TCHAR ch[256];
	va_list pArgs;

	va_start(pArgs, chString);
	vswprintf_s(ch, 256, chString, pArgs);
	fprintf(m_pLog, "ZFXD3DSkinManager");
	fwprintf(m_pLog, ch);
	fprintf(m_pLog, "\n");

	if (g_bLF)
	{
		fflush(m_pLog);
	}
}