#include <string>
#include "Andradion 2.h"

// need a few namespaces
using namespace std;
using namespace SoundLib;
using namespace GraphLib;
using namespace KeybLib;
using namespace MusicLib;

// turner.cpp, module for turner class

CTurner::CTurner() :
current_weapon(PISTOL), // default weapon is pistol
CGuy(TURNER) // set the type of token to TURNER
// the constructor for turner
{
	Init_Error_Ready_System();

	// now to create the weapons array
	weapons = new CTrigger *[NUM_WEAPONS];

	// check for failure
	if(NULL == weapons)
	{
		Return_With_Error(1);
	}
	
	// set up the weapons individually
	weapons[PISTOL] = new CTrigger(PISTOL);
	weapons[MACHINE_GUN] = new CTrigger(MACHINE_GUN);
	weapons[BAZOOKA] = new CTrigger(BAZOOKA);

	// check for failure
	if(
		NULL == weapons[PISTOL] ||
		NULL == weapons[MACHINE_GUN] ||
		NULL == weapons[BAZOOKA]
	)
	{
		Return_With_Error(1);
	}

	this->state = STATE_ALIVE;
	this->frames_in_this_state = FRAMES_PER_STEP;

	// that's all we needed to do
	//  the current position will be set by the creator

	Begin_Error_Table_Return_Nothing
		Define_Error(1,LEVEL_LOAD_ERROR)
	End_Error_Table_Exit_With_Error(NULL);
}

CTurner::~CTurner() // our destructor, of course
{
	int i; // looper

	// all we have to do is delete the weapons
	for(i = 0; i < NUM_WEAPONS; i++)
	{
		delete weapons[i];
		weapons[i] = NULL;
	}

	delete [] weapons;
	
	// get rid of the projectile data
}

// add to health
void CTurner::AddHealth(void)
{
	 assert(0.0f != this->health);
	 
	 health = __min(1.0f,this->health+HEALTH_INCREMENT);

	 if(STATE_HURT == this->state)
	 {
		 this->state = STATE_ALIVE; // recover automatically
	 }

	 // play the pickup sound, because we've picked up health
	 CMedia::Media().Sound(SPICKUP)->Play(0,0,0);
}

void CTurner::SetCurrentWeapon(int current_weapon_)
{
	// make sure the parameter is valid
	assert(current_weapon_ < NUM_WEAPONS);
	assert(current_weapon_ >= 0);

	// if they are the same anyway
	if(current_weapon == current_weapon_)
	{
		return; // just leave now
	}

	if(STATE_DEAD == this->state)
	{
		// just leave now in this case, too
		return;
	}
	
	// change the value of current_weapon
	current_weapon = current_weapon_;
	
	// play the weapon-switching noise
	CMedia::Media().Sound(SSWITCHWEAPONS)->Play(0,0,0);
}

