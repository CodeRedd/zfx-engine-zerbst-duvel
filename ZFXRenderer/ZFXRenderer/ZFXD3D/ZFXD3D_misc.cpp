//File: ZFXD3D_misc.cpp 
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd


#include "math.h"           // sqrt function
#include "ZFXD3D.h"         // class definition
#include <D3D9.h>
#include <D3DX9.h>

extern bool g_bLF;


/////////////////////////
// VIEW FUNCTIONS
/////////////////////////

//NOTE: this must be recalculated every time the projection or view matrices are changed
HRESULT ZFXD3D::SetView3D(const ZFXVector &vcRight, const ZFXVector &vcUp, const ZFXVector &vcDir, const ZFXVector &vcPos)
{
	if (!m_bRunning)
	{
		return E_FAIL;
	}

	m_mView3D._14 = m_mView3D._21 = m_mView3D._34 = 0.0f;
	m_mView3D._44 = 1.0f;

	m_mView3D._11 = vcRight.x;
	m_mView3D._21 = vcRight.y;
	m_mView3D._31 = vcRight.z;
	m_mView3D._41 = -(vcRight * vcPos);

	m_mView3D._12 = vcUp.x;
	m_mView3D._22 = vcUp.y;
	m_mView3D._32 = vcUp.z;
	m_mView3D._42 = -(vcUp * vcPos);

	m_mView3D._13 = vcDir.x;
	m_mView3D._23 = vcDir.y;
	m_mView3D._33 = vcDir.z;
	m_mView3D._43 = -(vcDir * vcPos);

	if (!m_bUseShaders)
	{
		if (FAILED(m_pDevice->SetTransform(D3DTS_VIEW, &m_mView3D)))
		{
			return ZFX_FAIL;
		}
	}

	CalcViewProjMatrix();
	CalcWorldViewProjMatrix();
	return ZFX_OK;
}

HRESULT ZFXD3D::SetViewLookAt(const ZFXVector &vcPos, const ZFXVector &vcPoint, const ZFXVector &vcWorldUp)
{
	ZFXVector vcDir, vcTemp, vcUp;

	vcDir = vcPoint - vcPos;
	vcDir.Normalize();

	//calculate up vector
	float fDot = vcWorldUp * vcDir;
	vcTemp = vcDir * fDot;
	vcUp = vcWorldUp - vcTemp;
	float fL = vcUp.GetLength();

	//if up vector is too short, use world y-axis instead
	if (fL < 1e-6f)
	{
		ZFXVector vcY;
		vcY.Set(0.0f, 1.0f, 0.0f);

		vcTemp = vcDir * vcDir.y;
		vcUp = vcY - vcTemp;

		fL = vcUp.GetLength();

		//use world z-axis if up vector is still too short
		if (fL < 1e-6f)
		{
			vcY.Set(0.0f, 0.0f, 1.0f);

			vcTemp = vcDir * vcDir.z;
			vcUp = vcY - vcTemp;

			fL = vcUp.GetLength();

			//we are in a weird state if these fail
			if (fL < 1e-6f)
			{
				return ZFX_FAIL;
			}
		}
	}

	vcUp /= fL;

	//build right vector
	ZFXVector vcRight;
	vcRight.Cross(vcUp, vcDir);

	//create and activate final matrix
	return SetView3D(vcRight, vcUp, vcDir, vcPos);
}

