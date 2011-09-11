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
#include "Net.h"
#include "Character.h"
#include "Glue.h"
#include "Fire.h"
#include "Gfx.h"
#include "Sound.h"

using std::pair;
using std::max;
using std::min;
using std::vector;
using std::abs;
using std::auto_ptr;

// constants used in this module
const BYTE BULLETTRAIL_R = 255;
const BYTE BULLETTRAIL_G = 255;
const BYTE BULLETTRAIL_B = 0;
const int MACHINEGUNSWAY = 10;
const FIXEDNUM BULLETSPEED = Fixed(20);
const int FRAMESTOEXPLOSION = 70;
const FIXEDNUM PROJECTILE_INITIALMOVEMENT = Fixed(15);
const FIXEDNUM PROJECTILE_SPEED = Fixed(10); 
const int MAX_EXPLOSIONBULGE = 5;
const int HORIZONTALCOLLISION_UPPER = 1;
const int HORIZONTALCOLLISION_LOWER = 2;
enum {FIRESTATE_GOING, FIRESTATE_FIRSTFRAMETODRAW,
      FIRESTATE_INACTIVE = -1};

Fire fires[MAX_FIRES];

Fire *Fire::UnusedSlot() {
  for (int i = 0; i < MAX_FIRES; i++) {
    if (fires[i].OkayToDelete()) {
      return fires + i;
    }
  }

  return 0;
}

bool Fire::OkayToDelete() const {return FIRESTATE_INACTIVE == state;}

void Fire::Setup(FIXEDNUM sx_, FIXEDNUM sy_, FIXEDNUM tx_, FIXEDNUM ty_) {
  direction = 0;
  remotely_generated = true;
  type = WEAPON_BAZOOKA;

  sx = sx_;
  sy = sy_;
  x = tx_;
  y = ty_;

  PlaySound();

  state = FIRESTATE_GOING;
  frames_since_explosion_started = 0;
}

void Fire::Setup(FIXEDNUM sx_, FIXEDNUM sy_, int direction_,
                  int type_, bool remotely_generated_) {
  FIXEDNUM mx, my;

  direction = direction_;
  type = type_;
  state = FIRESTATE_GOING;

  remotely_generated = remotely_generated_;

  GluInterpretDirection(direction, mx, my);

  x = sx_ + FixedMul(mx,PROJECTILE_INITIALMOVEMENT);
  y = sy_ + FixedMul(my,PROJECTILE_INITIALMOVEMENT);

  sx = x;
  sy = y;

  PlaySound();

  // do some extra things if we are using horizontal collision flags
  horizontal_collision_flags = 0 == mx ? HORIZONTALCOLLISION_LOWER : 0;

  // this is only imnportant if we are bazooka	
  frames_since_explosion_started = 0;
}

