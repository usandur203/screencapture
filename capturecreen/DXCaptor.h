#ifndef __DX_CAPTURE_H__
#define __DX_CAPTURE_H__
#include <ddraw.h>

//
// ʹ��ddraw��ʽץ��
// ����ͨGDI��ʽҪ��5��������(1920*1080�ķֱ�����)
// add by yshen on 2015/12/8
//

// ����DDraw�ĵ�������
typedef HRESULT (WINAPI *PFN_DirectDrawCreate)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);

class VideoDXCaptor
{  
public:  
    VideoDXCaptor(void);
	~VideoDXCaptor(void);

public:
	BOOL Init();
	VOID Deinit();

public:
	virtual BOOL CaptureImage(RECT &rect, void *pData, INT &nLen);
	virtual BOOL CaptureImage(void *pData, INT &nLen);
	virtual BOOL ResetDevice();

private:
	LPDIRECTDRAW        m_lpDDraw;
	LPDIRECTDRAWSURFACE m_lpDDSPrime;
	LPDIRECTDRAWSURFACE m_lpDDSBack;

	DDSURFACEDESC       m_DDSdesc;
	int                 m_iWidth, m_iHeight;
};
#endif