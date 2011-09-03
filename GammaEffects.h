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

