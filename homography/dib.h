#pragma once

#include <Windows.h>
#include <vector>

namespace afdib
{

class dib
{
public:
	enum pixeltype
	{
		pt_b8g8r8,
	};
	dib();
	virtual ~dib();
	
	bool isempty(void) const { return m_vScanlines.size()==0; }
	bool getopaque( void ) const { return getopaquepixel( m_PixelType ); }
	int getwidth( void ) const { return m_nWidth; }
	int getheight( void ) const { return m_nHeight; }
	pixeltype getpixeltype( void ) const { return m_PixelType; }
	int getbitsperchannel( void ) const { return getbitsperchannel( m_PixelType ); }
	int getbitsperpixel( void ) const {	return getbitsperpixel( m_PixelType ); }
	int getbytesperpixel( void ) const { return getbytesperpixel( m_PixelType ); }
	int getbytesperscanline( void ) const { return getbytesperscanline( m_nWidth, m_PixelType ); }
	int getallocsize( void ) const { return getheight() * getbytesperscanline(); }
	const std::vector<unsigned char>& getscanlines( void ) const { return m_vScanlines; }
	const unsigned char *getscanline(const int n)const{return n<0||n>=m_nHeight?nullptr:(&m_vScanlines[0])+(getbytesperscanline()*n);}
	
	void getbmihdr( BITMAPINFOHEADER *pBMIHdr ) const;
	BITMAPINFO *dib::createbitmapinfo( void ) const;
	void tidybmi( BITMAPINFO *p ) const;
	
	void destroy( void );
	bool create( const int nWidth, const int nHeight, const pixeltype pt );
	bool greyscale(void);
	unsigned char *getscanline(const int n){return n<0||n>=m_nHeight?nullptr:(&m_vScanlines[0])+(getbytesperscanline()*n);}
	void blt( const int x, const int y );

	static bool getopaquepixel( const pixeltype pt );
	static int getbitsperchannel( const pixeltype pt );
	static int getbitsperpixel( const pixeltype pt );
	static int getbytesperpixel( const pixeltype pt );
	static int getbytesperscanline( const int nWidth, const int nBitsPerPixel );
	static int getbytesperscanline( const int nWidth, const pixeltype pt ) { return getbytesperscanline( nWidth, getbitsperpixel( pt ) ); }

protected:
	int m_nWidth;
	int m_nHeight;
	pixeltype m_PixelType;
	std::vector<unsigned char> m_vScanlines;

	unsigned char getluminosity( const unsigned char b,const unsigned char g,const unsigned char r ) const
	{
		// OpenCV greyscale coeff: B0.114 + G0.587 + R0.299
		// Multiplied up by 2^22 to improve accuracy upto clamping to a 32 bit number
		return ( b * 478151 + g * 2462056 + r * 1254097 ) >> 22;
	}
	void setdibitstodevice( HDC dst, const int nXDest, const int nYDest, const int nWidth, const int nHeight, const BITMAPINFO *pSrc, unsigned char *pSrcdata, const int nXSrc, const int nYSrc, const int nStartScan, const int nScanLines ) const;

	bool createstg( const int nWidth, const int nHeight, const pixeltype pt );
};

}
