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

// if this line is not commented, then enemies will not attack you unless fired upon
//#define TESTMODE 

#include "StdAfx.h"
#include "Fixed.h"
#include "GfxLock.h"
#include "Logger.h"
#include "GammaEffects.h"
#include "Net.h"
#include "PowerUp.h"
#include "Glue.h"
#include "Fire.h"
#include "Character.h"
#include "Graphics.h"

using std::pair;
using std::max;
using std::min;
using std::swap;

// a small secret, if all the arrow keys are
//  pressed except one direction, the hero
//  will run in the direction opposite of the unpressed button

const FIXEDNUM HEALTHBONUSPERPACK[] = {Fixed(0.4),Fixed(0.25),Fixed(0.20)};
const int AMMOCAPACITY[] = {130, 500, 6};
const FIXEDNUM AMMOSTARTING[] = {Fixed(25 ),Fixed( 0 ),Fixed(0)};
const FIXEDNUM MIN_SAFEDISTANCESQUARED = Fixed(150*150);
const FIXEDNUM MIN_ALIGNMENT = Fixed(3); // how close an alien should be before it starts firing
const int FRAMESTORECOVER_HERO = 15;
const int FRAMESTORECOVER_ALIEN = 15;
const FIXEDNUM HURTFACTOR = Fixed(0.5);
const FIXEDNUM HEROSPEED            = Fixed(2.80f);
const FIXEDNUM HEROSPEED_MPFACTOR   = Fixed(1.20f);
const FIXEDNUM HEROSPEED_HURTFACTOR = Fixed(0.75f);
const FIXEDNUM HEROSPEED_RUNNINGFACTOR = Fixed(1.414f);

// speeds are in pixels per frame
const FIXEDNUM ALIENSPEED[3] = {Fixed(1), Fixed(2), Fixed(3)};

const int FRAMESTORELOAD[NUM_WEAPONS] = {15, 1, 100};

const FIXEDNUM HERO_WEAPONDAMAGE[][NUM_WEAPONS-1] =
{
  {Fixed(0.05f),Fixed(0.02f)}, // damage on dangnabit
  {Fixed(0.07f),Fixed(0.03f)}, // damage on my dear child
  {Fixed(0.1f) ,Fixed(0.035f)} // damage on danger danger
};

const FIXEDNUM ENEMY_WEAPONDAMAGE[][NUM_WEAPONS-1] =
{
  {Fixed(0.3f),Fixed(0.05f)},
  {Fixed(0.2f),Fixed(0.03f)},
  {Fixed(0.12f),Fixed(0.02f)}
};

const int AMMOPERPACK[][NUM_WEAPONS] =
{
  {Fixed(20),Fixed(70),Fixed(2)}, // ammo per pack on dangnabit
  {Fixed(15),Fixed(60),Fixed(2)},
  {Fixed(15),Fixed(55),Fixed(2)} // ammo per pack on my dear child
};

// how much damage is caused in one frame in each difficulty level to the hero
const FIXEDNUM HERO_BAZOOKADAMAGE[] =
{
  Fixed(0.003f),
  Fixed(0.006f),
  Fixed(0.010f)
};

// how much damage is caused in one frame in each difficulty level to any enemy
const FIXEDNUM ENEMY_BAZOOKADAMAGE[] =
{
  Fixed(0.024f),
  Fixed(0.020f),
  Fixed(0.018f)
};

const int FRAMESTODIE = 30;
const int FRAMESTOFIRE = 60; // how long an enemy will try firing in one spot
const int FRAMESTOSTEP = 6;
const int FRAMESTOSWEARAGAIN = 10;
const int HEALTH_METER_R = 255;
const int HEALTH_METER_G = 0;
const int HEALTH_METER_B = 0;
const int AMMO_METER_R = 0;
const int AMMO_METER_G = 255;
const int AMMO_METER_B = 255;

// health meter is a meter measuring Turner's level of pain, taking up the bottom of the screen
const int HEALTH_METER_DISTANCE_FROM_SIDES_OF_SCREEN = 5;
const int HEALTH_METER_HEIGHT = 15;

