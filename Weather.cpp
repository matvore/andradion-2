/** \file Weather.cpp
 * Contains static variables, constants, and functions that are internal
 * to the Weather module, which controls the game's weather.
 */

#include "stdafx.h"
#include "Fixed.h"
#include "Logger.h"
#include "Weather.h"
#include "Gfx.h"
#include "Character.h"
#include "Glue.h"
#include "Sound.h"

using namespace std;

const int MAX_RAINX = 1600, MAX_RAINY = 1200;
const int RAIN_DROPLENGTH = 10;
const int NUM_WEATHERSTATES = 15;
const int WEATHERSCRIPT_RANDOM = 11;
enum {HEROROOMSTATE_INSIDE,HEROROOMSTATE_OUTSIDE,HEROROOMSTATE_UNKNOWN};
const int RAIN_FRAMELENGTH = 3;

// weather state flags
const int WSF_LIGHTRAIN = 0x01;
const int WSF_HEAVYRAIN = 0x02;
const int WSF_THUNDER   = 0x04;
const int WSF_LIGHTNING = 0x08;
const int WSF_CRICKETS  = 0x10;

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
    0, 0, 0, 0, 0, 0
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
enum {WEATHERSOUND_LIGHTRAIN, WEATHERSOUND_HEAVYRAIN, WEATHERSOUND_CRICKETS,
      WEATHERSOUND_THUNDER1, WEATHERSOUND_THUNDER2, WEATHERSOUND_THUNDER3,
      NUM_WEATHERSOUNDS};
const int NUM_THUNDERSOUNDS = 3;

const int NUM_WEATHERSCRIPTS = 12;

const COLORREF CR_RAIN = RGB(255,255,255);

const FIXEDNUM LIGHTNING_BRIGHTNESS = Fixed(3.0f);

// the palette brightness is normal after half a sec
const DWORD LT_LIGHTNINGSTOP = 12;
  
// thunder comes AFTER lightning, even in the real world
const DWORD LT_THUNDER       = 22;

// every three seconds, lightning may strike
const DWORD LT_TIMETOSTRIKE  = 90;

// the chances of it striking at that time are 1/3
const int   LT_CHANCETOSTRIKE = 3; 

/** Determines whether the background music should be heard.
 * In general, music is not played when it is really dark or raining.
 * @return true iff the background music should be heard.
 */
static bool MusicAudible();

static void OneRainFrame(bool heavy_rain,
                         FIXEDNUM screen_x, FIXEDNUM screen_y);

static void OneThunderFrame(bool lightning);

/** Stops playback of all weather sounds.
 * @return true iff all the sounds are currently loaded.
 */
static bool Quiet();

/** Plays any looping sounds that are associated with the
 * current state. This function also takes into account whether or
 * not the player is inside. If the player is inside, then no weather
 * sounds are heard.
 */
static void StartSounds();

static int hero_room_state;
static int script_index;
static DWORD frames_since_start; // records number of frames since start of
                                 // current weather state  
static unsigned int current_state;
static BYTE rain_color;
static POINT rain_drops[NUM_RAINFRAMES][NUM_RAINDROPS];
static DWORD bolt_timer = LT_THUNDER+1;
static FIXEDNUM brightness = Fixed(1.0f);

void WtrAnalyzePalette() {rain_color = Gfx::Get()->MatchingColor(CR_RAIN);}

void WtrBeginScript(int script) {
  assert(current_state < NUM_WEATHERSTATES);
  
  script_index = script;

  // set current weather state by finding the first one
  //  for a given script index
  current_state = WEATHERPATTERN_STARTPOINT[script_index];

  if(current_state < 0 || current_state >= NUM_WEATHERSTATES) {
    current_state = rand()%NUM_WEATHERSTATES;
  }

  frames_since_start = 0;
  hero_room_state = HEROROOMSTATE_UNKNOWN;
}

FIXEDNUM WtrBrightness() {
  assert(current_state < NUM_WEATHERSTATES);

  if (HEROROOMSTATE_UNKNOWN == hero_room_state) {
    // reuse brightness value from the previous frame
  } else if (HEROROOMSTATE_INSIDE == hero_room_state) {
    brightness = Fixed(1.0f);
  } else if ((WEATHERSTATE_FLAGS[current_state] & WSF_LIGHTNING)
             && bolt_timer <= LT_LIGHTNINGSTOP) {
    brightness = LIGHTNING_BRIGHTNESS;
  } else {
    brightness = WEATHERSTATE_BRIGHTNESS[current_state];
  }

  return brightness;
}

