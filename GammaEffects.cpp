#include "StdAfx.h"
#include "Certifiable.h"
#include "Graphics.h"
#include "Fixed.h"
#include "GammaEffects.h"
#include "Logger.h"
#include "Palette.h"
#include "SurfaceLock.h"
#include "SurfaceLock256.h"
#include "Weather.h"

enum {GESTATE_UNINITIALIZED, GESTATE_INITIALIZED}; // gamma effects states
enum {GETYPE_NONE,GETYPE_BLOOD,GETYPE_HEALTH,GETYPE_AMMO}; // gamma effects types

// effects parameters
const int GE_FRAMESTOBLOOD =  18;
const int GE_FRAMESTOAMMO =   6;
const int GE_FRAMESTOHEALTH = 6;

static int state = GESTATE_UNINITIALIZED;
static DWORD frames_since_effect_started;
static int effect_type;
static FIXEDNUM previous_health;

void GamOneFrame() {
  assert(GESTATE_UNINITIALIZED != state);

  if(GETYPE_NONE == effect_type) {
    return;
  }

  FIXEDNUM scale = FixedCnvTo<long>(frames_since_effect_started);
  int lim = 0;
  DWORD max_time;
  const FIXEDNUM weather = WtrBrightness();

  switch(effect_type) {
  case GETYPE_BLOOD:
    scale /= GE_FRAMESTOBLOOD;
    scale = FixedMul(weather, scale);
    PalSetBrightnessFactor(weather, scale, scale);

    max_time = GE_FRAMESTOBLOOD;
    break;
  case GETYPE_HEALTH:
    scale /= GE_FRAMESTOHEALTH;

    scale = FixedMul(scale, weather);
    PalSetBrightnessFactor(weather * 2 - scale, weather * 2 - scale, weather * 2 - scale);

    max_time =	GE_FRAMESTOHEALTH;
    break;
  case GETYPE_AMMO:
    scale /= GE_FRAMESTOAMMO;
    scale = FixedMul(scale, weather);
    PalSetBrightnessFactor(scale, weather, scale);

    max_time = GE_FRAMESTOAMMO;
  }
		
  if(++frames_since_effect_started > max_time) {
    effect_type = GETYPE_NONE;
  }

  PalApplyBrightnessFactor();

}

void GamInitialize(CGraphics &gr)
{
  assert(GESTATE_UNINITIALIZED == state);
  effect_type = GETYPE_NONE;
  state = GESTATE_INITIALIZED; // assume we are not using it
}

void GamRelease() {
  state = GESTATE_UNINITIALIZED;
}

void GamPickupAmmo()
{
  assert(GESTATE_UNINITIALIZED != state);
  effect_type = GETYPE_AMMO;
  frames_since_effect_started = 0;
}

void GamPickupHealth(FIXEDNUM current_health)
{
  assert(GESTATE_UNINITIALIZED != state);
  effect_type = GETYPE_HEALTH;
  frames_since_effect_started = 0;
  previous_health = current_health;
}

void GamGetShot(FIXEDNUM current_health)
{
  assert(GESTATE_UNINITIALIZED != state);
  effect_type = GETYPE_BLOOD;
  frames_since_effect_started = 0;
  previous_health = current_health;
}

bool GamShowHealth() {
  return GETYPE_HEALTH == effect_type || GETYPE_BLOOD == effect_type;
}

FIXEDNUM GamVirtualHealth(FIXEDNUM current_health) {
  const FIXEDNUM scale = FixedCnvTo<long>(frames_since_effect_started)
    / ((GETYPE_HEALTH == effect_type) ? GE_FRAMESTOHEALTH : GE_FRAMESTOBLOOD);
  const FIXEDNUM unclamped_health = previous_health
    + FixedMul(current_health - previous_health, scale);

  return unclamped_health < Fixed(0.0f) ? Fixed(0.0f) :
    unclamped_health > Fixed(1.0f) ? Fixed(1.0f) : unclamped_health;
}
