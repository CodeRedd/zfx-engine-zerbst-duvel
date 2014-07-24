//File: CZFXModel.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "CZFXModel.h"

void CZFXModel::Init()
{
	// Settings
	m_pFile = NULL;								// File
	m_pVertices = NULL;							// Vertices
	m_pVertices_Orig = NULL;					// Vertices
	m_pFaces = NULL;							// Faces
	m_pMeshes = NULL;							// Meshes
	m_pMaterials = NULL;						// Materials
	m_ppIndices = NULL;							// Indices
	m_pIndices = NULL;							// Indices
	m_pAnimations = NULL;						// Animations
	m_pRenderDevice = NULL;						// Renderdevice
	m_uiStartFaceID = 0;						// Offset Faces
	m_uiStartVertexID = 0;						// Offset Vertices
	strcpy(m_cLogFileName, "MODELLOG.TXT");		// Logfilename
	m_bLog = true;								// Logging on
	m_uiLogLevel = 10;							// Loglevel
	m_fTime = 0.0;								// Time
	m_fStartTime = -1.0f;						// Time
	m_uiCurrentAnimation = 8;					// Animation   
	m_bAnimationComplete = false;				// Animationcompleted
	m_bAnimationRunOnce = true;					// Singleanimation
	m_bRenderBones = false;						// Renderbones
	m_bRenderNormals = true;					// Normalvectoren
}

HRESULT CZFXModel::CheckForChunks()
{
	bool bLoop = true;

	//loop until end chunk is found
	do
	{
		//seek the next chunk
		switch (GetNextChunk(m_sChunk))
		{
		case V1_HEADER:		ReadHeader();		break;
		case V1_VERTEX:		ReadVertices();		break;
		case V1_FACE:		ReadFaces();		break;
		case V1_MESH:		ReadMeshes();		break;
		case V1_MATERIAL:	ReadMaterials();	break;
		case V1_JOINT:		ReadJoints();		break;
		case V1_ANIMATION:	ReadAnimations();	break;
		case V1_END:		bLoop = false;		break;
		default: break;
		}
	}
	while ( bLoop );

	//are there animations?
	if (m_sHeader.uiNumJoints == 0)
	{
		//if not, we don't need this member
		delete [] m_pVertices_Orig;
		m_pVertices_Orig = NULL;
	}

	return S_OK;
}

