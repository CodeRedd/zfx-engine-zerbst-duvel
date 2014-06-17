//File: main.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

//Note: Authors admit that this code is meant to be a quick and dirty demo to show the shaders working, and could be vastly improved
//with OO techniques like smart class encapsulation, etc.

#pragma once

#include <Windows.h>
#include <ZFX3D.h>
#include <ZFX.h>

struct TANGENTVERTEX {
	ZFXVector	vcPos;	//vertex position
	ZFXVector	vcN;	//vertex normal
	float		fu;		//vertex u-coord
	float		fv;		//vertex v-coord
	ZFXVector	vcU;	//new tangent vector
	ZFXVector	vcV;	//new binormal vector
	ZFXVector	vcUxV;	//new tangent normal
};

LRESULT   WINAPI MsgProc(HWND, UINT, WPARAM, LPARAM);
HRESULT   ProgramStartup(wchar_t *chAPI);
HRESULT   ProgramCleanup();
HRESULT   ProgramTick();
HRESULT   Render(int);
HRESULT	  Render(ZFXVector);
HRESULT   RenderLight(float tx, float ty, float tz);
HRESULT   BuildAndSetShader();
HRESULT   BuildGeometry();
void      CreateCube(ZFXVector, float, float, float, TVERTEX*, WORD*, bool);
ZFXMatrix CalcTransAttenNoRot(const ZFXVector &vcPos, float fRadius);
void	  CalcTangentSpace(TANGENTVERTEX *tv1, TANGENTVERTEX *tv2, TANGENTVERTEX *tv3);
wchar_t*    HrToStr(HRESULT hr);