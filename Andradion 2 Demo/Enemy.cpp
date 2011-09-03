#include "Andradion 2.h"

// namespaces used
using namespace GraphLib;
using namespace std;
using namespace KeybLib;
using namespace SoundLib;

CEnemy::CEnemy(int model_) :
model(model_),
CGuy(ENEMY)
{
	Init_Error_Ready_System();

	// construct the enemy
	//  the model is constant
	
	// we should create the weapon in the body
	//  of the function
	weapon = new CTrigger(model);

	// the model values are parallel with their
	//  respective weapon types

	// check if the weapon could not be allocated
	if(NULL == weapon)
	{
		Return_With_Error(1);
	}

	// set the state to what the enemy's should be for now
	this->state = STATE_UNKNOWING;
	this->frames_in_this_state = 0;

	Begin_Error_Table_Return_Nothing
		Define_Error(1,OUT_OF_MEMORY);
	End_Error_Table_Exit_With_Error(NULL);
}

CEnemy::~CEnemy()
{
	// typical destructor

	delete weapon;
}

void CEnemy::Logic(unsigned char keyb[KEYBOARD_BUFFER_SIZE])
{
	int pttx; // ptt = proximity to turner
	int ptty;
	int ptt;
	int xf,yf;

	// perform a frame of logic (input being ignored)
	if(0 != this->frames_in_this_state)
	{
		this->frames_in_this_state--;
		if(this->frames_in_this_state < 0)
		{
			this->frames_in_this_state = 0;
		}

		// now maybe change our state
		if(0 == this->frames_in_this_state)
		{
			switch(this->state)
			{
			case STATE_ALIVE:
			case STATE_FIRING:
			case STATE_HURT:
				this->state = STATE_WALKING;
				this->frames_in_this_state = FRAMES_PER_STEP;
				break;
			case STATE_WALKING:
				this->state = STATE_ALIVE;
				this->frames_in_this_state = FRAMES_PER_STEP;
				break;
			case STATE_DYING:
				this->state = STATE_DEAD;
				break;
			default:
				assert(false); // an invalid state is used in CEnemy
			}
		}
	}

	switch(this->state)
	{
	// this switch will handle all possible states
	case STATE_HURT:
	case STATE_DYING:
	case STATE_DEAD:  // no movement here
		break;
	case STATE_ALIVE:
	case STATE_WALKING: // two "walking" states : STATE_ALIVE, STATE_WALKING

		// take some steps by figuring out the best direction to walk in
		//  see if walking horizontally in order to vertically align with turner
		//  would end us up in a wall.  If it would, then walk vertically

		if(
			WALKABLE_WALL !=
			CMedia::Media().WalkingData()[this->y/TILE_HEIGHT][((CTurner *)CMedia::Media().Tokens()[0])->GetX()/TILE_WIDTH] 
		) // walking horizontally
		{
			if(
				((CTurner *)CMedia::Media().Tokens()[0])->GetX()
				>
				this->x
			)
			{
				this->direction = DEAST;
			}
			else
			{
				this->direction = DWEST;
			}
		}
		else // walking vertically
		{
			if(
				((CTurner *)CMedia::Media().Tokens()[0])->GetY()
				>
				this->y
			)
			{
				this->direction = DSOUTH;
			}
			else
			{
				this->direction = DNORTH;
			}
		}

		int es; // enemy speed

		if(STATE_WALKING == this->state)
		{
			es = ENEMY_SPEED_2;
		}
		else
		{
			es = ENEMY_SPEED_1;
		}

		// now move based on our direction
		InterpretDirection(this->direction,xf,yf);

		this->Move(xf * es,yf*es);

		// check for wall
		this->ForceOutOfWall();

		// now if we are close enough, start firing
		if
		(abs(
			this->x - ((CTurner *)CMedia::Media().Tokens()[0])->GetX()
		) < (FATNESS >> 1))
		{
			// choose direction
			if(
				this->y < ((CTurner *)CMedia::Media().Tokens()[0])->GetY()
			)
			{
				this->direction = DSOUTH;
			}
			else
			{
				this->direction = DNORTH;
			}
		}
		else if
		(abs(
		this->y - ((CTurner *)CMedia::Media().Tokens()[0])->GetY()
		) < (FATNESS >> 1))
		{
			// choose direction
			if(
				this->x < ((CTurner *)CMedia::Media().Tokens()[0])->GetX()
			)
			{
				this->direction = DEAST;
			}
			else
			{
				this->direction = DWEST;
			}
		}
		else
		{
			// break so we don't set the state and fits var
			//  to firing ones!
			break;
			// don't forget that break does not exit the
			//  if blocks
		}

		this->state = STATE_FIRING;
		this->frames_in_this_state = FRAMES_TO_FIRE;

		break;
	case STATE_UNKNOWING: // only for aliens when they have no idea turner's here
		// check distance
		pttx = ((CTurner *)CMedia::Media().Tokens()[0])->GetX();
		pttx -= this->x;
		pttx = abs(pttx);

		ptty = ((CTurner *)CMedia::Media().Tokens()[0])->GetY();
		ptty -= this->y;
		ptty = abs(ptty);

		ptt = pttx * pttx + ptty * ptty;

		// complete our pythag theorem to get the distance
		//  between turner and the alien
		ptt = int(sqrt((double)ptt));

		// if we are close enough, then make the alien aware
		if(ptt < MAX_SAFE_PROX)
		{
			this->state = STATE_ALIVE;
			this->frames_in_this_state = FRAMES_PER_STEP;
		}

		break;
	case STATE_FIRING: // used only with aliens
		this->TryToFire(*(this->weapon));
		break;
	default:
		assert(false); // an invalid state is used in CEnemy
	}

}

