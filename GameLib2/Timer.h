// Timer.h: interface for the CTimer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TIMER_H__DEB6C740_3732_11D4_B6FD_0050040B0541__INCLUDED_)
#define AFX_TIMER_H__DEB6C740_3732_11D4_B6FD_0050040B0541__INCLUDED_

#include "StdAfx.h"

namespace NGameLib2
{
	class CTimer
	{
		friend bool operator <(const CTimer&,const CTimer&);
		friend bool operator >(const CTimer&,const CTimer&);
		friend bool operator >=(const CTimer&,const CTimer&);
		friend bool operator <=(const CTimer&,const CTimer&);

		friend CTimer operator +(const CTimer&,const CTimer&);
		friend CTimer operator -(const CTimer&,const CTimer&);

	public:
		static void Wait(double time);
		
		inline float  SecondsPassed32()    const {return (float)    this->TicksPassed()         / (float) ticks_per_second  ;}
		inline double SecondsPassed80()    const {return (double)   this->TicksPassed()         / (double)ticks_per_second  ;}
		inline int    SecondsPassedInt()   const {return (int)     (this->TicksPassed()         /         ticks_per_second) ;}
		inline long   SecondsPassedFixed() const {return (long)   ((this->TicksPassed() << 16)  /         ticks_per_second) ;}

		void Minutes(int& min) const;
		void MinutesSeconds(int& min,int& sec) const;
		void MinutesSecondsHundredths(int& min,int& sec,int& hundredths) const;

		void Pause();
		void Resume();
		void Restart();
		bool Paused() const;

		// returns true if there has been at least one clock tick
		//  since the last restart
		bool ElapsedTimeIsMeasurable() const;

		CTimer(const CTimer& r);
		CTimer();
		~CTimer();

		CTimer& operator =(const CTimer&);
		CTimer& operator +=(const CTimer&); // adds the time passed for r to the time passed for this
		CTimer& operator -=(const CTimer&); // subtracts the time passed for r from the time passed for this

	private:
		// timer state constants
		enum {TS_NEVERBEENCREATED,TS_USESSUPERIORTIMER,TS_USESLAMENTABLETIMER};

		inline LONGLONG TicksPassed() const
		{return NULL == pause ? Now() - last_count : *pause - last_count;}

		static inline LONGLONG Now()
		{if(TS_USESSUPERIORTIMER==state){LONGLONG now;QueryPerformanceCounter((LARGE_INTEGER *)&now);return now;}else{return GetTickCount();}}

		LONGLONG *pause; // equal to null if the timer is not currently paused, otherwise points to the tick count of when it was paused

		LONGLONG last_count;

		static LONGLONG ticks_per_second;
		static int state;
	};

	bool operator <(const CTimer&,const CTimer&);
	bool operator >(const CTimer&,const CTimer&);
	bool operator >=(const CTimer&,const CTimer&);
	bool operator <=(const CTimer&,const CTimer&);

	CTimer operator +(const CTimer&,const CTimer&);
	CTimer operator -(const CTimer&,const CTimer&);
}

#endif // !defined(AFX_TIMER_H__DEB6C740_3732_11D4_B6FD_0050040B0541__INCLUDED_)










