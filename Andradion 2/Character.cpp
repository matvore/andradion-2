// Character.cpp: implementation of the CCharacter::class.
//
//////////////////////////////////////////////////////////////////////

// if this line is not commented, then enemies will not attack you unless fired upon
//#define TESTMODE 

#include "StdAfx.h"
#include "Fixed.h"
#include "SharedConstants.h"
#include "GammaEffects.h"
#include "Net.h"
#include "PowerUp.h"
#include "Character.h"
#include "Fire.h"
#include "Glue.h"

// Comment the next three lines to allow this module to log
#define WriteLog(x)
#undef LogArg
#define LogArg(x)
#undef TryAndReport
#define TryAndReport(x) x

using std::pair;
using NGameLib2::CColor256;

#ifdef BORLAND
#define __max max
#define __min min
using std::max;
using std::min;
#endif

// constants for this module
const FIXEDNUM HEALTHBONUSPERPACK[] = {Fixed(0.4),Fixed(0.25),Fixed(0.20)};
const FIXEDNUM AMMOCAPACITY[] = {Fixed(130),Fixed(500),Fixed(6)};
const FIXEDNUM AMMOSTARTING[] = {Fixed(25 ),Fixed( 0 ),Fixed(0)};
const FIXEDNUM MIN_SAFEDISTANCESQUARED = Fixed(150*150);
const FIXEDNUM MIN_ALIGNMENT = Fixed(3); // how close an alien should be before it starts firing
const int FRAMESTORECOVER_HERO = 15;
const int FRAMESTORECOVER_ALIEN = 15;
const FIXEDNUM HURTFACTOR = Fixed(0.5);

// speeds are in pixels per frame
const FIXEDNUM ALIENSPEED[3] =
  {
    Fixed(1),
    Fixed(2),
    Fixed(3)
  };

const DWORD FRAMESTORELOAD[NUM_WEAPONS] =
  {
    15,
    1,
    100
  };

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

const FIXEDNUM HERO_BAZOOKADAMAGE[] =
  {
    // how much damage is caused in one frame in each difficulty level to the hero
    Fixed(0.003f),
    Fixed(0.006f),
    Fixed(0.010f)
  };

