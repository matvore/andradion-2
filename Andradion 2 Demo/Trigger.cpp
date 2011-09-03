#include "Andradion 2.h"

// namespaces we use here: graphics and keyboard
using namespace GraphLib;
using namespace KeybLib;
using namespace std;

CTrigger::CTrigger(int weapon_type_) :
weapon_type(weapon_type_),
ammo(START_AMMO[weapon_type_]),
IToken(TRIGGER)
{
	// nothing left to construct, except the state or frames before next fire
	this->state = 0;
}

CTrigger::~CTrigger()
{
	// nothing to destruct
}

void CTrigger::Draw(void)
{
	// trigger drawing is handled by the carrier
	//  so bye.
}

void CTrigger::Logic(unsigned char keyb[KEYBOARD_BUFFER_SIZE])
{
	// just reload the weapon by . . .

	this->state--; // decrementing the frame count before the next fire can take place 

	// never let the count go below zero by using this if test
	if(this->state < 0)
	{
		this->state = 0;
	}
}

// adding and subtracting ammo

void CTrigger::AddAmmo(void)
{
	float addition;

	addition = (1.0f / (float)WEAPON_CAPACITY[this->weapon_type]) * (float)WEAPON_POWERUP[this->weapon_type];

	ammo = __min(1.0f,ammo+addition);

	// play the pickup ammo sound
	CMedia::Media().Sound(SPICKUP)->Play(0,0,0);
}

void CTrigger::Fire(void)
{
	float subtraction;

	subtraction = 1.0f / (float)WEAPON_CAPACITY[this->weapon_type];

	ammo = __max(0.0f,ammo-subtraction);

	// set frames before next fire
	state = FRAMES_TO_RELOAD[weapon_type];

	// play the fire sound
	CMedia::Media().Sound(SPISTOL + this->weapon_type)->Play(0,0,0);
}