//loads data into the ZFXEngine buffers and skins
//optimization: sort faces by the material they are using
HRESULT CZFXModel::Prepare()
{
	//Init variables
	ULONG		ulNumIndices = 0;
	ULONG		ulNumVertices = 0;
	UINT		uiCurrentMat = 0;
	PWORD		pIndex = NULL; //temp array for copying indices
	ULONG		ulCounter = 0;
	LPMATERIAL  pMaterial = NULL;
	TCHAR		cTexture[256] = { 0 };
	PWCHAR		pcSeparator = NULL;
	ULONG		ulIndexCount = 0;

	//Set up the animation bones
	SetupBones();

	//sort indices by material they are using
	LOG(20, false, "Sort Indices By Material [%d]", m_sHeader.uiNumMaterials);

	//calculate max memory needed
	m_sHeader.ulNumIndices = m_sHeader.ulNumFaces * 3;
	pIndex = new WORD[ m_sHeader.ulNumIndices];

	m_ppIndices = new PVOID[m_sHeader.uiNumMaterials];
	ZeroMemory(m_ppIndices, sizeof(PVOID) * m_sHeader.uiNumMaterials);

	m_pIndices = new WORD[m_sHeader.ulNumIndices];
	ZeroMemory(m_pIndices, sizeof(WORD) * m_sHeader.ulNumIndices);

	m_puiNumIndices = new UINT[m_sHeader.uiNumMaterials];
	ZeroMemory(m_puiNumIndices, sizeof(UINT) * m_sHeader.uiNumMaterials);

	m_puiSkinBuffer = new UINT[m_sHeader.uiNumMaterials];
	ZeroMemory(m_puiSkinBuffer, sizeof(UINT) * m_sHeader.uiNumMaterials);

	//sort all faces into the index array
	do
	{
		ZeroMemory(pIndex, sizeof(WORD) * m_sHeader.ulNumIndices);

		//reset counter
		ulNumIndices = 0;

		//loop through all faces
		for (ulCounter = 0; ulCounter < m_sHeader.ulNumFaces; ulCounter++)
		{
			//still the same material?
			if (m_pFaces[ulCounter].uiMaterialID == uiCurrentMat)
			{
				m_pIndices[ulIndexCount++] = pIndex[ulNumIndices++] = (WORD)m_pFaces[ulCounter].ulIndices[0];
				m_pIndices[ulIndexCount++] = pIndex[ulNumIndices++] = (WORD)m_pFaces[ulCounter].ulIndices[1];
				m_pIndices[ulIndexCount++] = pIndex[ulNumIndices++] = (WORD)m_pFaces[ulCounter].ulIndices[2];
			}
		}

		//do we not have any indices for this material?
		if (!ulNumIndices)
		{
			//new material
			uiCurrentMat++;
			LOG(1, true, "STOP Error: Not Enough Indices...");
			continue;
		}

		m_puiNumIndices[uiCurrentMat] = ulNumIndices;
		m_ppIndices[uiCurrentMat] = new WORD[ulNumIndices];
		memcpy(m_ppIndices[uiCurrentMat], pIndex, sizeof(WORD) * ulNumIndices);

		//set current material
		pMaterial = &m_pMaterials[uiCurrentMat];

		//read material
		if (FAILED(m_pRenderDevice->GetSkinManager()->
			AddSkin((ZFXCOLOR*)&pMaterial->fAmbient, (ZFXCOLOR*)&pMaterial->fDiffuse, (ZFXCOLOR*)&pMaterial->fEmissive,
			(ZFXCOLOR*)&pMaterial->fSpecular, pMaterial->fSpecularPower, &m_puiSkinBuffer[uiCurrentMat])))
		{
			LOG(1, true, "FAILED [LOAD SKIN %d]", uiCurrentMat);
		}

		//prepare textures
		ZeroMemory(cTexture, sizeof(CHAR) * 256);
		pcSeparator = wcschr(wcsrev(wcsdup(m_pcFileName)), '/');

		if (!pcSeparator)
		{
			pcSeparator = wcschr(wcsrev(wcsdup(m_pcFileName)), 92);
		}

		if (pcSeparator)
		{
			wcscpy(cTexture, wcsrev(pcSeparator));
		}
		wcscat( cTexture, pMaterial->cTexture_1);

		//load textures
		if (FAILED(m_pRenderDevice->GetSkinManager()->AddTexture(m_puiSkinBuffer[uiCurrentMat], cTexture, false, 0, NULL, 0)))
		{
			LOG(1, true, "FAILED [LOAD TEXTURE %d]", pMaterial->cTexture_1);
		}

		//next material
		uiCurrentMat++;
	}
	while (uiCurrentMat != m_sHeader.uiNumMaterials);

	//cleanup
	delete[] pIndex;

	LOG(20, true, " done");
	return S_OK;
}

HRESULT CZFXModel::Animation()
{
	float		fElapsed = -1.0f;
	float		fStart = -1.0f;		//start of pose
	float		fEnd = -1.0f;		//end of pose
	LPANIMATION pAnimation = NULL;

	//does an animation exist at all?
	if (m_sHeader.uiNumJoints == 0)
	{
		return S_OK;
	}

	//run only once?
	if (m_bAnimationRunOnce && m_bAnimationComplete && !m_bAnimationChanged)
	{
		return S_OK;
	}

	//check time
	m_fTime = (float)GetTickCount();

	//if new then this is the new start time
	if (m_fStartTime == -1.0f)
	{
		m_fStartTime = m_fTime;
	}

	//calculate elapsed time
	fElapsed = m_fTime - m_fStartTime;

	//get current animation
	pAnimation = &m_pAnimations[m_uiCurrentAnimation];
	fStart = pAnimation->fStartFrame;
	fEnd = pAnimation->fEndFrame;

	//calculate frame position
	m_fFrame = fStart + (m_sHeader.fAnimationFPS / 2048) * fElapsed;

	//set new start frame if needed
	if (m_fFrame <= fStart)
	{
		m_fFrame = fStart;
	}

	//is it the end of the animation?
	if (m_fFrame >= fEnd)
	{
		m_fStartTime = m_fTime;
		m_fFrame = fStart;
		m_bAnimationComplete = true;
	}
	else
	{
		//prepare animation
		AnimationPrepare();

		//calculate vertex positions
		AnimationVertices();

		m_bAnimationComplete = false;
		m_bAnimationChanged = false;
	}
	return S_OK;
}

