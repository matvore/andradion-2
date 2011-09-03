// Weather.cpp: implementation of the CWeather class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Fixed.h"
#include "SharedConstants.h"
#include "Net.h"
#include "Weather.h"
#include "resource.h"
#include "Character.h"
#include "Fire.h"
#include "Glue.h"
#include "Palette.h"

using NGameLib2::CColor256;
using NGameLib2::CreateSBFromRawData;
using NGameLib2::CSurfaceLock256;
using NGameLib2::tstring;

const int NUM_WEATHERSTATES = 15;
const int WEATHERSCRIPT_RANDOM = 11;
enum {HEROROOMSTATE_INSIDE,HEROROOMSTATE_OUTSIDE,HEROROOMSTATE_UNKNOWN};

// weather state flags
const DWORD WSF_LIGHTRAIN = 1;
const DWORD WSF_HEAVYRAIN = 1 * 2;
const DWORD WSF_THUNDER   = 1 * 2 * 2;
const DWORD WSF_LIGHTNING = 1 * 2 * 2 * 2;
const DWORD WSF_CRICKETS  = 1 * 2 * 2 * 2 * 2;

const DWORD WEATHERSTATE_FLAGS[NUM_WEATHERSTATES] =
  {
    0,
    WSF_LIGHTRAIN,
    WSF_HEAVYRAIN,
    WSF_HEAVYRAIN | WSF_THUNDER,
    WSF_HEAVYRAIN | WSF_THUNDER | WSF_LIGHTNING,
    WSF_HEAVYRAIN,
    WSF_LIGHTRAIN,
    0,
    WSF_CRICKETS,
    0,0,0,0,0,0
  };

const int NUM_WEATHERPATTERNS = 12;
const int WEATHERPATTERN_STARTPOINT[NUM_WEATHERPATTERNS] =
  {
    0,
    0,
    0,
    1,
    2,
    4,
    6,
    7,
    8,
    0,
    9,
    -1 // N/A, completely random
  };

const int WEATHERPATTERN_ENDPOINT[NUM_WEATHERPATTERNS] =
  {
    0,
    NUM_WEATHERSTATES, // always cycles, never reaches endpoint
    8,
    1,
    2,
    4,
    6,
    8,
    8,
    4,
    0,
    -1, // N/A, completely random
  };

const DWORD WEATHERSTATE_LENGTH[NUM_WEATHERSTATES] =
  {3000,2400,3000,3300,3600,2100,2100,1800,9000,2550,2550,2250,2250,2250,2100};

const DWORD WEATHERSTATE_PLAYMUSIC = 0x7807;

const FIXEDNUM WEATHERSTATE_BRIGHTNESS[NUM_WEATHERSTATES] =
  {
    Fixed( 1.00f ),
    Fixed( 0.90f ),
    Fixed( 0.80f ),
    Fixed( 0.75f ),
    Fixed( 0.70f ),
    Fixed( 0.60f ),
    Fixed( 0.50f ),
    Fixed( 0.40f ),
    Fixed( 0.30f ),
    Fixed( 0.40f ),
    Fixed( 0.50f ),
    Fixed( 0.60f ),
    Fixed( 0.70f ),
    Fixed( 0.80f ),
    Fixed( 0.90f )
  };

const int NUM_RAINFRAMES = 8;
const int NUM_RAINDROPS = 20;
enum {WEATHERSOUND_LIGHTRAIN,WEATHERSOUND_HEAVYRAIN,WEATHERSOUND_CRICKETS,WEATHERSOUND_THUNDER1,WEATHERSOUND_THUNDER2,WEATHERSOUND_THUNDER3,NUM_WEATHERSOUNDS};
const int NUM_THUNDERSOUNDS = 3;

const int NUM_WEATHERSCRIPTS = 12;

const COLORREF CR_RAIN = RGB(255,255,255);

const FIXEDNUM LIGHTNING_BRIGHTNESS = Fixed(3.0f);