//saves the six planes that make up the view frustum into p
//NOTE: this must be recalculated every time the projection or view matrices are changed
HRESULT ZFXD3D::GetFrustum(ZFXPlane *p)
{
	//left plane
	p[0].m_vcN.x = -(m_mViewProj._14 + m_mViewProj._11);
	p[0].m_vcN.y = -(m_mViewProj._24 + m_mViewProj._21);
	p[0].m_vcN.z = -(m_mViewProj._34 + m_mViewProj._31);
	p[0].m_fD	 = -(m_mViewProj._44 + m_mViewProj._41);

	//right plane
	p[1].m_vcN.x = -(m_mViewProj._14 - m_mViewProj._11);
	p[1].m_vcN.y = -(m_mViewProj._24 - m_mViewProj._21);
	p[1].m_vcN.z = -(m_mViewProj._34 - m_mViewProj._31);
	p[1].m_fD	 = -(m_mViewProj._44 - m_mViewProj._41);

	//top plane
	p[2].m_vcN.x = -(m_mViewProj._14 - m_mViewProj._12);
	p[2].m_vcN.y = -(m_mViewProj._24 - m_mViewProj._22);
	p[2].m_vcN.z = -(m_mViewProj._34 - m_mViewProj._32);
	p[2].m_fD	 = -(m_mViewProj._44 - m_mViewProj._42);

	//bottom plane
	p[3].m_vcN.x = -(m_mViewProj._14 + m_mViewProj._12);
	p[3].m_vcN.y = -(m_mViewProj._24 + m_mViewProj._22);
	p[3].m_vcN.z = -(m_mViewProj._34 + m_mViewProj._32);
	p[3].m_fD	 = -(m_mViewProj._44 + m_mViewProj._42);

	//near plane
	p[4].m_vcN.x = -m_mViewProj._13;
	p[4].m_vcN.y = -m_mViewProj._23;
	p[4].m_vcN.z = -m_mViewProj._33;
	p[4].m_fD	 = -m_mViewProj._43;

	//far plane
	p[5].m_vcN.x = -(m_mViewProj._14 - m_mViewProj._13);
	p[5].m_vcN.y = -(m_mViewProj._24 - m_mViewProj._23);
	p[5].m_vcN.z = -(m_mViewProj._34 - m_mViewProj._33);
	p[5].m_fD	 = -(m_mViewProj._44 - m_mViewProj._43);

	//normalize normals
	for (int i = 0; i < 6; i++)
	{
		float fL = p[i].m_vcN.GetLength();
		p[i].m_vcN /= fL;
		p[i].m_fD  /= fL;
	}

	return ZFX_OK;		
}

//NOTE: note, this changes the projection matrices, so make sure to recalculate other values after using!
void ZFXD3D::SetClippingPlanes(float fNear, float fFar)
{
	m_fNear = fNear;
	m_fFar = fFar;

	//validate near and far positions to prevent weirdness once we bring this stuff to the depth buffer
	if (m_fNear <= 0.0f)
	{
		m_fNear = 0.01f;
	}
	if (m_fFar <= 1.0f)
	{
		m_fFar = 1.00f;
	}
	if (m_fNear >= m_fFar)
	{
		m_fNear = m_fFar;
		m_fFar = m_fNear + 1.0f;
	}

	//adjust 2D matrices
	Prepare2D();

	//adjust orthogonal projection
	float Q = 1.0f / (m_fFar - m_fNear);
	float X = -Q * m_fNear;
	m_mProjO[0]._33 = m_mProjO[1]._33 = Q;
	m_mProjO[2]._33 = m_mProjO[3]._33 = Q;
	m_mProjO[0]._43 = m_mProjO[1]._43 = X;
	m_mProjO[2]._43 = m_mProjO[3]._43 = X;

	//adjust perspective projection
	Q *= m_fFar;
	X = -Q * m_fNear;
	m_mProjO[0]._33 = m_mProjO[1]._33 = Q;
	m_mProjO[2]._33 = m_mProjO[3]._33 = Q;
	m_mProjO[0]._43 = m_mProjO[1]._43 = X;
	m_mProjO[2]._43 = m_mProjO[3]._43 = X;
}

//prepares orthogonal projection matrix and special view matrix for 2D view mode
void ZFXD3D::Prepare2D()
{
	//make identity matrix
	memset(&m_mProj2D, 0, sizeof(float)*16);
	memset(&m_mView2D, 0, sizeof(float)*16);
	m_mView2D._11 = m_mView2D._33 = m_mView2D._44 = 1.0f;

	//orthogonal proj matrix
	m_mProj2D._11 = 2.0f / (float)m_dwWidth;
	m_mProj2D._22 = 2.0f / (float)m_dwHeight;
	m_mProj2D._33 = 1.0f / (m_fFar - m_fNear);
	m_mProj2D._43 = -m_fNear*(1.0f/(m_fFar - m_fNear));
	m_mProj2D._44 = 1.0f;

	//2d view matrix
	float tx, ty, tz;
	tx = -((int)m_dwWidth) + m_dwWidth * 0.5f;
	ty = m_dwHeight - m_dwHeight * 0.5f;
	tz = m_fNear + 0.1f;

	m_mView2D._22 = -1.0f;
	m_mView2D._41 = tx;
	m_mView2D._42 = ty;
	m_mView2D._43 = tz;
}

