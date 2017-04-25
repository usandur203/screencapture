#ifndef __DXGI_SCREEN_CAPTURE_H__
#define __DXGI_SCREEN_CAPTURE_H__

// vs2010 = 1600
// vs2012 = 1700
// vs2013 = 1800
// ......
#define VS_VERSION 1700

// ������vs2012���ϵİ汾�ſ���ʹ��
#if _MSC_VER >= VS_VERSION 
#include <d3d11.h>
#include <dxgi1_2.h>
#endif

//
// ʹ��dxgi������ʽץ��
// ������win8�Ժ�Ĳ���ϵͳ,win7��ǰ��ϵͳ�޷�ʹ�ô˽ӿ�
// ץ���ٶȻ����ʵķ�Ծ
// add by yshen on 2016/03/25
// ע�⣺��֧��ץ�ֲ�����ֻ����ȫ��
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