// the ammo meter takes up a smaller portion of the screen in the lower right corner, and it is vertical
const int AMMO_METER_DISTANCE_FROM_SIDES_OF_SCREEN = 2;
const int AMMO_METER_HEIGHT = 30;
const int AMMO_METER_WIDTH = 8;

const int RECOIL_RANGE[] = {5, 5, 0};
const int RECOIL_RANGE_OFFSET[] = {-2, -2, 0};
const FIXEDNUM RECOIL_UNIT = Fixed(1.5);

CCharacter hero;
vector<CCharacter> enemies;

// used for enemies or hero in sp
void CCharacter::Setup(FIXEDNUM x,FIXEDNUM y,int model_,bool doing_mp) {
  this->model                   = model_;
  this->coor.first.x            = x;
  this->coor.first.y            = y+Fixed(TILE_HEIGHT/2);
  this->coor.second             = this->coor.first;
  this->health                  = Fixed(1);
  this->direction               = DSOUTH;
  this->frames_in_this_state    = 0;
  this->frames_not_having_sworn = 0;
  this->frames_since_last_fire  = 0;

  if(CHAR_TURNER != this->model && !doing_mp) {
    // simple single-player alien guy
    this->current_weapon = model_;
    this->state = CHARSTATE_UNKNOWING;
  } else {
    this->reset_gamma = true;
    this->current_weapon = WEAPON_PISTOL;
    this->state = CHARSTATE_ALIVE;
    ResetAmmo();
  }
}

void CCharacter::PlaySound()
{
  int wav;
  FIXEDNUM x_dist;
  FIXEDNUM y_dist;
  x_dist = hero.X();
  y_dist = hero.Y();
  x_dist *= -1;
  y_dist *= -1;
  x_dist += this->coor.first.x;
  y_dist += this->coor.first.y - Fixed(TILE_HEIGHT/2);
  if(CHAR_TURNER > this->model)
    {
      wav = (CHARSTATE_DYING == this->state) ? WAVSET_ALIENDEATH : WAVSET_ALIENHIT;
      wav += rand()%WAVSINASET;
    }
  else
    {
      wav = WAVSET_FIRSTNONALIEN;
      wav += (this->model - CHAR_TURNER) * (WAVSINASET+1);
      wav += (CHARSTATE_DYING == this->state) ? WAVSINASET : (rand()%WAVSINASET);
    }
  GluPlaySound(wav,x_dist,y_dist);
}

