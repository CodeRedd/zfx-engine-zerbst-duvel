//File: CModelStructs.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#pragma once

//chunk types
#define V1_HEADER				0x0100	//Header
#define V1_VERTEX				0x0200	//Vertices
#define V1_FACE					0x0300	//Faces
#define V1_MESH					0x0400	//Meshes
#define V1_MATERIAL				0x0500	//Materials
#define V1_JOINT				0x0600	//Joints
#define V1_JOINT_MAIN			0x0610	//Joints Main
#define V1_JOINT_KEYFRAME_ROT	0x0620	//Keyframe Rotation
#define V1_JOINT_KEYFRAME_POS	0x0630	//Keyframe Position
#define V1_ANIMATION			0x0700	//Animations
#define V1_END					0x9999	//End Chunk


//chunk
struct CHUNK_S
{
	WORD	wIdentifier;
	ULONG	ulSize;
};
typedef CHUNK_S* LPCHUNK;

struct CHUNKHEAD_S
{
	UCHAR	ucIdentifier[4];
	UCHAR	ucName[32];
	UCHAR	ucAuthor[32];
	UCHAR	ucEmail[32];
	UCHAR	ucType;
	UCHAR	ucVersion;
	ULONG	ulNumVertices;
	ULONG	ulNumIndices;
	ULONG	ulNumFaces;
	ULONG	ulNumMeshes;
	UINT	uiNumMaterials;
	UINT	uiNumJoints;
	float	fAnimationFPS;
	float	fCurrentTime;
	UINT	uiNumFrames;
	UINT	uiNumAnimations;
};
typedef CHUNKHEAD_S* LPCHUNKHEAD;

struct VERTEX_3F_S
{
	float		fXYZ[3];		//spatial coordinates
	float		fUV0[2];		//texture coords 1
	float		fUV1[2];		//texture coords 2
	ZFXVector	fNormal;		//normal vector
	USHORT		usReferences;	//references
	UINT		uiBoneID_A;		//bone ID 1
	float		fWeight_A;		//weight 1
	UINT		uiBoneID_B;		//bone ID 2
	float		fWeight_B;		//weight 2
	BYTE		byFlags;		//flags
};
typedef VERTEX_3F_S* LPVERTEX_3F;

struct FACE_S
{
	ULONG		ulIndices[3];	//indices
	ZFXVector	fNormal;		//normal vector
	ULONG		ulMeshID;		//mesh ID
	UINT		uiMaterialID;	//material ID
	BYTE		byFlags;		//flags
};
typedef FACE_S* LPFACE;

struct MATERIAL_S
{
	TCHAR		cName[32];			//name
	float		fAmbient[4];		//ambient color
	float		fDiffuse[4];		//diffuse color
	float		fSpecular[4];		//specular color
	float		fEmissive[4];		//emissive color
	float		fSpecularPower;		//specular power
	float		fTransparency;		//transparency
	TCHAR		cTexture_1[128];	//texture name
	TCHAR		cTexture_2[128];	//texture name
	BYTE		byFlags;			//flags

};
typedef MATERIAL_S* LPMATERIAL;

struct ANIMATION_S
{
	TCHAR		cName[64];			//name
	float		fStartFrame;		//start frame
	float		fEndFrame;			//end frame
	bool		bActive;			//is this animation active?
};
typedef ANIMATION_S* LPANIMATION;


struct JOINT_S
{
	TCHAR		cName[32];			//name
	TCHAR		cParentName[32];	//parent joint's name
	WORD		wParentID;			//parent joint's ID
	ZFXVector	vRotation;			//rotation
	ZFXVector	vPosition;			//position
	WORD		wNumKF_Rotation;	//number of rotation keyframes
	WORD		wNumKF_Position;	//number of position keyframes
	LPKF_ROT	pKF_Rotation;		//rotation keyframes
	LPKF_POS	pKF_Position;		//position keyframes
	bool		bAnimated;			//is this joint animated?
	BYTE		byFlags;			//flags
	ZFXMatrix   sMatrix;			//matrix
	ZFXMatrix	sMatrix_absolute;	//absolute matrix
	ZFXMatrix	sMatrix_relative;	//relative matrix
};
typedef JOINT_S* LPJOINT;

struct KF_ROT_S
{
	float		fTime;		//time
	ZFXVector	vRotation;	//rotation
};
typedef KF_ROT_S* LPKF_ROT;

struct KF_POS_S
{
	float		fTime;		//time
	ZFXVector	vPosition;	//position
};
typedef KF_POS_S* LPKF_POS;