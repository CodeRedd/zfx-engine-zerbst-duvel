//File: ZFXMovementController.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#pragma once

#include <ZFX3D.h>

class ZFXMovementController
{
public:
   ZFXMovementController();
   virtual ~ZFXMovementController();

   virtual void Update(float fElapsedTime) = 0;

   //accessor methods
   ZFXVector GetPos()      {return m_vcPos; }
   ZFXVector GetRight()    { return m_vcRight; }
   ZFXVector GetUp()       { return m_vcUp; }
   ZFXVector GetDir()      { return m_vcDir; }
   ZFXVector GetVelocity() { return m_vcV; }

protected:
   ZFXVector   m_vcPos;      // position
   ZFXVector   m_vcRight;    // right vector
   ZFXVector   m_vcUp;       // up vector
   ZFXVector   m_vcDir;      // direction (forward) vector
   ZFXVector   m_vcV;        // speed
   ZFXQuat     m_Quat;       // rotation quaternion

   //rotation speed on local axis
   float       m_fRollSpd;
   float       m_fPitchSpd;
   float       m_fYawSpd;

   float       m_fRollSpdMax;
   float       m_fPitchSpdMax;
   float       m_fYawSpdMax;

   //rotation angle on local axis
   float       m_fRotX;
   float       m_fRotY;
   float       m_fRotZ;

   float       m_fThrust;

   //methods
   virtual void RecalcAxes();
   virtual void Init();



};