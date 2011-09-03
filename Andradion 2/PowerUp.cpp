// PowerUp.cpp: implementation of the CPowerUp class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Fixed.h"
#include "SharedConstants.h"
#include "Character.h"
#include "Fire.h"
#include "Glue.h"
#include "PowerUp.h"
#include "Net.h"

// Comment the next two lines to allow this module to log
#define WriteLog(x)
#undef LogArg
#define LogArg(x)
#undef TryAndReport
#define TryAndReport(x) x

// functions to return info about the segment and performance of the music
using NGameLib2::Performance;
using NGameLib2::Segment;

// constants used in this module
const DWORD FRAMESTOREGENERATEPOWERUP = 1500;
const DWORD FRAMESTOROTATEPOWERUP = 10;
const FIXEDNUM POWERUP_FATNESS = Fixed(20);

const FIXEDNUM FREQUENCYFACTOR[] = {Fixed(1.1),Fixed(0.9),Fixed(2),Fixed(4)};
const DWORD REVERSEGUNNOISE = (1 << 3);

CPowerUp::CPowerUp(const CPowerUp& r)
{
	CPowerUp::reference_count++;
	this->x = r.x;
	this->y = r.y;
	this->type = r.type;

	if(true == r.is_ammo_set)
	{
		this->is_ammo_set = true;
		this->ammo_contained = new FIXEDNUM[NUM_WEAPONS];
		memcpy((void *)this->ammo_contained,(const void *)r.ammo_contained,sizeof(FIXEDNUM)*NUM_WEAPONS);
	}
	else
	{
		this->is_ammo_set = false;
		this->frames_since_picked_up = r.frames_since_picked_up;
	}
}

CPowerUp& CPowerUp::operator =(const CPowerUp& r)
{
	if(this == &r)
	{
		return *this;
	}

	this->x = r.x;
	this->y = r.y;
	this->type = r.type;
	if(true == r.is_ammo_set)
	{
		if(false == this->is_ammo_set)
		{
			this->is_ammo_set = true;
			this->ammo_contained = new FIXEDNUM[NUM_WEAPONS];
		}		
		memcpy((void *)this->ammo_contained,(const void *)r.ammo_contained,sizeof(FIXEDNUM)*NUM_WEAPONS);
	}
	else
	{
		if(true == this->is_ammo_set)
		{
			delete [] this->ammo_contained;
			this->is_ammo_set = false;
			this->frames_since_picked_up = 0;
		}
		this->frames_since_picked_up = r.frames_since_picked_up;
	}
	return *this;
}

CPowerUp::CPowerUp() : ammo_contained(new FIXEDNUM[NUM_WEAPONS])
{
	CPowerUp::reference_count++;
	this->x = this->y = Fixed(-1); // mark both these as invisible
	this->is_ammo_set = true; // and non-regeneratable
	                          //  until the setup function is called
}

void CPowerUp::Draw(CGraphics& gr)
{
	if(x < 0)
	{
		assert(y < 0);
		return; // no draw
	}

	int bmp;

	switch(this->type)
	{
	case POWERUP_PISTOLAMMO:
		bmp = BMPSET_PISTOL + CPowerUp::rotation_direction;
		break;
	case POWERUP_MACHINEGUNAMMO:
		bmp = BMPSET_MACHINEGUN + CPowerUp::rotation_direction;
		break;
	case POWERUP_BAZOOKAAMMO:
		bmp = BMPSET_BAZOOKA + CPowerUp::rotation_direction;
		break;
	default: //case POWERUP_HEALTHPACK:
		bmp = BMP_HEALTH;
	}

	FIXEDNUM tx = -GLUcenter_screen_x + Fixed(GAME_MODEWIDTH/2) + this->x - Fixed(TILE_WIDTH/2);
	FIXEDNUM ty = -GLUcenter_screen_y + Fixed(GAME_PORTHEIGHT/2) + this->y - Fixed(TILE_HEIGHT/2);

	RECT *target = &gr.TargetScreenArea();

	target->left = FixedCnvFrom<long>(tx);
	target->top = FixedCnvFrom<long>(ty);
	target->right = FixedCnvFrom<long>(tx)+TILE_WIDTH;
	target->bottom = FixedCnvFrom<long>(ty)+TILE_HEIGHT;
	
	// put on the screen
	gr.PutFastClip(*GLUbitmaps[bmp]);
}

void CPowerUp::Setup(FIXEDNUM x_, FIXEDNUM y_, int type_)
{
	assert(x_ >= 0);
	assert(y_ >= 0);
	this->x = x_;
	this->y = y_;
	if(true == this->is_ammo_set)
	{
		// this Powerup used to contain the data for
		//  an ammo set.  We must delete the ammo data
		//  and allocate it for timer use
		delete [] this->ammo_contained;
		this->is_ammo_set = false;
		// allocate memory for the timer we will use
		this->frames_since_picked_up = 0;
	}
	this->type = type_;
}

void CPowerUp::Setup(FIXEDNUM x_,FIXEDNUM y_,const FIXEDNUM *ammo)
{
	assert(x_ >= 0);
	assert(y_ >= 0);
	this->x = x_;
	this->y = y_;
	if(false == this->is_ammo_set)
	{
		// this powerup used to contain the data for a non-ammo set
		//  delete the memory used for the timer and allocate
		//  memory to store the ammo counts
		this->is_ammo_set = true;
		this->ammo_contained = new FIXEDNUM[NUM_WEAPONS];
	}
	this->type = 0; // find the weapon with the most ammo
	for(int i = 0; i < NUM_WEAPONS; i++)
	{
		this->ammo_contained[i] = ammo[i];
		if(this->ammo_contained[i] >= this->ammo_contained[this->type])
		{
			this->type = i;
		}
	}	
}