void CTurner::Logic(unsigned char keyb[KeybLib::KEYBOARD_BUFFER_SIZE])
{
	// STEP 1: CHECK FOR INACTIVE STATES //

	if(STATE_DEAD == this->state || STATE_WON == this->state)
	{
		// when turner's dead or has won, can't do anything

		return;
	}

	// STEP 2: UPDATE CURRENT MUSIC or PLAY MUSIC FOR FIRST TIME //

	string new_music;

	if(
		WALKABLE_INSIDE
		==
		CMedia::Media().WalkingData()[this->y/TILE_HEIGHT][this->x/TILE_WIDTH]
	)
	{
		if(
			(this->prevx < 0 && this->prevy < 0) ||
			WALKABLE_OUTSIDE
			==
			CMedia::Media().WalkingData()[this->prevy/TILE_HEIGHT][this->prevx/TILE_WIDTH]
		)
		{
			// play different music
			new_music = MUSIC_PATH;
			new_music += CMedia::Media().IndoorMusic();

			// combined the music path and file name to create
			//  the full path

			// now load it
			MusicLib::Play(new_music.c_str());
		}
		else
		{
			MusicLib::Play(NULL); // continue music
		}
	}
	else // we are walking outside
	{
		// check if we were previously inside
		if(
			(this->prevx < 0 && this->prevy < 0) ||
			WALKABLE_INSIDE
			==
			CMedia::Media().WalkingData()[this->prevy/TILE_HEIGHT][this->prevx/TILE_WIDTH]
		)
		{
			// assemble the path of the music file
			new_music = MUSIC_PATH;
			new_music += CMedia::Media().OutdoorMusic();
				// now load it
			MusicLib::Play(new_music.c_str());
		}
		else
		{
			MusicLib::Play(NULL); // continue music
		}
	}

	// STEP 3: APPLY TIMING LOGIC TO THE STATE //

	if(STATE_DEAD != this->state) // we may have to change our state
	{
		this->frames_in_this_state--;

		if(0 >= this->frames_in_this_state) // maybe it's time we change
		{
			// now let's see what to change to
			if(this->state == STATE_ALIVE)
			{
				this->state = STATE_WALKING;
				CMedia::Media().Sound(STURNERSTEP1)->Play(0,0,0);
				frames_in_this_state = FRAMES_PER_STEP;
			}
			else if (this->state == STATE_WALKING)
			{
				this->state = STATE_ALIVE;
				CMedia::Media().Sound(STURNERSTEP2)->Play(0,0,0);
				frames_in_this_state = FRAMES_PER_STEP;
			}
			else if (this->state == STATE_HURT)
			{
				this->state = STATE_ALIVE;
				frames_in_this_state  = FRAMES_PER_STEP;
			}
			else if (this->state == STATE_DYING)
			{
				this->state = STATE_DEAD;
				// keeps frames in this state at zero
				this->frames_in_this_state = 0;
			}
		}
	}

	// STEP 4: CHECK FOR SPACEBAR BUTTON DOWN //

	if(keyb[DIK_SPACE] & HIGHEST_BIT8)
	{
		// if its down, give the responsibility of its firing to our derived function
		this->TryToFire(*(this->weapons[this->current_weapon]));
	}

	// STEP 5: MOVE TURNER ACCORDING TO DIRECTIONAL BUTTONS //

	int x_mov; // pixels in x direction to move
	int y_mov; // pixels to move in y direction

	if(keyb[DIK_UP] & HIGHEST_BIT8)
	{
		y_mov = (-1);
		if(keyb[DIK_LEFT] & HIGHEST_BIT8)
		{
			this->direction = DNW;
			x_mov = (-1);
		}
		else if(keyb[DIK_RIGHT] & HIGHEST_BIT8)
		{
			this->direction = DNE;
			x_mov = 1;
		}
		else
		{
			this->direction = DNORTH;
			x_mov = 0;
		}
	}
	else if (keyb[DIK_DOWN] & HIGHEST_BIT8)
	{
		y_mov = 1;
		if(keyb[DIK_LEFT] & HIGHEST_BIT8)
		{
			this->direction = DSW;
			x_mov = (-1);
		}
		else if(keyb[DIK_RIGHT] & HIGHEST_BIT8)
		{
			this->direction = DSE;
			x_mov = 1;
		}
		else
		{
			this->direction = DSOUTH;
			x_mov = 0;
		}
	}
	else if(keyb[DIK_LEFT] & HIGHEST_BIT8)
	{
		this->direction = DWEST;
		x_mov = (-1);
		y_mov = 0;
	}
	else if(keyb[DIK_RIGHT] & HIGHEST_BIT8)
	{
		this->direction = DEAST;
		x_mov = 1;
		y_mov = 0;
	}
	else
	{
		x_mov = 0;
		y_mov = 0;
		
		if(STATE_ALIVE == this->state)
		{
			this->frames_in_this_state = FRAMES_PER_STEP;
		}
	}

	if(this->state == STATE_WALKING)
	{
		x_mov *= TURNER_SPEED_2;
		y_mov *= TURNER_SPEED_2;
	}
	else
	{
		x_mov *= TURNER_SPEED_1;
		y_mov *= TURNER_SPEED_1;
	}

	if(STATE_DYING != this->state && STATE_DEAD != this->state)
	{
		// move turner
		this->Move(x_mov,y_mov);
		if(0 != x_mov || 0 != y_mov)
		{
			this->ForceOutOfWall();
		}
	}

	// STEP 6: CHANGE WEAPONS IF APPROPRIATE //

	if(keyb[DIK_1] & HIGHEST_BIT8)
	{
		SetCurrentWeapon(PISTOL);
	}
	else if(keyb[DIK_2] & HIGHEST_BIT8)
	{
		SetCurrentWeapon(MACHINE_GUN);
	}
	else if(keyb[DIK_3] & HIGHEST_BIT8)
	{
		SetCurrentWeapon(BAZOOKA);
	}

	// STEP 7: CHECK FOR COLLISION WITH ANY KINDS OF OBJECTS //

	CAmmo *
		ammo;

	CHealthPack *
		hp;

	CLevelEnd *
		lend;

	// make a shortcut pointer into the tokens
	vector<IToken *> *tokens = &(CMedia::Media().Tokens());

	for(int i = WALL+1; i < tokens->size(); i++)
	{
		IToken *y = (*tokens)[i];

		switch(y->GetType())
		{
		case AMMO:
			ammo = (CAmmo *)y;
			
			if(STATE_GOTTEN == ammo->GetState() || 1.0f == this->weapons[ammo->GetAmmoType()]->GetAmmo())
			{
				break;
			}

			if(
				abs(this->x - ammo->GetX()) < OBJECT_FATNESS
				&&
				abs(this->y - ammo->GetY()) < OBJECT_FATNESS
			)
			{
				// do the thing
				this->weapons[ammo->GetAmmoType()]->AddAmmo();
				ammo->SetState(STATE_GOTTEN);
			}
			break;
		case HEALTH_PACK:
			hp = (CHealthPack *)y;

			// leave if the health pack is already picked up or we have full health
			if(STATE_GOTTEN == hp->GetState() || this->health == 1.0f)
			{
				break;
			}
			
			// check for contact
			if(
				abs(this->x - hp->GetX()) < OBJECT_FATNESS
				&&
				abs(this->y - hp->GetY()) < OBJECT_FATNESS
				)
			{
				// do the thing
				this->AddHealth();
				hp->SetState(STATE_GOTTEN);
			}
			break;
		case LEVEL_END:
			lend = (CLevelEnd *)y;

			// check for contact
			if(
				abs(this->x - lend->GetX()) < OBJECT_FATNESS
				&&
				abs(this->y - lend->GetY()) < OBJECT_FATNESS
			)
			{
				// we may have won
				if(NUM_LEVELS == lend->GetState())
				{
					this->state = STATE_WON;
					break;;
				}

				// do the thing
			
				CMedia::Media().LoadLevel(lend->GetState());
			}
			break;
		default:
			break;
		}
	}
}

