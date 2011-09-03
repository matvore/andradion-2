// Fire.cpp: implementation of the CFire class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Fixed.h"
#include "Net.h"
#include "SharedConstants.h"
#include "Fire.h"
#include "Character.h"
#include "Glue.h"

// Comment the next five lines to allow this module to log
#define WriteLog(x)
#undef LogArg
#define LogArg(x)
#undef TryAndReport
#define TryAndReport(x) 

using std::pair;
using NGameLib2::CColor256;
using NGameLib2::CSurfaceLock256;

#ifdef BORLAND
#define __max max
#define __min min
using std::max;
using std::min;
#endif

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

void CFire::Setup(FIXEDNUM sx_,FIXEDNUM sy_,FIXEDNUM tx_,FIXEDNUM ty_)
{
  this->direction = 0;
  this->remotely_generated = true;
  this->type = WEAPON_BAZOOKA;

  this->sx = sx_;
  this->sy = sy_;
  this->x = tx_;
  this->y = ty_;

  this->PlaySound();

  this->state = FIRESTATE_GOING;
  this->frames_since_explosion_started = 0;
}

void CFire::Setup(FIXEDNUM sx_,
		  FIXEDNUM sy_,
		  int direction_,
		  int type_,
		  bool remotely_generated_)
{
  FIXEDNUM mx;
  FIXEDNUM my;

  this->direction = direction_;
  this->type = type_;
  this->state = FIRESTATE_GOING;

  this->remotely_generated = remotely_generated_;

  if(false == this->remotely_generated && WEAPON_PISTOL == this->type)
    {
      NetSendPistolFireMessage(this->direction);
    }

  GluInterpretDirection(this->direction,mx,my);

  this->x = sx_ + FixedMul(mx,PROJECTILE_INITIALMOVEMENT);
  this->y = sy_ + FixedMul(my,PROJECTILE_INITIALMOVEMENT);

  this->sx = this->x;
  this->sy = this->y;

  this->PlaySound();

  // do some extra things if we are using horizontal collision flags
  this->horizontal_collision_flags = (0 == mx) ? HORIZONTALCOLLISION_LOWER : 0;

  // this is only imnportant if we are bazooka	
  this->frames_since_explosion_started = 0;
}

