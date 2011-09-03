#include "Certifiable.h"
#include "Bob.h"
#include "CompactMap.h"
#include "Graphics.h"
#include "SurfaceLock.h"

// Comment the next three lines to allow this module to log
#define WriteLog(x)
#undef LogArg
#define LogArg(x)
#undef TryAndReport
#define TryAndReport(x) x

namespace NGameLib2
{
	CSurfaceLock::CSurfaceLock() :
		CCertifiable(),
		area(NULL),
		lock_flags(DDLOCK_WAIT),
		target_surface(NULL)
	{
	}

	CSurfaceLock::CSurfaceLock(LPDIRECTDRAWSURFACE2 target_surface_,RECT *area_,DWORD lock_flags_) :
		CCertifiable(),
		area(area_),
		lock_flags(lock_flags_),
		target_surface(target_surface_)
	{
	}

	int CSurfaceLock::Certify()
	{
		assert(!this->Certified());

		memset((void *)&ddsd,0,sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);

		switch(this->target_surface->Lock(this->area,&this->ddsd,this->lock_flags,NULL))
		{
		case DD_OK:
			return CCertifiable::Certify();
		case DDERR_WASSTILLDRAWING:
			return SURFACELOCKERR_WASSTILLDRAWING;
		case DDERR_OUTOFMEMORY:
			return SURFACELOCKERR_OUTOFMEMORY;
		case DDERR_SURFACELOST:
			return SURFACELOCKERR_SURFACELOST;
		}

		return -1;
	}

	void CSurfaceLock::Uncertify()
	{
		assert(this->Certified());
		this->target_surface->Unlock(this->ddsd.lpSurface);
		CCertifiable::Uncertify();
	}

	void CSurfaceLock::SetTargetSurface(CGraphics& target_display_)
	{
		assert(!this->Certified());

		this->target_surface = target_display_.current_buffer;
	}

	void CSurfaceLock::SetTargetSurface(CBob& target_bob_)
	{
		assert(!this->Certified());

		this->target_surface = target_bob_.data;
	}
}