int WtrCurrentState() {
  assert(current_state < NUM_WEATHERSTATES);
  return current_state;
}

void WtrEndScript() {Quiet();}

void WtrInitialize() {
  // setup the raindrops
  for(int frame = 0; frame < NUM_RAINFRAMES; frame++) {
      for(int drop = 0; drop < NUM_RAINDROPS; drop++) {
        // address the point in question
        POINT *p = &rain_drops[frame][drop];
        
        p->x = rand()%MAX_RAINX;
        p->y = rand()%MAX_RAINY;
      }
  }
}

int WtrOneFrameIndoors() {
  assert(current_state < NUM_WEATHERSTATES);

  // hero is indoors
  // check if hero was just outdoors
  if(HEROROOMSTATE_INSIDE != hero_room_state) {
    hero_room_state = HEROROOMSTATE_INSIDE;
    StartSounds();
    
    return MusicAudible()
      ? WTROF_TURNMUSICON : WTROF_TURNMUSICOFF;
  } else {
    return WTROF_NOCHANGE;
  }
}

int WtrOneFrameOutdoors() {
  FIXEDNUM screen_x = GluContext()->center_screen_x;
  FIXEDNUM screen_y = GluContext()->center_screen_y;
  int music_op = WTROF_NOCHANGE;

  assert(current_state < NUM_WEATHERSTATES);

  // hero is outside
  // check if hero just left a room
  if(HEROROOMSTATE_OUTSIDE != hero_room_state) {
    hero_room_state = HEROROOMSTATE_OUTSIDE;
    StartSounds();
    music_op = MusicAudible()
      ? WTROF_TURNMUSICON : WTROF_TURNMUSICOFF;
  }

  // perform lightning/thunder/rain logic
  DWORD f = WEATHERSTATE_FLAGS[current_state];
  if(WSF_THUNDER & f) {
    if(WSF_LIGHTNING & f) {
      OneThunderFrame(true);
    } else {
      OneThunderFrame(false);
    }
  }
  
  if(WSF_LIGHTRAIN & f) {	
    OneRainFrame(false, screen_x, screen_y);
  } else if(WSF_HEAVYRAIN & f) {
    OneRainFrame(true, screen_x, screen_y);
  }

  assert(current_state < NUM_WEATHERSTATES);

  return music_op;
}

bool WtrPermitStateChange() {
  if(++frames_since_start < WEATHERSTATE_LENGTH[current_state]) {
    return false;
  } else {
    // on to the next weather state
    frames_since_start = 0;
    hero_room_state = HEROROOMSTATE_UNKNOWN;
    
    // advance to the next state
    if(WEATHERSCRIPT_RANDOM == script_index) {
      // doing the random weather script
      current_state = rand()%NUM_WEATHERSTATES;
    } else if (WEATHERPATTERN_ENDPOINT[script_index] != current_state &&
               NUM_WEATHERSTATES == ++current_state) {
      current_state = 0;
    }

    return true;
  }
}

void WtrRelease() {
  WriteLog("Release Wtr module\n");

  assert(current_state < NUM_WEATHERSTATES);
}

void WtrSetSoundPlaybackFrequency(unsigned long freq) {
  assert(current_state < NUM_WEATHERSTATES);
  
  for(int i = 0; i < NUM_WEATHERSOUNDS; i++) {
    IDirectSoundBuffer *snd = SndSound(i).Get();
    if(snd) {
      TryAndReport(snd->SetFrequency(freq));
    }
  }
}

void WtrSetState(int next_state) {
  assert(next_state < NUM_WEATHERSTATES
         && current_state < NUM_WEATHERSTATES);
  current_state = next_state;
  frames_since_start = 0;
  hero_room_state = HEROROOMSTATE_UNKNOWN;
}

static bool MusicAudible() {
  return WEATHERSTATE_PLAYMUSIC & (1 << current_state)
    || HEROROOMSTATE_INSIDE == hero_room_state;
}

