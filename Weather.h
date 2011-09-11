/** Values returned by <tt>WtrOneFrame</tt> that indicate one
 *  of the following:
 * <ul>
 *  <li>The state has changed and the music should be audible.</li>
 *  <li>The state has changed and the music should not be audible.</li>
 *  <li>The state has not changed.</li>
 * </ul>
 */
enum {WTROF_TURNMUSICON,
      WTROF_TURNMUSICOFF,
      WTROF_NOCHANGE};

/** Figures out which indices of the palette should be used for various
 * things. For example, all rain is of a particular color; whenever the
 * palette changes, this method should be called in case the palette
 * index containing the best approximation of the rain color has
 * changed.
 */
void WtrAnalyzePalette();

/** Sets the weather script to use.
 * @param script an integer indicating which weather script to use.
 */
void WtrBeginScript(int script);

/** Indicates how bright the screen should be. The sun, clouds and
 * rain all combine to affect how bright the scene should be.
 * @return a value in the range of 0.0 to 1.0 indicating the
 *  intended brightness of the screen.
 */
FIXEDNUM WtrBrightness();

/** Supplies an integer indicating the current weather state.
 * This indicator can then be passed to <tt>WtrSetState(int)</tt> to
 * change to the same state.
 * @return an integer indicating the current weather state.
 */
int WtrCurrentState();

/** Ends the current weather script. Call this after you are finished
 * making calls to <tt>WtrOneFrame</tt>.
 */
void WtrEndScript();

/** Prepares internal data structures for simulating weather. This
 * can be called only once as long as the IDirectSound interface to
 * use does not change.
 */ 
void WtrInitialize();

/** Executes the weather for one frame. This does not allow for state
 * changes because it does not increment the frame count.
 * Use this version of <tt>WtrOneFrame</tt> when the hero is indoors. 
 * @return one of the <tt>STATECHANGE_</tt> enumerated values
 *  indicating what to do with the music.
 */
int WtrOneFrameIndoors();

/** Executes the weather for one frame. This includes rendering the
 * rain and playing any necessary sound effects, but it does not allow
 * for state changes because it does not increment the frame count.
 * Use this version of <tt>WtrOneFrame</tt> when the hero is outdoors
 * @return one of the <tt>STATECHANGE_</tt> enumerated values
 *  indicating what to do with the music.
 */
int WtrOneFrameOutdoors();

/** Allows the change of weather state by incrementing the frame counter.
 * @return true iff the state changed.
 */
bool WtrPermitStateChange();

/** Frees memory associated with the Weather module. Requires that
 * the Weather module be initialized.
 */
void WtrRelease();

/** Changes the playback frequency of all the weather-related
 * sounds. This includes crickets, thunder, and rain.
 * @param freq the new frequency at which to play the sounds.
 */
void WtrSetSoundPlaybackFrequency(unsigned long freq);

/** Changes the current state. This is necessary if
 * <tt>WtrPermitStateChange()</tt> is not being called; the state
 * changes happen manually.
 * @param next_state the index of the weather state to enter.
 */
void WtrSetState(int next_state);