HRESULT CZFXModel::AnimationPrepare()
{
	//init variables
	LPJOINT		pJoint = NULL;
	ULONG		ulCounter = 0;
	UINT		uiLoop = 0;
	ZFXVector	sPosition;
	ZFXVector	sRotation;
	UINT		uiKeyPos = 0;
	UINT		uiKeyRot = 0;
	LPKF_ROT	pLastRot = NULL;	//last frame
	LPKF_ROT	pThisRot = NULL;	//next frame
	LPKF_ROT	pKeyRot = NULL;
	LPKF_POS	pLastPos = NULL;	//last frame
	LPKF_POS	pThisPos = NULL;	//next frame
	LPKF_POS	pKeyPos = NULL;
	float		fScale = 0.0f;
	ZFXMatrix	matTemp;
	ZFXMatrix	matFinal;

	matTemp.Identity();
	matFinal.Identity();

	//clip the animation
	if (m_fFrame > m_sHeader.uiNumFrames)
	{
		m_fFrame = 0;
	}

	//calculate absolute transform matrices
	for (ulCounter = 0; ulCounter < m_sHeader.uiNumJoints; ulCounter++)
	{
		//get current joint
		pJoint = &m_pJoints[ulCounter];

		//get data
		uiKeyPos = pJoint->wNumKF_Position;
		uiKeyRot = pJoint->wNumKF_Rotation;

		//recalculation necessary?
		if ((uiKeyRot + uiKeyPos) != 0)
		{
			//we need a new position or rotation
			pLastPos = NULL;
			pThisPos = NULL;
			pKeyPos = NULL;

			for (uiLoop = 0; uiLoop < uiKeyPos; uiLoop++)
			{
				//get current position
				pKeyPos = &pJoint->pKF_Position[uiLoop];

				//check time
				if (pKeyPos->fTime >= m_fFrame)
				{
					pThisPos = pKeyPos;
					break;
				}
				//we're not there yet
				pLastPos = pKeyPos;
			}

			//interpolate positions
			if (pLastPos && pThisPos)
			{
				//calculate scaling
				fScale = (m_fFrame - pLastPos->fTime) / (pThisPos->fTime - pLastPos->fTime);

				//interpolation
				sPosition = pLastPos->vPosition + (pThisPos->vPosition - pLastPos->vPosition) * fScale;
			}
			else if (!pLastPos)
			{
				//copy the position
				sPosition = pThisPos->vPosition;
			}
			else
			{
				sPosition = pLastPos->vPosition;
			}

			//apply rotation
			pLastRot = NULL;
			pThisRot = NULL;
			pKeyRot = NULL;

			for (uiLoop = 0; uiLoop < uiKeyRot; uiLoop++)
			{
				//get current rotation
				pKeyRot = &pJoint->pKF_Rotation[uiLoop];

				//check time
				if (pKeyRot->fTime >= m_fFrame)
				{
					pThisRot = pKeyRot;
					break;
				}
				//we're not there yet
				pLastRot = pKeyRot;
			}

			//interpolate rotations
			if (pLastRot && pThisRot)
			{
				sRotation = pLastRot->vRotation + (pThisRot->vRotation - pLastRot->vRotation) * fScale;
			}
			else if (!pLastPos)
			{
				//copy the position
				sRotation = pThisRot->vRotation;
			}
			else
			{
				sRotation = pLastRot->vRotation;
			}

			//joint matrix setup
			matTemp.SetTranslation( sPosition );
			matTemp.Rota( sRotation );

			//calculate relative matrix
			matFinal = matTemp * pJoint->sMatrix_relative;

			//is there a parent?
			if (pJoint->wParentID != 255)
			{
				//account for positioning of parent
				pJoint->sMatrix = matFinal * m_pJoints[pJoint->wParentID].sMatrix;
			}
			else
			{
				pJoint->sMatrix = matFinal;
			}
		}
		else
		{
			//no new matrix, so copy the old one
			pJoint->sMatrix = pJoint->sMatrix_relative;
		}
	}
	return S_OK;
}