HRESULT ZFXD3D::CalcPerspProjMatrix(float fFOV, float fAspect, D3DMATRIX *m)
{
	//clipping planes must not be too close together
	if (fabs(m_fFar - m_fNear) < 0.01f)
	{
		return ZFX_FAIL;
	}

	float sinFOV2 = sinf(fFOV/2);
	//sin = ~0 means that we have a way too small FOV
	if (fabs(sinFOV2) < 0.01f)
	{
		return ZFX_FAIL;
	}
	
	//trig calculations are slow! we want to do these only once.
	float cosFOV2 = cosf(fFOV/2);

	float w = fAspect * (cosFOV2 / sinFOV2);
	float h =	 1.0f * (cosFOV2 / sinFOV2);
	float Q = m_fFar / (m_fFar - m_fNear);

	memset(m, 0, sizeof(D3DMATRIX));
	(*m)._11 = w;
	(*m)._22 = h;
	(*m)._33 = Q;
	(*m)._34 = 1.0f;
	(*m)._43 = -Q*m_fNear;

	return ZFX_OK;	
}

//projects view onto projection plane
void ZFXD3D::CalcViewProjMatrix()
{
	ZFXMatrix *pA;
	ZFXMatrix *pB;

	//pick matrices depending on whether we're in 2D, perspective, or orthogonal
	if (m_Mode == EMD_TWOD)
	{
		pA = (ZFXMatrix*)&m_mProj2D;
		pB = (ZFXMatrix*)&m_mView2D;
	}
	else
	{
		pB = (ZFXMatrix*)&m_mView3D;
		if (m_Mode == EMD_PERSPECTIVE)
		{
			pA = (ZFXMatrix*)&(m_mProjP[m_nStage]);
		}
		else
		{
			pA = (ZFXMatrix*)&(m_mProjO[m_nStage]);
		}
	}

	//now, multiply to combine matrices
	ZFXMatrix *pM = (ZFXMatrix*) &m_mViewProj;
	(*pM) = (*pA) * (*pB);
}

//transforms vertices from world space to camera space and then projects (applied mostly for shaders)
void ZFXD3D::CalcWorldViewProjMatrix()
{
	ZFXMatrix *pProj;
	ZFXMatrix *pView;
	ZFXMatrix *pWorld;

	pWorld = (ZFXMatrix*)&m_mWorld;

	//pick matrices depending on whether we're in 2D, perspective, or orthogonal
	if (m_Mode == EMD_TWOD)
	{
		pProj = (ZFXMatrix*)&m_mProj2D;
		pView = (ZFXMatrix*)&m_mView2D;
	}
	else
	{
		pView = (ZFXMatrix*)&m_mView3D;
		if (m_Mode == EMD_PERSPECTIVE)
		{
			pProj = (ZFXMatrix*)&(m_mProjP[m_nStage]);
		}
		else
		{
			pProj = (ZFXMatrix*)&(m_mProjO[m_nStage]);
		}
	}

	ZFXMatrix *pCombo = (ZFXMatrix*)&m_mWorldViewProj;
	(*pCombo) = ((*pWorld) * (*pView) * (*pProj));
}

