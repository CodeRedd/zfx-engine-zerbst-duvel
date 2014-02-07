//File: ZFXRenderer.h
//Created by Stefan Zerbst and Oliver Duvel
//Reimplemented by Culver Redd
#include "ZFXRenderDevice.h"


//Header for the static library that will load the rendering DLL 
class ZFXRenderer
	{
	public:
		ZFXRenderer(HINSTANCE hInst);
		~ZFXRenderer();

		HRESULT				CreateDevice(TCHAR *chAPI);
		void				Release();
		LPZFXRENDERDEVICE	GetDevice() { return m_pDevice; }
		HINSTANCE			GetModule() { return m_hDLL; }


	private:
		ZFXRenderDevice		*m_pDevice;
		HINSTANCE			m_hInst;
		HMODULE				m_hDLL;
	};
typedef struct ZFXRenderer *LPZFXRENDERER;