bool CCharacter::Logic(GfxLock& lock, const CCharacter& target)
{
  // xd and yd are the distance in each dimension from the target
  FIXEDNUM xd = abs(GLUcenter_screen_x - (coor.first.x));
  FIXEDNUM yd = abs(GLUcenter_screen_y - (coor.first.y - Fixed(TILE_HEIGHT/2)));

  // make sure we aren't off the side
  //  of the screen
  if(xd >= Fixed((GAME_MODEWIDTH + TILE_WIDTH)/2)
     || yd >= Fixed((GAME_PORTHEIGHT + TILE_HEIGHT)/2)) {
    return false;
  }

  if(CHARSTATE_HURT != state && CHARSTATE_DYING != state) {
    // we can consider not going through logic this time
    if (CHARSTATE_DEAD == target.state || CHARSTATE_DEAD == state
        || GluWalkingData(target.X(),target.Y())
        != GluWalkingData(coor.first.x,
                          coor.first.y - Fixed(TILE_HEIGHT/2))) {
      return true;
    }
  }
	
  xd = target.coor.first.x - this->coor.first.x;
  yd = target.coor.first.y - this->coor.first.y;
  FIXEDNUM speed;

  frames_since_last_fire++;

  if(CHARSTATE_UNKNOWING == state) {
#if !defined(TESTMODE)
    // figure out if the hero is close enough to be noticed
    xd = FixedMul(xd,xd); 
    yd = FixedMul(yd,yd); 
    if(MIN_SAFEDISTANCESQUARED > xd + yd) {
      // this computer player is no longer unaware of turner's presence
      //  so change the state to ALIVE
      state = CHARSTATE_ALIVE;
    }
#endif

    return true;
  }

  hero.CheckForEnemyCollision(*this);

  FIXEDNUM xm, ym; // movement

  // check for timeout of state
  if(CHARSTATE_ALIVE==this->state || CHARSTATE_WALKING == this->state) {
    // see if we are close enough to start firing
    if(abs(xd) <= MIN_ALIGNMENT) {
      state = CHARSTATE_FIRING;
			
      // lining up horizontally and firing vertically
      direction = yd > 0 ? DSOUTH : DNORTH;

      return true;
    } else if(abs(yd) <= MIN_ALIGNMENT) {
      state = CHARSTATE_FIRING;

      // lining up vertically and firing horizontally
      direction = xd > 0 ? DEAST : DWEST;

      return true;
    } else {
      speed = ALIENSPEED[this->model];

      // use the filter movement glue function hypothetically to see if 
      //  aligning horizontally would be a good idea
      pair<POINT,POINT> hypothetical = coor;
      coor.second.y += yd;

      TryToMove(lock);

      // get closer to the target
      if(abs(xd) > abs(yd) && abs(coor.first.y + yd - coor.second.y) <= MIN_ALIGNMENT) {
	direction = yd > 0 ? DSOUTH : DNORTH;
      } else {
	direction = xd > 0 ? DEAST : DWEST;
      }

      // restore our coordinates we had backed up
      //  in the hypothetical pair
      coor = hypothetical;
    }

    // now use xd and yd to figure out
    //  how to walk in the specified direction
    GluInterpretDirection(direction, xm, ym);

    xm = FixedMul(xm, speed);
    ym = FixedMul(ym, speed);

    if(abs(xm) > abs(xd)) {
      coor.second.x = target.coor.first.x;
      state = CHARSTATE_FIRING;
      direction = yd > 0 ? DSOUTH : DNORTH;
    } else if(abs(ym) > abs(yd)) {
      coor.second.y = target.coor.first.y;
      state = CHARSTATE_FIRING;
      direction = xd > 0 ? DEAST : DWEST;
    } else {
      coor.second.x=coor.first.x + xm;
      coor.second.y=coor.first.y + ym;
      if(++frames_in_this_state > FRAMESTOSTEP) {
	state = !state;
	frames_in_this_state = 0;
      }
    }

    TryToMove(lock);
  } else if(CHARSTATE_FIRING == state) {
    if(++frames_in_this_state > FRAMESTOFIRE) {
      state = CHARSTATE_WALKING;
      frames_in_this_state = 0;
    }

    TryToFire();
  } else if(++frames_in_this_state > FRAMESTORECOVER_ALIEN) {
    state = CHARSTATE_DYING == state ? CHARSTATE_DEAD : CHARSTATE_WALKING;
    frames_in_this_state = 0;
  }
  
  return true;
}