static void OneRainFrame(bool heavy_rain,CSurfaceLock256& lock);
static void OneThunderFrame(bool lightning);
static int hero_room_state;
static int script_index;
static DWORD frames_since_start; // records number of frames since start of currentw eather state
static int current_state;
static BYTE rain_color;
static LPDIRECTSOUNDBUFFER sfx[6];
static POINT rain_drops[NUM_RAINFRAMES][NUM_RAINDROPS];

void WtrOneFrame()
{
  BeginProfile(Weather_One_Frame);
  // hero is indoors
  // check if hero was just outdoors
  if(HEROROOMSTATE_INSIDE != hero_room_state)
    {
      hero_room_state = HEROROOMSTATE_INSIDE;
      WtrNextState();
    }
  EndProfile();
}

void WtrOneFrame(CSurfaceLock256& lock)
{
  BeginProfile(Weather_One_Frame);
  // hero is outside
  // check if hero just left a room
  if(HEROROOMSTATE_OUTSIDE != hero_room_state)
    {
      hero_room_state = HEROROOMSTATE_OUTSIDE;
      WtrNextState();
    }

  // perform lightning/thunder/rain logic
  DWORD f = WEATHERSTATE_FLAGS[current_state];
  if(WSF_THUNDER & f)
    {
      if(WSF_LIGHTNING & f)
	{
	  OneThunderFrame(true);
	}
      else
	{
	  OneThunderFrame(false);
	}
    }
  if(WSF_LIGHTRAIN & f)
    {	
      OneRainFrame(false,lock);
    }
  else if(WSF_HEAVYRAIN & f)
    {
      OneRainFrame(true,lock);
    }

  // advance to next state if necessary
  if(++frames_since_start > WEATHERSTATE_LENGTH[current_state])
    {
      int old_state = current_state;
      // on to the next weather state
      frames_since_start = 0;
		
      // advance to the next state
      if(WEATHERSCRIPT_RANDOM == script_index)
	{
	  // doing the random weather script
	  current_state = rand()%NUM_WEATHERSTATES;
	}
      else if
	(
	 WEATHERPATTERN_ENDPOINT[script_index] != current_state &&
	 NUM_WEATHERSTATES == ++current_state
	 )
	{
	  current_state = 0;
	}

      if(true == NetSendWeatherStateMessage(current_state))
	{
	  // prepare for the next state
	  WtrNextState();
	}
      else
	{
	  // revert to old state
	  current_state = old_state;
	}
    }
  EndProfile(); // one frame of weather
}

void WtrSetScript(int script_index_)
{
  script_index = script_index_;

  // set current weather state by finding the first one
  //  for a given script index
  current_state = WEATHERPATTERN_STARTPOINT[script_index];

  if(current_state < 0 || current_state >= NUM_WEATHERSTATES)
    {
      current_state = rand()%NUM_WEATHERSTATES;
    }

  hero_room_state = HEROROOMSTATE_UNKNOWN;
}

const int RAIN_DROPLENGTH = 10;
void WtrBeginScript()
{
  // setup the raindrops
  for(int frame = 0; frame < NUM_RAINFRAMES; frame++)
    {
      for(int drop = 0; drop < NUM_RAINDROPS; drop++)
	{
	  // address the point in question
	  POINT *p = &rain_drops[frame][drop];

	  p->x = (rand()%(GAME_MODEWIDTH-RAIN_DROPLENGTH/2));
	  p->y = (rand()%(GAME_PORTHEIGHT-RAIN_DROPLENGTH));
	}
    }

  frames_since_start = 0;
}

void WtrAnalyzePalette()
{
  CColor256 c;

  // get rain color
  c.SetColor(CR_RAIN);
  c.Certify();
  rain_color = c.Color();
  c.Uncertify();
}

void WtrRelease()
{
  // get rid of direct sound stuff if
  //  necessary
  for(int i = 0 ;i  < NUM_WEATHERSOUNDS; i++)
    {
      if(NULL != sfx[i])
	{
	  TryAndReport(sfx[i]->Release());
	  sfx[i] = NULL;
	}
    }
}