HRESULT CZFXModel::AnimationVertices()
{
	//init variables
	ULONG		ulCounter = 0;
	CVERTEX		*pVertex = NULL;
	CVERTEX		*pVertex_Orig = NULL;
	ZFXVector	sVector;

	//reset bounding box
	m_sBBoxMin.x = 999999.0f;
	m_sBBoxMin.y = 999999.0f;
	m_sBBoxMin.z = 999999.0f;
	m_sBBoxMax.x = -999999.0f;
	m_sBBoxMax.y = -999999.0f;
	m_sBBoxMax.z = -999999.0f;

	//recalculate the vertices
	for (ulCounter = 0; ulCounter < m_sHeader.ulNumVertices; ulCounter++)
	{
		//get current vertex
		pVertex = &m_pVertices[ulCounter];
		pVertex_Orig = &m_pVertices_Orig[ulCounter];

		//do we have a bone?
		if (pVertex->fBone1 != 255.0f)
		{
			//1. get original (non-animated) vertex
			sVector.x = pVertex_Orig->x;
			sVector.y = pVertex_Orig->y;
			sVector.z = pVertex_Orig->z;

			//2. rotate the vertex
			sVector.RotateWith(m_pJoints[(UINT)pVertex_Orig->fBone1].sMatrix);

			//3. get position
			sVector += m_pJoints[(UINT)pVertex_Orig->fBone1].sMatrix.GetTranslation();

			//4. calculate new position
			pVertex->x = sVector.x;
			pVertex->y = sVector.y;
			pVertex->z = sVector.z;

			//5. animate the normals
			sVector.x = pVertex_Orig->vcN[0];
			sVector.y = pVertex_Orig->vcN[1];
			sVector.z = pVertex_Orig->vcN[2];
			sVector.RotateWith(m_pJoints[(UINT)pVertex_Orig->fBone1].sMatrix);
			pVertex->vcN[0] = sVector.x;
			pVertex->vcN[1] = sVector.y;
			pVertex->vcN[2] = sVector.z;

			//6. calculate bounding box
			m_sBBoxMax.x = max(m_sBBoxMax.x, pVertex->x);
			m_sBBoxMax.y = max(m_sBBoxMax.y, pVertex->y);
			m_sBBoxMax.z = max(m_sBBoxMax.z, pVertex->z);
			m_sBBoxMin.x = min(m_sBBoxMin.x, pVertex->x);
			m_sBBoxMin.y = min(m_sBBoxMin.y, pVertex->y);
			m_sBBoxMin.z = min(m_sBBoxMin.z, pVertex->z);
		}
	}

	//7. create AABB
	m_sAABB.vcMin.x = m_sBBoxMin.x;
	m_sAABB.vcMin.y = m_sBBoxMin.y;
	m_sAABB.vcMin.z = m_sBBoxMin.z;
	m_sAABB.vcMax.x = m_sBBoxMax.x;
	m_sAABB.vcMax.y = m_sBBoxMax.y;
	m_sAABB.vcMax.z = m_sBBoxMax.z;
	m_sAABB.vcCenter.x = (m_sBBoxMax.x - m_sBBoxMin.x) / 2;
	m_sAABB.vcCenter.y = (m_sBBoxMax.y - m_sBBoxMin.y) / 2;
	m_sAABB.vcCenter.z = (m_sBBoxMax.z - m_sBBoxMin.z) / 2;
	
	return S_OK;
}

////////////////////////////////////////////////////////////////

HRESULT CZFXModel::ReadHeader()
{
	//write to log
	LOG(20, false, "Reading Header...");

	//clear memory
	ZeroMemory(&m_sHeader, sizeof(CHUNKHEAD_S));

	//read the header
	fread(&m_sHeader, sizeof(CHUNKHEAD_S), 1, m_pFile);

	//seek to the end chunk to make sure it's there
	if (GetNextChunk(m_sChunk) == V1_END)
	{
		LOG(20, true, " OK");
		return S_OK;
	}

	LOG(1, true, " FAILED [Header]");
	return E_FAIL;
}

HRESULT CZFXModel::ReadVertices()
{
	//initialize variables
	ULONG	ulNumVertices = m_sHeader.ulNumVertices;
	LPVERTEX_3F pVertices = NULL;

	LOG(20, false, "Read Vertices [%d]", ulNumVertices);

	//allocate memory
	pVertices = new VERTEX_3F_S[ ulNumVertices ];
	if (!pVertices)
	{
		LOG(1, true, "FAILED [VERTICES]");
		return E_FAIL;
	}

	//read all vertices
	fread( pVertices, sizeof(VERTEX_3F_S), ulNumVertices, m_pFile);

	//allocate memory
	m_pVertices = new CVERTEX[ ulNumVertices ];
	m_pVertices_Orig = new CVERTEX[ ulNumVertices ];
	ZeroMemory(m_pVertices, sizeof(CVERTEX)*ulNumVertices);
	ZeroMemory(m_pVertices_Orig, sizeof(CVERTEX)*ulNumVertices);

	//convert vertices to CVERTEX
	for (ULONG ulCounter = 0; ulCounter < m_sHeader.ulNumVertices; ulCounter++)
	{
		memcpy(&m_pVertices[ulCounter].x, &pVertices[ulCounter].fXYZ, sizeof(float) * 3);
		memcpy(&m_pVertices[ulCounter].vcN, &pVertices[ulCounter].fNormal, sizeof(float)* 3);
		memcpy(&m_pVertices[ulCounter].tu, &pVertices[ulCounter].fUV0, sizeof(float)* 2);

		m_pVertices[ulCounter].fBone1 = (float)pVertices[ulCounter].uiBoneID_A;
		m_pVertices[ulCounter].fWeight1 = (float)pVertices[ulCounter].fWeight_A;
		m_pVertices[ulCounter].fBone2 = (float)pVertices[ulCounter].uiBoneID_B;
		m_pVertices[ulCounter].fWeight2 = (float)pVertices[ulCounter].fWeight_B;
	}

	//free memory
	delete [] pVertices;

	//search the end chunk
	if (GetNextChunk(m_sChunk) == V1_END)
	{
		LOG(20, true, " OK");
		return S_OK;
	}

	LOG(1, true, " FAILED [VERTICES]");
	return E_FAIL;
}

