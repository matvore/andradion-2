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

// if this line is not commented, then enemies will
//  not attack you unless fired upon
//#define TESTMODE 

#include "StdAfx.h"
#include "Fixed.h"
#include "GammaEffects.h"
#include "Net.h"
#include "PowerUp.h"
#include "Character.h"
#include "Glue.h"
#include "Fire.h"
#include "Gfx.h"
#include "GfxBasic.h"
#include "GfxPretty.h"
#include "Keyboard.h"
#include "Difficulty.h"
#include "Sound.h"

using namespace std;

// a small secret, if all the arrow keys are
//  pressed except one direction, the hero
//  will run in the direction opposite of the unpressed button

const FIXEDNUM MIN_SAFEDISTANCESQUARED = Fixed(150*150);

// how close an alien should be before it starts firing
const FIXEDNUM MIN_ALIGNMENT = Fixed(3);

const int FRAMESTORECOVER_HERO = 15;
const int FRAMESTORECOVER_ALIEN = 15;
const FIXEDNUM HURTFACTOR = Fixed(0.5);
const FIXEDNUM HEROSPEED            = Fixed(2.80f);
const FIXEDNUM HEROSPEED_MPFACTOR   = Fixed(1.20f);
const FIXEDNUM HEROSPEED_HURTFACTOR = Fixed(0.75f);
const FIXEDNUM HEROSPEED_RUNNINGFACTOR = Fixed(1.414f);

// speeds are in pixels per frame
const FIXEDNUM ALIENSPEED[3] = {Fixed(1), Fixed(2), Fixed(3)};

const int FRAMESTORELOAD[WEAPON_COUNT] = {15, 1, 100};

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

// health meter is a meter measuring Turner's level of pain, taking up the
// bottom of the screen
const int HEALTH_METER_DISTANCE_FROM_SIDES_OF_SCREEN = 5;
const int HEALTH_METER_HEIGHT = 15;

// the ammo meter takes up a smaller portion of the screen in the lower right
// corner, and it is vertical
const int AMMO_METER_DISTANCE_FROM_SIDES_OF_SCREEN = 2;
const int AMMO_METER_HEIGHT = 30;
const int AMMO_METER_WIDTH = 8;

const int RECOIL_RANGE[] = {5, 5, 0};
const int RECOIL_RANGE_OFFSET[] = {-2, -2, 0};
const FIXEDNUM RECOIL_UNIT = Fixed(1.5);

vector<Character> Character::all;

// used for enemies in SP, or hero in SP and MP
void Character::Setup(FIXEDNUM x, FIXEDNUM y, int model_, bool doing_mp,
                      bool controlled_by_human_) {
  this->model                   = model_;
  this->coor.first.x            = x;
  this->coor.first.y            = y+Fixed(TILE_HEIGHT/2);
  this->coor.second             = this->coor.first;
  this->health                  = Fixed(1);
  this->direction               = DSOUTH;
  this->frames_in_this_state    = 0;
  this->frames_not_having_sworn = 0;
  this->frames_since_last_fire  = 0;
  this->controlled_by_human     = controlled_by_human_;

  if(CHAR_TURNER != this->model && !doing_mp) {
    // simple single-player alien guy
    this->current_weapon = model_;
    this->state = CHARSTATE_UNKNOWING;
  } else {
    this->reset_gamma = true;
    this->current_weapon = WEAPON_PISTOL;
    this->state = CHARSTATE_ALIVE;
  }
}

void Character::AnalyzePalette() {
  Gfx *gfx = Gfx::Get();

  border_color = gfx->MatchingColor(RGB(255, 255, 255));
  ammo_color = gfx->MatchingColor(RGB(0, 255, 255));
  ammo_color_dim = gfx->MatchingColor(RGB(0, 128, 128));
  health_color = gfx->MatchingColor(RGB(255, 0, 0));
  health_color_dim = gfx->MatchingColor(RGB(128, 0, 0));
}

