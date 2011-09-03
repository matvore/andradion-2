#include "StdAfx.h"
#include "Fixed.h"
#include "SharedConstants.h"
#include "Certifiable.h"
#include "Graphics.h"
#include "Bob.h"
#include "Character.h"
#include "LevelEnd.h"


const FIXEDNUM LEVELEND_FATNESS = Fixed(20);
bool CLevelEnd::Collides(const CCharacter& ch)
{
	FIXEDNUM tx;
	FIXEDNUM ty;
	ch.GetLocation(tx,ty);

	if(
		abs(tx - this->x) < LEVELEND_FATNESS &&
		abs(ty - this->y) < LEVELEND_FATNESS
	)
	{
		return true;
	}
	else
	{
		return false;
	}
}