HRESULT CZFXModel::ReadFaces()
{
	ULONG	ulNumFaces = m_sHeader.ulNumFaces;

	LOG(20, false, "Reading Faces [%d]", ulNumFaces);

	//allocate memory
	m_pFaces = new FACE_S[ulNumFaces];
	if (!m_pFaces)
	{
		LOG(1, true, " FAILED [FACES]");
		return E_FAIL;
	}

	//read all faces
	fread(m_pFaces, sizeof(FACE_S), ulNumFaces, m_pFile);
	
	//look for end chunk
	if (GetNextChunk(m_sChunk) == V1_END)
	{
		LOG(20, true, " OK");
		return S_OK;
	}

	LOG(1, true, " FAILED [FACES]");
	return E_FAIL;
}

HRESULT CZFXModel::ReadMeshes()
{
	ULONG	ulNumMeshes = m_sHeader.ulNumMeshes;

	LOG(20, false, "Reading Meshes [%d]", ulNumMeshes);

	//allocate memory
	m_pMeshes = new MESH_S[ulNumMeshes];
	if (!m_pFaces)
	{
		LOG(1, true, " FAILED [MESHES]");
		return E_FAIL;
	}

	//read all meshes
	fread(m_pMeshes, sizeof(MESH_S), ulNumMeshes, m_pFile);

	//look for end chunk
	if (GetNextChunk(m_sChunk) == V1_END)
	{
		LOG(20, true, " OK");
		return S_OK;
	}

	LOG(1, true, " FAILED [MESHES]");
	return E_FAIL;
}

HRESULT CZFXModel::ReadMaterials()
{
	UINT	uiNumMat = m_sHeader.uiNumMaterials;

	LOG(20, false, "Reading Materials [%d]", uiNumMat);

	//allocate memory
	m_pMaterials = new MATERIAL_S[uiNumMat];
	if (!m_pMaterials)
	{
		LOG(1, true, " FAILED [MATERIALS]");
		return E_FAIL;
	}

	//read all materials
	fread(m_pMaterials, sizeof(MATERIAL_S), uiNumMat, m_pFile);

	//look for end chunk
	if (GetNextChunk(m_sChunk) == V1_END)
	{
		LOG(20, true, " OK");
		return S_OK;
	}

	LOG(1, true, " FAILED [MATERIALS]");
	return E_FAIL;
}

HRESULT CZFXModel::ReadJoints()
{
	bool	bLoop = true;
	UINT	uiLoop = 0;
	UINT	uiNumJoints = m_sHeader.uiNumJoints;
	LPJOINT pJoint = NULL;

	LOG(20, false, "Reading Joints [%d]...", uiNumJoints);

	//allocate memory
	m_pJoints = new JOINT_S[uiNumJoints];
	if (!m_pJoints)
	{
		LOG(1, true, " FAILED [JOINTS]");
		return E_FAIL;
	}

	//loop until end chunk found
	do
	{
		//find the next chunk
		switch (GetNextChunk(m_sChunk))
		{
		case V1_JOINT_MAIN:
			pJoint = &m_pJoints[uiLoop];
			ReadJoint_Main(pJoint);
			uiLoop++;
			break;
		case V1_JOINT_KEYFRAME_ROT:
			ReadJoint_KeyFrame_Rot( pJoint );
			break;
		case V1_JOINT_KEYFRAME_POS:
			ReadJoint_KeyFrame_Pos( pJoint );
			break;
		case V1_END:
			bLoop = false;
			break;
		}
	}
	while (bLoop);

	//we've already checked for end chunk, so make sure we broke out of the loop correctly
	if (!bLoop)
	{
		LOG(20, true, " OK");
		return S_OK;
	}

	LOG(1, true, " FAILED[JOINTS]");
	return E_FAIL;
}

