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
#include "Fixed.h"
#include "Gfx.h"
#include "MusicLib.h"
#include "Character.h"
#include "Glue.h"
#include "PowerUp.h"
#include "Net.h"
#include "Keyboard.h"
#include "Sound.h"

// constants used in this module
const DWORD FRAMESTOREGENERATEPOWERUP = 1500;
const DWORD FRAMESTOROTATEPOWERUP = 10;
const FIXEDNUM POWERUP_FATNESS = Fixed(20);

const FIXEDNUM FREQUENCYFACTOR[] = {Fixed(1.1),Fixed(0.9),Fixed(2),Fixed(4)};
const DWORD REVERSEGUNNOISE = (1 << 3);

CPowerUp::CPowerUp() : frames_since_picked_up(0), type(0) {
  reference_count++;
  x = y = Fixed(-1); // mark both these as invisible
}

void CPowerUp::Draw() {
  int bmp;
  FIXEDNUM tx, ty;
  Context *cxt = GluContext();

  if(x < 0) {
    assert(y < 0);
    return; // no draw
  }

  switch(type) {
  case ~POWERUP_PISTOLAMMO:
  case POWERUP_PISTOLAMMO:
    bmp = BMPSET_PISTOL + rotation_direction;
    break;
  case ~POWERUP_MACHINEGUNAMMO:
  case POWERUP_MACHINEGUNAMMO:
    bmp = BMPSET_MACHINEGUN + rotation_direction;
    break;
  case ~POWERUP_BAZOOKAAMMO:
  case POWERUP_BAZOOKAAMMO:
    bmp = BMPSET_BAZOOKA + rotation_direction;
    break;
  default:
    assert(POWERUP_HEALTHPACK == type);
    bmp = BMP_HEALTH;
  }

  tx = -cxt->center_screen_x + Fixed(GAME_MODEWIDTH/2) + x
    - Fixed(TILE_WIDTH/2);
  ty = -cxt->center_screen_y + Fixed(GAME_MODEHEIGHT/2) + y
    - Fixed(TILE_HEIGHT/2);

  cxt->Draw(bmp, FixedCnvFrom<long>(tx), FixedCnvFrom<long>(ty));
}

void CPowerUp::Setup(FIXEDNUM x_, FIXEDNUM y_, unsigned int type_) {
  assert(x_ >= 0);
  assert(y_ >= 0);
  assert(type_ < WEAPON_COUNT || POWERUP_HEALTHPACK == type_);
  
  x = x_;
  y = y_;
  if(type < 0) {
    frames_since_picked_up = 0;
  }
  type = type_;
}

void CPowerUp::Setup(FIXEDNUM x_, FIXEDNUM y_, const FIXEDNUM *ammo) {
  assert(x_ >= 0);
  assert(y_ >= 0);
  x = x_;
  y = y_;
  
  type = 0; // find the weapon with the most ammo
  
  for(int i = 0; i < WEAPON_COUNT; i++) {
    ammo_contained[i] = ammo[i];
    if(ammo_contained[i] >= ammo_contained[type]) {
      type = i;
    }
  }
  
  type = ~type;
}

int CPowerUp::CollidesWithHero() {
  FIXEDNUM tx;
  FIXEDNUM ty;
  Context *cxt = GluContext();
  Character::Ptr ch = cxt->hero;

  if(x < 0) {
    // too soon to do this; we are invisible
    assert(y < 0);
    return POWERUPCOLLIDES_NOTHINGHAPPENED;
  }

  ch->GetLocation(tx,ty);
	
  if(abs(x - tx) < POWERUP_FATNESS && abs(y - ty) < POWERUP_FATNESS &&
     !ch->Dead()) {
    if(type < 0) {
      // make sure the hero isn't full
      bool full_ammo = true;
      for(int i = 0; i < WEAPON_COUNT; i++) {
        full_ammo = full_ammo && cxt->AmmoFull(i);
      }
      
      if(full_ammo) {
        return POWERUPCOLLIDES_NOTHINGHAPPENED;
      }
    } else if (!KeyPressed(DIK_F)) {
      if(POWERUP_HEALTHPACK == type) {
        if (ch->HasFullHealth()) {
          // tell the player he can pick things
          //  up just for points
          GluPostForcePickupMessage();

          return POWERUPCOLLIDES_NOTHINGHAPPENED;
        }
      } else if(cxt->AmmoFull(type)) {
        // tell the player he can pick things
        //  up just for points
        GluPostForcePickupMessage();

        return POWERUPCOLLIDES_NOTHINGHAPPENED;
      }
    }

    SndPlay(WAV_GUNNOISE, FREQUENCYFACTOR[Type()],
            bool((1 << Type()) & REVERSEGUNNOISE));
    
    x *= -1;
    y *= -1;
    if (type >= 0) {
      frames_since_picked_up = 0;
    }

    if (!NetInGame()) {
      // add to the score
      GluContext()->ChangeScore(GluScoreDiffPickup(Type()));
    }

    return (type < 0)
      ? POWERUPCOLLIDES_PICKEDUPANDWILLNOTREGENERATE
      : POWERUPCOLLIDES_PICKEDUPANDWILLREGENERATE;
  } else {
    return POWERUPCOLLIDES_NOTHINGHAPPENED;
  }
}

void CPowerUp::Logic() {
  // this function is used to see if we need to regenerate
  //  if this function was called, we already know we are in an mp game
  if(type >= 0 && x<0 && ++frames_since_picked_up > FRAMESTOREGENERATEPOWERUP) {
    assert(y < 0);
    x *= -1;
    y *= -1;
  }
}

// statics
DWORD CPowerUp::rotation_timer;
int CPowerUp::rotation_direction = DEAST;
HANDLE CPowerUp::beat_event = 0;
unsigned int CPowerUp::reference_count = 0;

void CPowerUp::Rotate() {
  // make the weapons dance to the music if there is any
  if(beat_event) {
      if(WAIT_OBJECT_0 == WaitForSingleObject(beat_event,0)) {
        if(++rotation_direction >= RENDERED_DIRECTIONS) {
	      rotation_direction = 0;
	    }
      } else if(!Segment()) {
        // we don't use a beat event anymore
        CloseHandle(beat_event);
        beat_event = 0;
        Performance()->RemoveNotificationType(GUID_NOTIFICATION_MEASUREANDBEAT);
      }
      return;
  }

  if(++rotation_timer > FRAMESTOROTATEPOWERUP) {
    rotation_timer = 0;
    rotation_direction = (rotation_direction + 1) % RENDERED_DIRECTIONS;
  }

  if(Segment() && SUCCEEDED(Performance()->AddNotificationType
                            (GUID_NOTIFICATION_MEASUREANDBEAT))) {
    beat_event = CreateEvent(NULL, FALSE, FALSE, NULL);

    // tell the perf object to signal that event whenever there is
    //  a beat
    Performance()->SetNotificationHandle(beat_event, 0);
  }
}

CPowerUp::~CPowerUp() {
  if(!--reference_count && beat_event) {
    // this is the last of the PowerUp objects

    CloseHandle(beat_event);
    beat_event = 0;

    // remove notification type
    if(Performance()) {
      Performance()->RemoveNotificationType
        (GUID_NOTIFICATION_MEASUREANDBEAT);
    }
  }
}
