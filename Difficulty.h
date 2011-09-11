enum {DIFFLEVEL_DANGNABIT, DIFFLEVEL_MYDEARCHILD, DIFFLEVEL_DANGERDANGER,
      DIFFLEVEL_COUNT};

void DifSet(int diff_index);
std::string DifName(int diff_index);

/** Returns the current difficulty level.
 * @return the current difficulty level.
 */
int DifGet();

FIXEDNUM DifHealthBonusPerPack();
FIXEDNUM DifDamageToHero(int weapon);
FIXEDNUM DifDamageToEnemy(int weapon);

unsigned short DifAmmoPerPack(int weapon);