int CPowerUp::Collides(const CCharacter &ch)
{
	if(x < 0)
	{
		// too soon to do this; we are invisible
		assert(y < 0);
		return POWERUPCOLLIDES_NOTHINGHAPPENED;
	}

	FIXEDNUM tx;
	FIXEDNUM ty;

	ch.GetLocation(tx,ty);
	
	if(
		abs(this->x - tx) < POWERUP_FATNESS &&
		abs(this->y - ty) < POWERUP_FATNESS &&
		false == ch.Dead())
	{
		if(true == this->is_ammo_set)
		{
			// make sure the hero isn't full
			bool full_ammo = true;
			for(int i = 0; i < NUM_WEAPONS; i++)
			{
				if(false == ch.HasFullAmmo(i))
				{
					full_ammo = false;
				}
			}
			if(true == full_ammo)
			{
				return POWERUPCOLLIDES_NOTHINGHAPPENED;
			}
		}
		else if(!(GLUkeyb[DIK_F] & EIGHTHBIT))
		{
			if(POWERUP_HEALTHPACK == this->type)
			{
				if(true == ch.HasFullHealth())
				{
					// tell the player he can pick things
					//  up just for points
					GluPostForcePickupMessage();

					return POWERUPCOLLIDES_NOTHINGHAPPENED;
				}
			}
			else if(true == ch.HasFullAmmo(this->type))
			{
				// tell the player he can pick things
				//  up just for points
				GluPostForcePickupMessage();

				return POWERUPCOLLIDES_NOTHINGHAPPENED;
			}
		}

		GluPlaySound(WAV_GUNNOISE,FREQUENCYFACTOR[this->type],(1 << this->type) & REVERSEGUNNOISE ? true : false);

		this->x *= -1;
		this->y *= -1;
		if(false == this->is_ammo_set)
		{
			this->frames_since_picked_up = 0;
		}

		if(false == NetInGame())
		{
			// add to the score
			GluChangeScore(GluScoreDiffPickup(this->type));
		}

		return (true == this->is_ammo_set)
			? POWERUPCOLLIDES_PICKEDUPANDWILLNOTREGENERATE : POWERUPCOLLIDES_PICKEDUPANDWILLREGENERATE;
	}
	else
	{
		return POWERUPCOLLIDES_NOTHINGHAPPENED;
	}
}

void CPowerUp::Logic()
{
	// this function is used to see if we need to regenerate
	//  if this function was called, we already know we are in an mp game
	if
	(
		false == this->is_ammo_set
		&&
		this->x < 0
		&&
		++this->frames_since_picked_up >	FRAMESTOREGENERATEPOWERUP
	)
	{
		assert(this->y < 0);
		this->x *= -1;
		this->y *= -1;
	}
}

// statics
DWORD CPowerUp::rotation_timer;
int CPowerUp::rotation_direction = DEAST;
HANDLE CPowerUp::beat_event = NULL;
int CPowerUp::reference_count = 0;

void CPowerUp::Rotate()
{
	// make the weapons dance to the music if there is any
	if(NULL != CPowerUp::beat_event)
	{
		if(WAIT_OBJECT_0 == WaitForSingleObject(CPowerUp::beat_event,0))
		{
			if(++CPowerUp::rotation_direction >= RENDERED_DIRECTIONS)
			{
				CPowerUp::rotation_direction = 0;
			}
		}
		else if(NULL == Segment())
		{
			// we don't use a beat event anymore
			CloseHandle((HANDLE)CPowerUp::beat_event);
			CPowerUp::beat_event = NULL;
			Performance()->RemoveNotificationType(GUID_NOTIFICATION_MEASUREANDBEAT);
		}
		return;
	}

	if(++CPowerUp::rotation_timer > FRAMESTOROTATEPOWERUP)
	{
		CPowerUp::rotation_timer = 0;

		if(++CPowerUp::rotation_direction >= RENDERED_DIRECTIONS)
		{
			CPowerUp::rotation_direction = 0;
		}
	}

	if
	(
		NULL != Segment() &&
		SUCCEEDED
		(
			Performance()->AddNotificationType
			(
				GUID_NOTIFICATION_MEASUREANDBEAT
			)
		)
	)
	{
		// create a beat event
		CPowerUp::beat_event = CreateEvent(NULL,FALSE,FALSE,NULL);

		// tell the perf object to signal that event whenever there is
		//  a beat
		Performance()->SetNotificationHandle(CPowerUp::beat_event,0);
	}
}

CPowerUp::~CPowerUp()
{
	if(true == this->is_ammo_set)
	{
		delete [] this->ammo_contained;
	}
	
	if(--CPowerUp::reference_count == 0 && NULL != CPowerUp::beat_event)
	{
		// we are dying out . . .

		// close beat_handle member and set it to null
		CloseHandle((HANDLE)CPowerUp::beat_event);
		CPowerUp::beat_event = NULL;
		
		// remove notification type
		if(NULL != Performance())
		{
			Performance()->RemoveNotificationType(GUID_NOTIFICATION_MEASUREANDBEAT);
		}		
	}
}