HRESULT CZFXModel::ReadJoint_Main(LPJOINT pJoint)
{
	//log start
	LOG(20, false, "Reading Main Joint ");

	//read joints
	fread(pJoint, sizeof(JOINT_S), 1, m_pFile);

	//check for end chunk
	if (GetNextChunk(m_sChunk) == V1_END)
	{
		LOG(20, true, " OK");
		return S_OK;
	}

	LOG(1, true, " FAILED [JOINT_MAIN]");
	return E_FAIL;
}

HRESULT CZFXModel::ReadJoint_KeyFrame_Rot(LPJOINT pJoint)
{
	UINT	uiNumKeys = pJoint->wNumKF_Rotation;

	LOG(20, false, "Reading KF Rot [%d]", uiNumKeys);

	//allocate memory
	pJoint->pKF_Rotation = new KF_ROT_S[uiNumKeys];
	if (!pJoint->pKF_Rotation)
	{
		LOG(1, true, " FAILED [JOINT_KEYFRAME_ROTATIONS]");
		return E_FAIL;
	}

	//clear memory
	ZeroMemory(pJoint->pKF_Rotation, sizeof(KF_ROT_S) * uiNumKeys);

	//read all rotations
	fread(pJoint->pKF_Rotation, sizeof(KF_ROT_S), uiNumKeys, m_pFile);

	//look for end chunk
	if (GetNextChunk(m_sChunk) == V1_END)
	{
		LOG(20, true, " OK");
		return S_OK;
	}

	LOG(1, true, " FAILED [JOINT_KEYFRAME_ROTATIONS]");
	return E_FAIL;
}

HRESULT CZFXModel::ReadJoint_KeyFrame_Pos(LPJOINT pJoint)
{
	UINT	uiNumKeys = pJoint->wNumKF_Position;

	LOG(20, false, "Reading KF Pos [%d]", uiNumKeys);

	//allocate memory
	pJoint->pKF_Position = new KF_POS_S[uiNumKeys];
	if (!pJoint->pKF_Position)
	{
		LOG(1, true, " FAILED [JOINT_KEYFRAME_POSITIONS]");
		return E_FAIL;
	}

	//clear memory
	ZeroMemory(pJoint->pKF_Position, sizeof(KF_POS_S) * uiNumKeys);

	//read all rotations
	fread(pJoint->pKF_Position, sizeof(KF_POS_S), uiNumKeys, m_pFile);

	//look for end chunk
	if (GetNextChunk(m_sChunk) == V1_END)
	{
		LOG(20, true, " OK");
		return S_OK;
	}

	LOG(1, true, " FAILED [JOINT_KEYFRAME_POSITIONS]");
	return E_FAIL;
}

HRESULT CZFXModel::ReadAnimations()
{
	UINT	uiNumAnimations = m_sHeader.uiNumAnimations;

	LOG(20, false, "Reading Animations [%d]", uiNumAnimations);

	//allocate memory
	m_pAnimations = new ANIMATION_S[uiNumAnimations];
	if (m_pAnimations)
	{
		LOG(1, true, " FAILED [ANIMATIONS]");
		return E_FAIL;
	}

	//clear memory
	ZeroMemory(m_pAnimations, sizeof(ANIMATION_S) * uiNumAnimations);

	//read all rotations
	fread(m_pAnimations, sizeof(ANIMATION_S), uiNumAnimations, m_pFile);

	//look for end chunk
	if (GetNextChunk(m_sChunk) == V1_END)
	{
		LOG(20, true, " OK");
		return S_OK;
	}

	LOG(1, true, " FAILED [ANIMATIONS]");
	return E_FAIL;
}

////////////////////////////////////////////////////////////////

WORD CZFXModel::GetNextChunk(CHUNK_S &pChunk)
{
	//read the next chunk
	fread(&pChunk, sizeof( CHUNK_S), 1, m_pFile );

	return pChunk.wIdentifier;
}

