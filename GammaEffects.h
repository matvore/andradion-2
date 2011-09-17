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

/** Gamma effect types. */
enum {GETYPE_NONE, GETYPE_BLOOD,
      GETYPE_HEALTH, GETYPE_AMMO};

/** Initiates a gamma effect. In order for the effect to continue, be
 * sure to call <tt>GamOneFrame(FIXEDNUM)</tt> every frame.
 * @param type indicates the gamma effect.
 * @param current_health the health of the player before the start of
 *  the effect. For example, if the player has just been shot and the
 *  blood effect is initiated, this value is the player's healh before
 *  he was shot.
 */
void GamDoEffect(int type, FIXEDNUM current_health);

/** Indicates whether or not the health is in the process of changing.
 * @return true iff the player's virtual health is changing from
 *  frame-to-frame.
 */
bool GamHealthChanging();

FIXEDNUM GamVirtualHealth(FIXEDNUM current_health);

void GamInitializeWithMenuPalette();
void GamInitializeWithIntroPalette();
const BYTE *GamInitialize(const BYTE *ifs);

void GamOneFrame(FIXEDNUM overall_brightness);

