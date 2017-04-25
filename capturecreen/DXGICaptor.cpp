#include "stdafx.h"
//#include "../stdafx.h"
#include "DXGICaptor.h"
#include <vector>

#if _MSC_VER >= VS_VERSION
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#endif

#define RESET_OBJECT(obj) { if(obj) obj->Release(); obj = NULL; }
static BOOL g_bAttach = FALSE;

// DXGI_ERROR_DEVICE_REMOVED
// DXGI_ERROR_DEVICE_REMOVED
// DXGI_ERROR_DEVICE_RESET
VideoDXGICaptor::VideoDXGICaptor()
{
	m_bInit = FALSE;

#if _MSC_VER >= VS_VERSION
	m_hDevice = NULL;
	m_hContext = NULL;
	m_hDeskDupl = NULL;

	ZeroMemory(&m_dxgiOutDesc, sizeof(m_dxgiOutDesc));
#endif
}
VideoDXGICaptor::~VideoDXGICaptor()
{
	Deinit();
}
BOOL VideoDXGICaptor::Init()
{
	HRESULT hr = S_OK;

#if _MSC_VER >= VS_VERSION
	if (m_bInit)
	{
		return FALSE;
	}

	// Driver types supported
	D3D_DRIVER_TYPE DriverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
		D3D_DRIVER_TYPE_UNKNOWN,
	};
	UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

	// Feature levels supported
	D3D_FEATURE_LEVEL FeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_1
	};
	UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);

	D3D_FEATURE_LEVEL FeatureLevel;

	IDXGIFactory1 * pFactory = NULL;

	CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&pFactory);
	UINT i = 0;
	IDXGIAdapter * pAdapter;
	std::vector <IDXGIAdapter*> vAdapters;
	while (pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		vAdapters.push_back(pAdapter);
		++i;
	}

	//
	// Create D3D device
	//
	for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
	{
		hr = D3D11CreateDevice(vAdapters[1], DriverTypes[DriverTypeIndex], NULL, 0, FeatureLevels, NumFeatureLevels, D3D11_SDK_VERSION, &m_hDevice, &FeatureLevel, &m_hContext);
		if (SUCCEEDED(hr))
		{
			break;
		}
	}
	if (FAILED(hr))
	{
		return FALSE;
	}

	//
	// Get DXGI device
	//
	IDXGIDevice *hDxgiDevice = NULL;
	hr = m_hDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&hDxgiDevice));
	if (FAILED(hr))
	{
		return FALSE;
	}

	//
	// Get DXGI adapter
	//
	IDXGIAdapter *hDxgiAdapter = NULL;
	hr = hDxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&hDxgiAdapter));
	RESET_OBJECT(hDxgiDevice);
	if (FAILED(hr))
	{
		return FALSE;
	}

	//
	// Get output
	//
	INT nOutput = 1;
	IDXGIOutput *hDxgiOutput = NULL;
	hr = hDxgiAdapter->EnumOutputs(nOutput, &hDxgiOutput);
	RESET_OBJECT(hDxgiAdapter);
	if (FAILED(hr))
	{
		return FALSE;
	}

	//
	// get output description struct
	//
	hDxgiOutput->GetDesc(&m_dxgiOutDesc);
	
	//
	// QI for Output 1
	//
	IDXGIOutput1 *hDxgiOutput1 = NULL;
	hr = hDxgiOutput->QueryInterface(__uuidof(hDxgiOutput1), reinterpret_cast<void**>(&hDxgiOutput1));
	RESET_OBJECT(hDxgiOutput);
	if (FAILED(hr))
	{
		return FALSE;
	}

	//
	// Create desktop duplication
	//
	hr = hDxgiOutput1->DuplicateOutput(m_hDevice, &m_hDeskDupl);
	RESET_OBJECT(hDxgiOutput1);
	if (FAILED(hr))
	{
		return FALSE;
	}

	// 初始化成功
	m_bInit = TRUE;
	return TRUE;
#else
    // 小于vs2012,此功能不能实现
    return FALSE;
