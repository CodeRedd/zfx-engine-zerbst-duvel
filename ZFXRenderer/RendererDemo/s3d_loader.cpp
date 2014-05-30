//File: s3d_loader.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Included with minor changes directly from Zerbst and Duvel CD-ROM file

#include "s3d_loader.h"

ZFXModel::ZFXModel(const char *chFile, ZFXRenderDevice *pDevice, FILE *pLog) {
	m_nNumSkins = 0;
	m_nNumVertices = 0;
	m_nNumIndices = 0;
	m_pDevice = pDevice;
	m_pSkins = NULL;
	m_pIndices = NULL;
	m_pVertices = NULL;
	m_pCount = NULL;
	m_bReady = false;
	m_pBufferID = NULL;
	m_pVertices3T = NULL;
	m_pBufferID3T = NULL;

	m_pLog = pLog;
	FILE** pFile = &m_pFile;
	fopen_s(pFile, chFile, "r");

	if (pFile) {
		ReadFile();
		m_bReady = true;
	}
}

ZFXModel::~ZFXModel() {
	if (m_pIndices) {
		delete[] m_pIndices;
		m_pIndices = NULL;
	}
	if (m_pVertices) {
		delete[] m_pVertices;
		m_pVertices = NULL;
	}
	if (m_pVertices3T) {
		delete[] m_pVertices3T;
		m_pVertices3T = NULL;
	}
	if (m_pSkins) {
		delete[] m_pSkins;
		m_pSkins = NULL;
	}
	if (m_pCount) {
		delete[] m_pCount;
		m_pCount = NULL;
	}
	if (m_pBufferID) {
		delete[] m_pBufferID;
		m_pBufferID = NULL;
	}
	if (m_pBufferID3T) {
		delete[] m_pBufferID3T;
		m_pBufferID3T = NULL;
	}

	m_bReady = false;
	fclose(m_pLog);
}

HRESULT ZFXModel::Render(bool bStatic, bool b3T) {
	HRESULT  hr = ZFX_OK;
	UINT     i = 0;

	m_pDevice->SetAmbientLight(1.0f, 1.0f, 1.0f);


	if (bStatic) {
		for (i = 0; i<m_nNumSkins; i++) {
			if (b3T) {
				if (FAILED(m_pDevice->GetVertexCacheManager()->Render(m_pBufferID3T[i])))
					hr = ZFX_FAIL;
			}
			else {
				if (FAILED(m_pDevice->GetVertexCacheManager()->Render(m_pBufferID[i])))
					hr = ZFX_FAIL;
			}
		} // for
	}
	else {
		WORD *pIndices = m_pIndices;
		for (i = 0; i<m_nNumSkins; pIndices += m_pCount[i], i++) {
			if (b3T) {
				if (FAILED(m_pDevice->GetVertexCacheManager()->Render(
					VID_3T,
					m_nNumVertices,
					m_pCount[i],
					m_pVertices3T,
					pIndices,
					m_pSkins[i])))
					hr = ZFX_FAIL;
			}
			else {
				if (FAILED(m_pDevice->GetVertexCacheManager()->Render(
					VID_UU,
					m_nNumVertices,
					m_pCount[i],
					m_pVertices,
					pIndices,
					m_pSkins[i])))
					hr = ZFX_FAIL;
			}
		} // for
	} // else
	return hr;
}

