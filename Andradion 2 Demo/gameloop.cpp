#include </gamelib/timer.h>
#include "Andradion 2.h"

using namespace GenericClassLib;
using namespace std;
using namespace KeybLib;

bool gameloop(bool stop) {
	// the order of the three basic operations:
	//  logic
	//  draw

	CTurner *turner;
	CEnemy *enemy;

	int i; // some loop variables we'll need

	static CTimer timer;

	unsigned char keyb[KeybLib::KEYBOARD_BUFFER_SIZE];

	vector<IToken *> 
		*tokens; // keep a pointer to the array of tokens

	tokens = &(CMedia::Media().Tokens());

	if(STATE_WON == (*tokens)[TURNER]->GetState())
	{
		// pass control to the endgame function
		return endgloop(stop);
	}

	timer.Restart();

	// get input:
	KeybLib::Get_State(keyb);

	// logic:
	for(i = 0; i < tokens->size(); i++)
	{
		// variables for casting
		IToken *token;

		token = (*tokens)[i];

		switch(token->GetType())
		{
		case WALL:
		case LEVEL_END:
		case AMMO:
		case HEALTH_PACK:	
			break;
		case TURNER:
			assert(TURNER == i);
			turner = (CTurner *)token;
			turner->Logic(keyb);
			turner->GuyLogic();
			turner->weapons[turner->current_weapon]->Logic(NULL);
			break;
		case ENEMY:
			enemy = (CEnemy *)token;
			enemy->Logic(NULL);
			enemy->GuyLogic();
			enemy->weapon->Logic(NULL);
			break;
		default:
			assert(false); // an incorrect token is being stored
		}
	}

	// draw: (only call the wall draw function)
	((CWall *)((*tokens)[WALL]))->Draw();

	// flip
	gr->Flip();
	// sync
	do
	{
		// wait, be idle
	}
	while(timer.SecondsSinceLastRestart() < SECS_PER_FRAME);

	if(keyb[DIK_RETURN] & HIGHEST_BIT8)
	{
		CMedia::Media().DestroyTokens(true);
		
		
		if(keyb[DIK_RSHIFT] & HIGHEST_BIT8)
		{
			i = 0; // if both return and shift are down, start the whole game over
		}
		else
		{
			// otherwise just redo the current level
			i = CMedia::Media().GetCurrentLevel();
		}

		CMedia::Media().LoadLevel(i);
	}

	if(keyb[DIK_ESCAPE] & HIGHEST_BIT8)
	{
		return true;
	}
	else
	{
		return false;
	}
}
