// see Weather.doc for some notes

int WtrCurrentState();
void WtrNextState(int next_state);
void WtrNextState();
void WtrAnalyzePalette();
void WtrInitialize(LPDIRECTSOUND ds);
void WtrRelease();
void WtrBeginScript();
void WtrSetScript(int index_);
void WtrOneFrame(CSurfaceLock256& lock); // hero is outdoors
void WtrOneFrame(); // hero is indoors
void WtrSetSoundPlaybackFrequency(FIXEDNUM factor); // changes playback of all weather sounds
bool WtrSilence();
FIXEDNUM WtrBrightness();

