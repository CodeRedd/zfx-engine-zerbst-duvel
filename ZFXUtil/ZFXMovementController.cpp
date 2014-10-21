//File: ZFXMovementController.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFXMovementController.h"

ZFXMovementController::ZFXMovementController()
{
   Init();
}

ZFXMovementController::~ZFXMovementController()
{

}

void ZFXMovementController::Init()
{
   m_vcPos.Set(0.0f, 0.0f, 0.0f);
   m_vcRight.Set(1.0f, 0.0f, 0.0f);
   m_vcUp.Set(0.0f, 1.0f, 0.0f);
   m_vcDir.Set(0.0f, 1.0f, 0.0f);
   m_vcV.Set(0.0f, 0.0f, 0.0f);
   m_fRotX = m_fRotY = m_fRotZ = m_fThrust = 0.0f;
   m_fRollSpd = m_fPitchSpd = m_fYawSpd = 0.0f;
   m_Quat.x = m_Quat.y = m_Quat.z = 0.0f;
   m_Quat.w = 1.0f;
}

void ZFXMovementController::RecalcAxes()
{
   ZFXQuat     qFrame;
   ZFXMatrix   mat;

   static float f2PI = 6.283185f;

   //stay inside 360-degree range
   if ( m_fRotX > f2PI )
   {
      m_fRotX -= f2PI;
   }
   else if ( m_fRotX < -f2PI )
   {
      m_fRotX += f2PI;
   }

   if ( m_fRotY > f2PI )
   {
      m_fRotY -= f2PI;
   }
   else if ( m_fRotY < -f2PI )
   {
      m_fRotY += f2PI;
   }

   if ( m_fRotZ > f2PI )
   {
      m_fRotZ -= f2PI;
   }
   else if ( m_fRotZ < -f2PI )
   {
      m_fRotZ += f2PI;
   }

   //build a new quaternion
   qFrame.CreateFromEuler(m_fRotX, m_fRotY, m_fRotZ);

   //add to current rotation by multiplying quats
   m_Quat *= qFrame;

   //extract local axis
   m_Quat.GetMatrix(&mat);

   m_vcRight.x = mat._11;
   m_vcRight.y = mat._21;
   m_vcRight.z = mat._31;

   m_vcUp.x    = mat._12;
   m_vcUp.y    = mat._22;
   m_vcUp.z    = mat._32;

   m_vcDir.x   = mat._13;
   m_vcDir.y   = mat._23;
   m_vcDir.z   = mat._33;
}