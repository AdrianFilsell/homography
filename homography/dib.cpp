
#include "pch.h"
#include "dib.h"

namespace afdib
{

dib::dib()
{
	m_nWidth = 0;
	m_nHeight = 0;
	m_PixelType = pt_b8g8r8;
}

dib::~dib()
{
	destroy();
}

bool dib::create( const int nWidth, const int nHeight, const pixeltype pt )
{
	if(getwidth()==nWidth && getheight()==nHeight && getpixeltype()==pt)
		return true;
	destroy();
	if( createstg( nWidth, nHeight, pt ) )
	{
		m_nWidth = nWidth;
		m_nHeight = nHeight;
		m_PixelType = pt;
		return true;
	}
	destroy();
	return false;
}

void dib::destroy( void )
{
	m_nWidth = 0;
	m_nHeight = 0;
	m_PixelType = pt_b8g8r8;
	m_vScanlines.clear();
}

bool dib::createstg( const int nWidth, const int nHeight, const pixeltype pt )
{
	const int nAlloc = getbytesperscanline( nWidth, pt ) * nHeight;
	if( nAlloc == 0 )
		return false;
	m_vScanlines.resize(nAlloc);
	return true;
}

bool dib::getopaquepixel( const pixeltype pt )
{
	switch( pt )
	{
		case pt_b8g8r8:return true;
		default:
		{
			ASSERT( false );
			return false;
		}
	}
}

int dib::getbitsperchannel( const pixeltype pt )
{
	switch( pt )
	{
		case pt_b8g8r8:return 8;
		default:
		{
			ASSERT( false );
			return 0;
		}
	}
}

int dib::getbitsperpixel( const pixeltype pt )
{
	switch( pt )
	{
		case pt_b8g8r8:return 24;
		default:
		{
			ASSERT( false );
			return 0;
		}
	}
}

int dib::getbytesperpixel( const pixeltype pt )
{
	switch( pt )
	{
		case pt_b8g8r8:return 3;
		default:
		{
			ASSERT( false );
			return 0;
		}
	}
}

int dib::getbytesperscanline( const int nWidth, const int nBitsPerPixel )
{
	// Determine width in bits
	int nRetVal = nWidth * nBitsPerPixel;
	
	// Determine width in bytes, rounding up to the nearest byte
	nRetVal = ( nRetVal + 7 ) >> 3;
	
	// Add on any additional padding to make return correct multiple
	return ( ( ( nRetVal + 3 ) >> 2 ) << 2 );
}

void dib::getbmihdr( BITMAPINFOHEADER *pBMIHdr ) const
{
	memset( pBMIHdr, 0, sizeof(BITMAPINFOHEADER) );
	pBMIHdr->biSize = sizeof(BITMAPINFOHEADER);
	pBMIHdr->biWidth = getwidth();
	const bool bTopLeftOrigin = true;
	pBMIHdr->biHeight = bTopLeftOrigin ? -getheight() : getheight();
	pBMIHdr->biPlanes = 1;
	pBMIHdr->biBitCount = getbitsperpixel();
	pBMIHdr->biCompression = BI_RGB;
	pBMIHdr->biSizeImage = getbytesperscanline() * getheight();
}

void dib::tidybmi( BITMAPINFO *p ) const
{
	if( p )
		delete (char*)( p );
}

BITMAPINFO *dib::createbitmapinfo( void ) const
{
	// Check the pixel format
	switch( getpixeltype() )
	{
		case pt_b8g8r8:break;
		default:ASSERT( false );return nullptr;
	}

	const int nColours = 0;
	const int nBmpInfoSize = sizeof( BITMAPINFOHEADER ) + ( nColours * sizeof( RGBQUAD ) );
	BITMAPINFO *pBmpInfo = reinterpret_cast<BITMAPINFO*>( new char[nBmpInfoSize] );
		
	memset( pBmpInfo, 0, nBmpInfoSize );
	pBmpInfo->bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	pBmpInfo->bmiHeader.biWidth = getwidth();
	pBmpInfo->bmiHeader.biHeight = LONG( getheight() ) * -1; // bottom left origin need positive height
	pBmpInfo->bmiHeader.biPlanes = 1;
	pBmpInfo->bmiHeader.biBitCount = getbitsperpixel();
	pBmpInfo->bmiHeader.biCompression = BI_RGB;
	pBmpInfo->bmiHeader.biSizeImage = getheight() * getbytesperscanline();
	
	if( nColours > 0 )
	{
		ASSERT( false );
	}

	return pBmpInfo;
}

bool dib::greyscale(void)
{
	if(isempty())
		return false;
	switch(getpixeltype())
	{
		case pt_b8g8r8:
		{
			for(int nY=0;nY<getheight();++nY)
			{
				const unsigned char *pSrc=getscanline(nY);
				unsigned char *pDst=getscanline(nY);
				for(int nX=0;nX<getwidth();++nX,pSrc+=3,pDst+=3)
				{
					pDst[0]=getluminosity(pSrc[0],pSrc[1],pSrc[2]);
					pDst[1]=pDst[0];
					pDst[2]=pDst[0];
				}
			}
		}
		break;
		default:ASSERT(false);return false;
	}
	return true;
}

void dib::blt( const int x, const int y )
{
	HWND hWnd = ::GetDesktopWindow();
	HDC hDC = ::GetDC(hWnd);

	BITMAPINFO *pInfo = createbitmapinfo();

	setdibitstodevice( hDC,
					   x, y,
					   getwidth(),getheight(),
					   pInfo, getscanline(0),
					   0, getheight() - 1,
					   getheight() - 1, getheight() );
	
	tidybmi(pInfo);
	::ReleaseDC(hWnd,hDC);
}

void dib::setdibitstodevice( HDC dst, const int nXDest, const int nYDest, const int nWidth, const int nHeight, const BITMAPINFO *pSrc, unsigned char *pSrcdata, const int nXSrc, const int nYSrc, const int nStartScan, const int nScanLines ) const
{
	int n = SetDIBitsToDevice( dst, nXDest, nYDest, nWidth, nHeight, nXSrc, nYSrc, nStartScan, nScanLines, pSrcdata, pSrc, DIB_RGB_COLORS );
}

}
