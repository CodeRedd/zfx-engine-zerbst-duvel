//File: ZFXD3D_skinmgr.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFXD3D_skinmgr.h"   // class definition

extern bool g_bLF; //determines whether to flush the log file for crash safety

//constructor
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