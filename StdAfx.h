#include <windows.h>
#include <dsound.h>
#include <dmusicc.h>
#include <dmusici.h>
#include <dmusicf.h>
#include <set>
#include <cmath>
#include <vector>
#include <string>
#include <queue>
#include <dinput.h>
#include <ddraw.h>
#include <cstdlib>
#include <dplay.h>
#include <cstdio>
#include <memory.h>
#include <cassert>

extern DIOBJECTDATAFORMAT c_rgodfDIKeyboard[256];
extern const DIDATAFORMAT c_dfDIKeyboard;

typedef std::set<int> SET_INT;
typedef std::vector<int> VCTR_INT;
typedef std::basic_string<char> string;
typedef std::vector<string> VCTR_STRING;
typedef std::vector<LPDIRECTDRAWSURFACE2> VCTR_DIRECTDRAWSURFACE;
typedef std::vector<RECT> VCTR_RECTANGLE;
typedef std::vector<VCTR_RECTANGLE> VCTR_RECTANGLEVECTOR;
typedef std::vector<DWORD> VCTR_DWORD;

// data structures for info on leftover
typedef std::vector<BYTE> VCTR_BYTE;
typedef std::vector<std::pair<DWORD, VCTR_BYTE> > VCTR_RUN;
typedef std::vector<VCTR_RUN> VCTR_ROW;

// clipping info on left over
typedef std::vector<std::pair<int,int> > VCTR_MINMAXXROWOFFSET;

inline void *GetResPtr(const TCHAR *res_name,const TCHAR *res_type,HMODULE res_mod,WORD res_lang,HRSRC& res_handle,HGLOBAL& data_handle)
{
  return LockResource(data_handle = LoadResource(res_mod,res_handle = FindResourceEx(res_mod,res_type,res_name,res_lang)));
}

const char MEM_MSG[] =
"Restart your computer and start this program again with no others running.";
const char MEM_CAP[] = "Out of Memory";

#define MemoryAllocFunction(op,size,condition) {op; if(condition) {MessageBox(NULL,MEM_MSG,MEM_CAP,MB_ICONSTOP);exit(1);}}
