#include "StdAfx.h"
#include "Fixed.h"
#include "Glue.h"
#include "Net.h"
#include "Character.h"
#include "Fire.h"
#include "Graphics.h"

using std::pair;
using std::max;
using std::min;
using std::vector;
using std::abs;

// constants used in this module
const BYTE BULLETTRAIL_R = 255;
const BYTE BULLETTRAIL_G = 255;
const BYTE BULLETTRAIL_B = 0;
const int MACHINEGUNSWAY = 10;
const FIXEDNUM BULLETSPEED = Fixed(20);
const int FRAMESTOEXPLOSION = 70;
const FIXEDNUM PROJECTILE_INITIALMOVEMENT = Fixed(15);
const FIXEDNUM PROJECTILE_SPEED = Fixed(10); 
const int FIRECOLLIDES_NOCOLLISION = -3;
const int FIRECOLLIDES_HERO = -2;
const int FIRECOLLIDES_WALL = -1;
const int MAX_EXPLOSIONBULGE = 5;
const int HORIZONTALCOLLISION_UPPER = 1;
const int HORIZONTALCOLLISION_LOWER = 2;
enum {FIRESTATE_GOING, FIRESTATE_FIRSTFRAMETODRAW,
      FIRESTATE_INACTIVE = -1};

CFire fires[MAX_FIRES];

bool CFire::OkayToDelete() const {return FIRESTATE_INACTIVE == state;}