void CZFXModel::SetScaling(float fScale /*0.0f*/)
{
	ULONG		ulCounter = 0;	//counter
	ULONG		ulInner = 0;	//counter
	CVERTEX		*pVertex = NULL;
	float		fScaling = 0.0f;//final scale factor
	LPJOINT		pJoint	= NULL;	//joint

	//do we need to scale?
	if (fScale == 0.0f)
	{
		return;
	}

	//calculate bounding box
	m_sBBoxMin.x = 999999.0f;
	m_sBBoxMin.y = 999999.0f;
	m_sBBoxMin.z = 999999.0f;
	m_sBBoxMax.x = -999999.0f;
	m_sBBoxMax.y = -999999.0f;
	m_sBBoxMax.z = -999999.0f;

	for (ulCounter = 0; ulCounter < m_sHeader.ulNumVertices ; ulCounter++)
	{
		pVertex = &m_pVertices[ulCounter];
		//enlarge box if needed
		m_sBBoxMax.x = max( m_sBBoxMax.x, pVertex->x);
		m_sBBoxMax.y = max(m_sBBoxMax.x, pVertex->y);
		m_sBBoxMax.z = max(m_sBBoxMax.x, pVertex->z);
		m_sBBoxMin.x = min(m_sBBoxMin.x, pVertex->x);
		m_sBBoxMin.y = min(m_sBBoxMin.y, pVertex->y);
		m_sBBoxMin.z = min(m_sBBoxMin.z, pVertex->z);

	}

	//scale bounding box
	fScaling = ( m_sBBoxMax.y - m_sBBoxMin.y) / fScale;

	//scale vertices
	for (ulCounter = 0; ulCounter < m_sHeader.ulNumVertices; ulCounter++)
	{
		pVertex = &m_pVertices[ulCounter];
		pVertex->x /= fScaling;
		pVertex->y /= fScaling;
		pVertex->z /= fScaling;
	}

	//copy back to array if animation is present
	if (m_sHeader.uiNumJoints > 0)
	{
		memcpy(m_pVertices_Orig, m_pVertices, sizeof(CVERTEX) * m_sHeader.ulNumVertices);
	}

	//scale the bones
	for (ulCounter = 0; ulCounter < m_sHeader.uiNumJoints; ulCounter++)
	{
		pJoint = &m_pJoints[ulCounter];

		pJoint->vPosition.x /= fScaling;
		pJoint->vPosition.y /= fScaling;
		pJoint->vPosition.z /= fScaling;

		//scale keyframe positions for this bone
		for (ulInner = 0; ulInner < pJoint->wNumKF_Position; ulInner++)
		{
			pJoint->pKF_Position[ulInner].vPosition.x /= fScaling;
			pJoint->pKF_Position[ulInner].vPosition.y /= fScaling;
			pJoint->pKF_Position[ulInner].vPosition.z /= fScaling;

		}

		//build ZFXEngine AABB
		m_sAABB.vcMin.x = m_sBBoxMin.x;
		m_sAABB.vcMin.y = m_sBBoxMin.y;
		m_sAABB.vcMin.z = m_sBBoxMin.z;
		m_sAABB.vcMax.x = m_sBBoxMax.x;
		m_sAABB.vcMax.y = m_sBBoxMax.y;
		m_sAABB.vcMax.z = m_sBBoxMax.z;

		m_sAABB.vcCenter.x = ( m_sBBoxMax.x - m_sBBoxMin.x ) / 2;
		m_sAABB.vcCenter.y = (m_sBBoxMax.y - m_sBBoxMin.y) / 2;
		m_sAABB.vcCenter.z = (m_sBBoxMax.z - m_sBBoxMin.z) / 2;
	}
}

HRESULT CZFXModel::SetupBones()
{
	//init variables
	LPJOINT		pJoint = NULL;
	ULONG		ulCounter = 0;
	UINT		uiLoop = 0;
	UINT		uiParentID = 0;
	ZFXVector	sVector;
	CVERTEX		*pVertex = NULL;
	ZFXMatrix	matTemp;

	matTemp.Identity();

	//do we have bones at all?
	if (m_sHeader.uiNumJoints == 0)
	{
		return S_OK;
	}

	//build the matrices
	for (ulCounter = 0; ulCounter < m_sHeader.uiNumJoints; ulCounter++)
	{
		//get the joint
		pJoint = &m_pJoints[ulCounter];

		//set rotation to matrix
		pJoint->sMatrix_relative = CreateRotationMatrix( &pJoint->vRotation );

		//set position to matrix
		pJoint->sMatrix_relative._14 = pJoint->vPosition.x;
		pJoint->sMatrix_relative._24 = pJoint->vPosition.y;
		pJoint->sMatrix_relative._34 = pJoint->vPosition.z;

		//find the parent
		for (uiLoop = 0; uiLoop < m_sHeader.uiNumJoints; uiLoop++)
		{
			uiParentID = 255; //PROBLEM -- I don't like that we build in an assumption that the model will have less than 255 bones. True in 2004...not so true now?


			if (wcscmp(m_pJoints[uiLoop].cName, pJoint->cParentName) == 0)
			{
				uiParentID = uiLoop;
				break;
			}
		}

		//save off found ID
		pJoint->wParentID = uiParentID;

		//did we find a parent?
		if (uiParentID != 255)
		{
			//multiply parent's absolute matrix with joint's relative matrix to get joint's absolute matrix
			//assumes parent's matrices are ordered before their children, and thus already have calculated their absolute matrices
			pJoint->sMatrix_absolute = m_pJoints[uiParentID].sMatrix_absolute * pJoint->sMatrix_relative;
		}
		else
		{
			//no parent found, so relative is the same as absolute
			pJoint->sMatrix_absolute = pJoint->sMatrix_relative;
		}

		//calculate vertex transform matrix
		pJoint->sMatrix.TransposeOf(pJoint->sMatrix_absolute);

		//transpose the relative matrix
		matTemp = pJoint->sMatrix_relative;
		pJoint->sMatrix_relative.TransposeOf(matTemp);
	}

	//set up the vertices
	for (ulCounter = 0; ulCounter < m_sHeader.ulNumVertices; ulCounter++)
	{
		//get the current vertex
		pVertex = &m_pVertices_Orig[ulCounter];

		//continue only if there is a bone
		if (pVertex->fBone1 != 255.0f)
		{
			//get current matrix
			matTemp.Identity();
			matTemp = m_pJoints[(UINT) pVertex->fBone1].sMatrix;

			//rotate vertices
			sVector.x = pVertex->x;
			sVector.y = pVertex->y;
			sVector.z = pVertex->z;
			sVector -= matTemp.GetTranslation();
			sVector.InvRotateWith( matTemp );
			pVertex->x = sVector.x;
			pVertex->y = sVector.y;
			pVertex->z = sVector.z;

			//rotate normals
			sVector.x = pVertex->vcN[0];
			sVector.y = pVertex->vcN[1];
			sVector.z = pVertex->vcN[2];
			sVector.InvRotateWith( matTemp );
			pVertex->vcN[0] = sVector.x;
			pVertex->vcN[1] = sVector.y;
			pVertex->vcN[2] = sVector.z;
		}
	}
	return S_OK;
}