static void OneRainFrame(bool heavy_rain,
                         FIXEDNUM screen_x, FIXEDNUM screen_y) {
  static DWORD frame_count = 0;
  int frame_index = ++frame_count / RAIN_FRAMELENGTH;

  Gfx::Get()->Lock();
  
  assert(current_state < NUM_WEATHERSTATES);

  if(frame_index >= NUM_RAINFRAMES) {
    frame_index = 0;
    frame_count = 0;
  }

  // address the set of rain drops we need
  POINT *r = rain_drops[frame_index];

  // surface is locked already

  // now the rest of the function
  //  is different depending on light rain or hard rain

  // make typing shortcuts
  BYTE c = rain_color;
  FIXEDNUM diff_x = FixedCnvFrom<long>(screen_x) / -2;
  FIXEDNUM diff_y = FixedCnvFrom<long>(screen_y) / -2;

  int drops = heavy_rain ? NUM_RAINDROPS : NUM_RAINDROPS/4;
  // show all drops, and all at an angle
  for(int i = 0; i < drops; i++) {
    int x = r[i].x + diff_x;

    x %= Gfx::Get()->GetVirtualBufferWidth()-RAIN_DROPLENGTH/2;

    if(x < RAIN_DROPLENGTH/2)	{
      x += Gfx::Get()->GetVirtualBufferWidth()-RAIN_DROPLENGTH/2;
    }

    int y = r[i].y + diff_y;

    y %= (Gfx::Get()->GetVirtualBufferHeight()-RAIN_DROPLENGTH);	

    if(y < 0) {
      y += Gfx::Get()->GetVirtualBufferHeight()-RAIN_DROPLENGTH;
    }

    BYTE *s = Gfx::Get()->GetLockPtr(x, y);

    for(int j = 0; j < RAIN_DROPLENGTH/2; j++) {
      *s = c;
      s += Gfx::Get()->GetLockPitch();
      *s = c;
      s += Gfx::Get()->GetLockPitch() - 1;
    }
  }

  assert(current_state < NUM_WEATHERSTATES);

  Gfx::Get()->Unlock();
}

static void OneThunderFrame(bool lightning) {
  assert(current_state < NUM_WEATHERSTATES);
  
  // the following timing constants are all
  //  relative to whenever the lightning actually 
  //  strikes -- LT = lightning timing (in frames)

  static DWORD probability_timer = 0;

  if(++probability_timer > LT_TIMETOSTRIKE) {
    if(0 == rand()%LT_CHANCETOSTRIKE)	{
      // lightning!!
      bolt_timer = 0;
    }
    probability_timer = 0;
  }

  // see if we need to play the thunder sound
  bolt_timer += 1;

  if(LT_THUNDER == bolt_timer) {
    IDirectSoundBuffer *thunder_sound
      = SndSound(WEATHERSOUND_THUNDER1 + rand()%NUM_THUNDERSOUNDS).Get();

    if(thunder_sound) {
      thunder_sound->Stop();
      thunder_sound->Play(0,0,0);
    }
  }

  assert(current_state < NUM_WEATHERSTATES);
}

static bool Quiet() {
  for(int i = 0; i < NUM_WEATHERSOUNDS; i++) {
    IDirectSoundBuffer *snd = SndSound(i).Get();
    if(snd) {
      snd->Stop();
    } else {
      return false;
    }
  }

  return true;
}

static void StartSounds() {
  assert(current_state < NUM_WEATHERSTATES);

  if(Quiet() && HEROROOMSTATE_OUTSIDE == hero_room_state) {
    DWORD f = WEATHERSTATE_FLAGS[current_state]; 
    if(f & WSF_CRICKETS) {
      // play cricket noise looping
      SndSound(WEATHERSOUND_CRICKETS)->Play(0, 0, DSBPLAY_LOOPING);
    } else if(f & WSF_LIGHTRAIN) { 
      // play light rain sound
      SndSound(WEATHERSOUND_LIGHTRAIN)->Play(0, 0, DSBPLAY_LOOPING);
    } else if(f & WSF_HEAVYRAIN) {
      // play hard rain sound
      SndSound(WEATHERSOUND_HEAVYRAIN)->Play(0, 0, DSBPLAY_LOOPING);
    }
  }
}
