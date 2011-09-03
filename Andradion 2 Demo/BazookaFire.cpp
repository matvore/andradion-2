#include "Andradion 2.h"

using namespace GenericClassLib;
using namespace std;
using namespace KeybLib;
using namespace SoundLib;

CBazookaFire::CBazookaFire() :
IToken(BAZOOKA_FIRE)
{
	// setup the state
	this->state = STATE_INACTIVE;
}

CBazookaFire::~CBazookaFire()
{
	// nothing to destruct
}

bool CBazookaFire::Collision(int token_index)
{
	// find the token array
	vector<IToken *>
		* t = &(CMedia::Media().Tokens());

	CGuy *target;

	// get the fatness factor to use based on
	//  whether or not we are in an explosion state
	int fatness = (this->state > 0) ? (FATNESS << 1) : (FATNESS);

	switch((*t)[token_index]->GetType())
	{
	case TURNER:
	case ENEMY:
		target = (CGuy *)((*t)[token_index]);

		if(
			
		abs(
		this->x - target->GetX()
		) < fatness
			
		&&
			
		abs(
		this->y - target->GetY()
		) < fatness

		&& STATE_DEAD != target->GetState()
		&& STATE_DYING != target->GetState()
		&& STATE_WON != target->GetState()
			
		)
		{
			return true;
		}
		else
		{
			return false;
		}
	case WALL:
		if(
			(this->y < 0 || this->x < 0 ||
			this->x >
				CMedia::Media().WalkingData().numcols() * TILE_WIDTH - TILE_WIDTH/2
				||
			this->y > 
				CMedia::Media().WalkingData().numrows() * TILE_HEIGHT - TILE_HEIGHT / 2
				||
			WALKABLE_WALL ==
			CMedia::Media().WalkingData()[this->y/TILE_HEIGHT][this->x/TILE_WIDTH])
		)
		{
			return true;
		}
		else
		{
			return false;
		}
	default:
		return false;
	}
}

void CBazookaFire::Logic(unsigned char keyb[KEYBOARD_BUFFER_SIZE])
{
	if(STATE_INACTIVE == this->state)
	{
		// if we are inactive, then just leave
		return;
	}

	// how far the bullet moves for each cycle of logic
	int x_movement;
	int y_movement;

	int i;
	int j;
	
	// this function really has to be fast, so let's extract a pointer
	//  to the token vector
	vector<IToken *>
		* t = &(CMedia::Media().Tokens());

	// need this pointer
	CGuy *guy;

	if(0 <= this->state)
	{
		this->state--;

		for(i = 0; i < t->size(); i++)
		{
			IToken *token;

			if(false == this->Collision(i))
			{
				continue;
			}

			// find the target
			token = (*t)[i];
			
			if(WALL == token->GetType())
			{
				continue;
			}

			guy = (CGuy *)((*t)[i]);

			if(TURNER == guy->GetType())
			{
				guy->SubtractHealth(BAZOOKA_DAMAGE_TO_TURNER);
			}
			else if(ENEMY == guy->GetType())
			{
				guy->SubtractHealth(1.0f);
			}
			else
			{
				assert(false);
			}
		}
		
		return; // all done!
	}
	
	InterpretDirection(this->direction,x_movement,y_movement);

	// move the projectile once before we register collision
	//  so we don't hit ourselves
	this->x += INITIAL_PROJ_X_MOVEMENT * x_movement;
	this->y += INITIAL_PROJ_Y_MOVEMENT * y_movement;

	x_movement *= PROJECTILE_SPEED;
	y_movement *= PROJECTILE_SPEED;

	// we are doing the projectile thing
	//  loop through each token and take action based on
	//  what the token type is

	for
	(i = 0;
	i < NUM_PROJECTILE_LOGIC_LOOPS;
	i++, this->x += x_movement, this->y += y_movement)
	{
		bool collided = false;

		for(j = 0; j < t->size(); j++)
		{
			if(true == this->Collision(j))
			{
				collided= true;
				break;
			}
		}

		if(true == collided|| NUM_PROJECTILE_LOGIC_LOOPS - 1 == i)
		{
			// we need to start an explosion

			// set our state
			this->state = FRAMES_FOR_EXPLOSION;

			// skip to the end of the for loop
			break;
		}
	}
}

void CBazookaFire::Draw(void)
{
	int bulge;

	int tx; // where to put
	int ty;

	// offset memory to turner
	CTurner
		* turner = (CTurner *)CMedia::Media().Tokens()[0];

	// bazooka bullets are not drawn
	if(this->state > 0) 
	{
		// we are doing an explosion
		//  setup the bulge variable
		bulge = rand()%MAX_EXPLOSION_BULGE;

		tx = this->x - ((bulge+TILE_WIDTH) >> 1);
		ty = this->y - ((bulge+TILE_HEIGHT) >> 1);

		// translate to screen coordinates

		tx -= turner->GetX();
		tx += (GAME_MODE_WIDTH>>1);
		ty -= turner->GetY();
		ty += (GAME_MODE_HEIGHT>>1);

		// setup the area with a few complicated formulas
		gr->TargetScreenArea().SetArea(tx,ty,TILE_WIDTH+bulge,TILE_HEIGHT+bulge);

		// must be certified to be used
		gr->TargetScreenArea().Certify();

		// put with transparency
		gr->Put(CMedia::Media().Bitmap(BEXPLOSION),true);
	}
}