void CZFXModel::SetAnimation(UINT uiAnim)
{
	// In Range?
	if (uiAnim > m_sHeader.uiNumAnimations)
	{
		uiAnim = 0;
	}
	if (uiAnim < 0)
	{
		uiAnim = m_sHeader.uiNumAnimations;
	}

	// Set Animation
	m_uiCurrentAnimation = uiAnim;

	// Multiply Animations
	m_bAnimationRunOnce = false;
}

void CZFXModel::SetAnimation(bool bSingle, UINT uiAnim)
{
	// Set the wanted Animation
	SetAnimation(uiAnim);

	// Multiply Animations
	m_bAnimationChanged = true;
	m_bAnimationRunOnce = bSingle;
	m_bAnimationComplete = false;
}

ZFXMatrix CZFXModel::CreateRotationMatrix(ZFXVector *pVector)
{
	// Init variables
	float	sr, sp, sy, cr, cp, cy;
	ZFXMatrix matRet;

	matRet.Identity();

	sy = (float)sin(pVector->z);
	cy = (float)cos(pVector->z);
	sp = (float)sin(pVector->y);
	cp = (float)cos(pVector->y);
	sr = (float)sin(pVector->x);
	cr = (float)cos(pVector->x);

	matRet._11 = cp*cy;
	matRet._21 = cp*sy;
	matRet._31 = -sp;
	matRet._12 = sr*sp*cy + cr*-sy;
	matRet._22 = sr*sp*sy + cr*cy;
	matRet._32 = sr*cp;
	matRet._13 = (cr*sp*cy + -sr*-sy);
	matRet._23 = (cr*sp*sy + -sr*cy);
	matRet._33 = cr*cp;
	matRet._14 = 0.0;
	matRet._24 = 0.0;
	matRet._34 = 0.0;

	return matRet;
}

////////////////////////////////////////////////////////////////

void CZFXModel::LOG(UINT iLevel, bool bCR, PCHAR pcText, ...)
{
	// Variablen init
	va_list  args;		                               // Arguments
	char     cBuffer[4096];                            // linebuffer
	FILE     *pLog;                                    // file

	// do logging?
	if (!m_bLog)
	{
		return;
	}

	// correct loglevel?
	if (iLevel <= m_uiLogLevel)
	{
		// something to log?
		if (pcText)
		{
			// set the pointer to the beginning
			va_start(args, pcText);

			// copy all data to cBuffer
			vsprintf(cBuffer, pcText, args);

			// reset the list
			va_end(args);
		}

		// open the logfile
		fopen_s(&pLog, m_cLogFileName, "a");

		// file opened?
		if (!pLog)
		{
			return;
		}

		// write the wanted text into the logfile
		if (bCR)
		{
			fprintf(pLog, "%s\n", cBuffer);
		}
		else
		{
			fprintf(pLog, "%s", cBuffer);
		}
		// file close
		fclose(pLog);
	}
}
//----