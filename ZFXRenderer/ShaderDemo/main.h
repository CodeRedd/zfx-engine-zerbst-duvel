//File: main.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

//Note: Authors admit that this code is meant to be a quick and dirty demo to show the shaders working, and could be vastly improved
//with OO techniques like smart class encapsulation, etc.

#pragma once

#include <Windows.h>
#include <ZFX3D.h>
#include <ZFX.h>

LRESULT   WINAPI MsgProc(HWND, UINT, WPARAM, LPARAM);
HRESULT   ProgramStartup(wchar_t *chAPI);
HRESULT   ProgramCleanup();
HRESULT   ProgramTick();
HRESULT   Render(int);
HRESULT   BuildAndSetShader();
HRESULT   BuildGeometry();
void      CreateCube(ZFXVector, float, float, float, TVERTEX*, WORD*, bool);
wchar_t*    HrToStr(HRESULT hr);