void CTurner::Draw(void)
{
	int turner_frame_to_draw;
	int weapon_frame_to_draw;
	int direction_to_render;

	CColor meters;

	// our very first concern is drawing the health and weapon meters
	//  this is always done no matter what state we are in

	// set target screen area for the health meter
	gr->TargetScreenArea().SetArea(
		HEALTH_METER_X,METER_Y,METER_W,METER_H
	);

	// we need to be able to use this target area
	gr->TargetScreenArea().Certify();

	// set the color to black (the background of the meter)
	meters.SetColor(0,0,0);
	meters.Certify();

	// draw the rectangle while passing the color we want
	//  (this is the health meter)
	gr->Rectangle(meters);

	// now draw the measuring rectangle, which is variable
	
	// make the color unique
	meters.SetColor(HEALTH_METER_R,HEALTH_METER_G,HEALTH_METER_B);
	meters.Certify();

	// shrink the bar vertically
	gr->TargetScreenArea().SetHeight(
		gr->TargetScreenArea().GetHeight()-2
	);
	gr->TargetScreenArea().SetTop(
		gr->TargetScreenArea().GetTop()+1
	);

	int colored_width;

	colored_width = int(float(METER_W) * this->health);

	if(colored_width > 0)
	{
		gr->TargetScreenArea().SetWidth(
			colored_width
		);

		gr->TargetScreenArea().Certify();

		// draw our cool rectangle
		gr->Rectangle(meters);
	}

	// now the ammo meter

	// set the area and certify it for the ammo meter
	gr->TargetScreenArea().SetArea(
		AMMO_METER_X,
		METER_Y,
		METER_W,
		METER_H
	);

	// set the color
	meters.SetColor(0,0,0);
	meters.Certify();

	gr->TargetScreenArea().Certify();

	// draw the rectangle outline of the ammo meter
	gr->Rectangle(meters);

	// make the color unique
	meters.SetColor(AMMO_METER_R,AMMO_METER_G,AMMO_METER_B);
	meters.Certify();

	// shrink the bar vertically
	gr->TargetScreenArea().SetHeight(
		gr->TargetScreenArea().GetHeight()-2
	);
	gr->TargetScreenArea().SetTop(
		gr->TargetScreenArea().GetTop()+1
	);

	colored_width = int(float(METER_W) * this->weapons[current_weapon]->GetAmmo());

	if(colored_width > 0)
	{
		gr->TargetScreenArea().SetWidth(
			colored_width
		);

		gr->TargetScreenArea().Certify();

		// draw our cool rectangle
		gr->Rectangle(meters);
	}

	// select correct direction to render
	//  cull out extra direction data (DNE = DNORTH or DEAST, but not DNE)
	direction_to_render =
		(this->direction >= RENDERED_DIRECTIONS) ?
		(this->direction - RENDERED_DIRECTIONS) : (this->direction);

	// select correct foot position, or draw blood
	if(STATE_DYING == this->state)
	{
		turner_frame_to_draw = BDECAPITATE1 + (rand()&1);
	}
	else
	{
		turner_frame_to_draw = (STATE_WALKING == state ? (B2TURNER) : (B1TURNER)) + direction_to_render;
	}

	weapon_frame_to_draw =
		(this->current_weapon * RENDERED_DIRECTIONS + BPISTOL)
		+
		direction_to_render
	;

	// set the target rectangle to the center of the screen

	gr->TargetScreenArea().SetArea(
		(GAME_MODE_WIDTH >> 1) - (TILE_WIDTH >> 1),
		(GAME_MODE_HEIGHT >> 1) - (TILE_HEIGHT >> 1),
		TILE_WIDTH,
		TILE_HEIGHT
	);

	// we need to be able to use it, so it needs to be certified
	gr->TargetScreenArea().Certify();

	if(STATE_DEAD == this->state)
	{
		// draw blood stain
		turner_frame_to_draw = BBLOODSTAIN;
	}

	if((DNORTH == direction_to_render || DWEST == direction_to_render) && STATE_DEAD != this->state)
	{
		// if we face west, we need to draw the weapon first
		//  so switch the frame values, using direction_to_render as a temp space
		direction_to_render = turner_frame_to_draw;
		turner_frame_to_draw = weapon_frame_to_draw;
		weapon_frame_to_draw = direction_to_render;
	}

	// draw the turner frame and then the weapon frame, both with transparency

	gr->Put(CMedia::Media().Bitmap(turner_frame_to_draw),true);
	gr->Put(CMedia::Media().Bitmap(weapon_frame_to_draw),true);

	// turner looses blood when hurt
	if(STATE_HURT == this->state)
	{
		gr->Put(CMedia::Media().Bitmap(BDECAPITATE1+(rand()&1)),true);
	}

	bf.Draw(); // must draw bazooka firing too
	pf.Draw();
	mgf.Draw();
}
