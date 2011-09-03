#ifndef _77ADF344_46E9_11d4_B6FE_0050040B0541_INCLUDED_
#define _77ADF344_46E9_11d4_B6FE_0050040B0541_INCLUDED_

#include "StdAfx.h"

namespace NGameLib2
{
	const TCHAR MEM_MSG[] =
		TEXT("Restart your computer and start this program again with no others running.");

	const TCHAR MEM_CAP[] =
		TEXT("Out of Memory");
}

#ifndef BORLAND
#define MemoryAllocFunction(op,size,condition) \
do {int newres; _PNH newhandler; op; \
if(condition) { \
newhandler = _set_new_handler(NULL); \
_set_new_handler(newhandler); \
newres = newhandler(size); \
_set_new_handler(newhandler); \
if(0 == newres)	{MessageBox(NULL,NGameLib2::MEM_MSG,NGameLib2::MEM_CAP,MB_ICONSTOP);exit(1);}} \
} while(condition)
#else
#define MemoryAllocFunction(op,size,condition) {op; if(condition) {MessageBox(NULL,NGameLib2::MEM_MSG,NGameLib2::MEM_CAP,MB_ICONSTOP);exit(1);}}
#endif

#endif