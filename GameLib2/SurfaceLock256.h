#ifndef _7F5EFCE0_9579_11d4_B6FE_0050040B0541_INCLUDED_
#define _7F5EFCE0_9579_11d4_B6FE_0050040B0541_INCLUDED_

#include "StdAfx.h"

namespace NGameLib2
{
	class CGraphics;

	class CSurfaceLock256 : public CSurfaceLock
	{
	public:
		CSurfaceLock256(); // this constructor does nothing, just initiates so it is ready to be setup eventually
		CSurfaceLock256(LPDIRECTDRAWSURFACE2 target_surface_,RECT *area_ = NULL,DWORD lock_flags_ = DDLOCK_WAIT);
		virtual void Line(int x0,int y0,int x1,int y1,DWORD color,DWORD pattern = 0xffffffff);
	};
}

#endif