void CCharacter::Logic(GfxLock& lock) {
  static bool firing_last_frame = false;
  
  frames_since_last_fire++;

  if(coor.first.x < 0 || coor.first.y < 0) {
    GluGetRandomStartingSpot(coor.first);
    coor.first.y += Fixed(TILE_HEIGHT/2);
    coor.second = coor.first;
  }

  if(CHARSTATE_ALIVE == state || CHARSTATE_WALKING == state) {
    if(++frames_in_this_state > FRAMESTOSTEP
       && CHARSTATE_HURT != state) {
      frames_in_this_state = 0;
      state = !state;
    }

    // change weapons if necessary
    if((GLUkeyb[DIK_1] & EIGHTHBIT)
       && WEAPON_PISTOL != current_weapon) {
      if(!(GLUkeyb[DIK_2] & EIGHTHBIT)
         && !(GLUkeyb[DIK_3] & EIGHTHBIT)) {
        GluPlaySound(WAV_GUNNOISE, Fixed(1), false);
        current_weapon = WEAPON_PISTOL;
      }
    } else if((GLUkeyb[DIK_2] & EIGHTHBIT)
              && WEAPON_MACHINEGUN != current_weapon) {
      if(!(GLUkeyb[DIK_1] & EIGHTHBIT)
         && !(GLUkeyb[DIK_3] & EIGHTHBIT)) {
        GluPlaySound(WAV_GUNNOISE, Fixed(1), false);
        current_weapon = WEAPON_MACHINEGUN;
      }
    } else if((GLUkeyb[DIK_3] & EIGHTHBIT)
              && WEAPON_BAZOOKA != current_weapon) {
      if(!(GLUkeyb[DIK_1] & EIGHTHBIT)
         && !(GLUkeyb[DIK_2] & EIGHTHBIT)) {
        GluPlaySound(WAV_GUNNOISE, Fixed(1), false);
        current_weapon = WEAPON_BAZOOKA;
      }
    }
  } else if(CHARSTATE_HURT == state) {
    if(++frames_in_this_state > FRAMESTORECOVER_HERO) {
      frames_in_this_state = 0;
      state = CHARSTATE_ALIVE;
      reset_gamma = true;
    }
  } else if(CHARSTATE_DYING == state) {
    reset_gamma = true;
    GluPostSPKilledMessage();

    if(++frames_in_this_state > FRAMESTODIE) {
      reset_gamma = true;
      frames_in_this_state = 0;
      if(NetInGame()) {
        // resurrect ourselves into a new starting spot
        GluGetRandomStartingSpot(coor.first);
        coor.first.y += Fixed(TILE_HEIGHT/2);
        coor.second = coor.first;
        state = CHARSTATE_ALIVE;
        health = Fixed(1);
        ResetAmmo();
        current_weapon = WEAPON_PISTOL;
      } else {
        state = CHARSTATE_DEAD;
      }
    }
  }

  NetSetWeapon(current_weapon);

  bool walked = true;

  // now take care of movement
  if(CHARSTATE_WALKING == state
     || CHARSTATE_ALIVE == state
     || CHARSTATE_HURT == state) {
    DWORD directional_buttons = 0;
    bool running;
    int old_direction = direction;
    
    if(GLUkeyb[DIK_RIGHT] & EIGHTHBIT) {directional_buttons |= 1;}
    if(GLUkeyb[DIK_UP   ] & EIGHTHBIT) {directional_buttons |= 2;}
    if(GLUkeyb[DIK_LEFT ] & EIGHTHBIT) {directional_buttons |= 4;}
    if(GLUkeyb[DIK_DOWN ] & EIGHTHBIT) {directional_buttons |= 8;}

    switch(directional_buttons) {
    case 11: case 1 : direction = DEAST ; break;
    case 7 : case 2 : direction = DNORTH; break;
    case 14: case 4 : direction = DWEST ; break;
    case 13: case 8 : direction = DSOUTH; break;
    case 3 : direction = DNE; break;
    case 6 : direction = DNW; break;
    case 9 : direction = DSE; break;
    case 12: direction = DSW; break;
    default: // turner ain't moving his feet
      walked = false;
    }

    // check if three buttons are pressed
    running = (11 == directional_buttons ||
               7 == directional_buttons ||
               14 == directional_buttons ||
               13 == directional_buttons ||
               ((GLUkeyb[DIK_LCONTROL] & EIGHTHBIT
                 || GLUkeyb[DIK_RCONTROL] & EIGHTHBIT)
                && direction < RENDERED_DIRECTIONS));
		
    if(walked) {
      Walk(running);

      if((GLUkeyb[DIK_LSHIFT] & EIGHTHBIT)
         || (GLUkeyb[DIK_RSHIFT] & EIGHTHBIT)) {
          direction = old_direction;
      }
      
      TryToMove(lock);
    }
  } else {
    walked = false;
  }

  NetSetPosition(FixedCnvFrom<long>(coor.first.x),
                 FixedCnvFrom<long>(coor.first.y) - TILE_HEIGHT/2,
                 direction);
  
  // now see if they are trying to fire the gun . . .
  if(GLUkeyb[DIK_SPACE] & EIGHTHBIT && 0 != ammo[current_weapon]) {
    TryToFire();
    if(!firing_last_frame && WEAPON_BAZOOKA != current_weapon) {
      firing_last_frame = true;
    }		
  } else {
    if(firing_last_frame && WEAPON_BAZOOKA != current_weapon) {
      firing_last_frame = false;
      if(WEAPON_PISTOL == current_weapon) {
        frames_since_last_fire = FRAMESTORELOAD[WEAPON_PISTOL];
      }
    }
  }

  NetFireMachineGun(WEAPON_MACHINEGUN == current_weapon
                    && firing_last_frame);

  if(!walked) {
    if(CHARSTATE_WALKING == state || CHARSTATE_ALIVE == state) {
      frames_in_this_state = 0;
    }
  } else {
    PowerUpCollisions();
  }
}

