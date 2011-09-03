// Weather.h: interface for the CWeather class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WEATHER_H__F324ADE0_A2B3_11D4_B6FE_0050040B0541__INCLUDED_)
#define AFX_WEATHER_H__F324ADE0_A2B3_11D4_B6FE_0050040B0541__INCLUDED_

// see Weather.doc for some notes

using NGameLib2::CSurfaceLock256;

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

#endif // !defined(AFX_WEATHER_H__F324ADE0_A2B3_11D4_B6FE_0050040B0541__INCLUDED_)
