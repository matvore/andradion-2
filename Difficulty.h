/*
Copyright 2011 Matt DeVore

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

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
