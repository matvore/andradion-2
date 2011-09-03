/*
Copyright 2011 Matt DeVore

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "StdAfx.h"
#include "Timer.h"

// ticks per second with the normal timer
const LONGLONG TPS_LAMENTABLE = 1000;

CTimer::CTimer(const CTimer& r)
{
  this->last_count = r.last_count;
  if(NULL != r.pause)
    {
      this->pause = new LONGLONG(*r.pause);
    }
  else
    {
      this->pause = NULL;
    }
}

CTimer::CTimer() : pause(NULL)
{
  if(CTimer::state == TS_NEVERBEENCREATED)
    {
      if(0 == QueryPerformanceFrequency((LARGE_INTEGER *)&CTimer::ticks_per_second))
        {
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
  // this timer may be paused at the moment
  //  if so, this will delete the pause
  //  timer.  If not, deleting a NULL
  //  has no bad effects
  delete this->pause;
}

void CTimer::Restart()
{
  delete pause;
  pause = NULL;

  this->last_count = CTimer::Now();
}

void CTimer::Minutes(int& min) const
{
  min = this->SecondsPassedInt();
  min /= SECONDSPERMINUTE;
}

void CTimer::MinutesSeconds(int& min,int& sec) const
{
  min = this->SecondsPassedInt();
  sec = min % SECONDSPERMINUTE;
  min /= SECONDSPERMINUTE;
}

void CTimer::MinutesSecondsHundredths(int& min,int& sec,int& hundredths) const
{
  double the_time = this->SecondsPassed80();
  min = (int)the_time;
  sec = min % SECONDSPERMINUTE;
  min /= SECONDSPERMINUTE;
  // at this point, sec and min are prepared and ready
  //  now we have to calculate the number of hundredths of
  //  seconds
  hundredths = (int)(the_time * (double)HUNDREDTHSPERSECOND);
  hundredths %= HUNDREDTHSPERSECOND;
}

void CTimer::Pause()
{
  if(NULL == this->pause)
    {
      // let's pause this thing!
      this->pause = new LONGLONG(this->Now());
    }
}

void CTimer::Resume()
{
  if(NULL != this->pause)
    {
      this->last_count += (CTimer::Now() - *this->pause);
      delete this->pause;
      this->pause = NULL;
    }
}

bool CTimer::Paused() const
{
  return bool(NULL != this->pause);
}

CTimer& CTimer::operator =(const CTimer& r)
{
  this->last_count = r.last_count;
  if(NULL != r.pause)
    {
      this->pause = new LONGLONG(*r.pause);
    }
  else
    {
      this->pause = NULL;
    }

  return *this;
}

CTimer& CTimer::operator +=(const CTimer& r)
{
  this->last_count -= r.TicksPassed();

  return *this;
}

CTimer& CTimer::operator -=(const CTimer& r)
{
  this->last_count += r.TicksPassed();

  return *this;
}

bool operator <(const CTimer& l,const CTimer& r)
{
  return bool(l.TicksPassed() < r.TicksPassed());
}

bool operator >(const CTimer& l,const CTimer& r)
{
  return bool(l.TicksPassed() > r.TicksPassed());
}

bool operator >=(const CTimer& l,const CTimer& r)
{
  return bool(l.TicksPassed() >= r.TicksPassed());
}

bool operator <=(const CTimer& l,const CTimer& r)
{
  return bool(l.TicksPassed() <= r.TicksPassed());
}

CTimer operator +(const CTimer& l,const CTimer& r)
{
  CTimer me;

  me.last_count = CTimer::Now() - l.TicksPassed() - r.TicksPassed();

  return me;
}

CTimer operator -(const CTimer& l,const CTimer& r)
{
  CTimer me;

  me.last_count = CTimer::Now() - r.TicksPassed() + l.TicksPassed();

  return me;
}

bool CTimer::ElapsedTimeIsMeasurable() const
{
  return bool(this->last_count != CTimer::Now());
}

void CTimer::Wait(double time)
{
  CTimer timer;

  while(time > timer.SecondsPassed80()) {}
}

int CTimer::state = TS_NEVERBEENCREATED;
LONGLONG CTimer::ticks_per_second;
