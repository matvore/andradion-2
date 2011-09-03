#include "Andradion 2.h"

// namespaces we use here:  Graphics and keyboard
using namespace GraphLib;
using namespace KeybLib;

CAmmo::CAmmo(int weapon_type_) : 
weapon_type(weapon_type_), IToken(AMMO) // set the constant
{
	// just set the state
	this->state = STATE_UNGOTTEN;
}

CAmmo::~CAmmo()
{
	// nothing for destruction
}

void CAmmo::Draw(void)
{
	int target_x;
	int target_y;

	// if we are already gotten, then we aren't going to draw
	if(STATE_GOTTEN == this->state)
	{
		return;
	}

	// a pointer to the bitmap struct will be kept
	//  so a long complicated Put instruction is not
	//  necessary
	CColorMap*
		
	// figure out the index of this bitmap to CMedia
	weapon = // (CColorMap *)
		&CMedia::Media().Bitmap(
			BPISTOL + weapon_type * RENDERED_DIRECTIONS
		);
		

	// a pointer to the turner struct will be kept
	//  so we don't have to keep digging into the
	//  CMedia singleton
	CTurner* 

	// offset these coordinates to local screen
	//  coordinates by finding turner's position
	turner = (CTurner *)
		CMedia::Media().Tokens()[TURNER];

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

	// set the area with the recently calculated
	//  coordinates
	gr->TargetScreenArea().SetArea(
		target_x,
		target_y,
		TILE_WIDTH,
		TILE_HEIGHT
	);

	// certify the area so it can be used by CGraphics
	gr->TargetScreenArea().Certify();

	// now put the bitmap for ammo onto the screen
	//  with the specified screen area with trans-
	//  parency
	gr->Put(*weapon,true);
}

void CAmmo::Logic(unsigned char keyb[KEYBOARD_BUFFER_SIZE])
{
	// Ammo does not perform logic
}
