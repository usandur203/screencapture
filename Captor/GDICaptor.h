#ifndef __VIDEO_GDI_CAPTURE__
#define __VIDEO_GDI_CAPTURE__

class VideoGDICaptor
{
public:
	 VideoGDICaptor(void);
    ~VideoGDICaptor(void);

public:
    BOOL  Init();
	  void  Deinit(void);

public:
	virtual BOOL CaptureImage(RECT &rect, void *pData, INT &nLen);
	virtual BOOL CaptureImage(void *pData, INT &nLen);
	virtual BOOL ResetDevice();

private:
	int   CaptureScreen(const RECT &rect);
    BOOL  InitBitmapInfo(void);
    BOOL  CreateDIBBuffers(void);
	void  ByteAlign(RECT &rect);
	BOOL  DrawCursor();

private:
    HDC               m_hMemDC, m_hRootDC;
    HBITMAP           m_hBitmapMem;
	DEVMODE           m_origDevMode;
	BITMAPINFO        m_bitmapInfo;
	void             *m_pvBits;
	int                m_iWidth, m_iHeight;
};
#endif