bool CCharacter::DrawCharacter() {
  int bmp, target_x, target_y;
  int vd; // virtual direction
  FIXEDNUM tx, ty; // screen coordinates to put character

  // calculate visual direction
  vd = direction >= RENDERED_DIRECTIONS ? direction - RENDERED_DIRECTIONS : direction;

  if(CHARSTATE_DEAD == state) {
    bmp = BMP_BLOODSTAIN;
  } else if(CHARSTATE_DYING == state) {
    bmp = BMPSET_DECAPITATE+(rand()&1);
  } else {
    bmp = BMPSET_CHARACTERS + model * ANIMATIONFRAMESPERCHARACTER + vd;

    if(CHARSTATE_WALKING == state) {
      bmp += RENDERED_DIRECTIONS;
    }
  }

  bool donot_draw_weapon =
    (model == current_weapon && CHAR_EVILTURNER != model) ||
    (CHARSTATE_DEAD == state) ||
    (WEAPON_PISTOL == current_weapon && CHAR_CHARMIN == model);
	
  tx = coor.first.x - GLUcenter_screen_x + Fixed(GAME_MODEWIDTH/2) - Fixed(TILE_WIDTH/2);
  ty = coor.first.y - GLUcenter_screen_y + Fixed(GAME_PORTHEIGHT/2) - Fixed(TILE_HEIGHT);

  target_x = FixedCnvFrom<long>(tx);
  target_y = FixedCnvFrom<long>(ty);

  if(donot_draw_weapon) {
    GluDraw(bmp, target_x, target_y);
  } else {
    int weapon_bmp = BMPSET_WEAPONS
      + this->current_weapon * RENDERED_DIRECTIONS + vd;

    if(DSOUTH == vd || DEAST == vd || CHAR_CHARMIN == model) {
      swap(bmp, weapon_bmp);
    }

    GluDraw(weapon_bmp, target_x, target_y);
    GluDraw(bmp, target_x, target_y);
  }

  // draw blood if we are hurt
  if(CHARSTATE_HURT == state) {
    GluDraw(BMPSET_DECAPITATE+(rand()&1), target_x, target_y);
  }

  if (coor.first.x != coor.second.x || coor.first.y != coor.second.y) {
    coor.first = coor.second;
    return true;
  } else {
    return false;
  }
}

void CCharacter::DrawMeters(int show_health) {
  RECT target;

  if(SHOWHEALTH_YES == show_health || ((GamHealthChanging()
					|| CHARSTATE_HURT == this->state)
				       && SHOWHEALTH_IFHURT == show_health)) {
     const FIXEDNUM virtual_health = GamHealthChanging()
       ? GamVirtualHealth(health) : health;
     // set target screen area for the health meter
     target.left = HEALTH_METER_DISTANCE_FROM_SIDES_OF_SCREEN;
     target.bottom = GAME_PORTHEIGHT
       - HEALTH_METER_DISTANCE_FROM_SIDES_OF_SCREEN;
     target.top = target.bottom - HEALTH_METER_HEIGHT;

     // figure how long the meter will be, and draw it if it is
     //  more than zero
     target.right = target.left + FixedCnvFrom<long>
       ((GAME_MODEWIDTH - 2 * HEALTH_METER_DISTANCE_FROM_SIDES_OF_SCREEN)
        * FixedMul(virtual_health, virtual_health));

     if(target.right > target.left) {
        GfxRectangle(GfxGetPaletteEntry(RGB(HEALTH_METER_R,
                                            HEALTH_METER_G,
                                            HEALTH_METER_B)), &target);
     }
  }

  // now the ammo meter - first draw the background border (which is black)
  target.right = GAME_MODEWIDTH - AMMO_METER_DISTANCE_FROM_SIDES_OF_SCREEN;
  target.left = target.right - AMMO_METER_WIDTH;
  target.bottom = GAME_PORTHEIGHT - AMMO_METER_DISTANCE_FROM_SIDES_OF_SCREEN;
  target.top = target.bottom - AMMO_METER_HEIGHT;

  // draw the rectangle outline of the ammo meter
  GfxRectangle(0, &target);

  // make the actual measurement rectangle slightly thinner
  target.left++;
  target.right--;

  FIXEDNUM what_to_measure;

  // set what_to_measure to the percentage of ammo we have
  what_to_measure = FixedCnvTo<long>(frames_since_last_fire)
    / FRAMESTORELOAD[WEAPON_BAZOOKA];
	
  // we will show an alternate meter measuring time left to load if the player is using a bazooka
  if (WEAPON_BAZOOKA != current_weapon
      || what_to_measure >= Fixed(1.0f)
      || 0 == ammo[WEAPON_BAZOOKA]) {
    what_to_measure = FixedCnvTo<double>
      (sqrt(FixedCnvFrom<double>(ammo[current_weapon])));
    }

  target.top += AMMO_METER_HEIGHT
    - FixedCnvFrom<long>(what_to_measure * AMMO_METER_HEIGHT);

  if(target.top < target.bottom) {
     GfxRectangle(GfxGetPaletteEntry(RGB(AMMO_METER_R,
                                         AMMO_METER_G,
                                         AMMO_METER_B)), &target);
  }
}
 