void Character::PlaySound() {
  int wav;
  FIXEDNUM y_source = coor.first.y - Fixed(TILE_HEIGHT/2);

  if(CHAR_TURNER > this->model) {
    wav = (CHARSTATE_DYING == state) ? WAVSET_ALIENDEATH : WAVSET_ALIENHIT;
    wav += rand()%WAVSINASET;
  } else {
    wav = WAVSET_FIRSTNONALIEN;
    wav += (model - CHAR_TURNER) * (WAVSINASET+1);
    wav += (CHARSTATE_DYING == state) ? WAVSINASET : (rand()%WAVSINASET);
  }

  GluPlaySound(wav, coor.first.x, y_source);
}

bool Character::EnemyLogic() {
  Context *cxt = GluContext();

  // xd and yd are the distance in each dimension from the target
  FIXEDNUM xd = abs(cxt->center_screen_x - (coor.first.x));
  FIXEDNUM yd = abs(cxt->center_screen_y
                    - (coor.first.y - Fixed(TILE_HEIGHT/2)));

  // make sure we aren't off the side
  //  of the screen
  if(xd >= Fixed((GAME_MODEWIDTH + TILE_WIDTH)/2)
     || yd >= Fixed((GAME_MODEHEIGHT + TILE_HEIGHT)/2)) {
    return false;
  }

  if(CHARSTATE_HURT != state && CHARSTATE_DYING != state) {
    // we can consider not going through logic this time
    if (CHARSTATE_DEAD == cxt->hero->state || CHARSTATE_DEAD == state
        || GluWalkingData(cxt->hero->X(), cxt->hero->Y())
        != GluWalkingData(coor.first.x, coor.first.y - Fixed(TILE_HEIGHT/2))) {
      return true;
    }
  }
	
  xd = cxt->hero->coor.first.x - coor.first.x;
  yd = cxt->hero->coor.first.y - coor.first.y;
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

  cxt->hero->CheckForEnemyCollision(*this);

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

      TryToMove();

      // get closer to the target
      if(abs(xd) > abs(yd)
         && abs(coor.first.y + yd - coor.second.y) <= MIN_ALIGNMENT) {
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
      coor.second.x = cxt->hero->coor.first.x;
      state = CHARSTATE_FIRING;
      direction = yd > 0 ? DSOUTH : DNORTH;
    } else if(abs(ym) > abs(yd)) {
      coor.second.y = cxt->hero->coor.first.y;
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

    TryToMove();
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

void Character::Logic() {
  static bool firing_last_frame = false;
  Context *cxt = GluContext();
  
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
    if((KeyPressed(DIK_1))
       && WEAPON_PISTOL != current_weapon) {
      if(!(KeyPressed(DIK_2))
         && !(KeyPressed(DIK_3))) {
        SndPlay(WAV_GUNNOISE, Fixed(1), false);
        current_weapon = WEAPON_PISTOL;
      }
    } else if((KeyPressed(DIK_2))
              && WEAPON_MACHINEGUN != current_weapon) {
      if(!(KeyPressed(DIK_1))
         && !(KeyPressed(DIK_3))) {
        SndPlay(WAV_GUNNOISE, Fixed(1), false);
        current_weapon = WEAPON_MACHINEGUN;
      }
    } else if((KeyPressed(DIK_3))
              && WEAPON_BAZOOKA != current_weapon) {
      if(!(KeyPressed(DIK_1))
         && !(KeyPressed(DIK_2))) {
        SndPlay(WAV_GUNNOISE, Fixed(1), false);
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
        // regenerate ourselves into a new starting spot
        GluGetRandomStartingSpot(coor.first);
        coor.first.y += Fixed(TILE_HEIGHT/2);
        coor.second = coor.first;
        state = CHARSTATE_ALIVE;
        health = Fixed(1);
        cxt->AmmoReset();
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
    
    if (KeyPressed(DIK_RIGHT)) {directional_buttons |= 1;}
    if (KeyPressed(DIK_UP)) {directional_buttons |= 2;}
    if (KeyPressed(DIK_LEFT)) {directional_buttons |= 4;}
    if (KeyPressed(DIK_DOWN)) {directional_buttons |= 8;}

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
               ((KeyPressed(DIK_LCONTROL)
                 || KeyPressed(DIK_RCONTROL))
                && direction < RENDERED_DIRECTIONS));
		
    if(walked) {
      Walk(running);

      if((KeyPressed(DIK_LSHIFT)) || (KeyPressed(DIK_RSHIFT))) {
          direction = old_direction;
      }
      
      TryToMove();
    }
  } else {
    walked = false;
  }

  NetSetPosition(FixedCnvFrom<long>(coor.first.x),
                 FixedCnvFrom<long>(coor.first.y) - TILE_HEIGHT/2, direction);
  
  // now see if they are trying to fire the gun . . .
  if(KeyPressed(DIK_SPACE) && cxt->ammo[current_weapon]) {
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

  if (!walked && (CHARSTATE_WALKING == state || CHARSTATE_ALIVE == state)) {
    frames_in_this_state = 0;
  }
}

bool Character::DrawCharacter() {
  Context *cxt = GluContext();
  int bmp, target_x, target_y;
  int vd; // virtual direction
  FIXEDNUM tx, ty; // screen coordinates to put character

  // calculate visual direction
  vd = direction % RENDERED_DIRECTIONS;

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
	
  tx = coor.first.x - cxt->center_screen_x
    + Fixed(GAME_MODEWIDTH/2) - Fixed(TILE_WIDTH/2);
  ty = coor.first.y - cxt->center_screen_y
    + Fixed(GAME_MODEHEIGHT/2) - Fixed(TILE_HEIGHT);

  target_x = FixedCnvFrom<long>(tx);
  target_y = FixedCnvFrom<long>(ty);

  if(donot_draw_weapon) {
    cxt->Draw(bmp, target_x, target_y);
  } else {
    int weapon_bmp = BMPSET_WEAPONS
      + this->current_weapon * RENDERED_DIRECTIONS + vd;

    if(DSOUTH == vd || DEAST == vd || CHAR_CHARMIN == model) {
      swap(bmp, weapon_bmp);
    }

    cxt->Draw(weapon_bmp, target_x, target_y);
    cxt->Draw(bmp, target_x, target_y);
  }

  // draw blood if we are hurt
  if(CHARSTATE_HURT == state) {
    cxt->Draw(BMPSET_DECAPITATE+(rand()&1), target_x, target_y);
  }

  if (coor.first.x != coor.second.x || coor.first.y != coor.second.y) {
    coor.first = coor.second;
    return true;
  } else {
    return false;
  }
}

void Character::DrawMeters(bool show_health) {
  Context *cxt = GluContext();
  FIXEDNUM virtual_health = GamHealthChanging()
    ? GamVirtualHealth(health) : health;
  FIXEDNUM virtual_ammo;

  virtual_ammo = FixedCnvTo<long>(frames_since_last_fire)
    / FRAMESTORELOAD[WEAPON_BAZOOKA];

  // we will show an alternate meter measuring time left to load if the player
  // is using a bazooka
  if (WEAPON_BAZOOKA != current_weapon
      || virtual_ammo >= Fixed(1.0f) || !cxt->ammo[WEAPON_BAZOOKA]) {
    //virtual_ammo = FixedCnvTo<double>
    //  (sqrt(FixedCnvFrom<double>(cxt->AmmoAsPercentage(current_weapon))));
    virtual_ammo = cxt->AmmoAsPercentage(current_weapon);
  }

  if (GfxBasic::Get()) {
    DrawMetersBasic(show_health, virtual_health, virtual_ammo);
  } else {
    assert(GfxPretty::Get());
    DrawMetersPretty(virtual_health, virtual_ammo);
  }
}

void Character::DrawMetersPretty
(FIXEDNUM virtual_health, FIXEDNUM virtual_ammo) {
  GfxPretty *gfx = GfxPretty::Get();
  RECT target;
  int blocks;
  bool dim_block;

  assert(gfx);

  // FILL THE BORDER

  target.left = 0;
  target.top = 440;
  target.bottom = 442;
  target.right = 640;

  gfx->FrontBufferRectangle(&target, border_color);

  target.left = 319;
  target.bottom = 480;
  target.right = 321;

  gfx->FrontBufferRectangle(&target, border_color);

  // FILL THE BLACK REGIONS BESIDE THE BORDERS AND THE BLOCKS

  target.left = 0;
  target.top = 442;
  target.right = 319;
  target.bottom = 446;
  gfx->FrontBufferRectangle(&target, 0);

  target.left += 321;
  target.right += 321;
  gfx->FrontBufferRectangle(&target, 0);

  target.left = 0;
  target.top = 476;
  target.bottom = 480;
  target.right = 319;
  gfx->FrontBufferRectangle(&target, 0);

  target.left += 321;
  target.right += 321;
  gfx->FrontBufferRectangle(&target, 0);

  // DRAW THE COLORED BLOCKS FOR HEALTH
  target.left = 4;
  target.top = 446;
  target.right = 21;
  target.bottom = 476;

  virtual_health *= 15;

  blocks = FixedCnvFrom<int>(virtual_health);
  dim_block = virtual_health & 0x8000;

  if (dim_block) {
    blocks++;
  }

  for (int i = 1; i <= 15; i++) {
    BYTE color;

    if (i <= blocks) {
      color = (i == blocks && dim_block)
        ? health_color_dim : health_color;
    } else {
      color = 0;
    }

    gfx->FrontBufferRectangle(&target, color);

    target.left += 21;
    target.right += 21;
  }

  // DRAW THE BLACK AREAS BETWEEN THE COLORED BLOCKS
  target.left = 0;
  target.right = 4;
  target.top = 446;
  target.bottom = 476;
  for (int i = 0; i < 16; i++) {
    gfx->FrontBufferRectangle(&target, 0);
    target.left += 21;
    target.right += 21;
  }

  target.left = 321;
  target.right = 325;
  for (int i = 0; i < 16; i++) {
    gfx->FrontBufferRectangle(&target, 0);
    target.left += 21;
    target.right += 21;
  }

  // DRAW THE COLORED BLOCKS FOR AMMO
  target.left = 619;
  target.right = 636;

  virtual_ammo *= 15;
  blocks = FixedCnvFrom<int>(virtual_ammo);
  dim_block = virtual_ammo & 0x8000;

  if (dim_block) {
    blocks++;
  }

  for (int i = 1; i <= 15; i++) {
    BYTE color;

    if (i <= blocks) {
      color = (i == blocks && dim_block)
        ? ammo_color_dim : ammo_color;
    } else {
      color = 0;
    }

    gfx->FrontBufferRectangle(&target, color);

    target.left -= 21;
    target.right -= 21;
  }
}

void Character::DrawMetersBasic
(bool show_health, FIXEDNUM virtual_health, FIXEDNUM virtual_ammo) {
  Context *cxt = GluContext();
  RECT target;

  if(show_health || GamHealthChanging() || CHARSTATE_HURT == state) {
     // set target screen area for the health meter
     target.left = HEALTH_METER_DISTANCE_FROM_SIDES_OF_SCREEN;
     target.bottom = GAME_MODEHEIGHT
       - HEALTH_METER_DISTANCE_FROM_SIDES_OF_SCREEN;
     target.top = target.bottom - HEALTH_METER_HEIGHT;

     // figure how long the meter will be, and draw it if it is
     //  more than zero
     target.right = target.left + FixedCnvFrom<long>
       ((GAME_MODEWIDTH - 2 * HEALTH_METER_DISTANCE_FROM_SIDES_OF_SCREEN)
        * FixedMul(virtual_health, virtual_health));

     if(target.right > target.left) {
        Gfx::Get()->Rectangle(&target, health_color, false);
     }
  }

  // now the ammo meter - first draw the background border (which is black)
  target.right = GAME_MODEWIDTH - AMMO_METER_DISTANCE_FROM_SIDES_OF_SCREEN;
  target.left = target.right - AMMO_METER_WIDTH;
  target.bottom = GAME_MODEHEIGHT - AMMO_METER_DISTANCE_FROM_SIDES_OF_SCREEN;
  target.top = target.bottom - AMMO_METER_HEIGHT;

  // draw the rectangle outline of the ammo meter
  Gfx::Get()->Rectangle(&target, 0, false);

  // make the actual measurement rectangle slightly thinner
  target.left++;
  target.right--;

  target.top += AMMO_METER_HEIGHT
    - FixedCnvFrom<long>(virtual_ammo * AMMO_METER_HEIGHT);

  if (target.top < target.bottom) {
    Gfx::Get()->Rectangle(&target, ammo_color, false);
  }
}

void Character::SubtractHealth(int fire_type) {
  FIXEDNUM pain; // how much is actually taken away is calculated now
  Context *cxt = GluContext();
  
  if(CHARSTATE_DEAD == state) {
    return;
  }
	
  if (controlled_by_human) {
    GamDoEffect(GETYPE_BLOOD, health);

    if(WEAPON_BAZOOKA != fire_type) {
      pain = DifDamageToHero(fire_type);
      if(CHARSTATE_HURT == state) {
        pain = FixedMul(pain, HURTFACTOR);
      } else {
        state = CHARSTATE_HURT;
        frames_in_this_state = 0;
      }
    } else {
      // getting hit with bazooka
      if(CHARSTATE_HURT == state) {
        pain = DifDamageToHero(WEAPON_BAZOOKA);
      } else {
        pain = 0;
        state = CHARSTATE_HURT;
      }
      frames_in_this_state = 0;
    }
  } else {
    // we are not controlled by a human
    pain = (CHARSTATE_HURT == state || WEAPON_BAZOOKA != fire_type)
        ? DifDamageToEnemy(fire_type) : 0;

    state = CHARSTATE_HURT;
    frames_in_this_state = 0;
  }

  health = max(Fixed(0), health-pain);

  // put ourselves in a hurting state
  if(0 == health) {
    if(CHAR_EVILTURNER == model && !controlled_by_human) {
      // we just lost our disguise . . .
      Setup(coor.first.x, coor.first.y-Fixed(TILE_HEIGHT/2),
            CHAR_SALLY, false, false);
      state = CHARSTATE_HURT;

      PlaySound();
      GluContext()->ChangeScore(GluScoreDiffKill(CHAR_EVILTURNER));
    } else {
      // we are really dead . . .
      state = CHARSTATE_DYING;
      PlaySound();
      frames_in_this_state = 0;

      if(!controlled_by_human) {
        GluContext()->ChangeScore(GluScoreDiffKill(model));
      } else {
        NetDied(cxt->ammo);
      }
    }
  } else if(CHARSTATE_HURT != state || WEAPON_PISTOL == fire_type
            || ++frames_not_having_sworn > FRAMESTOSWEARAGAIN) {
    PlaySound();
    frames_not_having_sworn = 0;
    NetAdmitHit();
  }
}

void Character::TryToFire() {
  Fire *new_fire;
  Context *cxt = GluContext();

  if (!controlled_by_human) {
    if(CHARSTATE_FIRING != state || IsOffScreen()) {
      return;
    }
  } else if (CHARSTATE_DYING == state || CHARSTATE_DEAD == state) {
    return;
  }

  if(frames_since_last_fire < FRAMESTORELOAD[current_weapon]) {
    return;
  }

  // look for a good slot to use
  new_fire = Fire::UnusedSlot();

  if (!new_fire) {
    return;
  }

  if (controlled_by_human) {
    if (!cxt->ammo[current_weapon]) {
      // oh, out of ammo, nevermind
      return;
    }

    cxt->ammo[current_weapon]--;
  }

  new_fire->Setup(coor.second.x, coor.second.y-Fixed(TILE_HEIGHT/2),
                  direction, current_weapon, false);
  if (WEAPON_PISTOL == current_weapon) {
    NetFirePistol(direction);
  }

  frames_since_last_fire = 0;
}

void Character::PowerUpCollisions(vector<CPowerUp> *pups) {
  // checks to see if the character has found a powerup
  Context *cxt = GluContext();

  for(int i = 0; i < pups->size(); i++) {
    int type = (*pups)[i].Type();

    switch ((*pups)[i].CollidesWithHero()) {
    case POWERUPCOLLIDES_PICKEDUPANDWILLNOTREGENERATE:
      // we just got an ammo set
      if (!(i & ~0xffff)) {
        NetPickUpPowerUp((unsigned short)i);
      }

      for(int j = 0; j < WEAPON_COUNT; j++) {
        cxt->AmmoAdd(j, (*pups)[i].Ammo(j));
      }

      pups->erase(pups->begin() + i);

      GamDoEffect(GETYPE_AMMO, health);
      return;
    case POWERUPCOLLIDES_PICKEDUPANDWILLREGENERATE:
      if (!(i & 0xffff)) {
        NetPickUpPowerUp((unsigned short)i);
      }

      if(POWERUP_HEALTHPACK == type) {
        GamDoEffect(GETYPE_HEALTH, health);
        health = min(Fixed(1), health + DifHealthBonusPerPack());
      } else {
        cxt->AmmoAdd(type, DifAmmoPerPack(type));
        GamDoEffect(GETYPE_AMMO, health);
      }
      
      return;
    }
  }
}

bool Character::IsOffScreen() const {
  return abs(GluContext()->center_screen_x - coor.first.x)
    > Fixed((GAME_MODEWIDTH + TILE_WIDTH)/2)
    || abs(GluContext()->center_screen_y - (coor.first.y-Fixed(TILE_HEIGHT/2)))
    > Fixed((GAME_MODEHEIGHT + TILE_HEIGHT)/2);
}

void Character::TryToMove() {
  pair<POINT, POINT> l = coor; 

  if(l.first.x != l.second.x) {
    if(l.first.y != l.second.y) {
      // moving diagonally, let's try recursion

      // first move horizontally, and then move vertically
      coor.second.y = coor.first.y;

      TryToMove();

      coor.first = coor.second;

      coor.second.y = l.second.y;

      TryToMove();

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
				
        GluFilterMovement(&a.first, &a.second);
        GluFilterMovement(&b.first, &b.second);

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
				
        GluFilterMovement(&a.first, &a.second);
        GluFilterMovement(&b.first, &b.second);

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
      GluFilterMovement(&a.first, &a.second);
      GluFilterMovement(&b.first, &b.second);
      coor.second.y = min(a.second.y, b.second.y);
    } else {
      // moving up
      pair<POINT, POINT> a, b;
      a.first.x = a.second.x = l.first.x + Fixed(TILE_WIDTH/4);
      b.first.x = b.second.x = l.first.x - Fixed(TILE_WIDTH/4);
      a.first.y = b.first.y = l.first.y - Fixed(TILE_HEIGHT/2);
      a.second.y = b.second.y = l.second.y - Fixed(TILE_HEIGHT/2);
      GluFilterMovement(&a.first, &a.second);
      GluFilterMovement(&b.first, &b.second);
      coor.second.y = max(a.second.y, b.second.y)
        + Fixed(TILE_HEIGHT/2);
    }
  }
}

void Character::CheckForEnemyCollision(const Character& enemy) {
  if(abs(coor.second.x - enemy.coor.first.x) < FATNESS
     && abs(coor.second.y - enemy.coor.first.y) < FATNESS) {
    coor.second = coor.first;
  }
}

void Character::Setup(unsigned int model_) {
  model = model_;
  current_weapon = 0;
  state = 0;
  direction = DSOUTH;
  coor.first.x = 0;
  coor.first.y = 0;
  coor.second = coor.first;
  controlled_by_human = true;
}

void Character::Walk(bool running) {
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

BYTE Character::border_color;
BYTE Character::ammo_color;
BYTE Character::ammo_color_dim;
BYTE Character::health_color;
BYTE Character::health_color_dim;