void WtrInitialize(LPDIRECTSOUND ds)
{
  // load the weather sounds
  HRSRC res_handle;
  HGLOBAL data_handle;
  DWORD *sound_data = (DWORD *)GetResPtr(TEXT("SFX"),TEXT("DAT"),NULL,MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),res_handle,data_handle);

  sound_data = (DWORD *)((BYTE *)sound_data+*sound_data)+1;
  for(int i = 0; i < NUM_WEATHERSOUNDS; i++)
    {
      CreateSBFromRawData(ds,&sfx[i],(void *)(sound_data+1),*sound_data,0,SOUNDRESOURCEFREQ,SOUNDRESOURCEBPS,1);
      sound_data = (DWORD *)((BYTE *)sound_data+*sound_data)+1;
    }

  FreeResource(data_handle);
}

// this function is called when the state has changed
//  call right after setting entering/leaving a room
void WtrNextState()
{
#ifdef _DEBUG
  if(current_state < 0 || current_state >= NUM_WEATHERSTATES)
    {
      WriteLog("Invalid weather state, changing to zero");
      current_state = 0;
    } // end if invalid weather state
#endif

  if(WEATHERSTATE_PLAYMUSIC & (1 << current_state) || HEROROOMSTATE_INSIDE == hero_room_state)
    {
      tstring music_res;
      GluStrLoad(IDS_LEVELFILE1+GLUlevel*2,music_res);
      GluSetMusic(true,music_res.c_str());
    }
  else
    {
      GluStopMusic();
    }

  // stop all sounds
  bool sounds_were_loaded = WtrSilence();

  if(HEROROOMSTATE_OUTSIDE == hero_room_state)
    {
      PalSetBrightnessFactor(WEATHERSTATE_BRIGHTNESS[current_state]);
    }
  else
    {
      // normal brightness when indoors
      PalSetBrightnessFactor(Fixed(1.0f));
      return; // we're done already because of neutralization of Weather indoors
    }

  // now handle crickets/rain/lightning

  if(true == sounds_were_loaded)
    {
      // all sounds were loaded successfully, because that last loop finished w/o breaking
      DWORD f = WEATHERSTATE_FLAGS[current_state]; // get the flags we need
      if(f & WSF_CRICKETS)
	{
	  // play cricket noise looping
	  sfx[WEATHERSOUND_CRICKETS]->Play(0,0,DSBPLAY_LOOPING);
	}
      else if(f & WSF_LIGHTRAIN)
	{
	  // play light rain sound
	  sfx[WEATHERSOUND_LIGHTRAIN]->Play(0,0,DSBPLAY_LOOPING);
	}
      else if(f & WSF_HEAVYRAIN)
	{
	  // play hard rain sound
	  sfx[WEATHERSOUND_HEAVYRAIN]->Play(0,0,DSBPLAY_LOOPING);
	}
    }
}

// returns true if all the sounds have been loaded
bool WtrSilence()
{
  // this loop var will be used later,
  //  so for clarity it is declared right before the loop
  int i; 
  for(i = 0; i < NUM_WEATHERSOUNDS; i++)
    {
      if(NULL != sfx[i])
	{
	  sfx[i]->Stop();
	}
      else
	{
	  break;
	}
    }

  return bool(NUM_WEATHERSOUNDS == i);
}

