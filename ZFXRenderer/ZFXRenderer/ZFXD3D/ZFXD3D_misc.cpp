//File: ZFXD3D_misc.cpp 
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd


#include "math.h"          // sqrt function
#include "ZFXD3D.h"        // class definition
#include "D3D9.h"         // shader compiler

extern bool g_bLF;


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