void CFire::Logic() // returns true if it can be safely deleted
{
  if(FIRESTATE_INACTIVE == this->state)
    {
      return;
    }

  VCTR_INT collex;
  int coll;

  collex.resize(0);

  if(FIRESTATE_GOING == this->state)
    {
      if(WEAPON_BAZOOKA == this->type && true == this->remotely_generated)
	{
	  // do not need to perform movement
	  this->state = FIRESTATE_FIRSTFRAMETODRAW;
	  return;
	}

      pair<POINT,POINT> hypothetical;

      hypothetical.first.x = this->x;
      hypothetical.first.y = this->y;

      FIXEDNUM dest_x;
      FIXEDNUM dest_y;
      GluInterpretDirection(this->direction,dest_x,dest_y);
      FIXEDNUM proj_speed_x;
      proj_speed_x = FixedMul(dest_x,PROJECTILE_SPEED);
      FIXEDNUM proj_speed_y;
      proj_speed_y = FixedMul(dest_y,PROJECTILE_SPEED);
      dest_x = FixedMul(dest_x,BULLETSPEED);
      dest_y = FixedMul(dest_y,BULLETSPEED);
      dest_x += this->x;
      dest_y += this->y;

      do
	{
	  if(WEAPON_PISTOL == this->type)
	    {
	      if(proj_speed_x > 0)
		{
		  hypothetical.second.x = __min(dest_x,(int)hypothetical.first.x+proj_speed_x);
		}
	      else
		{
		  hypothetical.second.x = __max(dest_x,(int)hypothetical.first.x+proj_speed_x);
		}
	      if(proj_speed_y > 0)
		{
		  hypothetical.second.y = __min(dest_y,(int)hypothetical.first.y+proj_speed_y);
		}	
	      else
		{
		  hypothetical.second.y = __max(dest_y,(int)hypothetical.first.y+proj_speed_y);
		}
	    }
	  else
	    {
	      hypothetical.second.x = hypothetical.first.x+proj_speed_x;
	      hypothetical.second.y = hypothetical.first.y+proj_speed_y;
	    }

	  // check for collisions
	  coll = this->Collides();
	  if(WEAPON_BAZOOKA != this->type)
	    {
	      if(FIRECOLLIDES_HERO == coll)
		{
		  if(false == NetInGame())
		    {
		      GLUhero.SubtractHealth(this->type);
		    }
		  this->state = (WEAPON_PISTOL != this->type) ?
		    FIRESTATE_FIRSTFRAMETODRAW : FIRESTATE_INACTIVE;
		  return;
		}
	      else if(FIRECOLLIDES_NOCOLLISION != coll)
		{
		  if(false == this->remotely_generated)
		    {
		      NetSendHitMessage(coll,this->type);
		    }
		  this->state = (WEAPON_PISTOL != this->type) ?
		    FIRESTATE_FIRSTFRAMETODRAW : FIRESTATE_INACTIVE;
		  return;
		}
	    }
	  else if(FIRECOLLIDES_NOCOLLISION != coll)
	    {
	      this->state = FIRESTATE_FIRSTFRAMETODRAW;
	      return;
	    }

	  this->x = hypothetical.second.x;
	  this->y = hypothetical.second.y;

	  GluFilterMovement(hypothetical);

	  if(this->y != hypothetical.second.y)
	    {

	      this->state = (WEAPON_PISTOL != this->type) ?
		FIRESTATE_FIRSTFRAMETODRAW : FIRESTATE_INACTIVE;

	      return;
	    }
	  else
	    {
	      // we are doing a horizontal movement
	      if(this->x != hypothetical.second.x)
		{
		  this->horizontal_collision_flags |= HORIZONTALCOLLISION_UPPER;
		  if(HORIZONTALCOLLISION_LOWER & this->horizontal_collision_flags)
		    {
		      this->state = (WEAPON_PISTOL != this->type) ?
			FIRESTATE_FIRSTFRAMETODRAW : FIRESTATE_INACTIVE;
		      return;
		    }
		}
				
	      if(!(HORIZONTALCOLLISION_LOWER & this->horizontal_collision_flags))
		{
		  // let's try the lower one because so far,
		  //  it's been working out . . .
		  hypothetical.second.y += Fixed(TILE_HEIGHT/2);
		  hypothetical.second.x = this->x;
		  hypothetical.first.y = hypothetical.second.y;
	
		  GluFilterMovement(hypothetical);
		  if(this->x != hypothetical.second.x)
		    {
		      this->horizontal_collision_flags |= HORIZONTALCOLLISION_LOWER;
		      if(HORIZONTALCOLLISION_UPPER & this->horizontal_collision_flags)
			{
			  this->state = (WEAPON_PISTOL != this->type) ?
			    FIRESTATE_FIRSTFRAMETODRAW : FIRESTATE_INACTIVE;
			  return;
			}
		    }

		  hypothetical.second.y -= Fixed(TILE_HEIGHT/2);
		}
	    }
			
	  hypothetical.first = hypothetical.second;
	}
      while
	(
	 hypothetical.first.x != dest_x ||
	 hypothetical.first.y != dest_y || WEAPON_PISTOL != this->type
	 );
    }
  else if(WEAPON_BAZOOKA == this->type)
    {
      // exploding bazooka		
      if(++this->frames_since_explosion_started > FRAMESTOEXPLOSION)
	{
	  this->state = FIRESTATE_INACTIVE;
	  return;
	}
				
      this->state++;
      collex = this->CollidesEx();

      for(VCTR_INT::const_iterator iterate = collex.begin(); iterate != collex.end(); iterate++)
	{
	  if(FIRECOLLIDES_HERO == *iterate)
	    {
	      GLUhero.SubtractHealth(this->type);
	    }
	  else if((int)GLUenemies.size() > *iterate)
	    {
	      if(false == NetInGame())
		{
		  GLUenemies[*iterate].SubtractHealth(this->type);
		}
	    }
	}
    }
}

