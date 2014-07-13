//File: ZFXModel.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#pragma once

#include <Windows.h>
#include <cstdio>
#include <ZFXRenderDevice.h>
#include <ZFX.h>

#include "CModelStructs.h"

class CZFXModel
{
public: 

	UINT		m_uiLogLevel;       // Loglevel
	bool		m_bLog;				//should we log?
	char		m_cLogFileName[32]; // Logfilename

	void        Init();


protected:
	CVERTEX			*m_pVertices;		//vertices
	CVERTEX			*m_pVertices_Orig;	//vertices
	LPFACE			m_pFaces;			//faces
	LPMESH			m_pMeshes;			//meshes
	LPMATERIAL		m_pMaterials;		//materials
	LPJOINT			m_pJoints;			//joints
	LPANIMATION		m_pAnimations;		//animations
	PUINT			m_puiSkinBuffer;    // Skinbuffer
	PVOID			*m_ppIndices;       // Indices
	PWORD			m_pIndices;         // Indices
	PUINT			m_puiNumIndices;    // Number of Indices
	CHUNKHEAD_S		m_sHeader;			//model header

	FILE			*m_pFile;			//file
	CHUNK_S			m_sChunk;           //chunk
	VERTEX			m_sBBoxMin;                         
	VERTEX			m_sBBoxMax;
	ZFXAABB			m_sAABB;

	ZFXRenderDevice *m_pRenderDevice;   // Renderdevice


	HRESULT		CheckForChunks();
	HRESULT     ReadHeader();					
	HRESULT     ReadVertices();                
	HRESULT     ReadMaterials();               
	HRESULT     ReadFaces();                   
	HRESULT     ReadMesh();                    
	HRESULT     ReadJoints();                  
	HRESULT     ReadJoint_Main(LPJOINT pJoint); 
	HRESULT     ReadJoint_KeyFrame_Rot(LPJOINT pJoint);
	HRESULT     ReadJoint_KeyFrame_Pos(LPJOINT pJoint);
	HRESULT     ReadAnimations();              
	WORD		GetNextChunk( CHUNK_S &pChunk);
	void        SetScaling(float fScale = 0.0f);



	void		LOG(UINT iLevel, bool bCR, PCHAR pcText, ...); // Log


};