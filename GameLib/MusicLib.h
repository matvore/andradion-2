#ifndef __MUSICLIB_H__
#define __MUSICLIB_H__

// music lib for playing music
#include <direct.h>
#include <dmksctrl.h> // the DirectMusic header files
#include <dmusici.h>
#include <dmusicc.h>
#include <dmusicf.h>
#include "GameLib.h"
#include "SoundLib.h"

extern IDirectMusicPerformance *dmusic_int; // the interface we use:
extern IDirectMusicSegment *dmusic_seg; // the segment of music we are playing
extern IDirectMusicLoader *dmusic_loader; // our loader
extern bool we_made_com;

namespace MusicLib {
	IDirectMusicPerformance *Performance(void);
	IDirectMusicSegment *Segment(void);
	IDirectMusicLoader *Loader(void);

	Error_Ready Init( // inits music library; init after RobotSound
		HWND w // we need the handle of the main window
	); // This function returns non-zero on failure

	void Uninit( // deinits music library; deinit before RobotSound
		void // accepts no arguments
	); // has no return

	Error_Ready Play( // play a MIDI file in a loop
		const char *file_name // the name of the MIDI file to play 
	); // returns non-zero on failure
	// if you passs NULL as file_name, the function will check to see if any music
	//  is playing, and if not, will make the music continue.  Allows Looping.

	void Stop( // stop any looping MIDI file
		void // accepts no arguments
	); // has no return
}

#endif
