#include "StdAfx.h"
#include "Fixed.h"
#include "Difficulty.h"

using namespace std;

const FIXEDNUM HEALTHBONUSPERPACK[] = {Fixed(0.4),Fixed(0.25),Fixed(0.20)};
const FIXEDNUM HERO_WEAPONDAMAGE[DIFFLEVEL_COUNT][WEAPON_COUNT] = {
  {Fixed(0.05f), Fixed(0.020f), Fixed(0.003f)},
  {Fixed(0.07f), Fixed(0.030f), Fixed(0.006f)},
  {Fixed(0.10f), Fixed(0.035f), Fixed(0.010f)}
};

const FIXEDNUM ENEMY_WEAPONDAMAGE[DIFFLEVEL_COUNT][WEAPON_COUNT] = {
  {Fixed(0.30f), Fixed(0.05f), Fixed(0.024f)},
  {Fixed(0.20f), Fixed(0.03f), Fixed(0.020f)},
  {Fixed(0.12f), Fixed(0.02f), Fixed(0.018f)}
};

const unsigned short AMMOPERPACK[DIFFLEVEL_COUNT][WEAPON_COUNT] = {
  {20, 70, 2},
  {15, 60, 2},
  {15, 55, 2}
};

static int difficulty_level = 0;

void DifSet(int diff_index) {
  assert(diff_index >= 0 && diff_index < DIFFLEVEL_COUNT);
  difficulty_level = diff_index;
}

string DifName(int diff_index) {
  assert(diff_index >= 0 && diff_index < DIFFLEVEL_COUNT);

  switch (diff_index) {
  case DIFFLEVEL_DANGNABIT:
    return string("Dangnabit (try me first)");
  case DIFFLEVEL_MYDEARCHILD:
    return string("My Dear Child (hard)");
  case DIFFLEVEL_DANGERDANGER:
    return string("Danger! Danger! (very hard)");
  }
}

int DifGet() {return difficulty_level;}

FIXEDNUM DifHealthBonusPerPack() {return HEALTHBONUSPERPACK[difficulty_level];}

FIXEDNUM DifDamageToHero(int weapon) {
  return HERO_WEAPONDAMAGE[difficulty_level][weapon];
}

FIXEDNUM DifDamageToEnemy(int weapon) {
  return ENEMY_WEAPONDAMAGE[difficulty_level][weapon];
}

unsigned short DifAmmoPerPack(int weapon) {
  return AMMOPERPACK[difficulty_level][weapon];
}
