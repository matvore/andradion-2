// the level end class really doesn't do ANYTHING!

#include "Andradion 2.h"

CLevelEnd::CLevelEnd()
:
IToken(LEVEL_END)
{
	this->state = 0; // value not actually used . . . .
}

CLevelEnd::~CLevelEnd()
{
	// nothing to do here!
}

void CLevelEnd::Logic(unsigned char keyb[KeybLib::KEYBOARD_BUFFER_SIZE])
{
	// no logic here
}

void CLevelEnd::Draw(void)
{
	// abstract objects are not drawn
}
