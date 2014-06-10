//File: ZFX3D_CPU.cpp
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd

#include "ZFX3D.h"

//global SSE support var
bool g_bSSE = false;

bool ZFX3DInitCPU()
{
	CPUINFO info = GetCPUInfo();
	bool	bOS = OSSupportsSSE;

	g_bSSE = info.bSSE && bOS;

	return g_bSSE;
}

//Uses assembly language to query for info about the CPU and what SIMD tech it supports
CPUINFO GetCPUInfo()
{
	CPUINFO info;
	wchar_t *pStr = info.vendor;
	int n = 1;
	int *pn = &n;

	memset(&info, 0, sizeof(CPUINFO));

	//1: Vendor name, SSE2, SSE, MMX Support
	__try
	{
		__asm
		{
			mov eax, 0			//get vendor name
				CPUID

				mov	esi, pStr
				mov[esi], ebx		//first 4 chars
				mov[esi + 4], edx	//next 4 chars
				mov[esi + 8], ecx	//final 4 chars

				mov	eax, 1			//Feature List
				CPUID


				test edx, 04000000h //Test SSE2
				jz	 _NOSSE2		//Jump if negative
				mov[info.bSSE2], 1 //true

			_NOSSE2: test edx, 02000000h	//Test SSE
					 jz _NOSSE				//Jump if negative
					 mov[info.bSSE], 1	//true

				 _NOSSE : test edx, 00800000h		//test MMX
						  jz _EXIT1				//Jump if negative
						  mov[info.bMMX], 1		//true
					  _EXIT1 : //done
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		if (_exception_code() == STATUS_ILLEGAL_INSTRUCTION)
		{
			return info;	//CPU Inactive
		}
		return info;		//Unexpected error
	}

	//2: Test Extended Features
	__asm
	{
		mov eax, 80000000h		//Extended Features?
			CPUID
			cmp eax, 80000000h		//> 0x80?
			jbe _EXIT2				//Jump if negative
			mov[info.bEXT], 1		//true

			mov eax, 80000001h		//Feat-Bits to EDX
			CPUID
			test edx, 80000000h		//test 3DNow!
			jz _EXIT2				//Jump if negative
			mov[info.b3DNOW], 1	//true
		_EXIT2: //done
	}

	//3: vendor dependent things
	//Intel: CPU id
	//AMD: CPU id, 3dnow_ex, mmx_ex
	if (wcsncmp(info.vendor, L"GenuineIntel", 12) == 0 && info.bEXT)
	{
		__asm
		{
			mov eax, 1		//Feature List
				CPUID
				mov esi, pn		//Brand ID
				mov[esi], ebx
		}
		int m = 0;
		memcpy(&m, pn, sizeof(wchar_t)); //copy only the lower 8 bits
		n = m;
	}
	else if (wcsncmp(info.vendor, L"AuthenticAMD", 12) == 0 && info.bEXT)
	{
		__asm
		{
			mov eax, 1				//Feature List
				CPUID
				mov esi, pn				//CPU Type
				mov[esi], eax

				mov eax, 0x80000001		//Extra Feature Bits
				CPUID

				test edx, 0x40000000	//AMD Extended 3DNow!
				jz _AMD1				//Jump on error
				mov[info.b3DNOWEX], 1	//true
			_AMD1: test edx, 0x00400000 //AMD Extended MMX
				   jz _AMD2				//jump if negative
				   mov[info.bMMXEX], 1	//true
			   _AMD2 ://done
		}
	}
	else
	{
		if (info.bEXT)
		{
			; /*UNKNOWN ERROR*/
		}
		else
		{
			; /*NO Extended Feature List*/
		}
	}

	info.vendor[13] = '\0';
	GetCPUName(info.name, n, info.vendor);
	return info;
}

/**
* Get name to found processor id. Note that this method is not
* accurate in some cases. Never use in official release.
* TODO: Will have to update this with modern processor names
*/
void GetCPUName(wchar_t *chName, int n, const wchar_t *vendor)
{
	// Intel processors
	if (wcsncmp(vendor, L"GenuineIntel", 12) == 0) {
		switch (n) {
		case 0: {
			wsprintf(chName, L"< Pentium III/Celeron");
		} break;
		case 1: {
			wsprintf(chName, L"Pentium Celeron (1)");
		} break;
		case 2: {
			wsprintf(chName, L"Pentium III (2)");
		} break;
		case 3: {
			wsprintf(chName, L"Pentium III Xeon/Celeron");
		} break;
		case 4: {
			wsprintf(chName, L"Pentium III (4)");
		} break;
		case 6: {
			wsprintf(chName, L"Pentium III-M");
		} break;
		case 7: {
			wsprintf(chName, L"Pentium Celeron (7)");
		} break;
		case 8: {
			wsprintf(chName, L"Pentium IV (Genuine)");
		} break;
		case 9: {
			wsprintf(chName, L"Pentium IV");
		} break;
		case 10: {
			wsprintf(chName, L"Pentium Celeron (10)");
		} break;
		case 11: {
			wsprintf(chName, L"Pentium Xeon / Xeon-MP");
		} break;
		case 12: {
			wsprintf(chName, L"Pentium Xeon-MP");
		} break;
		case 14: {
			wsprintf(chName, L"Pentium IV-M / Xeon");
		} break;
		case 15: {
			wsprintf(chName, L"Pentium Celeron (15)");
		} break;

		default: { wsprintf(chName, L"Unknown Intel"); break; }
		}
	}
	// AMD processors
	else if (wcsncmp(vendor, L"AuthenticAMD", 12) == 0) {
		switch (n) {
		case 1660: {
			wsprintf(chName, L"Athlon / Duron (Model-7)");
		} break;
		case 1644: {
			wsprintf(chName, L"Athlon / Duron (Model-6)");
		} break;
		case 1596: {
			wsprintf(chName, L"Athlon / Duron (Model-3)");
		} break;
		case 1612: {
			wsprintf(chName, L"Athlon (Model-4)");
		} break;
		case 1580: {
			wsprintf(chName, L"Athlon (Model-2)");
		} break;
		case 1564: {
			wsprintf(chName, L"Athlon (Model-1)");
		} break;
		case 1463: {
			wsprintf(chName, L"K6-III (Model-9)");
		} break;
		case 1420: {
			wsprintf(chName, L"K6-2 (Model-8)");
		} break;
		case 1404: {
			wsprintf(chName, L"K6 (Model-7)");
		} break;
		case 1388: {
			wsprintf(chName, L"K6 (Model-6)");
		} break;
		case 1340: {
			wsprintf(chName, L"K5 (Model-3)");
		} break;
		case 1324: {
			wsprintf(chName, L"K5 (Model-2)");
		} break;
		case 1308: {
			wsprintf(chName, L"K5 (Model-1)");
		} break;
		case 1292: {
			wsprintf(chName, L"K5 (Model-0)");
		} break;

		default: { wsprintf(chName, L"Unknown AMD"); break; }
		}
	}
	return;
}

//Runs a test instruction to determine if the OS supports SSE SIMD technology
bool OSSupportsSSE()
{
	__try
	{
		_asm xorps xmm0, xmm0
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		if (_exception_code() == STATUS_ILLEGAL_INSTRUCTION)
		{
			return false; //SSE not supported
		}
		return false; //unexpected error
	}

	return true;
}