#ifndef __VIDEO_GDI_CAPTURE__
#define __VIDEO_GDI_CAPTURE__

class VideoGDICaptor
{
public:
	VideoGDICaptor(void);
	~VideoGDICaptor(void);
public:
	bool  Init();
	void  Deinit(void);

public:
	virtual bool CaptureImage(RECT &rect, void *pData, INT &nLen);
	virtual bool CaptureImage(void *pData, INT &nLen);
	virtual bool ResetDevice();

private:
	int   CaptureScreen(const RECT &rect);
    bool  InitBitmapInfo(void);
    bool  CreateDIBBuffers(void);
	void  ByteAlign(RECT &rect);
	bool  DrawCursor();

private:
    HDC               m_hMemDC, m_hRootDC;
    HBITMAP           m_hBitmapMem;
	DEVMODE           m_origDevMode;
	BITMAPINFO        m_bitmapInfo;
	void             *m_pvBits;
	int                m_iWidth, m_iHeight;
};
#endif
