#include "StdAfx.h"
#include "Fixed.h"
#include "Graphics.h"
#include "MusicLib.h"
#include "Glue.h"
#include "Character.h"
#include "PowerUp.h"
#include "Net.h"

// constants used in this module
const DWORD FRAMESTOREGENERATEPOWERUP = 1500;
const DWORD FRAMESTOROTATEPOWERUP = 10;
const FIXEDNUM POWERUP_FATNESS = Fixed(20);

const FIXEDNUM FREQUENCYFACTOR[] = {Fixed(1.1),Fixed(0.9),Fixed(2),Fixed(4)};
const DWORD REVERSEGUNNOISE = (1 << 3);

CPowerUp::CPowerUp(const CPowerUp& r) {
  reference_count++;
  x = r.x;
  y = r.y;
  type = r.type;

  if(r.type < 0) {
    ammo_contained = new FIXEDNUM[NUM_WEAPONS];
    memcpy(ammo_contained,
           r.ammo_contained,
           sizeof(FIXEDNUM) * NUM_WEAPONS);
  } else {
    frames_since_picked_up = r.frames_since_picked_up;
  }
}

CPowerUp& CPowerUp::operator =(const CPowerUp& r) {
  if(this != &r) {
    x = r.x;
    y = r.y;
    if(r.type < 0) {
      if(type >= 0) {
        ammo_contained = new FIXEDNUM[NUM_WEAPONS];
      }
    
      memcpy(ammo_contained,
             r.ammo_contained,
             sizeof(FIXEDNUM) * NUM_WEAPONS);
    } else {
      if(type < 0) {
        delete [] ammo_contained;
      }
    
      frames_since_picked_up = r.frames_since_picked_up;
    }
  
    type = r.type;
  }
  
  return *this;
}

CPowerUp::CPowerUp() : frames_since_picked_up(0), type(0) {
  reference_count++;
  x = y = Fixed(-1); // mark both these as invisible
}

void CPowerUp::Draw() {
  if(x < 0) {
    assert(y < 0);
    return; // no draw
  }

  int bmp;

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

  FIXEDNUM tx = -GLUcenter_screen_x + Fixed(GAME_MODEWIDTH/2)
    + x - Fixed(TILE_WIDTH/2);
  FIXEDNUM ty = -GLUcenter_screen_y + Fixed(GAME_PORTHEIGHT/2)
    + y - Fixed(TILE_HEIGHT/2);

  GluDraw(bmp, FixedCnvFrom<long>(tx), FixedCnvFrom<long>(ty));
}

void CPowerUp::Setup(FIXEDNUM x_, FIXEDNUM y_, unsigned int type_) {
  assert(x_ >= 0);
  assert(y_ >= 0);
  assert(type_ < NUM_WEAPONS || POWERUP_HEALTHPACK == type_);
  
  x = x_;
  y = y_;
  if(type < 0) {
    delete [] ammo_contained;
    frames_since_picked_up = 0;
  }
  type = type_;
}

void CPowerUp::Setup(FIXEDNUM x_, FIXEDNUM y_, const FIXEDNUM *ammo) {
  assert(x_ >= 0);
  assert(y_ >= 0);
  x = x_;
  y = y_;
  
  if(type >= 0) {
    ammo_contained = new FIXEDNUM[NUM_WEAPONS];
  }
  
  type = 0; // find the weapon with the most ammo
  
  for(int i = 0; i < NUM_WEAPONS; i++) {
    ammo_contained[i] = ammo[i];
    if(ammo_contained[i] >= ammo_contained[type]) {
      type = i;
    }
  }
  
  type = ~type;
}

int CPowerUp::Collides(const CCharacter &ch) {
  if(x < 0) {
    // too soon to do this; we are invisible
    assert(y < 0);
    return POWERUPCOLLIDES_NOTHINGHAPPENED;
  }

  FIXEDNUM tx;
  FIXEDNUM ty;

  ch.GetLocation(tx,ty);
	
  if(abs(x - tx) < POWERUP_FATNESS &&
     abs(y - ty) < POWERUP_FATNESS &&
     !ch.Dead()) {
    if(type < 0) {
      // make sure the hero isn't full
      bool full_ammo = true;
      for(int i = 0; i < NUM_WEAPONS; i++) {
        full_ammo = full_ammo && ch.HasFullAmmo(i);
      }
      
      if(full_ammo) {
        return POWERUPCOLLIDES_NOTHINGHAPPENED;
      }
    } else if(!(GLUkeyb[DIK_F] & EIGHTHBIT)) {
      if(POWERUP_HEALTHPACK == type) {
        if(ch.HasFullHealth()) {
          // tell the player he can pick things
          //  up just for points
          GluPostForcePickupMessage();

          return POWERUPCOLLIDES_NOTHINGHAPPENED;
        }
      } else if(ch.HasFullAmmo(type)) {
        // tell the player he can pick things
        //  up just for points
        GluPostForcePickupMessage();

        return POWERUPCOLLIDES_NOTHINGHAPPENED;
      }
    }

    GluPlaySound(WAV_GUNNOISE,FREQUENCYFACTOR[Type()],(1 << Type()) & REVERSEGUNNOISE ? true : false);
    
    x *= -1;
    y *= -1;
    if(type >= 0) {
      frames_since_picked_up = 0;
    }

    if(!NetInGame()) {
      // add to the score
      GluChangeScore(GluScoreDiffPickup(Type()));
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
  if(type >= 0 && x < 0
     && ++frames_since_picked_up > FRAMESTOREGENERATEPOWERUP) {
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

void CPowerUp::Rotate()
{
  // make the weapons dance to the music if there is any
  if(beat_event)
    {
      if(WAIT_OBJECT_0 == WaitForSingleObject(beat_event,0))
	{
	  if(++rotation_direction >= RENDERED_DIRECTIONS)
	    {
	      rotation_direction = 0;
	    }
	}
      else if(!Segment())
	{
	  // we don't use a beat event anymore
	  CloseHandle(beat_event);
	  beat_event = 0;
	  Performance()->RemoveNotificationType(GUID_NOTIFICATION_MEASUREANDBEAT);
	}
      return;
    }

  if(++rotation_timer > FRAMESTOROTATEPOWERUP)
    {
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
  if(type < 0) {
    delete [] ammo_contained;
  }

  if(0 == --reference_count && beat_event) {
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