void CFire::Setup(FIXEDNUM sx_, FIXEDNUM sy_,
                  FIXEDNUM tx_, FIXEDNUM ty_) {
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

void CFire::Setup(FIXEDNUM sx_, FIXEDNUM sy_, int direction_,
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

void CFire::Logic() {
  if(FIRESTATE_INACTIVE == state) {
    return;
  }

  vector<int> collex;
  int coll;

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
      coll = Collides();
      if(WEAPON_BAZOOKA != type) {
        if(FIRECOLLIDES_HERO == coll) {
          if(!NetInGame()) {
            hero.SubtractHealth(type);
          }

          state = WEAPON_PISTOL != type
            ? FIRESTATE_FIRSTFRAMETODRAW : FIRESTATE_INACTIVE;
          break;
        } else if(FIRECOLLIDES_NOCOLLISION != coll) {
          if(!remotely_generated) {
            if (NetInGame()) {
              NetHit(coll, type);
            } else {
              enemies[coll].SubtractHealth(type);
            }
          }
          state = WEAPON_PISTOL != type
            ? FIRESTATE_FIRSTFRAMETODRAW : FIRESTATE_INACTIVE;
          break;
        }
      } else if(FIRECOLLIDES_NOCOLLISION != coll) {
        state = FIRESTATE_FIRSTFRAMETODRAW;
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
    // exploding bazooka		
    if(++frames_since_explosion_started > FRAMESTOEXPLOSION) {
      state = FIRESTATE_INACTIVE;
      return;
    }
				
    state++;
    collex = CollidesEx();

    for(vector<int>::const_iterator iterate = collex.begin();
        iterate != collex.end(); iterate++) {
      if(FIRECOLLIDES_HERO == *iterate) {
        hero.SubtractHealth(type);
      } else if(enemies.size() > *iterate) {
        if(!NetInGame()) {
          enemies[*iterate].SubtractHealth(type);
        }
      }
    }
  }
}

class bullet_trail_line_pattern {
  int visible_points_left;
  int points_to_start;

public:
  bullet_trail_line_pattern(int length) {
    visible_points_left = (rand() % 16) + 16;
    points_to_start = length <= visible_points_left
      ? 0 : (rand() % (length - visible_points_left));
  }

  bool operator()() {
    return points_to_start-- <= 0 && visible_points_left-- > 0;
  }
};

void CFire::Draw() {
  if(FIRESTATE_INACTIVE == state) {
    return; // no business drawing an inactive projectile
  }

  if(WEAPON_MACHINEGUN == type) {
    state = FIRESTATE_INACTIVE; // no longer needed after drawn once
  }

  if(WEAPON_PISTOL == type) {

    int target_x = FixedCnvFrom<long>(x - GLUcenter_screen_x)
      -TILE_WIDTH/2+GAME_MODEWIDTH/2;
    int target_y = FixedCnvFrom<long>(y - GLUcenter_screen_y)
      -TILE_HEIGHT/2+GAME_PORTHEIGHT/2;
		
    GluDraw(BMP_BULLET, target_x, target_y);
    return; 
  }

  int bulge;

  // take care of explosions first
  if(WEAPON_BAZOOKA == type) {
    // draw explosion now
    bulge = rand()%MAX_EXPLOSIONBULGE;
		
    RECT target;
    target.left = FixedCnvFrom<long>(x - GLUcenter_screen_x)
      + GAME_MODEWIDTH/2 - bulge - TILE_WIDTH/2;
    target.top = FixedCnvFrom<long>(y - GLUcenter_screen_y)
      + GAME_PORTHEIGHT/2 - bulge - TILE_HEIGHT/2;
    target.right = target.left + TILE_WIDTH + bulge*2;
    target.bottom = target.top + TILE_HEIGHT + bulge*2;

    GluDrawScale(BMP_EXPLOSION, &target);

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

  int x0 = FixedCnvFrom<long>(sx-GLUcenter_screen_x) + GAME_MODEWIDTH/2;
  int y0 = FixedCnvFrom<long>(sy-GLUcenter_screen_y) + GAME_PORTHEIGHT/2;
  int x1 = FixedCnvFrom<long>(x-GLUcenter_screen_x) + GAME_MODEWIDTH/2
    + swayx;
  int y1 = FixedCnvFrom<long>(y-GLUcenter_screen_y) + GAME_PORTHEIGHT/2
    + swayy;

  if(GfxClipLine(x0, y0, x1, y1)) {
    GfxLock lock(GfxLock::Back());

    GfxLine<bullet_trail_line_pattern, bullet_trail_line_color>
      (lock(x0, y0), lock.Pitch(), x1 - x0, y1 - y0,
       bullet_trail_line_color(),
       bullet_trail_line_pattern(max(abs(x1 - x0), abs(y1 - y0))));
  }
}

vector<int> CFire::CollidesEx() {
  FIXEDNUM tx, ty, fatness;
  vector<int> ret;
  ret.resize(0);
	
  fatness = FATNESS;

  if(WEAPON_BAZOOKA == type && FIRESTATE_GOING != state) {
    fatness *= 3;
  }
	
  // check for collision with enemy
  for(vector<CCharacter>::iterator iterate = enemies.begin();
      iterate != enemies.end(); iterate++) {
    iterate->GetLocation(tx,ty);

    if(abs(tx - x) < fatness && abs(ty - y) < fatness
       && !iterate->Dead()) {
      ret.resize(ret.size()+1,iterate - enemies.begin());
    }
  }

  // check for collision with hero
  hero.GetLocation(tx,ty);
  if(abs(tx - x) < fatness && abs(ty - y) < fatness
     && !hero.Dead()) {
    ret.resize(ret.size()+1,FIRECOLLIDES_HERO); // we found a winner
  }

  // return with our results
  return ret;
}

int CFire::Collides() {
  FIXEDNUM tx, ty;

  assert(WEAPON_BAZOOKA != type || FIRESTATE_GOING == state);
	
  // check for collision with hero
  hero.GetLocation(tx,ty);
  if(abs(tx - x) < FATNESS && abs(ty - y) < FATNESS && !hero.Dead()) {
    return FIRECOLLIDES_HERO; 
  }

  // check for collision with enemy
  for(vector<CCharacter>::iterator iterate = enemies.begin();
      iterate != enemies.end(); iterate++) {
    iterate->GetLocation(tx,ty);

    if(abs(tx - x) < FATNESS && abs(ty - y) < FATNESS
       && !iterate->Dead()) {
      return iterate - enemies.begin();
    }
  }

  return FIRECOLLIDES_NOCOLLISION;
}

CFire::CFire() : state(FIRESTATE_INACTIVE) {}

// picks the best color for bullet trails out of the palette
void CFire::PickBestBulletTrailColor() {
  bullet_trail_color = GfxGetPaletteEntry(RGB(BULLETTRAIL_R,
                                              BULLETTRAIL_G,
                                              BULLETTRAIL_B));
}

BYTE CFire::bullet_trail_color;

void CFire::PlaySound() {
  // play the sound of the weapon firing
  FIXEDNUM hx, hy;

  hero.GetLocation(hx, hy);

  GluPlaySound(WAVSET_WEAPONS + type, x - hx, y - hy);
}
