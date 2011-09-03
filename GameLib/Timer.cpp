// Timer.cpp: implementation of the CTimer class.
//
//////////////////////////////////////////////////////////////////////

#include <cassert>
#include "Timer.h"

namespace GenericClassLib
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTimer::CTimer()
{
	LONGLONG tps;

	if(CTimer::state == TS_NEVERBEENCREATED)
	{
		if(0 == QueryPerformanceFrequency((LARGE_INTEGER *)&tps))
		{
			CTimer::ticks_per_second = (float)tps;
			state = TS_USESSUPERIORTIMER;
		}
		else
		{
			CTimer::ticks_per_second = TPS_LAMENTABLE;
			state = TS_USESLAMENTABLETIMER;
		}
	}
	this->Restart();
}

CTimer::~CTimer()
{
	// nothing to destroy
}

void CTimer::Restart()
{
	switch(CTimer::state)
	{
	case TS_USESSUPERIORTIMER:
		QueryPerformanceCounter((LARGE_INTEGER *)&(this->last_count_hp));

		break;
	case TS_USESLAMENTABLETIMER:
		this->last_count = GetTickCount();

		break;
	case TS_NEVERBEENCREATED:
		assert(false); // a timer is being used without being created

		// incase we got here on account of running a
		//  non-debug version, give the compiler something to
		//  compile
		// not exactly logically necessary, but may cause
		//  compile errors if omitted
		break; 
	}
}

float CTimer::SecondsSinceLastRestart()
{
	LONGLONG timer = 0;
	float difference; // eventually returned value

	switch(CTimer::state)
	{
	case TS_USESSUPERIORTIMER:
		assert(QueryPerformanceCounter((LARGE_INTEGER *)&timer));

		difference = (float)abs(int(timer - this->last_count_hp));

		break;
	case TS_USESLAMENTABLETIMER:
		difference = (float)abs(GetTickCount() - this->last_count);

		break;
	case TS_NEVERBEENCREATED:
		assert(false); // a timer is being used without being created

		// incase we got here on account of running a
		//  non-debug version, give some kind of value
		//  to difference
		difference = float(TS_NEVERBEENCREATED); 
	}

	// do an operation to convert difference to a measure of
	//  seconds
	difference /= CTimer::ticks_per_second;

	// give the caller the amount of time passed since last
	//  time CTimer::Restart() was called
	return difference;
}

int CTimer::state = TS_NEVERBEENCREATED;
float CTimer::ticks_per_second;

}