HRESULT ZFXD3D::SetMode(ZFXENGINEMODE Mode, int nStage)
{
	D3DVIEWPORT9 d3dVP;

	if (!m_bRunning)
	{
		return E_FAIL;
	}
	if (nStage > 3 || nStage < 0)
	{
		nStage = 0;
	}
	if (m_Mode != Mode)
	{
		m_Mode = Mode;
	}

	//flush all cached view data prior to changing mode
	m_pVertexMan->ForcedFlushAll();

	//if 2D then use 2D matrices
	if (Mode == EMD_TWOD)
	{
		//set viewport
		d3dVP.X			= 0;
		d3dVP.Y			= 0;
		d3dVP.Width		= m_dwWidth;
		d3dVP.Height	= m_dwHeight;
		d3dVP.MinZ		= 0.0f;
		d3dVP.MaxZ		= 1.0f;

		if (FAILED(m_pDevice->SetViewport(&d3dVP)))
		{
			return ZFX_FAIL;
		}

		if (!m_bUseShaders)
		{
			if (FAILED(m_pDevice->SetTransform(D3DTS_PROJECTION, &m_mProj2D)))
			{
				return ZFX_FAIL;
			}
			if (FAILED(m_pDevice->SetTransform(D3DTS_VIEW, &m_mView2D)))
			{
				return ZFX_FAIL;
			}
		}
	}
	//perspective or orthogonal projection
	else
	{
		m_nStage = nStage;

		//set viewport
		d3dVP.X = m_VP[nStage].X;
		d3dVP.Y = m_VP[nStage].Y;
		d3dVP.Width = m_VP[nStage].Width;
		d3dVP.Height = m_VP[nStage].Height;
		d3dVP.MinZ = 0.0f;
		d3dVP.MaxZ = 1.0f;

		if (FAILED(m_pDevice->SetViewport(&d3dVP)))
		{
			return ZFX_FAIL;
		}

		if (!m_bUseShaders)
		{
			if (FAILED(m_pDevice->SetTransform(D3DTS_VIEW, &m_mView3D)))
			{
				return ZFX_FAIL;
			}
			if (m_Mode == EMD_PERSPECTIVE)
			{
				if (FAILED(m_pDevice->SetTransform(D3DTS_PROJECTION, &m_mProjP[nStage])))
				{
					return ZFX_FAIL;
				}
			}
			else
			{//EMD_ORTHOGONAL
				if (FAILED(m_pDevice->SetTransform(D3DTS_PROJECTION, &m_mProjO[nStage])))
				{
					return ZFX_FAIL;
				}
			}
		}

		CalcViewProjMatrix();
		CalcWorldViewProjMatrix();
	}
	return ZFX_OK;
}

HRESULT ZFXD3D::InitStage(float fFOV, ZFXVIEWPORT *pView, int nStage)
{
	float fAspect;
	bool  bOwnRect=false;

	if (!pView)
	{
		ZFXVIEWPORT vpOwn = { 0, 0, m_dwWidth, m_dwHeight};
		memcpy(&m_VP[nStage], &vpOwn, sizeof(RECT));
	}
	else
	{
		memcpy(&m_VP[nStage], pView, sizeof(RECT));
	}

	if (nStage > 3 || nStage < 0)
	{
		nStage = 0;
	}

	fAspect = (float)(m_VP[nStage].Height) / m_VP[nStage].Width;

	//perspective projection
	if (FAILED(this->CalcPerspProjMatrix(fFOV, fAspect, &m_mProjP[nStage])))
	{
		return ZFX_FAIL;
	}

	//orthogonal projection
	memset(&m_mProjO[nStage], 0, sizeof(float)* 16);
	m_mProjO[nStage]._11 = 2.0f / m_VP[nStage].Width;
	m_mProjO[nStage]._22 = 2.0f / m_VP[nStage].Height;
	m_mProjO[nStage]._33 = 1.0f / (m_fFar - m_fNear);
	m_mProjO[nStage]._43 = -(m_fNear * m_mProjO[nStage]._33);
	m_mProjO[nStage]._44 = 1.0f;
	return ZFX_OK;
}

