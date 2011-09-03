#include "Andradion 2.h"

using namespace GenericClassLib;
using namespace std;
using namespace KeybLib;
using namespace SoundLib;

CPistolFire::CPistolFire() :
CBullet()
{
	this->state = STATE_NOTFIRED;
}

CPistolFire::~CPistolFire()
{
	// nothing to destruct
}

void CPistolFire::Logic(unsigned char keyb[KEYBOARD_BUFFER_SIZE])
{
	if(STATE_NOTFIRED == this->state)
	{
		return;
	}
	// how far the bullet moves for each cycle of logic
	int x_movement;
	int y_movement;
	
	// this function really has to be fast, so let's extract a pointer
	//  to the token vector
	vector<IToken *>
		* t = &(CMedia::Media().Tokens());

	// need these pointers
	CGuy *target;

	// setup the direction factors

	InterpretDirection(this->direction,x_movement,y_movement);
	
	this->x += x_movement * PROJECTILE_SPEED;
	this->y += y_movement * PROJECTILE_SPEED;

	// we are doing the projectile thing
	//  loop through each token and take action based on
	//  what the token type is

	int collision;
	float damage; // damage done

	collision = this->Collision();

	if(collision >= 0)
	{
		damage = 0.0f;

		// we've hit something
		switch((*t)[collision]->GetType())
		{
		case TURNER:
			damage = BULLET_DAMAGE_TO_TURNER;
		case ENEMY:
			if(0.0f == damage)
			{
				damage = BULLET_DAMAGE_TO_ENEMY;
			}
			target = (CGuy *)((*t)[collision]);
			target->SubtractHealth(damage);
			break;
		default:
			break;
		}

		this->state = STATE_NOTFIRED;
	}
}

void CPistolFire::Draw(void)
{
	if(STATE_NOTFIRED == this->state)
	{
		return;
	}

	CTurner *turner =
		(CTurner *)CMedia::Media().Tokens()[TURNER];

	int tx;
	int ty;

	tx = this->x - turner->GetX() + (GAME_MODE_WIDTH/2) - (TILE_WIDTH/2);
	ty = this->y - turner->GetY() + (GAME_MODE_HEIGHT/2) - (TILE_HEIGHT/2);

	gr->TargetScreenArea().SetArea(
		tx,ty,TILE_WIDTH,TILE_HEIGHT
	);

	gr->TargetScreenArea().Certify();

	// put with transparency
	gr->Put(CMedia::Media().Bitmap(BBULLET),true);
}
