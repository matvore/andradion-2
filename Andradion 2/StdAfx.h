#if     !defined(_C77F1780_DEBC_11d4_B6FE_0050040B0541_INCLUDED_)
#define          _C77F1780_DEBC_11d4_B6FE_0050040B0541_INCLUDED_

#pragma warning (disable : 4786)

// these constants allow for compiling against
//  old directX versions (except for DirectMusic,
//  it's not really old!)
#define DIRECTINPUT_VERSION 0x0300
#define DIRECTSOUND_VERSION 0x0300

#include <set>
#include <cmath>
#include <vector>
#include <string>
#include <queue>
#include <dinput.h>
#include <cstdlib>
#include <dplay.h>
#include <dsound.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include "/GameLib2/matrix.cpp"
#include "/GameLib2/Timer.h"
#include "/GameLib2/Certifiable.h"
#include "/GameLib2/Color.h"
#include "/GameLib2/Bob.h"
#include "/GameLib2/Menu.h"
#include "/GameLib2/MusicLib.h"
#include "/GameLib2/CompactMap.h"
#include "/GameLib2/Graphics.h"
#include "/GameLib2/RawLoad.h"
#include "/GameLib2/ColorNP.h"
#include "/GameLib2/Color256.h"
#include "/GameLib2/SurfaceLock.h"
#include "/GameLib2/SurfaceLock256.h"
#include "/GameLib2/Logger.h"
#include "/GameLib2/Profiler.h"

typedef std::set<int> SET_INT;
typedef std::vector<int> VCTR_INT;
typedef std::vector<NGameLib2::tstring> VCTR_STRING;

inline void *GetResPtr(const TCHAR *res_name,const TCHAR *res_type,HMODULE res_mod,WORD res_lang,HRSRC& res_handle,HGLOBAL& data_handle)
{
  return LockResource(data_handle = LoadResource(res_mod,res_handle = FindResourceEx(res_mod,res_type,res_name,res_lang)));
}

#endif