const DWORD RAIN_FRAMELENGTH = 3;
void OneRainFrame(bool heavy_rain,CSurfaceLock256& lock)
{
  static DWORD frame_count = 0;
  int frame_index = ++frame_count/RAIN_FRAMELENGTH;

  if(frame_index >= NUM_RAINFRAMES)
    {
      frame_index = 0;
      frame_count = 0;
    }

  // address the set of rain drops we need
  POINT *r = rain_drops[frame_index];

  // surface is locked already

  // now the rest of the function
  //  is different depending on light rain or hard rain

  // make typing shortcuts
  int p = lock.SurfaceDesc().lPitch;
  BYTE *s = (BYTE *)lock.SurfaceDesc().lpSurface;
  BYTE c = rain_color;
  FIXEDNUM diff_x = FixedCnvFrom<long>(GLUcenter_screen_x) / -2;
  FIXEDNUM diff_y = FixedCnvFrom<long>(GLUcenter_screen_y) / -2;

  int drops = (true == heavy_rain) ? NUM_RAINDROPS : NUM_RAINDROPS/4;
  // show all drops, and all at an angle
  for(int i = 0; i < drops; i++)
    {
      int x = r[i].x + diff_x;

      x %= GAME_MODEWIDTH-RAIN_DROPLENGTH/2;

      if(x < RAIN_DROPLENGTH/2)
	{
	  x += GAME_MODEWIDTH-RAIN_DROPLENGTH/2;
	}

      int y = r[i].y + diff_y;

      y %= (GAME_PORTHEIGHT-RAIN_DROPLENGTH);	

      if(y < 0)
	{
	  y += GAME_PORTHEIGHT-RAIN_DROPLENGTH;
	}

      BYTE *s0 = s + x + y * p;

      for(int j = 0; j < RAIN_DROPLENGTH/2; j++)
	{
	  *s0 = c;
	  s0 += p;
	  *s0 = c;
	  s0 += p - 1;
	}
    }
}



static void OneThunderFrame(bool lightning)
{
  // the following timing constants are all
  //  relative to whenever the lightning actually 
  //  strikes - LT = lightning timing (in frames)
  const DWORD LT_LIGHTNINGSTOP = 12; // the palette brightness is normal after half a sec
  const DWORD LT_THUNDER       = 22; // thunder comes AFTER lightning, even in the real world
  const DWORD LT_TIMETOSTRIKE  = 90; // every three seconds, lightning may strike
  const int   LT_CHANCETOSTRIKE = 3; // the chances of it striking at that time are 1/3

  static DWORD probability_timer = 0;
  static DWORD bolt_timer = LT_THUNDER+1;

  if(++probability_timer > LT_TIMETOSTRIKE)
    {
      if(0 == rand()%LT_CHANCETOSTRIKE)
	{
	  // lightning!!
	  bolt_timer = 0;
	  if(true == lightning)
	    {
	      PalSetBrightnessFactor(LIGHTNING_BRIGHTNESS);
	    }
	}
      probability_timer = 0;
    }

  // see if we need to play the thunder sound
  bolt_timer += 1;

  if(LT_THUNDER == bolt_timer)
    {
      IDirectSoundBuffer *thunder_sound = sfx[WEATHERSOUND_THUNDER1 + rand()%NUM_THUNDERSOUNDS];

      if(NULL != thunder_sound)
	{
	  thunder_sound->Stop();
	  thunder_sound->Play(0,0,0);
	}
    }
  // see if we need to stop the bright lightning
  else if(LT_LIGHTNINGSTOP == bolt_timer)
    {
      PalSetBrightnessFactor(WEATHERSTATE_BRIGHTNESS[current_state]);
    }
}


void WtrNextState(int next_state)
{
  WriteLog("Entering WtrNextState(int) with parameter of %d" LogArg(next_state));
  // this function is called by the CNet class when the
  // SetWeatherMessage was received 
  current_state = next_state;
  WtrNextState();
  WriteLog("Leaving WtrNextState(int)");
}

int WtrCurrentState()
{
  return current_state;
}

void WtrSetSoundPlaybackFrequency(FIXEDNUM factor)
{
  DWORD x = FixedCnvFrom<long>(SOUNDRESOURCEFREQ * factor);
  // x is the new playback frequency of all sounds
  for(int i = 0; i < NUM_WEATHERSOUNDS; i++)
    {
      if(NULL != sfx[i])
	{
	  TryAndReport(sfx[i]->SetFrequency(x));
	}
    }
}