void Fire::Logic() {
  if(FIRESTATE_INACTIVE == state) {
    return;
  }

  if(FIRESTATE_GOING == state) {
    if(WEAPON_BAZOOKA == type && remotely_generated) {
      // do not need to perform movement
      state = FIRESTATE_FIRSTFRAMETODRAW;
      return;
    }

    pair<POINT, POINT> hypothetical;

    hypothetical.first.x = x;
    hypothetical.first.y = y;

    FIXEDNUM dest_x, dest_y, proj_speed_x, proj_speed_y;
    
    GluInterpretDirection(direction, dest_x, dest_y);
    proj_speed_x = FixedMul(dest_x, PROJECTILE_SPEED);
    proj_speed_y = FixedMul(dest_y, PROJECTILE_SPEED);
    dest_x = FixedMul(dest_x, BULLETSPEED) + x;
    dest_y = FixedMul(dest_y, BULLETSPEED) + y;

    do {
      vector<int> collides;

      if (WEAPON_PISTOL == type) {
        if(proj_speed_x > 0) {
          hypothetical.second.x
            = min(dest_x, (int)hypothetical.first.x + proj_speed_x);
        } else {
          hypothetical.second.x
            = max(dest_x, (int)hypothetical.first.x + proj_speed_x);
        }

        if(proj_speed_y > 0) {
          hypothetical.second.y
            = min(dest_y, (int)hypothetical.first.y + proj_speed_y);
        } else {
          hypothetical.second.y
            = max(dest_y, (int)hypothetical.first.y + proj_speed_y);
        }
      } else {
        hypothetical.second.x = hypothetical.first.x + proj_speed_x;
        hypothetical.second.y = hypothetical.first.y + proj_speed_y;
      }

      // check for collisions
      Collides(&collides);

      if (!collides.empty()) {
        Character::Ptr coll(Character::Get(collides[0]));
        if(WEAPON_BAZOOKA != type) {
          if(coll->ControlledByHuman()) {
            if(!NetInGame()) {
              coll->SubtractHealth(type);
            }

            state = WEAPON_PISTOL != type
              ? FIRESTATE_FIRSTFRAMETODRAW : FIRESTATE_INACTIVE;
          } else {
            if(!remotely_generated) {
              if (NetInGame()) {
                // in a network game, the character of the first index is the
                //  local player, so subtract one to find the index of the
                //  remote player; because the second index in the Character
                //  array is the first remote player
                NetHit(collides[0] - 1, type);
              } else {
                coll->SubtractHealth(type);
              }
            }

            state = WEAPON_PISTOL != type
              ? FIRESTATE_FIRSTFRAMETODRAW : FIRESTATE_INACTIVE;
          }
        } else {
          state = FIRESTATE_FIRSTFRAMETODRAW;
        }

        break;
      }

      x = hypothetical.second.x;
      y = hypothetical.second.y;

      GluFilterMovement(&hypothetical.first, &hypothetical.second);

      if(y != hypothetical.second.y) {

        state = WEAPON_PISTOL != type
          ? FIRESTATE_FIRSTFRAMETODRAW : FIRESTATE_INACTIVE;

        break;
      } else {
          // we are doing a horizontal movement
          if(x != hypothetical.second.x) {
              horizontal_collision_flags |= HORIZONTALCOLLISION_UPPER;
              if(HORIZONTALCOLLISION_LOWER
                 & horizontal_collision_flags) {
                state = WEAPON_PISTOL != type
                  ? FIRESTATE_FIRSTFRAMETODRAW : FIRESTATE_INACTIVE;
                break;
              }
          }
				
          if(!(HORIZONTALCOLLISION_LOWER
               & horizontal_collision_flags)) {
            // let's try the lower one because so far,
            //  it's been working out . . .
            hypothetical.second.y += Fixed(TILE_HEIGHT/2);
            hypothetical.second.x = x;
            hypothetical.first.y = hypothetical.second.y;
	
            GluFilterMovement(&hypothetical.first, &hypothetical.second);
            if(x != hypothetical.second.x) {
                horizontal_collision_flags |= HORIZONTALCOLLISION_LOWER;
                if(HORIZONTALCOLLISION_UPPER
                   & horizontal_collision_flags) {
                  state = WEAPON_PISTOL != type
                    ? FIRESTATE_FIRSTFRAMETODRAW : FIRESTATE_INACTIVE;
                  break;
                }
            }

            hypothetical.second.y -= Fixed(TILE_HEIGHT/2);
          }
      }
			
      hypothetical.first = hypothetical.second;
    } while (hypothetical.first.x != dest_x
             || hypothetical.first.y != dest_y
             || WEAPON_PISTOL != type);

    if (x < 0) {
      x = 0;
    }

    if (y < 0) {
      y = 0;
    }

  } else if(WEAPON_BAZOOKA == type) {
    vector<int> collides;

    // exploding bazooka		
    if(++frames_since_explosion_started > FRAMESTOEXPLOSION) {
      state = FIRESTATE_INACTIVE;
      return;
    }

    state++;
    Collides(&collides);

    for (vector<int>::iterator itr = collides.begin();
        itr != collides.end(); itr++) {
      Character::Ptr ch = Character::Get(*itr);

      if (ch->ControlledByHuman() || !NetInGame()) {
        ch->SubtractHealth(type);
      }
    }
  }
}

class BulletTrailDrawer : public Gfx::LineDrawer {
protected:
  int length;

  int rand0, rand1;
  
  int visible_points_left;
  int points_to_start;

  BYTE color;

public:
  BulletTrailDrawer(BYTE color, int length)
    : length(length), color(color), rand0(rand()), rand1(rand()) {}

  virtual void BeginLine() {
    visible_points_left = (rand0 % 16) + 16;
    points_to_start = length <= visible_points_left
      ? 0 : (rand1 % (length - visible_points_left));
  }

  virtual int GetNextPixel() {
    if (points_to_start-- <= 0 && visible_points_left-- > 0) {
      return color;
    } else {
      return -1;
    }
  }
};

