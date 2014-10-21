//File: ZFXMCFree.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#pragma once

#include "ZFXMovementController.h"

class ZFXMCFree : public ZFXMovementController
{
public:
   ZFXMCFree() { ZFXMovementController::Init(); }
   virtual ~ZFXMCFree() { }

   virtual void Update(float fElapsedTime);

   void AddRotationSpeed(float sx, float sy, float sz);
   void SetRotationSpeed(float sx, float sy, float sz);
   void SetRotationSpeedX(float f)     { m_fPitchSpd = f; }
   void SetRotationSpeedY( float f )   { m_fYawSpd = f; }
   void SetRotationSpeedZ( float f )   { m_fRollSpd = f; }
   void AddThrust(float f)             { m_fThrust += f; }
   void SetThrust(float f)             { m_fThrust = f; }

   void SetRotation(float rx, float ry, float rz);
   void SetPos(ZFXVector &vc)       { memcpy(&m_vcPos, &vc, sizeof(ZFXVector)); }
   void SetRight( ZFXVector &vc )   { memcpy( &m_vcRight, &vc, sizeof( ZFXVector ) ); }
   void SetUp( ZFXVector &vc )      { memcpy( &m_vcUp, &vc, sizeof( ZFXVector ) ); }
   void SetDir( ZFXVector &vc )     { memcpy( &m_vcDir, &vc, sizeof( ZFXVector ) ); }
};