void ZFXModel::ReadFile(void)
{
	ZFXCOLOR cA, cD, cE, cS;
	UINT     i, nNumFaces, nNumTris = 0;
	float    fPower = 0;
	
	TCHAR	  Line[80];
	TCHAR     Texture[80];
	TRI     *pTris = NULL;


	// READ SKINS
	SEEK(Line, L"BEGIN_SKINS");
	NEXT(Line);
	swscanf_s(Line, L"%d;", &m_nNumSkins);
	m_pSkins = new UINT[m_nNumSkins];
	for (i = 0; i<m_nNumSkins; i++)
	{
		NEXT(Line);  NEXT(Line);// skip opening brag
		swscanf_s(Line, L"%f, %f, %f, %f;", &cA.fR, &cA.fG, &cA.fB, &cA.fA);           NEXT(Line);
		swscanf_s(Line, L"%f,%f,%f,%f;", &cD.fR, &cD.fG, &cD.fB, &cD.fA);              NEXT(Line);
		swscanf_s(Line, L"%f,%f,%f,%f;", &cE.fR, &cE.fG, &cE.fB, &cE.fA);              NEXT(Line);
		swscanf_s(Line, L"%f,%f,%f,%f,%f;", &cS.fR, &cS.fG, &cS.fB, &cS.fA, &fPower);  NEXT(Line);
		//swscanf_s(Line, L"%s", &Texture);

		// add skin to skin-manager
		m_pDevice->GetSkinManager()->AddSkin(&cA, &cD, &cE, &cS, fPower, &m_pSkins[i]);
		//m_pDevice->GetSkinManager()->AddTexture(m_pSkins[i], Texture, false, 0, NULL, 0); //BUG: Causing Access Violations
		NEXT(Line); // skip closing brag
	}


	// READ VERTICES
	rewind(m_pFile);
	SEEK(Line, L"BEGIN_VERTICES");
	NEXT(Line);
	swscanf_s(Line, L"%d;", &m_nNumVertices);
	m_pVertices = new VERTEX[m_nNumVertices];
	m_pVertices3T = new VERTEX3T[m_nNumVertices];
	for (i = 0; i<m_nNumVertices; i++)
	{
		NEXT(Line);
		swscanf_s(Line, L"%f,%f,%f,%f,%f;", &m_pVertices[i].x,
			&m_pVertices[i].y, &m_pVertices[i].z,
			&m_pVertices[i].tu, &m_pVertices[i].tv);

		memset(&m_pVertices3T[i], 0, sizeof(VERTEX3T));
		m_pVertices3T[i].x = m_pVertices[i].x;
		m_pVertices3T[i].y = m_pVertices[i].y;
		m_pVertices3T[i].z = m_pVertices[i].z;
		m_pVertices3T[i].tu0 = m_pVertices[i].tu;
		m_pVertices3T[i].tv0 = m_pVertices[i].tv;
	}


	// READ FACES (n-sided polys) TO COUNT TRIS NEEDED
	rewind(m_pFile);
	SEEK(Line, L"BEGIN_FACES");
	NEXT(Line);
	swscanf_s(Line, L"%d;", &nNumFaces);
	pTris = new TRI[nNumFaces];
	for (i = 0; i<nNumFaces; i++)
	{
		NEXT(Line);
		swscanf_s(Line, L"%d,%d,%d;%d", &pTris[nNumTris].i0,
			&pTris[nNumTris].i1, &pTris[nNumTris].i2,
			&pTris[nNumTris].nMat);
		nNumTris++;
	}

	// sort tris by material
	qsort((void*)pTris, nNumTris, sizeof(TRI), (CMPFUNC)SortTris);



	// COUNT INDICES FOR EACH MATERIAL
	UINT nOldMat = pTris[0].nMat;
	m_pCount = new UINT[m_nNumSkins];
	m_pBufferID = new UINT[m_nNumSkins];
	m_pBufferID3T = new UINT[m_nNumSkins];
	memset(m_pCount, 0, sizeof(UINT)*m_nNumSkins);
	memset(m_pBufferID, 0, sizeof(UINT)*m_nNumSkins);
	memset(m_pBufferID3T, 0, sizeof(UINT)*m_nNumSkins);
	m_pIndices = new WORD[nNumTris * 3];
	m_nNumIndices = nNumFaces * 3;
	for (i = 0; i<nNumTris; i++)
	{
		// copy indices in indexlist
		m_pIndices[i * 3] = pTris[i].i0;
		m_pIndices[i * 3 + 1] = pTris[i].i1;
		m_pIndices[i * 3 + 2] = pTris[i].i2;

		// count indices per material
		if (pTris[i].nMat != nOldMat)
			nOldMat = pTris[i].nMat;
		else
			m_pCount[pTris[i].nMat] += 3;
	}


	WORD *pIndices = m_pIndices;
	for (i = 0; i<m_nNumSkins; pIndices += m_pCount[i], i++) {

		if (FAILED(m_pDevice->GetVertexCacheManager()->CreateStaticBuffer(
			VID_UU,
			m_pSkins[i],
			m_nNumVertices,
			m_pCount[i],
			m_pVertices,
			pIndices,
			&m_pBufferID[i])))
		{
			fprintf(m_pLog, "CREATE STATIC FAILED\n");
		}

		if (FAILED(m_pDevice->GetVertexCacheManager()->CreateStaticBuffer(
			VID_3T,
			m_pSkins[i],
			m_nNumVertices,
			m_pCount[i],
			m_pVertices3T,
			pIndices,
			&m_pBufferID3T[i])))
		{
			fprintf(m_pLog, "CREATE STATIC 3T FAILED\n");
		}
	}
	delete[] pTris;
}

int SortTris(const TRI *arg1, const TRI *arg2) {
	if (arg1->nMat > arg2->nMat)
	{
		return 1;
	}
	else if (arg1->nMat < arg2->nMat)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int instr(const TCHAR *string, const TCHAR *substring) {
	TCHAR a, c;
	int  nStart;
	int j;
	int  nLng_SubStr = wcslen(substring),
		nLng_Str = wcslen(string);

	if ((nLng_SubStr <= 1) || (nLng_SubStr > nLng_Str))
	{
		return -1;
	}

	memcpy(&a, &substring[0], sizeof(TCHAR));
	nStart = wcscspn(string, &a);

	while (nStart < nLng_Str) 
	{
		if (string[nStart] != a) 
		{
			nStart++;
			continue;
		}
		for (j = 1; j<nLng_SubStr; j++) 
		{
			memcpy(&c, &substring[j], sizeof(TCHAR));
			if (string[nStart + j] != c) 
			{
				break;
			}
		} // for 
		if (j == nLng_SubStr)
		{
			return nStart;
		}
		else
		{
			nStart++;
		}
	} // while
	return -1;
}