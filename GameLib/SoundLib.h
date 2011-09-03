#ifndef __SOUNDLIB_H__
#define __SOUNDLIB_H__

// RobotSound.h
//  All this does is play sounds.  Doesn't rely on other libraries.
//  Some libraries may rely on IT though.
#include <windows.h>  // these are just used by the SOB creator
#include <mmsystem.h> 
#include <dsound.h> // duh!
#include <cassert>
#include "GameLib.h"

namespace SoundLib {
	LPDIRECTSOUND Direct_Sound(void);

	// remember Blitter OBjects? well these are Sound OBjects
	typedef LPDIRECTSOUNDBUFFER SOB;

	// this function will start direct sound:
	int Init(HWND w);
	// return value: 0 on success, non-zero on failure
	// w: handle to the window

	// this function will unitialize the direct sound
	void Uninit(void);

	// ok, now the rest are SOB functions:
	//  sobs are assumed to be 8-bit mono wav's; otherwise, they don't work
	int Create_SOB(SOB *s, const char* file_name);
	// return value: zero on success, non-zero failure
	// s: the pointer to a sound buffer which will contain the loaded sound
	// file_name: the name of the wav file to load the sound from

	int Create_SOB(SOB *dest_s,SOB source_s); // allows sharing of sound buffer data

	// this next SOB function will destroy a SOB
	void Destroy_SOB(SOB s);
	// s: the SOB to destroy
}

#endif
