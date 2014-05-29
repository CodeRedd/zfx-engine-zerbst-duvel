//File: s3d_loader.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#pragma once

#include <Windows.h>
#include <ZFXRenderDevice.h>

struct TRI {
	WORD i0, i1, i2;  // original indices
	WORD n0, n1, n2;  // new indices
	UINT nMat;
};

typedef int(*CMPFUNC) (const void *arg1, const void *arg2);
int SortTris(const TRI *arg1, const TRI *arg2);

class ZFXModel
{
protected:
	ZFXRenderDevice *m_pDevice;

	UINT		m_nNumSkins;
	UINT		*m_pSkins;	
	UINT		m_nNumVertices;
	VERTEX		*m_pVertices;
	VERTEX3T	*m_pVertices3T;
	UINT		*m_pBufferID3T;
	UINT		m_nNumIndices;
	WORD		*m_pIndices;
	UINT		*m_pCount; //indices per material
	UINT		*m_pBufferID; //static buffer
	FILE		*m_pFile;
	FILE		*m_pLog;
	bool		m_bReady;

	void ReadFile();

public:
	ZFXModel(const char *chFile, ZFXRenderDevice *pDevice, FILE *pLog);
	~ZFXModel();

	HRESULT Render(bool bStatic, bool b3T);

};
#define SEEK(str, key) while (instr(str,key)==-1){ fgetws(str,80,m_pFile);}
#define NEXT(str) fgetws(str,80, m_pFile);
int instr(const TCHAR *string, const TCHAR *substring);

