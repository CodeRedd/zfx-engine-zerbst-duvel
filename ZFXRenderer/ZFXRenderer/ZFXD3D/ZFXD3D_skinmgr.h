//File: ZFXD3D_skinmgr.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include <d3d9.h>
#include "../ZFXRenderDevice.h"  // material manager base class

#define MAX_ID 65535

class ZFXD3DSkinManager : public ZFXSkinManager
{
	friend class ZFXD3DVCache;

public:
	
	ZFXD3DSkinManager(LPDIRECT3DDEVICE9 pDevice, FILE *pLog);
	~ZFXD3DSkinManager();

	HRESULT AddSkin(const ZFXCOLOR *pcAmbient, const ZFXCOLOR *pcDiffuse, const ZFXCOLOR *pcEmissive,
					const ZFXCOLOR *pcSpecular, float fSpecPower, UINT *nSkinID);

	HRESULT AddTexture(UINT nSkinID, const TCHAR *chName, bool bAlpha, float fAlpha,
					ZFXCOLOR *cColorKeys, DWORD dwNumColorKeys);

	HRESULT AddTextureHeightMapAsBump(UINT nSkinID, const TCHAR *chName);

	bool MaterialEqual(const ZFXMATERIAL *pMat0, const ZFXMATERIAL *pMat1);

	ZFXSKIN		 GetSkin(UINT nSkinID);

	ZFXMATERIAL	 GetMaterial(UINT nMatID);

	const TCHAR* GetTextureName(UINT nTexID, float *pfAlpha, ZFXCOLOR *pAK, UCHAR *pNum);

	void LogCurrentStatus(TCHAR *chLog, bool bDetail);

protected:
	LPDIRECT3DDEVICE9  m_pDevice;
	FILE			   *m_pLog;

	inline	bool ColorEqual(const ZFXCOLOR *pCol0, const ZFXCOLOR *pCol1);

	HRESULT CreateTexture(ZFXTEXTURE *pTexture, bool bAlpha);

	HRESULT SetAlphaKey( LPDIRECT3DTEXTURE9 *ppTexture, UCHAR R, UCHAR G, UCHAR B, UCHAR A );

	HRESULT SetTransparency(LPDIRECT3DTEXTURE9 *ppTexture, UCHAR Alpha);

	DWORD	MakeD3DColor(UCHAR R, UCHAR G, UCHAR B, UCHAR A);

	void	Log(TCHAR *, ...);
};