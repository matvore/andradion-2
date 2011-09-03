#include "Andradion 2.h"

// a simple class that provides framework for any moving
//  gun-toter

CGuy::CGuy(int token_type) :
IToken(token_type),
health(1.0f)
{
	this->direction = DSOUTH;
}

CGuy::~CGuy()
{
	// nothing to destruct
}

void CGuy::TryToFire(CTrigger& weapon)
{
	CTurner *turner =
		(CTurner *)CMedia::Media().Tokens()[TURNER];

	int xf,yf;

	if(ENEMY== this->type && STATE_HURT == turner->GetState())
	{
		return; //never fire when turner is already in pain
	}

	if(
		(STATE_WALKING == this->state ||
		STATE_ALIVE == this->state  ||
		STATE_FIRING == this->state)
		&& weapon.CanFire()
	)
	{
		weapon.Fire();

		// create the projectile
		switch(weapon.GetWeaponType())
		{
		case BAZOOKA:
			bf.SetState(STATE_PROJECTILE_READY);

			bf.SetDirection(this->direction);
			bf.SetPosition(this->x,this->y);
			break;
		case MACHINE_GUN:
			mgf.SetState(STATE_FIRED);
			mgf.SetDirection(this->direction);
			mgf.SetPosition(this->x,this->y);
			break;
		case PISTOL:
			pf.SetState(STATE_FIRED);
			pf.SetDirection(this->direction);

			InterpretDirection(this->direction,xf,yf);

			pf.SetPosition(this->x+INITIAL_PROJ_X_MOVEMENT*xf,this->y+INITIAL_PROJ_Y_MOVEMENT*yf);

			break;
		default:
			assert(false); // invalid weapon type found
		}

		// play the appropriate sound
		switch(weapon.GetWeaponType())
		{
		case PISTOL:
			CMedia::Media().Sound(SPISTOL)->Play(0,0,0);
			break;
		case MACHINE_GUN:
			CMedia::Media().Sound(SMACHINEGUN)->Play(0,0,0);
			break;
		case BAZOOKA:
			CMedia::Media().Sound(SBAZOOKA)->Play(0,0,0);
			break;
		default:
			assert(false); // an invalid weapon is being used
		}
	}
}

// this function makes guy move backwards if he's at a wall
void CGuy::ForceOutOfWall(void)
{
	if(
		WALKABLE_WALL == CMedia::Media().WalkingData()[y/TILE_HEIGHT][x/TILE_WIDTH]
	)
	{
		this->x = this->prevx;
		this->y = this->prevy;
	}

	int maxx = CMedia::Media().WalkingData().numcols() * TILE_WIDTH - TILE_WIDTH/2;
	int maxy = CMedia::Media().WalkingData().numrows() * TILE_HEIGHT - TILE_HEIGHT/2;

	int minx = TILE_WIDTH/2;
	int miny = 0;

	if(this->x < minx)
	{
		this->x = minx;
		this->prevx = this->x;
	}
	else if(this->x > maxx)
	{
		this->x = maxx;
		this->prevx = this->x;
	}

	if(this->y < miny)
	{
		this->y = miny;
		this->prevy = this->y;
	}
	else if(this->y > maxy)
	{
		this->y = maxy;
		this->prevy = this->y;
	}
}

// subtract from health
void CGuy::SubtractHealth(float health_)
{

	if(TURNER == this->type && STATE_HURT == this->state)
	{
		return; // never get hurt while we're already hurting
	}

	// make sure we are not already dead
	if(STATE_DEAD == this->state || STATE_DYING == this->state)
	{
		// just return without doing any work
		return;
	}

	int sound_index;

	// do the operation
	health = __max(0.0f,health-health_);
	
	if(0.0f == health) // if we've hit bottom
	{
		this->state = STATE_DYING; // we are dying

		this->frames_in_this_state = FRAMES_TO_DIE;

		// setup the sound index
		if(TURNER == this->type)
		{
			sound_index = STURNERDEATH;
		}
		else
		{
			sound_index =
				SALIENDEATH1 +
				(rand()%NUM_REDUNDANT_SOUNDS);
		}
	}
	else
	{
		this->state = STATE_HURT; // we are hurting

		// all other action needs to be taken according to
		//  token type

		if(TURNER == this->type)
		{
			frames_in_this_state = TURNER_FRAMES_TO_HURT; // start recovering again
		
			// play sound that turner's hurting
			sound_index = 
				STURNERHIT1 +
				(rand()%NUM_REDUNDANT_SOUNDS);
		}
		else
		{
			frames_in_this_state = ENEMY_FRAMES_TO_HURT;

			// play sound that alien's hurting
			sound_index =
				SALIENHIT1 +
				(rand()%NUM_REDUNDANT_SOUNDS);
		}
	}

	// play the sound no matter what it is
	CMedia::Media().Sound(sound_index)->Play(0,0,0);
}

void CGuy::GuyLogic()
{
	bf.Logic(NULL);
	mgf.Logic(NULL);
	pf.Logic(NULL);
}
