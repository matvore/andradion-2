#include "StdAfx.h"
#include "Fixed.h"
#include "LevelEnd.h"

const FIXEDNUM LEVELEND_FATNESS = Fixed(20);

bool CLevelEnd::Collides(FIXEDNUM tx, FIXEDNUM ty) const {
   return abs(tx - x) < LEVELEND_FATNESS && abs(ty - y) < LEVELEND_FATNESS;
}