#endif
}
VOID VideoDXGICaptor::Deinit()
{
	if (!m_bInit)
	{
		return;
	}

#if _MSC_VER >= VS_VERSION
	m_bInit = FALSE;

	if (m_hDeskDupl)
	{
		m_hDeskDupl->Release();
		m_hDeskDupl = NULL;
	}

	if (m_hDevice)
	{
		m_hDevice->Release();
		m_hDevice = NULL;
	}

	if (m_hContext)
	{
		m_hContext->Release();
		m_hContext = NULL;
	}
#endif
}
BOOL VideoDXGICaptor::AttatchToThread(VOID)
{
	if (g_bAttach)
	{
		return TRUE;
	}
	
	HDESK hCurrentDesktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
	if (!hCurrentDesktop)
	{
		return FALSE;
	}

	// Attach desktop to this thread
	BOOL bDesktopAttached = SetThreadDesktop(hCurrentDesktop);
	CloseDesktop(hCurrentDesktop);
	hCurrentDesktop = NULL;

	g_bAttach = TRUE;

	return bDesktopAttached;
}

BOOL VideoDXGICaptor::CaptureImage(RECT &rect, void *pData, INT &nLen)
{
	return QueryFrame(pData, nLen);
}
BOOL VideoDXGICaptor::CaptureImage(void *pData, INT &nLen)
{
	return QueryFrame(pData, nLen);
}
BOOL VideoDXGICaptor::ResetDevice()
{
	Deinit();
	return Init();
}

//Bitmap file headers and utilities
#pragma pack (push)
#pragma pack(1)

typedef struct {
	unsigned short    bfType;
	unsigned int   bfSize;
	unsigned short    bfReserved1;
	unsigned short    bfReserved2;
	unsigned int   bfOffBits;
} BITMAPFILEHEADER_OWN;


typedef struct {
	unsigned int      biSize;
	int       biWidth;
	int       biHeight;
	unsigned short       biPlanes;
	unsigned short       biBitCount;
	unsigned int      biCompression;
	unsigned int      biSizeImage;
	int       biXPelsPerMeter;
	int       biYPelsPerMeter;
	unsigned int      biClrUsed;
	unsigned int      biClrImportant;
}  BITMAPINFOHEADER_OWN;

#pragma pack(pop)


bool SaveImageAsBMP(unsigned char* ptr, int width, int height, const char* fileName, int channelsize, unsigned char *bmphead, int headsize)
{
	FILE *stream;
	char* ppix = (char*)ptr;
	int pixel_size = channelsize;
	printf("Save Image: %s \n", fileName);
	fopen_s(&stream,fileName, "wb");

//	if (NULL == stream)
	//	return false;

	int alignSize = width * pixel_size;
	alignSize ^= 0x03;
	alignSize++;
	alignSize &= 0x03;

	int rowLength = width * pixel_size + alignSize;

	if (bmphead) {
		if (headsize != fwrite(bmphead, 1, headsize, stream)) {
			// cann't write BITMAPFILEHEADER
			goto ErrExit;
		}
	}
	else {
		BITMAPFILEHEADER_OWN fileHeader;
		BITMAPINFOHEADER_OWN infoHeader;



		fileHeader.bfReserved1 = 0x0000;
		fileHeader.bfReserved2 = 0x0000;

		infoHeader.biSize = sizeof(BITMAPINFOHEADER_OWN);
		infoHeader.biWidth = width;
		infoHeader.biHeight = height;
		infoHeader.biPlanes = 1;
		infoHeader.biBitCount = pixel_size * 8;
		infoHeader.biCompression = 0L;//BI_RGB;
		infoHeader.biSizeImage = rowLength * height;
		infoHeader.biXPelsPerMeter = 0;
		infoHeader.biYPelsPerMeter = 0;
		infoHeader.biClrUsed = 0; // max available
		infoHeader.biClrImportant = 0; // !!!
		fileHeader.bfType = 0x4D42;
		fileHeader.bfSize = sizeof(BITMAPFILEHEADER_OWN) + sizeof(BITMAPINFOHEADER_OWN) + rowLength * height;
		fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER_OWN) + sizeof(BITMAPINFOHEADER_OWN);

		if (sizeof(BITMAPFILEHEADER_OWN) != fwrite(&fileHeader, 1, sizeof(BITMAPFILEHEADER_OWN), stream)) {
			// cann't write BITMAPFILEHEADER
			goto ErrExit;
		}

		if (sizeof(BITMAPINFOHEADER_OWN) != fwrite(&infoHeader, 1, sizeof(BITMAPINFOHEADER_OWN), stream)) {
			// cann't write BITMAPINFOHEADER_OWN
			goto ErrExit;
		}
	}

	unsigned char buffer[4];
	int x, y;

	for (y = 0; y< height; y++)
	{
		for (x = 0; x<width; x++)
		{
			if (pixel_size != fwrite(ppix + (((height - 1 - y)*width + x) * pixel_size), 1, pixel_size, stream)) {
				goto ErrExit;
			}
		}
		memset(buffer, 0x00, pixel_size);

		fwrite(buffer, 1, alignSize, stream);
	}

	fclose(stream);
	return true;
