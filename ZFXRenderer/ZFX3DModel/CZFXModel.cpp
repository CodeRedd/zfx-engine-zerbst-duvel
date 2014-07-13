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
	strcpy(m_cLogFileName, "MODELLOG.TXT");	// Logfilename
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
		case V1_MESH:		ReadMesh();			break;
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

	for (ulCounter = 0; , ulCounter < m_sHeader.ulNumVertices; ulCounter++)
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