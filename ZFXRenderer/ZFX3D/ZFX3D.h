//File: ZFX3D.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include <Windows.h>


//CPU Info Stuff
typedef struct CPUINFO_TYPE {
	bool bSSE;			//Streaming SIMD Extensions
	bool bSSE2;			//Streaming SIMD Extensions 2
	bool b3DNOW;		//3DNow! (vendor independent)
	bool bMMX;			//MMX support
	TCHAR name[48];		//cpu name
	bool bEXT;			//extended features available
	bool bMMXEX;		//MMX (AMD specific extensions)
	bool b3DNOWEX;		//3DNow! (AMD specific extensions)
	TCHAR vendor[13];	//vendor name
}	CPUINFO;

CPUINFO GetCPUInfo();
void GetCPUName(TCHAR *chName, int n, const TCHAR *vendor);
bool OSSupportsSSE();
bool ZFX3DInitCPU();