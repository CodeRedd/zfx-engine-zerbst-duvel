//File: ZFXD3D_vcache.h 
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#pragma once

#ifndef ZFXD3D_VCACHE_H
#define ZFXD3D_VCACHE_H
#endif


#include <d3d9.h>
#include "ZFXD3D.h"
#include "ZFXD3D_skinmgr.h"
#include "ZFX.h"

class ZFXD3DVCManager;
class ZFXD3DVCache;
class ZFXD3D;

#define NUM_CACHES      10    // number of caches in manager


struct ZFXSTATICBUFFER
{
	int		nStride;
	UINT	nSkinID;
	bool	bIndic;
	int		nNumVerts;
	int		nNumIndic;
	int		nNumTris;
	DWORD	dwFVF;
	LPDIRECT3DVERTEXBUFFER9 pVB;
	LPDIRECT3DINDEXBUFFER9  pIB;
};

struct ZFXINDEXBUFFER {
	int   nNumIndis;
	int   nNumTris;
	LPDIRECT3DINDEXBUFFER9  pIB;
};

class ZFXD3DVCManager : public ZFXVertexCacheManager
{
public:
	ZFXD3DVCManager(ZFXD3DSkinManager *pSkinMan, LPDIRECT3DDEVICE9 pDevice, ZFXD3D *pZFXD3D, UINT nMaxVerts, UINT nMaxIndic, FILE *pLog);
	~ZFXD3DVCManager();

	HRESULT CreateStaticBuffer(ZFXVERTEXID VertexID, UINT nSkinID, UINT nVerts, UINT nIndic, const void *pVerts, const WORD *pIndic, UINT *pnID);

	HRESULT CreateIndexBuffer(UINT, const WORD*, UINT*);

	HRESULT Render(ZFXVERTEXID VertexID, UINT nVerts, UINT nIndic, const void *pVerts, const WORD *pIndic, UINT SkinID);

	HRESULT RenderNaked(UINT, const void*, bool);

	HRESULT Render(UINT nSBufferID);

	HRESULT Render(UINT, UINT, UINT);

	HRESULT Render(UINT, UINT, UINT, UINT, UINT);

	HRESULT RenderPoints(ZFXVERTEXID VertexID, UINT nVerts, const void *pVerts,	const ZFXCOLOR *pClr);

	HRESULT RenderLines(ZFXVERTEXID VertexID, UINT nVerts, const void *pVerts, const ZFXCOLOR *pClr, bool bStrip);

	HRESULT RenderLine(const float *fStart,	const float *fEnd, const ZFXCOLOR *pClr);

	HRESULT ForcedFlushAll();
	HRESULT ForcedFlush(ZFXVERTEXID VertexID);

	DWORD	GetActiveCache()			{ return m_dwActiveCache; }
	void	SetActiveCache(DWORD dwID)	{ m_dwActiveCache = dwID; }
	ZFXD3D* GetZFXD3D()					{ return m_pZFXD3D; }

	void    InvalidateStates(void);

	ZFXRENDERSTATE GetShadeMode(void);

private:
	ZFXD3DSkinManager  *m_pSkinMan;
	LPDIRECT3DDEVICE9	m_pDevice;
	ZFXD3D			   *m_pZFXD3D;

	ZFXSTATICBUFFER	   *m_pSB;
	ZFXINDEXBUFFER	   *m_pIB;
	UINT				m_nNumSB;
	UINT                m_nNumIB;
	ZFXD3DVCache       *m_CachePS[NUM_CACHES];    // position only
	ZFXD3DVCache       *m_CacheUU[NUM_CACHES];    // Untransformed Unlit
	ZFXD3DVCache       *m_CacheUL[NUM_CACHES];    // Untransformed Lit
	ZFXD3DVCache       *m_CacheCA[NUM_CACHES];    // character animation
	ZFXD3DVCache       *m_Cache3T[NUM_CACHES];    // three textures
	ZFXD3DVCache       *m_CacheTV[NUM_CACHES];    // uu with tanget
	DWORD				m_dwActiveCache;
	DWORD				m_dwActiveSB;
	DWORD				m_dwActiveIB;
	FILE			   *m_pLog;
};

class ZFXD3DVCache
{
public:

	ZFXD3DVCache(UINT nVertsMax, UINT nIndicMax, UINT nStride, ZFXD3DSkinManager *pSkinMan, LPDIRECT3DDEVICE9 pDevice,
		ZFXD3DVCManager *pManager, DWORD dwID, DWORD dwFVF, FILE *pLog);

	~ZFXD3DVCache();

	HRESULT Flush(bool bUseShaders);

	HRESULT Add(UINT nVerts, UINT nIndic, const void *pVerts, const WORD *pIndic, bool bUseShaders);

	void SetSkin(UINT SkinID, bool bUseShaders);
	bool UsesSkin(UINT SkinID) { return m_SkinID == SkinID; }
	bool IsEmpty()  { if (m_nNumVerts > 0){ return false; } return true; }
	int	 NumVerts() { return m_nNumVerts; }

private:
	LPDIRECT3DVERTEXBUFFER9		m_pVB;
	LPDIRECT3DINDEXBUFFER9		m_pIB;
	LPDIRECT3DDEVICE9			m_pDevice;
	ZFXD3DSkinManager			*m_pSkinMan;
	ZFXD3DVCManager				*m_pManager;
	ZFXSKIN						m_Skin;
	UINT						m_SkinID;
	DWORD						m_dwID;
	DWORD						m_dwFVF;
	FILE						*m_pLog;

	UINT	m_nNumVertsMax; //max number of vertices in the buffer
	UINT	m_nNumIndicMax; //max number of indices in the buffer
	UINT	m_nNumVerts;	//current number of vertices
	UINT	m_nNumIndic;	//current number of indices
	UINT	m_nStride;		//stride of a vertex
};