void CCharacter::SubtractHealth(int fire_type)
{
  FIXEDNUM pain; // how much is actually taken away is calculated now
  bool we_are_hero = (&hero == this);
  
  if(CHARSTATE_DEAD == this->state) {
    return;
  }
	
  // figure out if we're the hero
  if(we_are_hero) {
    // we are the hero

    // do gamma effects
    GamDoEffect(GETYPE_BLOOD, health);

    if(WEAPON_BAZOOKA != fire_type) {
      pain = HERO_WEAPONDAMAGE[GLUdifficulty][fire_type];
      if(CHARSTATE_HURT == this->state) {
        pain = FixedMul(pain,HURTFACTOR);
      } else {
        this->state = CHARSTATE_HURT;
        this->frames_in_this_state = 0;
      }
    } else {
      // getting hit with bazooka
      if(CHARSTATE_HURT == this->state) {
        pain = HERO_BAZOOKADAMAGE[GLUdifficulty];
      } else {
        pain = 0;
        this->state = CHARSTATE_HURT;
      }
      this->frames_in_this_state = 0;
    }
  } else {
    // we are not the hero
    if(WEAPON_BAZOOKA != fire_type) {
      pain = ENEMY_WEAPONDAMAGE[GLUdifficulty][fire_type];
    } else {
      if(CHARSTATE_HURT == this->state) {
        pain = ENEMY_BAZOOKADAMAGE[GLUdifficulty];
      } else {
        pain = 0;
      }
    }	
    this->state = CHARSTATE_HURT;
    this->frames_in_this_state = 0;
  }

  this->health = max(Fixed(0),this->health-pain);

  // put ourselves in a hurting state
  if(0 == health) {
    if(CHAR_EVILTURNER == model && !we_are_hero) {
      // we just lost our disguise . . .
      Setup(coor.first.x, coor.first.y-Fixed(TILE_HEIGHT/2),CHAR_SALLY,false);
      state = CHARSTATE_HURT; // override setup function's value of CHARSTATE_UNKNOWING
      PlaySound();
      GluChangeScore(GluScoreDiffKill(CHAR_EVILTURNER));
    } else {
      // we are really dead . . .
      state = CHARSTATE_DYING;
      PlaySound();
      frames_in_this_state = 0;
			
      if(!we_are_hero) {
        GluChangeScore(GluScoreDiffKill(model));
      } else {
        WORD compressed_ammo[NUM_WEAPONS];

        for (int i = 0; i < NUM_WEAPONS; i++) {
          compressed_ammo[i] = ammo[i] & 0xffff0000
            ? 0x0000ffff : ammo[i];
        }

        NetDied(compressed_ammo);
      }
    }
  } else if(CHARSTATE_HURT != state ||
            WEAPON_PISTOL == fire_type ||
            ++frames_not_having_sworn > FRAMESTOSWEARAGAIN) {
    PlaySound(); 
    frames_not_having_sworn = 0;
    NetAdmitHit();
  }
}

