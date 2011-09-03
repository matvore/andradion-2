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
#include "Graphics.h"
#include "Fixed.h"
#include "Logger.h"
#include "GammaEffects.h"
#include "Palette.h"

// effects parameters
const int GE_FRAMESTOBLOOD =  18;
const int GE_FRAMESTOAMMO =   6;
const int GE_FRAMESTOHEALTH = 6;

static DWORD frames_since_effect_started;
static int effect_type;
static FIXEDNUM previous_health;

void GamOneFrame(FIXEDNUM overall_brightness) {
  if(GETYPE_NONE == effect_type) {
    PalSetBrightnessFactor(overall_brightness,
                           overall_brightness, overall_brightness);
    return;
  }

  FIXEDNUM scale = FixedCnvTo<long>(frames_since_effect_started);
  int lim = 0;
  DWORD max_time;

  switch(effect_type) {
  case GETYPE_BLOOD:
    scale /= GE_FRAMESTOBLOOD;
    scale = FixedMul(overall_brightness, scale);
    PalSetBrightnessFactor(overall_brightness, scale, scale);

    max_time = GE_FRAMESTOBLOOD;
    break;
  case GETYPE_HEALTH:
    scale /= GE_FRAMESTOHEALTH;

    scale = FixedMul(scale, overall_brightness);

    overall_brightness = overall_brightness * 2 - scale;
    
    PalSetBrightnessFactor(overall_brightness, overall_brightness,
                           overall_brightness);

    max_time = GE_FRAMESTOHEALTH;
    break;
  case GETYPE_AMMO:
    scale /= GE_FRAMESTOAMMO;
    scale = FixedMul(scale, overall_brightness);
    PalSetBrightnessFactor(scale, overall_brightness, scale);

    max_time = GE_FRAMESTOAMMO;
  }
		
  if(++frames_since_effect_started > max_time) {
    effect_type = GETYPE_NONE;
  }
}

void GamInitializeWithMenuPalette() {
  PalInitializeWithMenuPalette();
}

void GamInitializeWithIntroPalette() {
  PalInitializeWithIntroPalette();
}

const BYTE *GamInitialize(const BYTE *ifs) {return PalInitialize(ifs);}

void GamDoEffect(int type, FIXEDNUM current_health) {
  assert(type == GETYPE_NONE || type == GETYPE_BLOOD
         || type == GETYPE_HEALTH || type == GETYPE_AMMO);

  effect_type = type;
  frames_since_effect_started = 0;
  previous_health = current_health;
}

bool GamHealthChanging() {
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
