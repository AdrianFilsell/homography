#pragma once

#include <string>
#include <memory>
#include "dib.h"

struct jpeg_compress_struct;
struct jpeg_decompress_struct;

namespace afdib
{

class jpeg
{
public:
	static std::shared_ptr<dib> load8bpp(LPCTSTR lpsz);
	static bool save8bpp(std::shared_ptr<const dib> sp,LPCTSTR lpsz);
protected:
	static std::shared_ptr<dib> load_jgl(LPCTSTR lpsz);
	static std::shared_ptr<dib> filesrc_load_jgl(LPCTSTR lpsz);
	static std::shared_ptr<dib> load_jgl_ex(struct jpeg_decompress_struct *pCinfo);

	static bool save_jgl(std::shared_ptr<const dib> sp,LPCTSTR lpsz);
	static bool filesrc_save_jgl(std::shared_ptr<const dib> sp,LPCTSTR lpsz);
	static bool save_jgl_ex(std::shared_ptr<const dib> sp,struct jpeg_compress_struct *pCinfo);
};

}