void CCharacter::TryToFire()
{
  bool we_are_hero = bool(this == &hero);

  if(!we_are_hero) {
    // we are not the hero; make sure we are in a firing state
    if(CHARSTATE_FIRING != state || IsOffScreen()) {
      return;
    }
  } else if(CHARSTATE_DYING == state || CHARSTATE_DEAD == state) {
    return;
  }

  // look for a good slot to use
  int i;
  for(i = 0; i < MAX_FIRES; i++) {
    if(fires[i].OkayToDelete()) {
      // we found memory that the CFire object can occupy
      break;
    }
  }

  if(MAX_FIRES == i) {
    // no slot available yet
    return;
  }

  if(frames_since_last_fire < FRAMESTORELOAD[current_weapon]) {
    return;
  }

  FIXEDNUM bullets = ammo[current_weapon] * AMMOCAPACITY[current_weapon];

  if(we_are_hero && bullets < Fixed(0.5)) {
    // oh, out of ammo, nevermind
    return;
  }

  bullets -= Fixed(1);

  // convert bullets back into percentage
  bullets /= AMMOCAPACITY[current_weapon];

  if (bullets < 0) {
    bullets = 0;
  }

  ammo[current_weapon] = bullets;

  fires[i].Setup(coor.second.x, coor.second.y-Fixed(TILE_HEIGHT/2),
                 direction, current_weapon, false);
  if (WEAPON_PISTOL == current_weapon) {
    NetFirePistol(direction);
  }

  frames_since_last_fire = 0;
}

void CCharacter::PowerUpCollisions() {
  // checks to see if the character has found a powerup

  for(VCTR_POWERUP::iterator iterate = GLUpowerups.begin();
      iterate != GLUpowerups.end(); iterate++) {
    int type = iterate->Type(), index = iterate - GLUpowerups.begin();

    switch(iterate->Collides(*this)) {
    case POWERUPCOLLIDES_PICKEDUPANDWILLNOTREGENERATE:
      // we just got an ammo set
      if (!(index & ~0xffff)) {
        NetPickUpPowerUp((unsigned short)index);
      }

      for(int j = 0; j < NUM_WEAPONS; j++) {
        ammo[j] += min(Fixed(1), ammo[type] + iterate->Ammo(j));
      }
		
      GLUpowerups.erase(iterate);

      GamDoEffect(GETYPE_AMMO, health);
      return;
    case POWERUPCOLLIDES_PICKEDUPANDWILLREGENERATE:
      if (!(index & 0xffff)) {
        NetPickUpPowerUp(short(index));
      }

      if(POWERUP_HEALTHPACK == type) {
        GamDoEffect(GETYPE_HEALTH, health);
        health = min(Fixed(1),
                     health + HEALTHBONUSPERPACK[GLUdifficulty]);
      } else {
        ammo[type] = min(
            Fixed(1), ammo[type] +
            AMMOPERPACK[GLUdifficulty][type] / AMMOCAPACITY[type]);
        GamDoEffect(GETYPE_AMMO, health);
      }
      
      return;
    }
  }
}

bool CCharacter::IsOffScreen() const {
  return abs(GLUcenter_screen_x - coor.first.x)
    > Fixed((GAME_MODEWIDTH + TILE_WIDTH)/2)
    || abs(GLUcenter_screen_y - (coor.first.y-Fixed(TILE_HEIGHT/2)))
    > Fixed((GAME_PORTHEIGHT + TILE_HEIGHT)/2);
}

