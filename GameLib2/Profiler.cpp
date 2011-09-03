#ifdef _DEBUG

#include "stdafx.h"
#include "Profiler.h"

// Comment the next three lines to allow this module to log
#define WriteLog(x)
#undef LogArg
#define LogArg(x)
#undef TryAndReport
#define TryAndReport(x) x

using std::vector;

const float INVALID_PERCENTAGE = -1.0f;

LONGLONG start_frame;
float timer_frequency;

struct ProfileSample
{
  DWORD profile_instances; // number of times ProfileBegin called
  DWORD open_profiles; // number of times ProfileBegin called w/o ProfileEnd
  LONGLONG start_time;
  LONGLONG accumulator; // all samples this frame added together
  LONGLONG children_sample_time; // time taken by all children
  DWORD num_parents;
  DWORD parent; // contains id of parent
  float ave;
  float min;
  float max;

  tstring name;
};

typedef vector<ProfileSample> VCTR_PROFILESAMPLE;
VCTR_PROFILESAMPLE profile_data;

CProfile::CProfile(DWORD id,const TCHAR *name)
{
  if(profile_data.size() <= id)
    {
      // the profile_data array is not large enough to store a new profile data sample
      ProfileSample x;
      x.profile_instances = 0;
      x.min = x.max = x.ave = INVALID_PERCENTAGE;
      profile_data.resize(id+1,x);
    }
		
  // get pointer to the sample in question
  ProfileSample *x = &profile_data[id];

  if(0 == x->profile_instances++)
    {
      // this is the first time the profile was started
      x->accumulator = 0;
      x->children_sample_time = 0;
      x->name = name; // copy the name
      x->open_profiles = 0;
    }

  QueryPerformanceCounter((LARGE_INTEGER *)&x->start_time);

  // although putting any executable code within an assert statement is usually a no-no, this
  //  code is only compiled in debug mode anyway, so this code will always be executed
  assert(1 == ++x->open_profiles || !"Too many instances of the same profile open at once");

  this->data = x;
}

CProfile::~CProfile()
{
  // get back pointer to sample data
  ProfileSample *x = this->data;
  LONGLONG profile_time;
  QueryPerformanceCounter((LARGE_INTEGER *)&profile_time);
  profile_time -= x->start_time;
  x->open_profiles--;

  if(1 == x->profile_instances) // this is the first time the profile was being used, so figure out our parentage
    {
      x->parent = 0-1;
      x->num_parents = 0;
      // count all parents and find the immediate parent
      for(VCTR_PROFILESAMPLE::iterator inner = profile_data.begin(); inner != profile_data.end(); inner++)
	{
	  if(inner->open_profiles > 0)
	    {
	      x->num_parents++;
	      if((DWORD)-1 == x->parent || inner->start_time >= profile_data[x->parent].start_time) {x->parent = inner - profile_data.begin();}
	    }
	}
    }

  if((DWORD)-1 != x->parent) {profile_data[x->parent].children_sample_time += profile_time;}

  // save sample time in accumulater
  x->accumulator += profile_time;
}

void InitializeProfiler(DWORD default_size)
{
  // make sure no profiles are still open
  for(VCTR_PROFILESAMPLE::iterator i = profile_data.begin(); i != profile_data.end(); i++)
    {
      if(i->profile_instances > 0 && i->open_profiles > 0) {assert(!"Call to ResetProfileData when there are still profiles open");}
    }

  profile_data.clear();
  ProfileSample x;
  x.profile_instances = 0;
  x.min = x.max = x.ave = INVALID_PERCENTAGE;
  profile_data.resize(default_size,x);

  LONGLONG timer_frequency_longlong;
  QueryPerformanceFrequency((LARGE_INTEGER *)&timer_frequency_longlong);
  timer_frequency = (float)timer_frequency_longlong;
}

inline void addstring(vector<tstring>& rows,const TCHAR *str)
{
  // appends a string to the end of a vector
  rows.insert(rows.end(),tstring(str));
}

void GetProfileData(vector<tstring>& text_rows)
{
  text_rows.clear();

  LONGLONG frame_time;
  QueryPerformanceCounter((LARGE_INTEGER *)&frame_time);
  frame_time -= start_frame;		
  float seconds_per_frame = (float)(frame_time) / timer_frequency;

  text_rows.reserve(profile_data.size()+2);

  addstring(text_rows,TEXT("  Ave :   Min :   Max :   # : Profile Name"));
  addstring(text_rows,TEXT("------------------------------------------"));
		
  for(VCTR_PROFILESAMPLE::iterator i = profile_data.begin(); i != profile_data.end(); i++)
    {
      if(i->profile_instances > 0)
	{
	  // we have found some valid profile data
	  float percentage = float(i->accumulator - i->children_sample_time);
	  percentage /= (float)(frame_time);
	  percentage *= 100.0f;

	  if(percentage > 100.0f)
	    {
	      i->profile_instances = 0;
	      continue;
	    }

	  // store profile in history :
	  if(INVALID_PERCENTAGE == i->ave)
	    {
	      // first time entering into history
	      i->ave = i->min = i->max = percentage;
	    }
	  else
	    {
	      // adding more to history
	      float new_ratio = 0.8f * seconds_per_frame;
	      if(new_ratio > 1.0f) {new_ratio = 1.0f;}
	      float old_ratio =  1.0f - new_ratio;

	      new_ratio *= percentage;

	      i->ave *= old_ratio;
	      i->ave += new_ratio;
	      if(percentage > i->min)
		{
		  // we don't have a new min record, so average it in
		  i->min *= old_ratio;
		  i->min += new_ratio;
		}
	      else
		{
		  // we have a new minimum time
		  i->min = percentage;
		}

	      if(percentage < i->max)
		{
		  // we don't have a new max record, so average it in
		  i->max *= old_ratio;
		  i->max += new_ratio;
		}
	      else
		{
		  // we have a new maximum time
		  i->max = percentage;
		}
	    }
	  // end of storeing profile in history
				
	  // add all this data into a string
	  tstring data;
	  TCHAR buffer[16];
	  float *p = &i->ave;
	  do
	    {
	      _stprintf(buffer,TEXT("%5.1f"),*p);
	      data += buffer;
	      data += TEXT(" : ");
	    }
	  while(p++ != &i->max);

	  _stprintf(buffer,TEXT("%3d"),i->profile_instances);
	  data += buffer;
	  data += TEXT(" : ");

	  // add indentation:  more for each parent it has
	  for(DWORD j = 0;j < i->num_parents; j++) {data += TCHAR(' ');}
	  data += i->name;

	  addstring(text_rows,data.c_str());

	  i->profile_instances = 0;
	} // endif is valid profile data
    } // end for each profile
} // end GetProfileData()

void StartProfileFrame()
{
  QueryPerformanceCounter((LARGE_INTEGER *)&start_frame);
}

#endif
