// pretty simple machine gun firing function

#include <cassert>
#include <KeybLib.h>
#include <GraphLib.h>
#include "classes.h"

using namespace std;
using namespace KeybLib;
using namespace GraphLib;

CMachineGunFire::CMachineGunFire() :
CBullet()
{
	this->state = STATE_NOTFIRED;
	
	// nothing to construct
}

CMachineGunFire::~CMachineGunFire() 
{
	// nothing to deconstruct
}

void CMachineGunFire::Draw()
{
	CTurner *turner = // get a pointer to turner
		(CTurner *)CMedia::Media().Tokens()[TURNER];

	// the possible pen styles
	const int NUM_PEN_STYLES = 6;
	const int PEN_STYLES[NUM_PEN_STYLES] =
	{
		PS_SOLID,
		PS_DOT,
		PS_DASH,
		PS_DASHDOT,
		PS_DASHDOTDOT,
		PS_NULL
	};

	const int MGFLW = 1;
	const COLORREF MGFLCOLORREF = RGB(255,255,0);

	if(STATE_NOTFIRED == this->state)
	{
		return;
	}

	HDC dc;
	
	gr->Buffer(1)->GetDC(&dc);

	HPEN yellow = CreatePen(rand()%NUM_PEN_STYLES,MGFLW,MGFLCOLORREF);

	HPEN oldpen = (HPEN)SelectObject(dc,yellow);

	SetBkMode(dc,TRANSPARENT);

	int sx = this->lb.GetX() - turner->GetX() + (GAME_MODE_WIDTH>>1);
	int sy = this->lb.GetY() - turner->GetY() + (GAME_MODE_HEIGHT >> 1);

	int tx = this->x - turner->GetX() + (GAME_MODE_WIDTH >> 1);
	int ty = this->y - turner->GetY() + (GAME_MODE_HEIGHT >> 1);

	int swaying = (rand()%MACHINE_GUN_SWAY)-MACHINE_GUN_SWAY/2;

	if((const int)tx == sx)
	{
		tx += swaying;
	}
	else if((const int)ty == sy)
	{
		ty += swaying;
	}
	else
	{
		tx += swaying;
		ty += (rand()%MACHINE_GUN_SWAY)-MACHINE_GUN_SWAY/2;
	}

	// draw the line
	MoveToEx(dc,sx,sy,NULL);

	LineTo(dc,tx,ty);

	// select the old pen
	DeleteObject(SelectObject(dc,oldpen));

	gr->Buffer(1)->ReleaseDC(dc);

	this->state= STATE_NOTFIRED;
}

void CMachineGunFire::Logic(unsigned char keyb[KEYBOARD_BUFFER_SIZE])
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

	// move the projectile one time before we register collisions so we don't hit ourselves
	this->x += INITIAL_PROJ_X_MOVEMENT * x_movement;
	this->y += INITIAL_PROJ_Y_MOVEMENT * y_movement;

	x_movement *= PROJECTILE_SPEED;
	y_movement *= PROJECTILE_SPEED;

	// record current x and y
	this->lb.SetPosition(this->x,this->y);

	// we are doing the projectile thing
	//  loop through each token and take action based on
	//  what the token type is

	for
	(int i = 0;
	i < NUM_PROJECTILE_LOGIC_LOOPS;
	i++, this->x += x_movement, this->y += y_movement)
	{
		int collision;
		float damage; // damage done

		damage = 0.0f;

		collision = this->Collision();

		if(collision >= 0)
		{
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

			// skip to the end of the outtermost for loop
			i = NUM_PROJECTILE_LOGIC_LOOPS;
		}
	}
}
