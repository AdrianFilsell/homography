
#include "pch.h"
#include "jpeg.h"
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <setjmp.h>
extern "C"
{
	#include "../libjpeg/jpeg-8c/jpeglib.h"
}

struct my_error_mgr
{
	struct jpeg_error_mgr pub;	// "public" fields
	jmp_buf setjmp_buffer;	// for return to caller
};
typedef struct my_error_mgr * my_error_ptr;
METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
	// cinfo->err really points to a my_error_mgr struct, so coerce pointer
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	// Return control to the setjmp point
	longjmp(myerr->setjmp_buffer, 1);
}
METHODDEF(void)
my_output_message(j_common_ptr cinfo)
{
}

namespace afdib
{

bool jpeg::save8bpp(std::shared_ptr<const dib> sp,LPCTSTR lpsz)
{
	return save_jgl(sp,lpsz);
}

bool jpeg::save_jgl(std::shared_ptr<const dib> sp,LPCTSTR lpsz)
{
	return filesrc_save_jgl(sp,lpsz);
}

bool jpeg::filesrc_save_jgl(std::shared_ptr<const dib> sp,LPCTSTR lpsz)
{
	if(!sp || sp->getbitsperchannel() != 8 || sp->getbitsperpixel() != 24)
		return false;

	FILE * outfile;
	if( ( _tfopen_s( &outfile, lpsz, _T("wb") ) ) )
		return false;

	struct jpeg_compress_struct cinfo;
	jpeg_create_compress( &cinfo );
	jpeg_stdio_dest( &cinfo, outfile );

	bool bSaved = save_jgl_ex(sp,&cinfo);
	jpeg_destroy_compress(&cinfo);

	fclose( outfile );
	return bSaved;
}

bool jpeg::save_jgl_ex(std::shared_ptr<const dib> sp,struct jpeg_compress_struct *pCinfo)
{
	struct my_error_mgr jerr;

	pCinfo->err = jpeg_std_error(&jerr.pub);

	pCinfo->image_width = sp->getwidth();
	pCinfo->image_height = sp->getheight();

	pCinfo->in_color_space = JCS_RGB;
	pCinfo->input_components = 3;

	const int nQuality=100;
	jpeg_set_defaults( pCinfo );
	jpeg_set_quality( pCinfo, nQuality, true );

	const unsigned char *pSrc = sp->getscanline(0);
	const int nBytesPerScanline = sp->getbytesperscanline();
	std::vector<unsigned char> vScanline(nBytesPerScanline);
	unsigned char *pRow = &vScanline.front();
	JSAMPROW rows[1] = {pRow};

	jpeg_start_compress( pCinfo, true );

	int nScanline = 0;
	const bool bBGRFlip = pCinfo->in_color_space == JCS_RGB;
	while( pCinfo->next_scanline < pCinfo->image_height)
	{
		const unsigned char *pSrcRow = pSrc + nScanline++ * nBytesPerScanline;
		memcpy( pRow, pSrcRow, nBytesPerScanline );
		if(bBGRFlip)
		{
			for(int nX=0;nX<sp->getwidth();++nX)
				std::swap(pRow[(nX*3)+0],pRow[(nX*3)+2]);
		}
		jpeg_write_scanlines( pCinfo, rows, 1 );
	}
	jpeg_finish_compress( pCinfo );

	return true;
}

std::shared_ptr<dib> jpeg::load8bpp(LPCTSTR lpsz)
{
	return load_jgl(lpsz);
}

std::shared_ptr<dib> jpeg::load_jgl(LPCTSTR lpsz)
{
	return filesrc_load_jgl(lpsz);
}

std::shared_ptr<dib> jpeg::filesrc_load_jgl(LPCTSTR lpsz)
{
	// File/path source
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	FILE *pInfile;
	if( _tfopen_s( &pInfile, lpsz, L"rb" ) )
		return nullptr;
		
	cinfo.err = jpeg_std_error( &jerr.pub );
	jerr.pub.error_exit = my_error_exit;
	jerr.pub.output_message = my_output_message;
	if( setjmp( jerr.setjmp_buffer ) )
	{
		jpeg_destroy_decompress( &cinfo );
		fclose( pInfile );
		return nullptr;
	}
		
	jpeg_create_decompress( &cinfo );
	jpeg_stdio_src( &cinfo, pInfile );
		
	std::shared_ptr<dib> spDib = load_jgl_ex(&cinfo);

	jpeg_destroy_decompress( &cinfo );
	fclose( pInfile );

	return spDib;
}

std::shared_ptr<dib> jpeg::load_jgl_ex(struct jpeg_decompress_struct *pCinfo)
{
	jpeg_read_header( pCinfo, true );

	pCinfo->scale_num = 1;
	pCinfo->scale_denom = 1;
	
	pCinfo->do_fancy_upsampling = false;
	pCinfo->do_block_smoothing = false;
	pCinfo->dct_method = JDCT_IFAST;

	bool bRGBFlipChannels = false;
	afdib::dib::pixeltype pixeltype = afdib::dib::pt_b8g8r8;
	switch( pCinfo->out_color_space )
	{
		case JCS_CMYK:
		case JCS_YCCK:
		case JCS_GRAYSCALE:return false; // not yet supported
		default:bRGBFlipChannels = true;pCinfo->out_color_space = JCS_RGB;break;
	}

	jpeg_calc_output_dimensions( pCinfo );
	std::shared_ptr<dib> spDib(new afdib::dib);
	if(!spDib->create( pCinfo->output_width, pCinfo->output_height, pixeltype ) )
		return nullptr;
	const int nBytesPerScanline = spDib->getbytesperscanline();
	const int nReadMaxAlloc = 1024 * 1024 * 5;

	int nReadAllocRows = ( nReadMaxAlloc / spDib->getbytesperscanline() );
	if( nReadAllocRows < 1 )
		nReadAllocRows = 1;
	else
	if( nReadAllocRows > spDib->getheight() )
		nReadAllocRows = spDib->getheight();

	JSAMPROW buffer[1];
	jpeg_start_decompress( pCinfo );

	while( pCinfo->output_scanline < pCinfo->output_height )
	{
		const int nRow = pCinfo->output_scanline;

		unsigned char *pScan = spDib->getscanline(nRow);
		
		buffer[0] = reinterpret_cast< JSAMPROW >( pScan );
		jpeg_read_scanlines( pCinfo, buffer, 1 );
		if( bRGBFlipChannels )
			for( int nX = 0 ; nX < int(pCinfo->output_width) ; ++nX )
				std::swap(pScan[nX*3+0],pScan[nX*3+2]);
	}
	jpeg_finish_decompress( pCinfo );
	return spDib;
}

}