ErrExit:
	fclose(stream);
	return false;
}

BOOL VideoDXGICaptor::QueryFrame(void *pImgData, INT &nImgSize)
{
#if _MSC_VER >= VS_VERSION
	if (!m_bInit || !AttatchToThread())
	{
		return FALSE;
	}

	nImgSize = 0;

	IDXGIResource *hDesktopResource = NULL;
	DXGI_OUTDUPL_FRAME_INFO FrameInfo;
	HRESULT hr = m_hDeskDupl->AcquireNextFrame(500, &FrameInfo, &hDesktopResource);
	if (FAILED(hr))
	{
		//
		// 在一些win10的系统上,如果桌面没有变化的情况下，
		// 这里会发生超时现象，但是这并不是发生了错误，而是系统优化了刷新动作导致的。
		// 所以，这里没必要返回FALSE，返回不带任何数据的TRUE即可
		//
		return TRUE;
	}

	//
	// query next frame staging buffer
	//
	ID3D11Texture2D *hAcquiredDesktopImage = NULL;
	hr = hDesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&hAcquiredDesktopImage));
	RESET_OBJECT(hDesktopResource);
	if (FAILED(hr))
	{
		return FALSE;
	}

	//
	// copy old description
	//
	D3D11_TEXTURE2D_DESC frameDescriptor;
	hAcquiredDesktopImage->GetDesc(&frameDescriptor);

	//
	// create a new staging buffer for fill frame image
	//
	ID3D11Texture2D *hNewDesktopImage = NULL;
	frameDescriptor.Usage = D3D11_USAGE_STAGING;
	frameDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	frameDescriptor.BindFlags = 0;
	frameDescriptor.MiscFlags = 0;
	frameDescriptor.MipLevels = 1;
	frameDescriptor.ArraySize = 1;
	frameDescriptor.SampleDesc.Count = 1;
	hr = m_hDevice->CreateTexture2D(&frameDescriptor, NULL, &hNewDesktopImage);
	if (FAILED(hr))
	{
		RESET_OBJECT(hAcquiredDesktopImage);
		m_hDeskDupl->ReleaseFrame();
		return FALSE;
	}

	//
	// copy next staging buffer to new staging buffer
	//
	m_hContext->CopyResource(hNewDesktopImage, hAcquiredDesktopImage);

	RESET_OBJECT(hAcquiredDesktopImage);
	m_hDeskDupl->ReleaseFrame();

	//
	// create staging buffer for map bits
	//
	IDXGISurface *hStagingSurf = NULL;
	hr = hNewDesktopImage->QueryInterface(__uuidof(IDXGISurface), (void **)(&hStagingSurf));
	RESET_OBJECT(hNewDesktopImage);
	if (FAILED(hr))
	{
		return FALSE;
	}

	//
	// copy bits to user space
	//
	DXGI_MAPPED_RECT mappedRect;
	hr = hStagingSurf->Map(&mappedRect, DXGI_MAP_READ);
	if (SUCCEEDED(hr))
	{
	//	nImgSize = 1920*1080*3;// GetWidth() * GetHeight() * 3;
	//	PrepareBGR24From32(mappedRect.pBits, (BYTE*)pImgData, m_dxgiOutDesc.DesktopCoordinates);
		char filename[100] = { 0 };
		double tt = GetTickCount();
		sprintf_s(filename,"%d-f.bmp",tt);
		SaveImageAsBMP((unsigned char*)mappedRect.pBits,1920,1080,filename,4,NULL,0);
		hStagingSurf->Unmap();
	}

	RESET_OBJECT(hStagingSurf);
	return SUCCEEDED(hr);
#else
	return FALSE;
#endif
}