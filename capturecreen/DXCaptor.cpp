#include "stdafx.h"
#include "DxCaptor.h"

#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

 VideoDXCaptor::VideoDXCaptor(void)  
 { 
	 m_lpDDraw = NULL; 
	 m_lpDDSPrime = NULL; 
	 m_lpDDSBack = NULL;
 }
VideoDXCaptor::~VideoDXCaptor(void) 
{ 
	Deinit();
}

BOOL VideoDXCaptor::Init()
{
	HMODULE hDll = LoadLibrary("ddraw.dll");
	if (hDll == NULL)
	{
		("无法载入ddraw.dll\n");
		return FALSE;
	}

	// 载入ddraw的导入函数
	PFN_DirectDrawCreate DirectDrawCreateFunc = (PFN_DirectDrawCreate)GetProcAddress(hDll, "DirectDrawCreate");
	if (DirectDrawCreateFunc == NULL)
	{
		("无法找到访问点：DirectDrawCreate\n");
		return FALSE;
	}

	HRESULT hr = DirectDrawCreateFunc(NULL, &m_lpDDraw, NULL);
	if (FAILED(hr))
	{
		("DirectDrawCreate失败\n");
		return FALSE;
	}

	hr = m_lpDDraw->SetCooperativeLevel(NULL, DDSCL_NORMAL);
	if (FAILED(hr))
	{
		("SetCooperativeLevel失败\n");
		return FALSE; 
	}

	DDSURFACEDESC DDSdesc;
	ZeroMemory(&DDSdesc, sizeof(DDSdesc));
	DDSdesc.dwSize         = sizeof(DDSdesc);
	DDSdesc.dwFlags        = DDSD_CAPS;
	DDSdesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	hr = m_lpDDraw->CreateSurface(&DDSdesc, &m_lpDDSPrime, NULL);
	if (FAILED(hr))
	{
		("CreateSurface 主表面失败\n");
		return FALSE; 
	}

	ZeroMemory(&DDSdesc, sizeof(DDSdesc));
	DDSdesc.dwSize  = sizeof(DDSdesc);
	DDSdesc.dwFlags = DDSD_ALL;
	hr = m_lpDDSPrime->GetSurfaceDesc(&DDSdesc);
	if (FAILED(hr))
	{
		("GetSurfaceDesc失败\n");
		return FALSE;
	}

	// 备份描述信息
	memcpy(&m_DDSdesc, &DDSdesc, sizeof(DDSdesc));

	DDSdesc.dwFlags        = DDSD_CAPS | DDSD_HEIGHT |DDSD_WIDTH; 
	DDSdesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	hr = m_lpDDraw->CreateSurface(&DDSdesc, &m_lpDDSBack, 0);
	if (FAILED(hr))
	{
		("CreateSurface 后备表面失败\n");
		return FALSE;
	}
	return TRUE;
}
VOID VideoDXCaptor::Deinit()
{
	if (m_lpDDSBack)
	{
		m_lpDDSBack->Release();
		m_lpDDSBack = NULL;
	}

	if (m_lpDDSPrime)
	{
		m_lpDDSPrime->Release();
		m_lpDDSPrime = NULL;
	}

	if (m_lpDDraw)
	{
		m_lpDDraw->Release();
		m_lpDDraw = NULL;
	}
}

BOOL VideoDXCaptor::CaptureImage(RECT &rect, void *pData, INT &nLen)
{
	if (m_lpDDSBack == NULL)
	{
		("DDraw对象未初始化\n");
		return FALSE;
	}

	HRESULT hr = m_lpDDSBack->BltFast(rect.left, rect.top, m_lpDDSPrime, &rect, DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);
	if (FAILED(hr))
	{
		("BltFast失败\n");
		return FALSE;
	}

	DDSURFACEDESC surfDesc;
	ZeroMemory(&surfDesc, sizeof(surfDesc)); 
	surfDesc.dwSize = sizeof(surfDesc);
	//hr = lpDDSPrime->Lock(&rect, &surfdesc, DDLOCK_READONLY | DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR , NULL);
	hr = m_lpDDSBack->Lock(&rect, &surfDesc, DDLOCK_READONLY | DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR , NULL);
	if (FAILED(hr))
	{
		("Lock失败\n");
		return FALSE;
	}

	// 这里拷贝的是32位数据，我们只需要24位的RGB数据
	//memcpy(pBuf, (BYTE*)surfDesc.lpSurface, surfDesc.dwWidth * surfDesc.dwHeight * surfDesc.ddpfPixelFormat.dwRGBBitCount / 8);

	// 将抓上来的图像转成RGB数据(去掉Alpha值)
	if (m_DDSdesc.ddpfPixelFormat.dwRGBBitCount == 32)
	{
		nLen = (rect.right - rect.left) * (rect.bottom - rect.top) * 3;
		PrepareBGR24From32((BYTE*)surfDesc.lpSurface, (BYTE*)pData, rect);
	}
	else
	{
		nLen = (rect.right - rect.left) * (rect.bottom - rect.top) * 2;
		PrepareBGR24From16((BYTE*)surfDesc.lpSurface, (BYTE*)pData, rect);
	}

	//lpDDSPrime->Unlock(surfdesc.lpSurface);
	 m_lpDDSBack->Unlock(surfDesc.lpSurface);

	return TRUE;
}
BOOL VideoDXCaptor::CaptureImage(void *pData, INT &nLen)
{
	RECT rect;
	rect.left = 0; rect.top = 0; rect.right = m_iWidth; rect.bottom = m_iHeight;

	return CaptureImage(rect, pData, nLen);
}
BOOL VideoDXCaptor::ResetDevice()
{
	Deinit();
	return Init();
}