//File: ZFXMCFirstPerson.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#pragma once

#include "ZFXMovementController.h"

class ZFXMCFirstPerson : public ZFXMovementController
{
public:
   ZFXMCFirstPerson();
   virtual ~ZFXMCFirstPerson();

   virtual void Update(float fElapsedTime);

   void        GetRotation(float *pfX, float *pfY, float *pfZ);
   ZFXVector   GetRotation();

   void SetRotationSpeedX(float f)  { m_fPitchSpd = f; }
   void SetRotationSpeedY(float f)  { m_fYawSpd = f; }
   void SetSpeed(float a)           { m_fSpeed = a; }
   void SetStrafeSpeed(float a)      { m_fStrafe = a; }

   void SetRotation(float rx, float ry, float rz);
   void SetPos(ZFXVector &vc)       { memcpy(&m_vcPos, &vc, sizeof(ZFXVector)); }
   void SetRight( ZFXVector &vc )   { memcpy( &m_vcRight, &vc, sizeof( ZFXVector ) ); }
   void SetUp( ZFXVector &vc )      { memcpy( &m_vcUp, &vc, sizeof( ZFXVector ) ); }
   void SetDir( ZFXVector &vc )     { memcpy( &m_vcDir, &vc, sizeof( ZFXVector ) ); }

private:
   float m_fSpeed;   //forward speed
   float m_fStrafe;  //sideways speed

   void RecalcAxes();
};