const FIXEDNUM ENEMY_BAZOOKADAMAGE[] =
  {
    // how much damage is caused in one frame in each difficulty level to any enemy
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

void CCharacter::Setup(FIXEDNUM x,FIXEDNUM y,int model_,bool doing_mp) // used for enemies or hero in sp
{
  this->model                   = model_;
  this->coor.first.x            = x;
  this->coor.first.y            = y+Fixed(TILE_HEIGHT/2);
  this->coor.second             = this->coor.first;
  this->health                  = Fixed(1);
  this->direction               = DSOUTH;
  this->frames_in_this_state    = 0;
  this->frames_not_having_sworn = 0;
  this->frames_since_last_fire  = 0;

  if(CHAR_TURNER != this->model && false == doing_mp)
    {
      // simple single-player alien guy
      this->current_weapon = model_;
      this->state = CHARSTATE_UNKNOWING;
    }
  else
    {
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
  x_dist = GLUhero.X();
  y_dist = GLUhero.Y();
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

void CCharacter::Logic(const CCharacter& target)
{
  BeginProfile(Enemy_Logic);

  // xd and yd are the distance in each dimension from the target
  FIXEDNUM xd = abs(GLUcenter_screen_x - (this->coor.first.x                            ));
  FIXEDNUM yd = abs(GLUcenter_screen_y - (this->coor.first.y - Fixed(TILE_HEIGHT/2)));

  // make sure we aren't off the side
  //  of the screen
  if(xd >= Fixed((GAME_MODEWIDTH + TILE_WIDTH)/2) || yd >= Fixed((GAME_PORTHEIGHT + TILE_HEIGHT)/2))
    {
      return;
    }

  // if we got here, we were not off the side of the screen, so
  //  add ourselves to the drawing order list
  GLUdrawing_order.insert(TCharacterPointer(this));

  if(CHARSTATE_HURT != this->state && CHARSTATE_DYING != this->state)
    {
      // we can consider not going through logic this time
      if
	(
	 CHARSTATE_DEAD == target.state                   ||
	 CHARSTATE_DEAD == this->state                    ||
	 GluWalkingData(target.X(),target.Y()) !=
	 GluWalkingData(this->coor.first.x,this->coor.first.y - Fixed(TILE_HEIGHT/2))
	 )
	{
	  // nothing needs to happen right now
	  return;
	}
    }
	
  xd = target.coor.first.x - this->coor.first.x;
  yd = target.coor.first.y - this->coor.first.y;
  FIXEDNUM speed;

  this->frames_since_last_fire++;


  if(CHARSTATE_UNKNOWING == this->state)
    {
#if !defined(TESTMODE)
      // figure out if the hero is close
      //  enough to be noticed
      xd = FixedMul(xd,xd); // square xd
      yd = FixedMul(yd,yd); // square yd
      if(MIN_SAFEDISTANCESQUARED > xd + yd)
	{
	  // this computer player is no longer unaware of turner's presence
	  //  so change the state to ALIVE
	  this->state = CHARSTATE_ALIVE;
	}
#endif

      return;
    }

  GLUhero.CheckForEnemyCollision(*this);

  FIXEDNUM xm; // movement
  FIXEDNUM ym;

  // check for timeout of state
  if(CHARSTATE_ALIVE==this->state || CHARSTATE_WALKING == this->state)
    {
      // see if we are close enough to start firing
      if(abs(xd) <= MIN_ALIGNMENT)
	{
	  this->state = CHARSTATE_FIRING;
			
	  // lining up horizontally and firing vertically
	  this->direction = yd > 0 ? DSOUTH : DNORTH;

	  return;
	}
      else if(abs(yd) <= MIN_ALIGNMENT)
	{
	  this->state = CHARSTATE_FIRING;

	  // lining up vertically and firing horizontally
	  this->direction = xd > 0 ? DEAST : DWEST;

	  return;
	}
      else
	{
	  speed = ALIENSPEED[this->model];

	  // use the filter movement glue function hypothetically to see if 
	  //  aligning horizontally would be a good idea
	  pair<POINT,POINT> hypothetical;
	  hypothetical = this->coor;
	  this->coor.second.y += yd;

	  this->TryToMove();

	  // get closer to the target
	  if(abs(xd) > abs(yd) && abs(this->coor.first.y + yd - this->coor.second.y) <= MIN_ALIGNMENT)
	    {
	      // line up horizontally by changing our y coordinate
	      if(yd > 0) {this->direction = DSOUTH;}
	      else       {this->direction = DNORTH;}
	    }	
	  else
	    {
	      // line up vertically   by changing our x coordinate
	      if(xd > 0) {this->direction = DEAST ;}
	      else       {this->direction = DWEST ;}
	    }

	  // restore our coordinates we had backed up
	  //  in the hypothetical pair
	  this->coor = hypothetical;
	}

      // now use xd and yd to figure out
      //  how to walk in the specified direction
      GluInterpretDirection(this->direction,xm,ym);

      xm = FixedMul(xm,speed);
      ym = FixedMul(ym,speed);

      if(abs(xm) > abs(xd))
	{
	  this->coor.second.x = target.coor.first.x;
	  this->state = CHARSTATE_FIRING;
	  this->direction = yd > 0 ? DSOUTH : DNORTH;
	}
      else if(abs(ym) > abs(yd))
	{
	  this->coor.second.y = target.coor.first.y;
	  this->state = CHARSTATE_FIRING;
	  this->direction = xd > 0 ? DEAST : DWEST;
	}
      else
	{
	  this->coor.second.x=this->coor.first.x + xm;
	  this->coor.second.y=this->coor.first.y + ym;
	  if(++this->frames_in_this_state > FRAMESTOSTEP)
	    {
	      this->state = !this->state;
	      this->frames_in_this_state = 0;
	    }
	}

      this->TryToMove();
    }
  else if(CHARSTATE_FIRING == this->state)
    {
      if(++this->frames_in_this_state > FRAMESTOFIRE)
	{
	  this->state = CHARSTATE_WALKING;
	  this->frames_in_this_state = 0;
	}
      this->TryToFire();
    }
  else /* we are hurt or are dying */ if(++this->frames_in_this_state > FRAMESTORECOVER_ALIEN)
    {
      this->state = CHARSTATE_DYING == this->state ? CHARSTATE_DEAD : CHARSTATE_WALKING;
      this->frames_in_this_state = 0;
    }
  EndProfile(); // enemy logic
}

void CCharacter::Logic()
{
  BeginProfile(Hero_Logic);
  GLUdrawing_order.insert(TCharacterPointer(this));
  this->frames_since_last_fire++;
  static bool firing_last_frame = false;

  if(this->coor.first.x < 0 || this->coor.first.y < 0)
    {
      GluGetRandomStartingSpot(this->coor.first);
      this->coor.first.y += Fixed(TILE_HEIGHT/2);
      this->coor.second = this->coor.first;
    }

  if(CHARSTATE_ALIVE == this->state || CHARSTATE_WALKING == this->state)
    {
      if(++this->frames_in_this_state > FRAMESTOSTEP && CHARSTATE_HURT != this->state) 
	{
	  this->frames_in_this_state = 0;
	  this->state = !this->state;
	}

      // change weapons if necessary
      if(GLUkeyb[DIK_1] & EIGHTHBIT && WEAPON_PISTOL != this->current_weapon)
	{
	  if(!(GLUkeyb[DIK_2] & EIGHTHBIT) && !(GLUkeyb[DIK_3] & EIGHTHBIT))
	    {
	      GluPlaySound(WAV_GUNNOISE,Fixed(1),false);
	      this->current_weapon = WEAPON_PISTOL;
	      NetSendWeaponChangeMessage(WEAPON_PISTOL);
	    }
	}
      else if(GLUkeyb[DIK_2] & EIGHTHBIT && WEAPON_MACHINEGUN != this->current_weapon)
	{
	  if(!(GLUkeyb[DIK_1] & EIGHTHBIT) && !(GLUkeyb[DIK_3] & EIGHTHBIT))
	    {
	      GluPlaySound(WAV_GUNNOISE,Fixed(1),false);
	      this->current_weapon = WEAPON_MACHINEGUN;
	      NetSendWeaponChangeMessage(WEAPON_MACHINEGUN);
	    }
	}
      else if(GLUkeyb[DIK_3] & EIGHTHBIT && WEAPON_BAZOOKA != this->current_weapon)
	{
	  if(!(GLUkeyb[DIK_1] & EIGHTHBIT) && !(GLUkeyb[DIK_2] & EIGHTHBIT))
	    {
	      GluPlaySound(WAV_GUNNOISE,Fixed(1),false);
	      this->current_weapon = WEAPON_BAZOOKA;
	      NetSendWeaponChangeMessage(WEAPON_BAZOOKA);
	    }
	}
    }
  else if(CHARSTATE_HURT == this->state)
    {
      if(++this->frames_in_this_state > FRAMESTORECOVER_HERO)
	{
	  this->frames_in_this_state = 0;
	  this->state = CHARSTATE_ALIVE;
	  this->reset_gamma = true;
	}
    }
  else if(CHARSTATE_DYING == this->state)
    {
      this->reset_gamma = true;
      GluPostSPKilledMessage();

      if(++this->frames_in_this_state > FRAMESTODIE)
	{
	  this->reset_gamma = true;
	  this->frames_in_this_state = 0;
	  if(true == NetInGame())
	    {
	      // resurrect ourselves into a new starting spot
	      GluGetRandomStartingSpot(this->coor.first);
	      this->coor.first.y += Fixed(TILE_HEIGHT/2);
	      this->coor.second = this->coor.first;
	      this->state = CHARSTATE_ALIVE;
	      this->health = Fixed(1);
	      ResetAmmo();
	      this->current_weapon = WEAPON_PISTOL;
	    }
	  else
	    {
	      this->state = CHARSTATE_DEAD;
	    }
	}
    }

  bool walked = true; // assume we are walking

  // now take care of movement
  if(CHARSTATE_WALKING == this->state || CHARSTATE_ALIVE == this->state || CHARSTATE_HURT == this->state)
    {
      FIXEDNUM speed = HEROSPEED;
      int new_direction = -1;

      DWORD directional_buttons = 0;
      if(GLUkeyb[DIK_RIGHT] & EIGHTHBIT) {directional_buttons |= 1;}
      if(GLUkeyb[DIK_UP   ] & EIGHTHBIT) {directional_buttons |= 2;}
      if(GLUkeyb[DIK_LEFT ] & EIGHTHBIT) {directional_buttons |= 4;}
      if(GLUkeyb[DIK_DOWN ] & EIGHTHBIT) {directional_buttons |= 8;}

      // may apply mp factor (multiplayers walk faster)
      FIXEDNUM running_factor;
      if(true == NetInGame())
	{
	  speed = FixedMul(speed,HEROSPEED_MPFACTOR);
	  running_factor = Fixed(1.0f); // no running in MP
	}
      else
	{
	  running_factor = HEROSPEED_RUNNINGFACTOR;
	}

      switch(directional_buttons)
	{
	case 11:	case 1 : new_direction = DEAST ; break;
	case 7 : case 2 : new_direction = DNORTH; break;
	case 14:	case 4 : new_direction = DWEST ;	break;
	case 13: case 8 : new_direction = DSOUTH;	break;
	case 3 :       	new_direction = DNE   ; break;
	case 6 :          new_direction = DNW   ;	break;
	case 9 : 			new_direction = DSE   ; break;
	case 12:          new_direction = DSW   ;	break;
	default: // turner ain't moving his feet
	  walked = false;
	  speed = 0;
	}

      // check if three buttons are pressed
      if
	(
	 11 == directional_buttons ||
	 7 == directional_buttons ||
	 14 == directional_buttons ||
	 13 == directional_buttons ||
	 ((GLUkeyb[DIK_LCONTROL] & EIGHTHBIT || GLUkeyb[DIK_RCONTROL] & EIGHTHBIT) && new_direction < RENDERED_DIRECTIONS)
	 )
	{
	  speed = FixedMul(speed,running_factor);
	}
		
      if(true == walked)
	{
	  FIXEDNUM mx;
	  FIXEDNUM my;
	  GluInterpretDirection(new_direction,mx,my);

	  // may apply hurt factor
	  if(CHARSTATE_HURT == this->state)
	    {
	      speed = FixedMul(speed,HEROSPEED_HURTFACTOR);
	    }

	  mx = FixedMul(mx,speed);
	  my = FixedMul(my,speed);

	  this->coor.second.x = this->coor.first.x + mx;
	  this->coor.second.y = this->coor.first.y + my;

	  if(!(GLUkeyb[DIK_LSHIFT] & EIGHTHBIT) && !(GLUkeyb[DIK_RSHIFT] & EIGHTHBIT))
	    {
	      this->direction = new_direction;
	    }
	  else if(this->direction >= RENDERED_DIRECTIONS)
	    {
	      this->direction -= RENDERED_DIRECTIONS;
	    }
	  this->TryToMove();
	}
    }
  else
    {
      walked = false;
    }

  // now see if they are trying to fire the gun . . .
  if(GLUkeyb[DIK_SPACE] & EIGHTHBIT && 0 != this->ammo[this->current_weapon])
    {
      this->TryToFire();
      if(false == firing_last_frame && WEAPON_BAZOOKA != this->current_weapon)
	{
	  firing_last_frame = true;
	  if(WEAPON_MACHINEGUN == this->current_weapon)
	    {
	      NetSendMachineGunFireStartMessage();
	    }
	}		
    }
  else
    {
      if(true == firing_last_frame && WEAPON_BAZOOKA != this->current_weapon)
	{
	  firing_last_frame = false;
	  if(WEAPON_PISTOL == this->current_weapon)
	    {
	      this->frames_since_last_fire = FRAMESTORELOAD[WEAPON_PISTOL];
	    }
	  else
	    {
	      NetSendMachineGunFireStopMessage();
	    }
	}
    }

  if(false == walked)
    {
      if(CHARSTATE_WALKING == this->state || CHARSTATE_ALIVE == this->state)
	{
	  this->frames_in_this_state = 0;
	}
    }
  else
    {
      this->PowerUpCollisions();
    }
  EndProfile(); // hero logic
}

bool CCharacter::DrawCharacter(CGraphics& gr)
{
  BeginProfile(Draw_Character);

  int bmp;

  int vd; // visual direction (what direction it appears the guy is walking in)

  // calculate visual direction
  if(this->direction >= RENDERED_DIRECTIONS)
    {
      vd = this->direction - RENDERED_DIRECTIONS;
    }
  else
    {
      vd = this->direction;
    }

  if(CHARSTATE_DEAD == this->state)
    {
      bmp = BMP_BLOODSTAIN;
    }
  else if(CHARSTATE_DYING == this->state)
    {
      bmp = BMPSET_DECAPITATE+(rand()&1);
    }
  else
    {
      bmp = BMPSET_CHARACTERS;

      bmp += this->model * ANIMATIONFRAMESPERCHARACTER;

      if(CHARSTATE_WALKING == this->state)
	{
	  bmp += RENDERED_DIRECTIONS;
	}

      bmp += vd;
    }

  bool donot_draw_weapon =
    ((const int)(this->model) == this->current_weapon && this->model != CHAR_EVILTURNER) ||
    (CHARSTATE_DEAD == this->state) ||
    (WEAPON_PISTOL == this->current_weapon && CHAR_CHARMIN == this->model);
	
  FIXEDNUM tx; // screen coordinates to put character
  FIXEDNUM ty;

  tx = this->coor.first.x - GLUcenter_screen_x + Fixed(GAME_MODEWIDTH/2);
  ty = this->coor.first.y - GLUcenter_screen_y + Fixed(GAME_PORTHEIGHT/2);

  tx -= Fixed(TILE_WIDTH/2);
  ty -= Fixed(TILE_HEIGHT/1); // for some reason, the compiler gets error when the "/1" is left out

  RECT *target = &gr.TargetScreenArea();

  target->left = FixedCnvFrom<long>(tx);
  target->right = FixedCnvFrom<long>(tx) + TILE_WIDTH;
  target->top = FixedCnvFrom<long>(ty);
  target->bottom = FixedCnvFrom<long>(ty) + TILE_HEIGHT;

  if(true == donot_draw_weapon)
    {
      WriteLog("Drawing character without drawing weapon");
      this->BlitterRectRecoil(gr,*GLUbitmaps[bmp]);
    }
  else if(CHAR_CHARMIN == this->model || DSOUTH==vd || DEAST == vd)
    {
      WriteLog("Draw character first and then draw weapon");
      TryAndReport(gr.PutFastClip(*GLUbitmaps[bmp]));
      // shake the weapon
      BlitterRectRecoil(gr,*GLUbitmaps[BMPSET_WEAPONS+this->current_weapon*RENDERED_DIRECTIONS+vd]);
    }
  else
    {
      WriteLog("Draw weapon first and then draw the character");
      // shake the weapon
      BlitterRectRecoil(gr,*GLUbitmaps[BMPSET_WEAPONS+this->current_weapon*RENDERED_DIRECTIONS+vd]);
      TryAndReport(gr.PutFastClip(*GLUbitmaps[bmp]));
    }

  WriteLog("Done drawing character and weapon");

  // draw blood if we are hurt
  if(CHARSTATE_HURT == this->state)
    {
      WriteLog("Need to draw blood");
      TryAndReport(gr.PutFastClip(*GLUbitmaps[BMPSET_DECAPITATE+(rand()&1)]));
    }

  if
    (
     (const)this->coor.first.x != (const)this->coor.second.x ||
     (const)this->coor.first.y != (const)this->coor.second.y
     )
    {
      this->coor.first = this->coor.second; // copy over the coordinates
      return true; // return true, meaning we haved moved
    }
  else
    {
      return false; // meaning we haven't moved
    }
  EndProfile(); // draw character
}

void CCharacter::DrawMeters(CGraphics& gr,int show_health)
{
  BeginProfile(Draw_Meters);
  RECT *target = &gr.TargetScreenArea();

  if(SHOWHEALTH_YES == show_health || (CHARSTATE_HURT == this->state && SHOWHEALTH_NO != show_health))
    {
      // set target screen area for the health meter
      target->left = HEALTH_METER_DISTANCE_FROM_SIDES_OF_SCREEN;
      target->bottom = GAME_PORTHEIGHT - HEALTH_METER_DISTANCE_FROM_SIDES_OF_SCREEN;
      target->top = target->bottom - HEALTH_METER_HEIGHT;

      // figure how long the meter will be, and draw it if it is
      //  more than zero
      target->right = target->left + FixedCnvFrom<long>((GAME_MODEWIDTH - 2 * HEALTH_METER_DISTANCE_FROM_SIDES_OF_SCREEN) * FixedMul(this->health,this->health));

      if(target->right > target->left)
	{
	  // make the color unique
	  CColor256 meters;
	  meters.SetColor(HEALTH_METER_R,HEALTH_METER_G,HEALTH_METER_B);
	  meters.Certify();

	  // draw our cool rectangle
	  gr.Rectangle(meters.Color());
	}
    }

  // now the ammo meter - first draw the background border (which is black)
  target->right = GAME_MODEWIDTH - AMMO_METER_DISTANCE_FROM_SIDES_OF_SCREEN;
  target->left = target->right - AMMO_METER_WIDTH;
  target->bottom = GAME_PORTHEIGHT - AMMO_METER_DISTANCE_FROM_SIDES_OF_SCREEN;
  target->top = target->bottom - AMMO_METER_HEIGHT;

  // draw the rectangle outline of the ammo meter
  gr.Rectangle(0);

  // make the actual measurement rectangle slightly thinner
  target->left++;
  target->right--;

  FIXEDNUM what_to_measure;

  // set what_to_measure to the percentage of ammo we have
  what_to_measure = FixedCnvTo<long>(this->frames_since_last_fire) / FRAMESTORELOAD[WEAPON_BAZOOKA];
	
  // we will show an alternate meter measuring time left to load if the player is using a bazooka
  if(WEAPON_BAZOOKA != this->current_weapon || what_to_measure >= Fixed(1.0f) || 0 == this->ammo[WEAPON_BAZOOKA])
    {
      what_to_measure = FixedCnvTo<double>(sqrt(FixedCnvFrom<double>(this->ammo[this->current_weapon])));
    }

  target->top += AMMO_METER_HEIGHT - FixedCnvFrom<long>(what_to_measure * AMMO_METER_HEIGHT);

  if(target->top < target->bottom)
    {
      // make the color unique
      CColor256 meters;
      meters.SetColor(AMMO_METER_R,AMMO_METER_G,AMMO_METER_B);
      meters.Certify();

      // draw our cool rectangle
      gr.Rectangle(meters.Color());
    }

  EndProfile(); // draw meters
}

void CCharacter::SubtractHealth(int fire_type)
{
  if(CHARSTATE_DEAD == this->state)
    {
      return;
    }

  FIXEDNUM pain; // how much is actually taken away is calculated now

  bool we_are_hero = (&GLUhero == this);
	
  // figure out if we're the hero
  if(true == we_are_hero)
    {
      // we are the hero

      // do gamma effects
      GamGetShot();

      if(WEAPON_BAZOOKA != fire_type)
	{
	  pain = HERO_WEAPONDAMAGE[GLUdifficulty][fire_type];
	  if(CHARSTATE_HURT == this->state)
	    {
	      pain = FixedMul(pain,HURTFACTOR);
	    }
	  else
	    {
	      this->state = CHARSTATE_HURT;
	      this->frames_in_this_state = 0;
	    }
	}
      else
	{
	  // getting hit with bazooka
	  if(CHARSTATE_HURT == this->state)
	    {
	      pain = HERO_BAZOOKADAMAGE[GLUdifficulty];
	    }
	  else
	    {
	      pain = 0;
	      this->state = CHARSTATE_HURT;
	    }
	  this->frames_in_this_state = 0;
	}
    }
  else
    {
      // we are not the hero
      if(WEAPON_BAZOOKA != fire_type)
	{
	  pain = ENEMY_WEAPONDAMAGE[GLUdifficulty][fire_type];
	}
      else
	{
	  if(CHARSTATE_HURT == this->state)
	    {
	      pain = ENEMY_BAZOOKADAMAGE[GLUdifficulty];
	    }
	  else
	    {
	      pain = 0;
	    }
	}			
      this->state = CHARSTATE_HURT;
      this->frames_in_this_state = 0;
    }

  this->health = __max(Fixed(0),this->health-pain);

  // put ourselves in a hurting state
  if(0 == this->health)
    {
      if(CHAR_EVILTURNER == this->model && false == we_are_hero)
	{
	  // we just lost our disguise . . .
	  this->Setup(this->coor.first.x,this->coor.first.y-Fixed(TILE_HEIGHT/2),CHAR_SALLY,false);
	  this->state = CHARSTATE_HURT; // override setup function's value of CHARSTATE_UNKNOWING
	  this->PlaySound();
	  GluChangeScore(GluScoreDiffKill(CHAR_EVILTURNER));
	}
      else
	{
	  // we are really dead . . .
	  this->state = CHARSTATE_DYING;
	  this->PlaySound();
	  this->frames_in_this_state = 0;
			
	  if(false == we_are_hero)
	    {
	      GluChangeScore(GluScoreDiffKill(this->model));
	    }
	  else if(true == NetInGame())
	    {
	      NetSendDeathMessage(this->ammo);
	    }
	}
    }
  else
    {
      if
	(
	 CHARSTATE_HURT != this->state ||
	 WEAPON_PISTOL == fire_type ||
	 ++this->frames_not_having_sworn > FRAMESTOSWEARAGAIN
	 )
	{
	  this->PlaySound(); 
	  this->frames_not_having_sworn = 0;
	  NetSendAdmitHitMessage(); // we've been hit!
	}
    }
}

void CCharacter::TryToFire()
{
  bool we_are_hero = bool(this == &GLUhero);

  if(false == we_are_hero)
    {
      // we are not the hero; make sure we are in a firing state
      if(CHARSTATE_FIRING != this->state || this->IsOffScreen())
	{
	  return;
	}
    }
  else if(CHARSTATE_DYING == this->state || CHARSTATE_DEAD == this->state)
    {
      return;
    }

  // look for a good slot to use
  int i;
  for(i = 0; i < MAX_FIRES; i++)
    {
      if(GLUfires[i].OkayToDelete())
	{
	  // we found memory that the CFire object can occupy
	  break;
	}
    }

  if(MAX_FIRES == i)
    {
      // no slot available yet
      return;
    }

  if(this->frames_since_last_fire < FRAMESTORELOAD[this->current_weapon])
    {
      return;
    }

  FIXEDNUM bullets = FixedMul(this->ammo[current_weapon],AMMOCAPACITY[current_weapon]);
  if(true == we_are_hero && bullets < Fixed(0.5))
    {
      // oh, out of ammo, nevermind
      return;
    }

  bullets -= Fixed(1);

  // convert bullets back into percentage
  bullets = FixedDiv(bullets,AMMOCAPACITY[current_weapon]);

  if(bullets < 0) {bullets = 0;}

  this->ammo[this->current_weapon] = bullets;

  GLUfires[i].Setup(this->coor.second.x,this->coor.second.y-Fixed(TILE_HEIGHT/2),this->direction,this->current_weapon,false);

  this->frames_since_last_fire = 0;
}

void CCharacter::PowerUpCollisions()
{
  // checks to see if the character has found a powerup

  for(VCTR_POWERUP::iterator iterate = GLUpowerups.begin(); iterate != GLUpowerups.end(); iterate++)
    {
      int type = iterate->Type();

      int j;

      switch(iterate->Collides(*this))
	{
	case POWERUPCOLLIDES_NOTHINGHAPPENED:
	  break;
	case POWERUPCOLLIDES_PICKEDUPANDWILLNOTREGENERATE:
	  // we just got an ammo set
	  NetSendPowerUpPickUpMessage(iterate-GLUpowerups.begin());

	  for(j = 0; j < NUM_WEAPONS; j++)
	    {
	      this->ammo[j] += __min(Fixed(1),this->ammo[type]+iterate->Ammo(j));
	    }
		
	  GLUpowerups.erase(iterate);

	  // we erased something, so retry this index by doing i-=1
	  iterate--;

	  GamPickupAmmo();
	  break;
	default:
	  //case POWERUPCOLLIDES_PICKEDUPANDWILLREGENERATE:
	  NetSendPowerUpPickUpMessage(iterate-GLUpowerups.begin());

	  if(POWERUP_HEALTHPACK == type)
	    {
	      this->health = __min(Fixed(1),this->health+HEALTHBONUSPERPACK[GLUdifficulty]);
	      GamPickupHealth();
	    }
	  else
	    {
	      this->ammo[type] = __min(Fixed(1),this->ammo[type]+FixedDiv(AMMOPERPACK[GLUdifficulty][type],AMMOCAPACITY[type]));
	      GamPickupAmmo();
	    }
	}
    }
}

#pragma warning (disable : 4035)

bool CCharacter::IsOffScreen() const
{
  return
    (
     abs(GLUcenter_screen_x - this->coor.first.x) > Fixed((GAME_MODEWIDTH + TILE_WIDTH)/2) ||
     abs(GLUcenter_screen_y - (this->coor.first.y-Fixed(TILE_HEIGHT/2))) > Fixed((GAME_PORTHEIGHT + TILE_HEIGHT)/2)
     );
}

#pragma warning (default : 4035)

void CCharacter::TryToMove()
{
  // called by CGlue when the surface is locked and we can try to move

  pair<POINT,POINT> l = this->coor; // keep a local copy of our coordinates

  if((const int)l.first.x != l.second.x)
    {
      if((const int)l.first.y != l.second.y)
	{
	  // moving diagonally, let's try recursion

	  // first move horizontall,y and then move vertically
	  this->coor.second.y = this->coor.first.y;

	  this->TryToMove();

	  this->coor.first = this->coor.second;

	  this->coor.second.y = l.second.y;

	  this->TryToMove();

	  this->coor.first = l.first;
	}
      else
	{
	  // moving horizontally
	  if(l.first.x < l.second.x)
	    {
	      // moving to the right

	      pair<POINT,POINT> a;
	      pair<POINT,POINT> b;

	      a.first.y = a.second.y = l.first.y                    ;
	      b.first.y = b.second.y = l.first.y - Fixed(TILE_HEIGHT/2);
				
	      a.first.x = l.first.x+Fixed(TILE_WIDTH/4)               ;
	      a.second.x = l.second.x+Fixed(TILE_WIDTH/4)              ;
	      b.first.x = a.first.x;
	      b.second.x = a.second.x;
				
	      GluFilterMovement(a);
	      GluFilterMovement(b);

	      this->coor.second.x = __min(a.second.x,b.second.x)-Fixed(TILE_WIDTH/4);

	      //assert(this->coor.second.x >= this->coor.first.x);
	      //assert((const)this->coor.second.y == (const)this->coor.first.y);
	    }
	  else
	    {
	      // moving to the left

	      pair<POINT,POINT> a;
	      pair<POINT,POINT> b;

	      a.first.y = a.second.y = l.first.y;
	      b.first.y = b.second.y = l.first.y - Fixed(TILE_HEIGHT/2);
				
	      a.first.x = l.first.x-Fixed(TILE_WIDTH/4);
	      a.second.x = l.second.x-Fixed(TILE_WIDTH/4);
	      b.first.x = a.first.x;
	      b.second.x = a.second.x;
				
	      GluFilterMovement(a);
	      GluFilterMovement(b);

	      this->coor.second.x = __max(a.second.x,b.second.x)+Fixed(TILE_WIDTH/4);

	      //assert(this->coor.second.x <= this->coor.first.x);
	      //assert((const)this->coor.second.y == (const)this->coor.first.y);
	    }
	}
    }
  else if((const int)l.first.y != l.second.y)
    {
      // moving vertically
      if(l.first.y < l.second.y)
	{
	  // moving down
	  pair<POINT,POINT> a;
	  pair<POINT,POINT> b;
	  a.first.x = a.second.x = l.first.x + Fixed(TILE_WIDTH/4);
	  b.first.x = b.second.x = l.first.x - Fixed(TILE_WIDTH/4);
	  a.first.y = b.first.y = l.first.y;
	  a.second.y = b.second.y = l.second.y ;
	  GluFilterMovement(a);
	  GluFilterMovement(b);
	  this->coor.second.y = __min(a.second.y,b.second.y);
	  //assert(this->coor.second.y >= this->coor.first.y);
	  //assert((const)this->coor.second.x == (const)this->coor.first.x);
	}
      else
	{
	  // moving up
	  pair<POINT,POINT> a;
	  pair<POINT,POINT> b;
	  a.first.x = a.second.x = l.first.x + Fixed(TILE_WIDTH/4);
	  b.first.x = b.second.x = l.first.x - Fixed(TILE_WIDTH/4);
	  a.first.y = b.first.y = l.first.y-Fixed(TILE_HEIGHT/2);
	  a.second.y = b.second.y = l.second.y-Fixed(TILE_HEIGHT/2);
	  GluFilterMovement(a);
	  GluFilterMovement(b);
	  this->coor.second.y = __max(a.second.y,b.second.y)+Fixed(TILE_HEIGHT/2);
	  //assert(this->coor.second.y <= this->coor.first.y);
	  //assert((const)this->coor.second.x == (const)this->coor.first.x);
	}
    }
}

void CCharacter::ResetAmmo()
{
  for(int i = 0; i < NUM_WEAPONS; i++)
    {
      this->ammo[i] = FixedDiv(AMMOSTARTING[i],AMMOCAPACITY[i]);
    }
}

void CCharacter::CheckForEnemyCollision(const CCharacter& enemy)
{
  if(abs(this->coor.second.x - enemy.coor.first.x) < FATNESS && abs(this->coor.second.y - enemy.coor.first.y) < FATNESS)
    {
      this->coor.second = this->coor.first;
    }
}

void CCharacter::CalculateSector(int& row,int& col)
{
  this->sector_row = row = (FixedCnvFrom<long>(this->coor.first.y) - TILE_HEIGHT/2) / SECTOR_HEIGHT;
  this->sector_col = col = FixedCnvFrom<long>(this->coor.first.x)  / SECTOR_WIDTH;
}

void CCharacter::BlitterRectRecoil(CGraphics& gr,CBob& b)
{
  /*
    if(0 == this->frames_since_last_fire)
    {
    int change = 
    FixedCnvFrom<long>
    (
    ((rand()%RECOIL_RANGE[this->current_weapon])+
    RECOIL_RANGE_OFFSET[this->current_weapon])*RECOIL_UNIT
    );

    RECT *r = &gr.TargetScreenArea();
    r->top    += change;
    r->bottom += change;
    TryAndReport(gr.PutFastClip(b));
    r->top    -= change;
    r->bottom -= change;
    } // end if we fired this frame
    else*/
  {
    // no recoil, blit normally
    TryAndReport(gr.PutFastClip(b));
  } // end if we haven't fired this frame  
} // end function BlitterRectRecoil
