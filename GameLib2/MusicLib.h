#ifndef _77ADF346_46E9_11d4_B6FE_0050040B0541_INCLUDED_
#define _77ADF346_46E9_11d4_B6FE_0050040B0541_INCLUDED_

// music lib for playing music
#include "StdAfx.h"

namespace NGameLib2 {
	IDirectMusicPerformance *Performance();
	IDirectMusicSegment *Segment();
	IDirectMusicLoader *Loader();
	double DefaultTempo(); // returns original tempo of current segment

	const int MUSICLIBERR_DMUSICNOTAVAIL = 3;
	const int MUSICLIBERR_COMNOTAVAIL  = 2;
	const int MUSICLIBERR_PORTNOTAVAIL = 1;

	int MusicInit( // inits music library; init after RobotSound
		HWND w, // we need the handle of the main window
		LPDIRECTSOUND ds
	); // This function returns non-zero on failure

	// deinits music library
	void MusicUninit();

	// allows looping.  Call this function to start playing a midi from resource
	//  call MusicPlay(const TCHAR *file_name) in order to loop the same midi
	int MusicPlay(
		bool loop,
		const TCHAR *res_name,
		const TCHAR *res_type,
		HMODULE res_mod = NULL,
		WORD res_lang = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)
	);

	int MusicPlay( // play a MIDI file in a loop
		bool loop,
		const TCHAR *file_name // the name of the MIDI file to play 
	); // returns non-zero on failure
	// if you passs NULL as file_name, the function will check to see if any music
	//  is playing, and if not, will make the music continue.  Allows Looping.

	// stop the currently playing midi file
	void MusicStop();

	// set the tempo
	void SetTempo(double tempo);


}

#endif