void CFire::Draw(CGraphics& gr)
{
  if(FIRESTATE_INACTIVE == this->state)
    {
      return; // no business drawing an inactive projectile
    }

  if(WEAPON_MACHINEGUN == this->type)
    {
      this->state = FIRESTATE_INACTIVE; // no longer needed after drawn once
    }

  if(WEAPON_PISTOL == this->type)
    {
      RECT *target = &gr.TargetScreenArea();
      target->left = FixedCnvFrom<long>(this->x - GLUcenter_screen_x)-TILE_WIDTH/2+GAME_MODEWIDTH/2;
      target->top = FixedCnvFrom<long>(this->y- GLUcenter_screen_y)-TILE_HEIGHT/2+GAME_PORTHEIGHT/2;
      target->right = target->left+TILE_WIDTH;
      target->bottom = target->top+TILE_HEIGHT;
		
      gr.PutFastClip(*GLUbitmaps[BMP_BULLET]);
      return; // we can leave now
    }

  int bulge;

  // take care of explosions first
  if(WEAPON_BAZOOKA == this->type)
    {
      // draw explosion now
      bulge = rand()%MAX_EXPLOSIONBULGE;
		
      RECT *target = &gr.TargetScreenArea();
      target->left = FixedCnvFrom<long>(this->x - GLUcenter_screen_x) + GAME_MODEWIDTH/2 - bulge - TILE_WIDTH/2;
      target->top = FixedCnvFrom<long>(this->y - GLUcenter_screen_y) + GAME_PORTHEIGHT/2 - bulge - TILE_HEIGHT/2;
      target->right = target->left + TILE_WIDTH + bulge*2;
      target->bottom = target->top + TILE_HEIGHT + bulge*2;

      gr.Buffer(1)->SetClipper(GLUclipper);
		
      gr.Put(*GLUbitmaps[BMP_EXPLOSION],DDBLT_WAIT | DDBLT_KEYSRC);

      gr.Buffer(1)->SetClipper(NULL);

      if(FIRESTATE_FIRSTFRAMETODRAW != this->state)
	{
	  // return so we don't draw the smoke trail
	  return;
	}
    }

  // draw a yellow line from sx,sy to x,y
  int swayx;
  int swayy;

  // only machine guns have a swaying motion and bullet patterns
  DWORD line_pattern = 0xffffffff;

  if(WEAPON_MACHINEGUN == this->type)
    {
      GluInterpretDirection(this->direction,swayx,swayy);
      if(0 != swayx && 0 != swayy)
	{
	  swayx = (rand()%MACHINEGUNSWAY)-(MACHINEGUNSWAY/2);
	  swayy = (rand()%MACHINEGUNSWAY)-(MACHINEGUNSWAY/2);
	}
      else if(0 != swayx)
	{
	  swayy = (rand()%MACHINEGUNSWAY)-(MACHINEGUNSWAY/2);
	  swayx = 0;
	}
      else if(0 != swayy)
	{
	  swayx = (rand()%MACHINEGUNSWAY)-(MACHINEGUNSWAY/2);
	  swayy = 0;
	}

      // do a line pattern-thing real cool
      int bit = rand()&31;

      // clear out first part of pattern
      int i;
      int limit = __min(32,bit+16);
      for(i = bit; i < limit; i++)
	{
	  line_pattern &= ~(1<<i);
	}
		
      // clear out second part
      limit = 16 - (32-bit);
      for(i = 0; i < limit; i++)
	{
	  line_pattern &= ~(1<<i);
	}

      // now sixteen consectutive bits have been cleared out
    }
  else
    {
      swayx = 0;
      swayy = 0;
    }

  // draw our line

  int x0 = FixedCnvFrom<long>(this->sx-GLUcenter_screen_x)+GAME_MODEWIDTH/2;
  int y0 = FixedCnvFrom<long>(this->sy-GLUcenter_screen_y)+GAME_PORTHEIGHT/2;
  int x1 = FixedCnvFrom<long>(this->x-GLUcenter_screen_x)+GAME_MODEWIDTH/2+swayx;
  int y1 = FixedCnvFrom<long>(this->y-GLUcenter_screen_y)+GAME_PORTHEIGHT/2+swayy;

  CSurfaceLock256 lock;

  lock.SetArea(NULL);
  lock.SetLockBehaviorFlags(DDLOCK_WAIT | DDLOCK_WRITEONLY);
  lock.SetTargetSurface(gr);
	
  if(true == gr.ClipLine(x0,y0,x1,y1) && 0 == lock.Certify())
    {
      lock.Line(x0,y0,x1,y1,CFire::bullet_trail_color,line_pattern);
      lock.Uncertify();
    }

  // and we're done
}

