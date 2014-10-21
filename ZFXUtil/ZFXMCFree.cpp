//File: ZFXMCFree.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFXMCFree.h"

void ZFXMCFree::SetRotation( float rx, float ry, float rz )
{
   m_fRotX = rx;
   m_fRotY = ry;
   m_fRotZ = rz;
   RecalcAxes();
}

void ZFXMCFree::AddRotationSpeed( float sx, float sy, float sz )
{
   m_fPitchSpd += sx;
   m_fYawSpd   += sy;
   m_fRollSpd  += sz;
}

void ZFXMCFree::SetRotationSpeed( float sx, float sy, float sz )
{
   m_fPitchSpd = sx;
   m_fYawSpd   = sy;
   m_fRollSpd  = sz;
}

void ZFXMCFree::Update( float fElapsedTime )
{
   //add rotation speed
   m_fRotX = (m_fPitchSpd * fElapsedTime );
   m_fRotY = ( m_fYawSpd * fElapsedTime );
   m_fRotZ = ( m_fRollSpd * fElapsedTime );

   //recalculate speed
   m_vcV = m_vcDir * m_fThrust * fElapsedTime;

   //adjust position
   m_vcPos += m_vcV;

   //recalculate axes
   RecalcAxes();
}