POINT ZFXD3D::Transform3DTo2D(const ZFXVector &vcPoint)
{
	POINT pt;
	float fClip_x, fClip_y;
	float fXp, fYp, fWp;
	DWORD dwWidth, dwHeight;

	//if 2D mode, take the whole screen
	if (m_Mode == EMD_TWOD)
	{
		dwWidth = m_dwWidth;
		dwHeight = m_dwHeight;
	}
	//else take viewport dims
	else
	{
		dwWidth = m_VP[m_nStage].Width;
		dwHeight = m_VP[m_nStage].Height;
	}
	fClip_x = (float)(dwWidth  >> 1);
	fClip_y = (float)(dwHeight >> 1);

	//transformation and projection
	fXp = (m_mViewProj._11*vcPoint.x) + (m_mViewProj._21*vcPoint.y) + (m_mViewProj._31*vcPoint.z) + m_mViewProj._41;
	fYp = (m_mViewProj._12*vcPoint.x) + (m_mViewProj._22*vcPoint.y) + (m_mViewProj._32*vcPoint.z) + m_mViewProj._42;
	fWp = (m_mViewProj._14*vcPoint.x) + (m_mViewProj._24*vcPoint.y) + (m_mViewProj._34*vcPoint.z) + m_mViewProj._44;

	float fWpInv = 1.0f / fWp;

	//convert from [-1,1] coordinate space to viewport size
	pt.x = (LONG)((1.0f + (fXp * fWpInv)) * fClip_x);
	pt.y = (LONG)((1.0f + (fYp * fWpInv)) * fClip_y);
	return pt;
}

void ZFXD3D::Transform2DTo3D(const POINT &pt, ZFXVector *vcOrig, ZFXVector *vcDir)
{
	D3DMATRIX *pView = NULL, *pProj = NULL;
	ZFXMatrix mInvView;
	ZFXVector vcS;
	DWORD	  dwWidth, dwHeight;

	//2d mode
	if (m_Mode == EMD_TWOD)
	{
		dwWidth = m_dwWidth;
		dwHeight = m_dwHeight;

		pView = &m_mView2D;
	}
	//else orthogonal or perspective proj
	else
	{
		dwWidth = m_VP[m_nStage].Width;
		dwHeight = m_VP[m_nStage].Height;

		pView = &m_mView3D;

		if (m_Mode == EMD_PERSPECTIVE)
		{
			pProj = &m_mProjP[m_nStage];
		}
		else
		{
			pProj = &m_mProjO[m_nStage];
		}
	}

	//scale to viewport and invert the projection
	vcS.x = (((pt.x*2.0f) / dwWidth) - 1.0f) / m_mProjP[m_nStage]._11;
	vcS.y = (((pt.y*2.0f) / dwHeight) - 1.0f) / m_mProjP[m_nStage]._22;
	vcS.z = 1.0f;

	//invert view matrix
	mInvView.InverseOf(*((ZFXMatrix*)&m_mView3D._11));

	//ray from screen to world coords
	(*vcDir).x = (vcS.x * mInvView._11) + (vcS.y * mInvView._21) + (vcS.z * mInvView._31);
	(*vcDir).y = (vcS.x * mInvView._12) + (vcS.y * mInvView._22) + (vcS.z * mInvView._32);
	(*vcDir).z = (vcS.x * mInvView._13) + (vcS.y * mInvView._23) + (vcS.z * mInvView._33);

	//inverse translation
	(*vcOrig).x = mInvView._41;
	(*vcOrig).y = mInvView._42;
	(*vcOrig).z = mInvView._43;

	//normalize
	(*vcDir).Normalize();
}

void ZFXD3D::SetWorldTransform(const ZFXMatrix *mWorld)
{
	//check for cached data
	m_pVertexMan->ForcedFlushAll();

	//fill class attribute
	if (!mWorld)
	{
		ZFXMatrix m;
		m.Identity();
		memcpy(&m_mWorld, &m, sizeof(D3DMATRIX));
	}
	else
	{
		memcpy(&m_mWorld, mWorld, sizeof(D3DMATRIX));
	}

	//recalculate dependent values
	CalcWorldViewProjMatrix();

	//if using shaders, set a constant
	if (m_bUseShaders)
	{
		ZFXMatrix mTranspose;
		mTranspose.TransposeOf(*(ZFXMatrix*)&m_mWorldViewProj);
		m_pDevice->SetVertexShaderConstantF(0, (float*)&mTranspose, 4);
	}
	else
	{
		m_pDevice->SetTransform(D3DTS_WORLD, &m_mWorld);
	}
}