void CCharacter::TryToMove(GfxLock& lock) {
  pair<POINT, POINT> l = coor; 

  if(l.first.x != l.second.x) {
    if(l.first.y != l.second.y) {
      // moving diagonally, let's try recursion

      // first move horizontally, and then move vertically
      coor.second.y = coor.first.y;

      TryToMove(lock);

      coor.first = coor.second;

      coor.second.y = l.second.y;

      TryToMove(lock);

      coor.first = l.first;
    } else {
      // moving horizontally
      if(l.first.x < l.second.x) {
        // moving to the right

        pair<POINT, POINT> a, b;

        a.first.y = a.second.y = l.first.y;
        b.first.y = b.second.y = l.first.y - Fixed(TILE_HEIGHT/2);
				
        a.first.x = l.first.x+Fixed(TILE_WIDTH/4);
        a.second.x = l.second.x+Fixed(TILE_WIDTH/4);
        b.first.x = a.first.x;
        b.second.x = a.second.x;
				
        GluFilterMovement(&a.first, &a.second, lock);
        GluFilterMovement(&b.first, &b.second, lock);

        coor.second.x = min(a.second.x, b.second.x)
          - Fixed(TILE_WIDTH/4);

        //assert(this->coor.second.x >= this->coor.first.x);
        //assert((const)this->coor.second.y == (const)this->coor.first.y);
      } else {
        // moving to the left

        pair<POINT, POINT> a, b;

        a.first.y = a.second.y = l.first.y;
        b.first.y = b.second.y = l.first.y - Fixed(TILE_HEIGHT/2);
				
        a.first.x = l.first.x-Fixed(TILE_WIDTH/4);
        a.second.x = l.second.x-Fixed(TILE_WIDTH/4);
        b.first.x = a.first.x;
        b.second.x = a.second.x;
				
        GluFilterMovement(&a.first, &a.second, lock);
        GluFilterMovement(&b.first, &b.second, lock);

        coor.second.x = max(a.second.x, b.second.x)
          + Fixed(TILE_WIDTH/4);
      }
    }
  } else if(l.first.y != l.second.y) {
    // moving vertically
    if(l.first.y < l.second.y) {
      // moving down
      pair<POINT, POINT> a, b;
      a.first.x = a.second.x = l.first.x + Fixed(TILE_WIDTH/4);
      b.first.x = b.second.x = l.first.x - Fixed(TILE_WIDTH/4);
      a.first.y = b.first.y = l.first.y;
      a.second.y = b.second.y = l.second.y ;
      GluFilterMovement(&a.first, &a.second, lock);
      GluFilterMovement(&b.first, &b.second, lock);
      coor.second.y = min(a.second.y, b.second.y);
    } else {
      // moving up
      pair<POINT, POINT> a, b;
      a.first.x = a.second.x = l.first.x + Fixed(TILE_WIDTH/4);
      b.first.x = b.second.x = l.first.x - Fixed(TILE_WIDTH/4);
      a.first.y = b.first.y = l.first.y - Fixed(TILE_HEIGHT/2);
      a.second.y = b.second.y = l.second.y - Fixed(TILE_HEIGHT/2);
      GluFilterMovement(&a.first, &a.second, lock);
      GluFilterMovement(&b.first, &b.second, lock);
      coor.second.y = max(a.second.y, b.second.y)
        + Fixed(TILE_HEIGHT/2);
    }
  }
}

void CCharacter::ResetAmmo() {
  for(int i = 0; i < NUM_WEAPONS; i++) {
    ammo[i] = AMMOSTARTING[i] / AMMOCAPACITY[i];
  }
}

void CCharacter::CheckForEnemyCollision(const CCharacter& enemy) {
  if(abs(coor.second.x - enemy.coor.first.x) < FATNESS
     && abs(coor.second.y - enemy.coor.first.y) < FATNESS) {
    coor.second = coor.first;
  }
}

void CCharacter::CalculateSector(int& row,int& col) {
  sector_row = row = (FixedCnvFrom<long>(coor.first.y) - TILE_HEIGHT/2)
    / SECTOR_HEIGHT;
  sector_col = col = FixedCnvFrom<long>(coor.first.x)  / SECTOR_WIDTH;
}

void CCharacter::Setup(unsigned int model_) {
  model = model_;
  current_weapon = 0;
  state = 0;
  direction = DSOUTH;
  coor.first.x = 0;
  coor.first.y = 0;
  coor.second = coor.first;
}

void CCharacter::Walk(bool running) {
  FIXEDNUM speed = HEROSPEED, mx, my;

  if (NetInGame()) {
    speed = FixedMul(speed, HEROSPEED_MPFACTOR);
  } else if (running) {
    speed = FixedMul(speed, HEROSPEED_RUNNINGFACTOR);
  }

  if (CHARSTATE_HURT == state) {
    speed = FixedMul(speed, HEROSPEED_HURTFACTOR);
  }

  GluInterpretDirection(direction, mx, my);

  mx = FixedMul(mx, speed);
  my = FixedMul(my, speed);

  coor.second.x = coor.first.x + mx;
  coor.second.y = coor.second.y + my;
}

