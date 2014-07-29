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
	HRESULT		Prepare();
	HRESULT     Update(float fTime);               
	HRESULT     Render();
	void        SetAnimation(UINT uiAnim);         
	void        SetAnimation(bool bSingle, UINT uiAnim);

protected:
	CVERTEX			*m_pVertices;		//vertices
	CVERTEX			*m_pVertices_Orig;	//non-animated vertices
	LPFACE			m_pFaces;			//faces
	LPMESH			m_pMeshes;			//meshes
	LPMATERIAL		m_pMaterials;		//materials
	LPJOINT			m_pJoints;			//joints
	LPANIMATION		m_pAnimations;		//animations
	PUINT			m_puiSkinBuffer;    // Skinbuffer
	PVOID			*m_ppIndices;       // array of arrays of indices using the same skin
	PWORD			m_pIndices;         // array of all indices sorted by skin
	PUINT			m_puiNumIndices;    // array of the number of indices per skin
	CHUNKHEAD_S		m_sHeader;			//model header
	PWCHAR			m_pcFileName;       // filename
	UINT			m_uiCurrentAnimation;                
	float			m_fTime;                             
	float			m_fStartTime;                        
	float			m_fFrame;
	bool			m_bAnimationChanged;
	bool			m_bAnimationComplete;
	bool			m_bAnimationRunOnce;
	bool			m_bRenderBones;
	bool			m_bRenderNormals;

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
	HRESULT     ReadMeshes();                    
	HRESULT     ReadJoints();                  
	HRESULT     ReadJoint_Main(LPJOINT pJoint); 
	HRESULT     ReadJoint_KeyFrame_Rot(LPJOINT pJoint);
	HRESULT     ReadJoint_KeyFrame_Pos(LPJOINT pJoint);
	HRESULT     ReadAnimations();      

	WORD		GetNextChunk( CHUNK_S &pChunk);
	void        SetScaling(float fScale = 0.0f);
	HRESULT		SetupBones();
	HRESULT     RenderBones();                
	HRESULT     RenderNormals();              
	HRESULT		Animation();
	HRESULT		AnimationPrepare();
	HRESULT		AnimationVertices();


	ZFXMatrix	CreateRotationMatrix(ZFXVector *pVector);
	
	void		LOG(UINT iLevel, bool bCR, PCHAR pcText, ...); // Log


};