/////////////////////////
// SHADER FUNCTIONS
/////////////////////////

void ZFXD3D::PrepareShaderStuff()
{
	D3DCAPS9 d3dCaps;

	//get hardware capabilities
	if (FAILED(m_pDevice->GetDeviceCaps(&d3dCaps)))
	{
		m_bCanDoShaders = false;
		return;
	}

	//we will support shader versions 2.0+ (i.e. DirectX 9.0+). We could probably support only 3.0+ (DirectX 9.0c+) but we're being generous.
	if (d3dCaps.VertexShaderVersion < D3DVS_VERSION(2, 0))
	{
		m_bCanDoShaders = false;
		return;
	}
	if (d3dCaps.PixelShaderVersion < D3DPS_VERSION(2, 0))
	{
		m_bCanDoShaders = false;
		return;
	}

	//vertex declaration for vertex shaders
	D3DVERTEXELEMENT9 declPVertex[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		D3DDECL_END()
	};

	D3DVERTEXELEMENT9 declVertex[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	D3DVERTEXELEMENT9 declLVertex[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		{ 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	D3DVERTEXELEMENT9 declCVertex[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
		D3DDECL_END()
	};

	D3DVERTEXELEMENT9 decl3TVertex[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
		{ 0, 40, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
		D3DDECL_END()
	};

	D3DVERTEXELEMENT9 declTVertex[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 },
		D3DDECL_END()
	};

	//create these vertex declarations
	m_pDevice->CreateVertexDeclaration(declPVertex, &m_pDeclPVertex);
	m_pDevice->CreateVertexDeclaration(declVertex, &m_pDeclVertex);
	m_pDevice->CreateVertexDeclaration(declLVertex, &m_pDeclLVertex);
	m_pDevice->CreateVertexDeclaration(declCVertex, &m_pDeclCVertex);
	m_pDevice->CreateVertexDeclaration(decl3TVertex, &m_pDecl3TVertex);
	m_pDevice->CreateVertexDeclaration(declTVertex, &m_pDeclTVertex);

	//since we've created our own vertex declarations we don't need to use FVF
	m_pDevice->SetFVF(NULL);

	m_bCanDoShaders = true;
}

HRESULT ZFXD3D::CreateVShader(const void *pData, UINT nSize, bool bLoadFromFile, bool bIsCompiled, UINT *pID)
{
	LPD3DXBUFFER	pCode = NULL;
	LPD3DXBUFFER	pDebug = NULL;
	HRESULT			hrC = ZFX_OK, hrA = ZFX_OK;
	DWORD			*pVS = NULL;
	HANDLE			hFile = NULL, hMap = NULL;

	//is there room for another shader?
	if (m_nNumVShaders >= (MAX_SHADER - 1))
	{
		return ZFX_OUTOFMEMORY;
	}

	// 1. Already compiled shader
	if (bIsCompiled)
	{
		//from file
		if (bLoadFromFile)
		{
			hFile = CreateFile((LPCWSTR)pData, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				return ZFX_FILENOTFOUND;
			}

			hMap = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, 0);
			pVS = (DWORD*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
		}
		//from RAM pointer
		else
		{
			pVS = (DWORD*)pData;
		}
	}
	// 2. NEEDS TO BE COMPILED
	else
	{
		//from file
		if (bLoadFromFile)
		{
			hrA = D3DXAssembleShaderFromFile((TCHAR*)pData, NULL, NULL, 0, &pCode, &pDebug);
		}
		//from RAM pointer
		else
		{
			hrA = D3DXAssembleShader((char*)pData, nSize-1, NULL, NULL, 0, &pCode, &pDebug);
		}

		//check for errors
		if (SUCCEEDED(hrA))
		{
			pVS = (DWORD*)pCode->GetBufferPointer();
		}
		else
		{
			Log(L"error: AssembleShader() failed");
			if (pDebug->GetBufferPointer())
			{
				Log(L"Shader debugger says: %s", (TCHAR*)pDebug->GetBufferPointer());
			}
			return ZFX_FAIL;
		}
	}

	//create the shader object
	if (FAILED(hrC = m_pDevice->CreateVertexShader(pVS, &m_pVShader[m_nNumVShaders])))
	{
		Log(L"error: CreateVertexShader() failed");
		return ZFX_FAIL;
	}

	//save ID of this shader
	if (pID)
	{
		(*pID) = m_nNumVShaders;
	}

	//free resources
	if (bIsCompiled && bLoadFromFile)
	{
		UnmapViewOfFile(pVS);
		CloseHandle(hMap);
		CloseHandle(hFile);
	}

	m_nNumVShaders++;
	return ZFX_OK;
}

HRESULT ZFXD3D::ActivateVShader(UINT nID, ZFXVERTEXID VertexID)
{
	//check validity of system and ID parameter
	if (!m_bCanDoShaders)
	{
		return ZFX_NOSHADERSUPPORT;
	}
	if (nID >= m_nNumVShaders)
	{
		ZFX_INVALIDID;
	}

	//render out vertex caches
	m_pVertexMan->ForcedFlushAll();

	//get vertex size and format
	switch (VertexID)
	{
	case VID_PS:
		if (FAILED(m_pDevice->SetVertexDeclaration(m_pDeclPVertex)))
		{
			return ZFX_FAIL;
		}
		break;
	case VID_UU:
		if (FAILED(m_pDevice->SetVertexDeclaration(m_pDeclVertex)))
		{
			return ZFX_FAIL;
		}
		break;
	case VID_UL:
		if (FAILED(m_pDevice->SetVertexDeclaration(m_pDeclLVertex)))
		{
			return ZFX_FAIL;
		}
		break;
	case VID_CA:
		if (FAILED(m_pDevice->SetVertexDeclaration(m_pDeclCVertex)))
		{
			return ZFX_FAIL;
		}
		break;	
	case VID_3T:
		if (FAILED(m_pDevice->SetVertexDeclaration(m_pDecl3TVertex)))
		{
			return ZFX_FAIL;
		}
			break;	
	case VID_TV:
		if (FAILED(m_pDevice->SetVertexDeclaration(m_pDeclTVertex)))
		{
			return ZFX_FAIL;
		}
		break;
	default:
		return ZFX_INVALIDID;
	}

	//set the shader
	if (FAILED(m_pDevice->SetVertexShader(m_pVShader[nID])))
	{
		return ZFX_FAIL;
	}

	return ZFX_OK;
}

HRESULT ZFXD3D::CreatePShader(const void *pData, UINT nSize, bool bLoadFromFile, bool bIsCompiled, UINT *pID)
{
	LPD3DXBUFFER	pCode = NULL;
	LPD3DXBUFFER	pDebug = NULL;
	HRESULT			hrC = ZFX_OK, hrA = ZFX_OK;
	DWORD			*pVS = NULL;
	HANDLE			hFile = NULL, hMap = NULL;

	//is there room for another shader?
	if (m_nNumPShaders >= (MAX_SHADER - 1))
	{
		return ZFX_OUTOFMEMORY;
	}

	// 1. Already compiled shader
	if (bIsCompiled)
	{
		//from file
		if (bLoadFromFile)
		{
			hFile = CreateFile((LPCWSTR)pData, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				return ZFX_FILENOTFOUND;
			}

			hMap = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, 0);
			pVS = (DWORD*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
		}
		//from RAM pointer
		else
		{
			pVS = (DWORD*)pData;
		}
	}
	// 2. NEEDS TO BE COMPILED
	else
	{
		//from file
		if (bLoadFromFile)
		{
			hrA = D3DXAssembleShaderFromFile((TCHAR*)pData, NULL, NULL, 0, &pCode, &pDebug);
		}
		//from RAM pointer
		else
		{
			hrA = D3DXAssembleShader((char*)pData, nSize - 1, NULL, NULL, 0, &pCode, &pDebug);
		}

		//check for errors
		if (SUCCEEDED(hrA))
		{
			pVS = (DWORD*)pCode->GetBufferPointer();
		}
		else
		{
			Log(L"error: AssembleShader() failed");
			if (pDebug->GetBufferPointer())
			{
				Log(L"Shader debugger says: %s", (TCHAR*)pDebug->GetBufferPointer());
			}
			return ZFX_FAIL;
		}
	}

	//create the shader object
	if (FAILED(hrC = m_pDevice->CreatePixelShader(pVS, &m_pPShader[m_nNumPShaders])))
	{
		Log(L"error: CreatePixelShader() failed");
		return ZFX_FAIL;
	}

	//save ID of this shader
	if (pID)
	{
		(*pID) = m_nNumPShaders;
	}

	//free resources
	if (bIsCompiled && bLoadFromFile)
	{
		UnmapViewOfFile(pVS);
		CloseHandle(hMap);
		CloseHandle(hFile);
	}

	m_nNumPShaders++;
	return ZFX_OK;
}

HRESULT ZFXD3D::ActivatePShader(UINT nID)
{
	if (!m_bCanDoShaders)
	{
		return ZFX_NOSHADERSUPPORT;
	}
	if (nID >= m_nNumPShaders)
	{
		return ZFX_INVALIDID;
	}

	//render out vertex caches
	m_pVertexMan->ForcedFlushAll();

	if (FAILED(m_pDevice->SetPixelShader(m_pPShader[nID])))
	{
		return ZFX_FAIL;
	}

	return ZFX_OK;
}

void ZFXD3D::SetBackfaceCulling(ZFXRENDERSTATE rs)
{
	m_pVertexMan->ForcedFlushAll();
	if (rs == RS_CULL_CW)
	{
		m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	}
	else if (rs == RS_CULL_CCW)
	{
		m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	}
	else if (rs == RS_CULL_NONE)
	{
		m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	}
}

void ZFXD3D::SetDepthBufferMode(ZFXRENDERSTATE rs)
{
	m_pVertexMan->ForcedFlushAll();
	if (rs == RS_DEPTH_READWRITE)
	{
		m_pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
		m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	}
	else if (rs == RS_DEPTH_READONLY)
	{
		m_pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
		m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	}
	else if (rs == RS_DEPTH_NONE)
	{
		m_pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	}
}

inline DWORD FtoDW(FLOAT f) { return *((DWORD*)&f); }

void ZFXD3D::SetShadeMode(ZFXRENDERSTATE smd, float f, const ZFXCOLOR *pClr)
{
	m_pVertexMan->ForcedFlushAll();

	//copy new color if any
	if (pClr)
	{
		memcpy(&m_clrWire, pClr, sizeof(ZFXCOLOR));
		m_pVertexMan->InvalidateStates();
	}

	//no changes in mode
	if (smd == m_ShadeMode)
	{
		//but maybe a change in size?
		if (smd == RS_SHADE_POINTS)
		{
			m_pDevice->SetRenderState(D3DRS_POINTSIZE, FtoDW(f));
		}

		return;
	}

	if (smd == RS_SHADE_TRIWIRE)
	{
		//real direct3d wireframe mode
		m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		m_ShadeMode = smd;
	}
	else
	{
		if (smd != RS_SHADE_SOLID) 
		{
			m_pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
		}
		m_pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
		m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		m_ShadeMode = smd;
	}

	if (smd == RS_SHADE_POINTS)
	{
		if (f > 0.0f)
		{
			m_pDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE);
			m_pDevice->SetRenderState(D3DRS_POINTSCALEENABLE, TRUE);
			m_pDevice->SetRenderState(D3DRS_POINTSIZE, FtoDW(f));
			m_pDevice->SetRenderState(D3DRS_POINTSIZE_MIN, FtoDW(0.00f));
			m_pDevice->SetRenderState(D3DRS_POINTSCALE_A,  FtoDW(0.00f));
			m_pDevice->SetRenderState(D3DRS_POINTSCALE_B,  FtoDW(0.00f));
			m_pDevice->SetRenderState(D3DRS_POINTSCALE_C,  FtoDW(1.00f));
		}
		else
		{ 
			m_pDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
			m_pDevice->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
		}
	}
	else
	{
		m_pDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
		m_pDevice->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
	}

	//update dependent states
	m_pVertexMan->InvalidateStates();
}

ZFXRENDERSTATE ZFXD3D::GetShadeMode() { return m_ShadeMode; }