void CEnemy::Draw(void)
{
	// draw the bazooka fire
	bf.Draw();
	mgf.Draw();
	pf.Draw();

	int enemy_frame_to_draw;
	int direction_to_render;

	int target_x;
	int target_y;

	// create a shortcut pointer to turner
	CTurner *turner = (CTurner *)CMedia::Media().Tokens()[0];

	// select correct direction to render
	//  cull out extra direction data (DNE = DNORTH or DEAST, but not DNE)
	direction_to_render =
		(this->direction >= RENDERED_DIRECTIONS) ?
		(this->direction - RENDERED_DIRECTIONS) : (this->direction);

	// select correct foot position, or draw blood
	if(STATE_DYING == this->state)
	{
		enemy_frame_to_draw = BDECAPITATE1 + (rand()&1);
	}
	else
	{
		enemy_frame_to_draw = 
			((STATE_WALKING == state) ?
			(B1DON) : (B2DON))
			+
			direction_to_render
		;

		// add more based on what model to draw
		enemy_frame_to_draw +=
			RENDERED_DIRECTIONS * this->model;
	}

	target_x = this->x
	// this is a formula for translating a coor
	//  to screen coor, and is used for both x
	//  position and y position
		- turner->GetX() + (GAME_MODE_WIDTH>>1);

	target_y = this->y

		- turner->GetY() + (GAME_MODE_HEIGHT>>1);

	// translate again so the coordinates specify the
	//  center
	target_x -= TILE_WIDTH >> 1;
	target_y -= TILE_HEIGHT >> 1;

	gr->TargetScreenArea().SetArea(target_x,target_y,TILE_WIDTH,TILE_HEIGHT);
	gr->TargetScreenArea().Certify();

	if(STATE_DEAD == this->state)
	{
		enemy_frame_to_draw = BBLOODSTAIN;
	}

	// draw the enemy sprite

	gr->Put(CMedia::Media().Bitmap(enemy_frame_to_draw),true);

	// sprite looses blood when hurt
	if(STATE_HURT == this->state)
	{
		// 50/50 chance of drawing blood
		if(0 != (rand()&1))
		{
			gr->Put(CMedia::Media().Bitmap(BDECAPITATE1 + (rand()&1)),true);
		}
	}
}