vector<int> CFire::CollidesEx()
{
  FIXEDNUM tx;
  FIXEDNUM ty;
  FIXEDNUM fatness;
  vector<int> ret;
  ret.resize(0);
	
  fatness = FATNESS;

  if(WEAPON_BAZOOKA == this->type && FIRESTATE_GOING != this->state)
    {
      fatness *= 3;
    }
	
  // check for collision with enemy
  VCTR_CHARACTER *e = &(GLUenemies);
	
  for(VCTR_CHARACTER::iterator iterate = e->begin(); iterate != e->end(); iterate++)
    {
      iterate->GetLocation(tx,ty);

      if(abs(tx - this->x) < fatness && abs(ty - this->y) < fatness && false == iterate->Dead())
	{
	  ret.resize(ret.size()+1,iterate - e->begin());
	}
    }

  // check for collision with hero
  GLUhero.GetLocation(tx,ty);
  if(abs(tx - this->x) < fatness && abs(ty - this->y) < fatness && false == GLUhero.Dead())
    {
      ret.resize(ret.size()+1,FIRECOLLIDES_HERO); // we found a winner
    }

  // return with our results
  return ret;
}

int CFire::Collides()
{
  FIXEDNUM tx;
  FIXEDNUM ty;

  assert(WEAPON_BAZOOKA != this->type || FIRESTATE_GOING == this->state);
	
  // check for collision with hero
  GLUhero.GetLocation(tx,ty);
  if(abs(tx - this->x) < FATNESS && abs(ty - this->y) < FATNESS && false == GLUhero.Dead())
    {
      return FIRECOLLIDES_HERO; // we found a winner
    }

  // check for collision with enemy
  VCTR_CHARACTER *e = &(GLUenemies);
	
  for(VCTR_CHARACTER::iterator iterate = e->begin();iterate != e->end(); iterate++)
    {
      iterate->GetLocation(tx,ty);

      if(abs(tx - this->x) < FATNESS && abs(ty - this->y) < FATNESS && false == iterate->Dead())
	{
	  return (DWORD(iterate) - DWORD(e->begin())) / sizeof(CCharacter);
	}
    }

  // return with our results
  return FIRECOLLIDES_NOCOLLISION;
}

CFire::CFire() : state(FIRESTATE_INACTIVE)
{
}

void CFire::PickBestBulletTrailColor()
{
  // picks the best color for bullet trails out of the palette
  CColor256 c(BULLETTRAIL_R,BULLETTRAIL_G,BULLETTRAIL_B);
  c.Certify();
  CFire::bullet_trail_color = c.Color();
}

BYTE CFire::bullet_trail_color;

void CFire::PlaySound()
{
  // play the sound of the weapon firing

  FIXEDNUM hx; // hero coordinates
  FIXEDNUM hy;

  GLUhero.GetLocation(hx,hy);

  GluPlaySound(
	       WAVSET_WEAPONS+this->type,
	       this->x - hx,
	       this->y - hy
	       );
}

