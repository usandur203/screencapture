#ifndef __DXGI_SCREEN_CAPTURE_H__
#define __DXGI_SCREEN_CAPTURE_H__

// vs2010 = 1600
// vs2012 = 1700
// vs2013 = 1800
// ......
#define VS_VERSION 1700

// 必须在vs2012以上的版本才可以使用
#if _MSC_VER >= VS_VERSION 
#include <d3d11.h>
#include <dxgi1_2.h>
#endif

//
// 使用dxgi技术方式抓屏
// 适用于win8以后的操作系统,win7以前的系统无法使用此接口
// 抓屏速度会有质的飞跃
// add by yshen on 2016/03/25
// 注意：不支持抓局部屏，只能是全屏
//

class VideoDXGICaptor
{
public:
	VideoDXGICaptor();
	~VideoDXGICaptor();

public:
	BOOL Init();
	VOID Deinit();

public:
	virtual BOOL CaptureImage(RECT &rect, void *pData, INT &nLen);
	virtual BOOL CaptureImage(void *pData, INT &nLen);
	virtual BOOL ResetDevice();

private:
	BOOL  AttatchToThread(VOID);
	BOOL  QueryFrame(void *pImgData, INT &nImgSize);

private:
	BOOL                    m_bInit;
	int                     m_iWidth, m_iHeight;

#if _MSC_VER >= VS_VERSION
	ID3D11Device           *m_hDevice;
	ID3D11DeviceContext    *m_hContext;

	IDXGIOutputDuplication *m_hDeskDupl;
	DXGI_OUTPUT_DESC        m_dxgiOutDesc;
#endif
};
#endif