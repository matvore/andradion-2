// Timer.h: interface for the CTimer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TIMER_H__DEB6C740_3732_11D4_B6FD_0050040B0542__INCLUDED_)
#define AFX_TIMER_H__DEB6C740_3732_11D4_B6FD_0050040B0542__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>

namespace GenericClassLib
{
	// timer states
	const int TS_NEVERBEENCREATED = 0;
	const int TS_USESSUPERIORTIMER = 1;
	const int TS_USESLAMENTABLETIMER = 2;

	const float TPS_LAMENTABLE = 1000.0;

	class CTimer
	{
	public:
		float SecondsSinceLastRestart(void);
		void Restart(void);
		CTimer();
		virtual ~CTimer();

	private:
		union
		{		
			DWORD last_count;
			LONGLONG last_count_hp;
		};

		static float ticks_per_second; // used only when using superior timer
		static int state;
	};
}

#endif // !defined(AFX_TIMER_H__DEB6C740_3732_11D4_B6FD_0050040B0541__INCLUDED_)
