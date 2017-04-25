#include "../stdafx.h"
#include <stdio.h>
#include "VideoGDICaptor.h"

#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

VideoGDICaptor::VideoGDICaptor(void)
{
    m_hMemDC      = NULL;
    m_hRootDC     = NULL;
    m_hBitmapMem  = NULL;
	m_pvBits      = NULL;
	m_iWidth      = 0;
	m_iHeight     = 0;
}
VideoGDICaptor::~VideoGDICaptor(void)
{
	Deinit();
}
BOOL VideoGDICaptor::Init(void)
{
	if (!InitBitmapInfo())
	{
        ("Fail to init bitmap!\n");
        return FALSE;
    }

    if (!CreateDIBBuffers())
	{
        ("Fail to create screen buffer!\n");
        return FALSE;
    }

    return TRUE;
}
void VideoGDICaptor::Deinit()
{
	if (m_hRootDC != NULL)
	{
		DeleteDC(m_hRootDC);
		m_hRootDC = NULL;
	}

	if (m_hMemDC != NULL)
	{
		DeleteDC(m_hMemDC);
		m_hMemDC = NULL;
	}

	if (m_hBitmapMem != NULL) 
	{
		DeleteObject(m_hBitmapMem);
		m_hBitmapMem = NULL;
	}
}
BOOL VideoGDICaptor::InitBitmapInfo(void)
{ 
	if (m_hRootDC)
	{
		("Root HDC has been existed!\n");
		return FALSE;
	}

	m_hRootDC = CreateDC(("DISPLAY"), NULL, NULL, NULL);
	if (!m_hRootDC)
	{
		("Fail to get root HDC!\n");
		return FALSE;
	}

	m_hMemDC = CreateCompatibleDC(m_hRootDC);
    if (!m_hMemDC)
	{
        ("Create compatible HDC from root error!\n");
        return FALSE;
    }

    m_hBitmapMem = CreateCompatibleBitmap(m_hRootDC, m_iWidth, m_iHeight);
    if (!m_hBitmapMem)
	{
        ("Create bitmap error!\n");
        return FALSE;
    }

    return TRUE;
}
BOOL VideoGDICaptor::CreateDIBBuffers(void)
{
	m_bitmapInfo.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
    m_bitmapInfo.bmiHeader.biBitCount = 0;
    if (GetDIBits(m_hMemDC, m_hBitmapMem, 0, 1, NULL, &m_bitmapInfo, DIB_RGB_COLORS) == 0)
	{
        ("Can not get bitmap format!\n");
        return FALSE;
    }

	
	m_bitmapInfo.bmiHeader.biCompression = BI_RGB;
	m_bitmapInfo.bmiHeader.biHeight      = -abs(m_bitmapInfo.bmiHeader.biHeight);
    HBITMAP hBitmapTmp = CreateDIBSection(m_hMemDC, &m_bitmapInfo, DIB_RGB_COLORS, &m_pvBits, NULL, 0);
    if (!hBitmapTmp)
	{
        ("Can not use fast blit!\n");
	}

    if (m_hBitmapMem != NULL)
	{
        DeleteObject(m_hBitmapMem);
        m_hBitmapMem = NULL;
    }
	m_hBitmapMem = hBitmapTmp;
    return TRUE;
}
int VideoGDICaptor::CaptureScreen(const RECT &rect)
{
	// 检测和启动时的分辨率是否一致，如果不一致，则停止抓屏
	INT nWidth  = GetSystemMetrics(SM_CXSCREEN);
    INT nHeight = GetSystemMetrics(SM_CYSCREEN);
	if ((m_iWidth != nWidth) && (m_iHeight != nHeight))
	{
		return -1;
	}

	GdiFlush();
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(m_hMemDC, m_hBitmapMem);
	if (hOldBitmap)
	{
		BitBlt(m_hMemDC, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, m_hRootDC, rect.left, rect.top, SRCCOPY | CAPTUREBLT);
		SelectObject(m_hMemDC, hOldBitmap);
	}
	return 0;
}
BOOL VideoGDICaptor::CaptureImage(RECT &rect, void *pData, INT &nLen)
{
	// 对齐处理
	ByteAlign(rect);
	nLen = m_bitmapInfo.bmiHeader.biWidth * m_bitmapInfo.bmiHeader.biHeight * 3;

	CaptureScreen(rect);
	//拷贝RGB数据(位图数据)
	if (m_bitmapInfo.bmiHeader.biBitCount == 32)
	{
		PrepareBGR24From32((BYTE*)m_pvBits, (BYTE*)pData, rect);
	}
	else
	{
		PrepareBGR24From16((BYTE*)m_pvBits, (BYTE*)pData, rect);
	}

	return TRUE;
}
BOOL VideoGDICaptor::CaptureImage(void *pData, INT &nLen)
{
	RECT rect;
	rect.left = 0; rect.top = 0; rect.right = m_iWidth; rect.bottom = m_iHeight;
	
	return CaptureImage(rect, pData, nLen);
}
BOOL VideoGDICaptor::ResetDevice()
{
	Deinit();
	return Init();
}
void  VideoGDICaptor::ByteAlign(RECT &rect)
{
	int nWidth = rect.right - rect.left;
	int nHeight = rect.bottom - rect.top;

	//
	// 对齐4字节边界
	// 因为GDI的原因，分辨率必须要是4字节的边界，否则抓出来的图像重新渲染的话，
	// 会出现花屏现象，这里处理的目的就是为了保证是4字节的边界。——yshen on 2014-11-25
	//

	int nMod = nWidth % 4;
	if (nMod)
	{
		rect.left  += (nMod / 2);
		rect.right -= (nMod / 2);
	}

	nMod = nHeight % 4;
	if (nMod)
	{
		rect.top    += (nMod / 2);
		rect.bottom -= (nMod / 2);
	}
}
BOOL VideoGDICaptor::DrawCursor()
{
	HCURSOR hCursor = GetCursor();
    POINT ptCursor;
    GetCursorPos(&ptCursor);

    ICONINFO IconInfo = { 0 };
    if(GetIconInfo(hCursor, &IconInfo))
    {
        ptCursor.x -= IconInfo.xHotspot;
        ptCursor.y -= IconInfo.yHotspot;
        if(NULL != IconInfo.hbmMask)
		{
            DeleteObject(IconInfo.hbmMask);
		}

        if(NULL != IconInfo.hbmColor)
		{
            DeleteObject(IconInfo.hbmColor);
		}
	}
    return DrawIconEx(m_hMemDC, ptCursor.x, ptCursor.y, hCursor, 0, 0, 0, NULL, DI_NORMAL | DI_COMPAT);
}