#pragma warning (disable : 4786)

#define DIRECTDRAW_VERSION 0x0300
#define DIRECTSOUND_VERSION 0x0300

#include <dsound.h>
#include <direct.h>
#include <dmksctrl.h>
#include <dmusici.h>
#include <dmusicc.h>
#include <dmusicf.h>
#include <cassert>
#include <ddraw.h>
#include <cstring>
#include <utility>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <windows.h>
#include <new.h>
#include <tchar.h>

namespace NGameLib2
{
	typedef std::vector<LPDIRECTDRAWSURFACE2> VCTR_DIRECTDRAWSURFACE;
	typedef std::vector<RECT> VCTR_RECTANGLE;
	typedef std::vector<VCTR_RECTANGLE> VCTR_RECTANGLEVECTOR;
	typedef std::vector<DWORD> VCTR_DWORD;

	// data structures for info on leftover
	typedef std::vector<BYTE> VCTR_BYTE;
	typedef std::vector<std::pair<DWORD,VCTR_BYTE> > VCTR_RUN;
	typedef std::vector<VCTR_RUN> VCTR_ROW;

	// clipping info on left over
	typedef std::vector<std::pair<int,int> > VCTR_MINMAXXROWOFFSET;

	typedef std::basic_string<TCHAR> tstring;
}
