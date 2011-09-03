#include "Andradion 2.h"

using namespace GenericClassLib;
using namespace std;
using namespace KeybLib;
using namespace SoundLib;

CBullet::CBullet() :
IToken(BULLET)
{
	// nothing to construct
}

CBullet::~CBullet()
{
	// nothing to destruct
}

int CBullet::Collision(void)
{
	// find the token array
	vector<IToken *>
		* t = &(CMedia::Media().Tokens());

	CGuy
		* target;

	for(int j = 0; j < t->size(); j++)
	{	
		// figure the type
		switch((*t)[j]->GetType())
		{
		case TURNER:
		case ENEMY:
			target = (CGuy *)((*t)[j]);

			if(
			
			abs(
			this->x - target->GetX()
			) <	FATNESS
			
			&&
			
			abs(
			this->y - target->GetY()
			) < FATNESS

			&& STATE_DEAD != target->GetState()
			
			)
			{
				return j;
			}
			break;
		case WALL:
			if(
				this->y < 0 || this->x < 0 ||
				this->x >
					CMedia::Media().WalkingData().numcols() * TILE_WIDTH - TILE_WIDTH/2
					||
				this->y > 
					CMedia::Media().WalkingData().numrows() * TILE_HEIGHT - TILE_HEIGHT / 2
					||
				WALKABLE_WALL ==
				CMedia::Media().WalkingData()[this->y/TILE_HEIGHT][this->x/TILE_WIDTH]
			)
			{
				return j;
			}
			break;
		default:
			break;
		}
	}

	return -1; // we couldn't collide
}
