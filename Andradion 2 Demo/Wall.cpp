// wall.cpp module
//  for CWall class, note that this class does not have an active
//  Logic function, but the Draw function is most complicated
//  because it involves calling all the other draw functions of
//  the tokens

#include <vector>
#include "Andradion 2.h"

using namespace KeybLib;
using namespace GraphLib;
using namespace std;

CWall::CWall()
:
IToken(WALL)
{
	// setup some members
	this->state = 0; // not necessary, just to clarify the unnecesarity
}

CWall::~CWall()
{
	// nothing to destroy
}

void CWall::Logic(unsigned char keyb[KEYBOARD_BUFFER_SIZE])
{
	// nothing to do here
}

void CWall::Draw(void)
{
	vector<IToken *> *ts =
		&(CMedia::Media().Tokens());
	IToken *t;
	CTurner *
		// dig into the token array for turner
		turner = (CTurner *)(*ts)[TURNER];

	// target area, calculate now
	int tx = GAME_MODE_WIDTH / 2 - turner->GetX();
	int ty = GAME_MODE_HEIGHT / 2 - turner->GetY();
	int tw = CMedia::Media().WalkingData().numcols()*TILE_WIDTH;
	int th = CMedia::Media().WalkingData().numrows()*TILE_HEIGHT;

	// fill the screen with black
	CColor black(0,0,0,0);

	gr->TargetScreenArea().SetArea(0,0,GAME_MODE_WIDTH,GAME_MODE_HEIGHT);
	gr->TargetScreenArea().Certify();

	black.Certify();

	gr->Rectangle(black);

	// blit lower bitmap
	// set target area to the whole screen
	gr->TargetScreenArea().SetArea(tx,ty,tw,th);

	// assert it for use
	gr->TargetScreenArea().Certify();

	// put the lower bitmap down
	gr->Put(
		CMedia::Media().LowerBitmapOfLevel(),
		false // no transparency
	);

	// put down all the other characters
	// start at two so we skip turner and the wall
	for(int i = 2; i < ts->size(); i++)
	{
		t = (*ts)[i];
		switch(t->GetType())
		{
		case LEVEL_END:
			break; // does not draw itself
		case ENEMY:
			((CEnemy *)t)->Draw();
			break;
		case AMMO:
			((CAmmo *)t)->Draw();
			break;
		case HEALTH_PACK:
			((CHealthPack *)t)->Draw();
			break;
		default:
			assert(false); // an incorrect token type was found
		}
	}

	// blit turner
	turner->Draw();

	// check if we are not inside
	if(WALKABLE_INSIDE != CMedia::Media().WalkingData()[turner->GetY()/TILE_HEIGHT][turner->GetX()/TILE_WIDTH])
	{
		// blit lower bitmap
		// set target area to the whole screen
		gr->TargetScreenArea().SetArea(tx,ty,tw,th);

		// assert it for use
		gr->TargetScreenArea().Certify();

		// blit the upper level bitmap
		gr->Put(
			CMedia::Media().UpperBitmapOfLevel(),
			true // transparency, definitely
		);
	}
}
