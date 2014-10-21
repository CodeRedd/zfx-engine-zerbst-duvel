//File: ZFXMCFirstPerson.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFXMCFirstPerson.h"

void ZFXMCFirstPerson::SetRotation( float rx, float ry, float rz )
{
   m_fRotX = rx;
   m_fRotY = ry;
   m_fRotZ = rz;
   RecalcAxes();
}

void ZFXMCFirstPerson::GetRotation( float *pfX, float *pfY, float *pfZ )
{
   if ( pfX )
   {
      *pfX = m_fRotX;
   }

   if ( pfY )
   {
      *pfY = m_fRotY;
   }

   if ( pfZ )
   {
      *pfZ = m_fRotZ;
   }
}

ZFXVector ZFXMCFirstPerson::GetRotation()
{
   return ZFXVector(m_fRotX, m_fRotY, m_fRotZ);
}

void ZFXMCFirstPerson::RecalcAxes()
{
   ZFXMatrix mat;

   static float f2PI = 6.283185f;

   //keep horiz. rotation within 360-degree bound
   if ( m_fRotY > f2PI )
   {
      m_fRotY -= f2PI;
   }
   else if ( m_fRotY < -f2PI )
   {
      m_fRotY += f2PI;
   }

   //cap the vertical rotation to 80 degrees
   if ( m_fRotY > 1.4f )
   {
      m_fRotY = 1.4f;
   }
   else if ( m_fRotY < -1.4f )
   {
      m_fRotY = -1.4f;
   }

   //initializing axes
   m_vcRight = ZFXVector(1.0f, 0.0f, 0.0f);
   m_vcUp    = ZFXVector(0.0f, 1.0f, 0.0f);
   m_vcDir   = ZFXVector(0.0f, 0.0f, 1.0f);

   //rotate around y-axis first
   mat.RotaArbi(m_vcRight, m_fRotY);
   m_vcRight = m_vcRight * mat;
   m_vcDir   = m_vcDir * mat;

   //rotate around x-axis next
   mat.RotaArbi(m_vcRight, m_fRotX);
   m_vcUp   = m_vcUp * mat;
   m_vcDir  = m_vcDir * mat;

   //reconstruct our coordinate system to correct for rounding errors
   m_vcDir.Normalize();
   m_vcRight.Cross(m_vcUp, m_vcDir);
   m_vcRight.Normalize();
   m_vcUp.Cross(m_vcDir, m_vcRight);
   m_vcUp.Normalize();
}

void ZFXMCFirstPerson::Update( float fElapsedTime )
{
   ZFXVector vcS;

   //add rotation speed
   m_fRotX += (m_fPitchSpd * fElapsedTime );
   m_fRotY += ( m_fYawSpd * fElapsedTime );
   m_fRotZ += ( m_fRollSpd * fElapsedTime );

   //calculate axis
   RecalcAxes();

   //calculate final velocity vectors
   m_vcV = m_vcDir * m_fSpeed * fElapsedTime;
   vcS   = m_vcRight * m_fStrafe * fElapsedTime;

   //adjust position
   m_vcPos += m_vcV + vcS;
}