void Fire::Draw() {
  Context *cxt = GluContext();

  if(FIRESTATE_INACTIVE == state) {
    return; // no business drawing an inactive projectile
  }

  if(WEAPON_MACHINEGUN == type) {
    state = FIRESTATE_INACTIVE; // no longer needed after drawn once
  }

  if(WEAPON_PISTOL == type) {
    int target_x = FixedCnvFrom<long>(x - cxt->center_screen_x)
      -TILE_WIDTH/2+GAME_MODEWIDTH/2;
    int target_y = FixedCnvFrom<long>(y - cxt->center_screen_y)
      -TILE_HEIGHT/2+GAME_MODEHEIGHT/2;
		
    cxt->Draw(BMP_BULLET, target_x, target_y);
    return; 
  }

  int bulge;

  // take care of explosions first
  if(WEAPON_BAZOOKA == type) {
    // draw explosion now
    bulge = rand()%MAX_EXPLOSIONBULGE;
		
    RECT target;
    target.left = FixedCnvFrom<long>(x - cxt->center_screen_x)
      + GAME_MODEWIDTH/2 - bulge - TILE_WIDTH/2;
    target.top = FixedCnvFrom<long>(y - cxt->center_screen_y)
      + GAME_MODEHEIGHT/2 - bulge - TILE_HEIGHT/2;
    target.right = target.left + TILE_WIDTH + bulge*2;
    target.bottom = target.top + TILE_HEIGHT + bulge*2;

    cxt->DrawScale(BMP_EXPLOSION, &target);

    if(FIRESTATE_FIRSTFRAMETODRAW == state && !remotely_generated) {
      NetFireBazooka(FixedCnvFrom<long>(x), FixedCnvFrom<long>(y));
    } else {
      // return so we don't draw the smoke trail
      return;
    }
  }

  // draw a yellow line from sx,sy to x,y
  int swayx, swayy;

  if(WEAPON_MACHINEGUN == type) {
    GluInterpretDirection(direction,swayx,swayy);
    if(0 != swayx && 0 != swayy) {
      swayx = (rand()%MACHINEGUNSWAY)-(MACHINEGUNSWAY/2);
      swayy = (rand()%MACHINEGUNSWAY)-(MACHINEGUNSWAY/2);
    } else if(0 != swayx) {
      swayy = (rand()%MACHINEGUNSWAY)-(MACHINEGUNSWAY/2);
      swayx = 0;
    } else if(0 != swayy) {
      swayx = (rand()%MACHINEGUNSWAY)-(MACHINEGUNSWAY/2);
      swayy = 0;
    }
  } else {
    swayx = swayy = 0;
  }

  // draw our line

  int x0 = FixedCnvFrom<long>(sx-cxt->center_screen_x) + GAME_MODEWIDTH/2;
  int y0 = FixedCnvFrom<long>(sy-cxt->center_screen_y) + GAME_MODEHEIGHT/2;
  int x1 = FixedCnvFrom<long>(x-cxt->center_screen_x) + GAME_MODEWIDTH/2
    + swayx;
  int y1 = FixedCnvFrom<long>(y-cxt->center_screen_y) + GAME_MODEHEIGHT/2
    + swayy;

  if(Gfx::Get()->ClipLine(&x0, &y0, &x1, &y1)) {
    Gfx::Get()->Lock();

    Gfx::Get()->DrawLine
      (x0, y0, x1, y1, auto_ptr< ::BulletTrailDrawer>
       (new ::BulletTrailDrawer(bullet_trail_color,
                                max(abs(x1 - x0), abs(y1 - y0)))).get());

    Gfx::Get()->Unlock();
  }
}

void Fire::Collides(std::vector<int> *dest) {
  FIXEDNUM fatness = FATNESS;

  if(WEAPON_BAZOOKA == type && FIRESTATE_GOING != state) {
    fatness *= 3;
  }

  for (int ch_index = 0; ch_index < Character::Count(); ch_index++) {
    Character::Ptr ch = Character::Get(ch_index);
    FIXEDNUM tx, ty;

    ch->GetLocation(tx, ty);

    if(abs(tx - x) < fatness && abs(ty - y) < fatness && !ch->Dead()) {
      dest->push_back(ch_index);
    }
  }
}

Fire::Fire() : state(FIRESTATE_INACTIVE) {}

// picks the best color for bullet trails out of the palette
void Fire::AnalyzePalette() {
  bullet_trail_color = Gfx::Get()->MatchingColor
    (RGB(BULLETTRAIL_R, BULLETTRAIL_G, BULLETTRAIL_B));
}

BYTE Fire::bullet_trail_color;

void Fire::PlaySound() {GluPlaySound(WAVSET_WEAPONS + type, x, y);}
