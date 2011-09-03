// LevelEnd.cpp: implementation of the CLevelEnd class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Fixed.h"
#include "SharedConstants.h"
#include "Character.h"
#include "LevelEnd.h"

// Comment the next two lines to allow this module to log
#define WriteLog(x)
#undef LogArg
#define LogArg(x)
#undef TryAndReport
#define TryAndReport(x) x

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
