// include our libraries
#include <GameLib.h>
#include <SoundLib.h>
#include <MusicLib.h>
#include <KeybLib.h>
#include <GraphLib.h>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <fstream>
#include <cstring>
#include <cmath>

#include "classes.h"
#include "constants.h"

// get windows
#include <windows.h>

#ifndef __ANDRADION_2_H__
#define __ANDRADION_2_H__

extern GraphLib::CGraphics *gr;
extern HWND wnd;
extern HINSTANCE instance;

// bool(bool) syntax:
//  pass true when the more powerful needs to quit
//  return true when the less powerful is quitting
//  if true is passed, true MUST BE RETURNED
//  if true is returned, the function WILL NOT BE CALLED AGAIN

bool mainloop(bool stop);
	bool intrloop(bool stop);
	bool gameloop(bool stop);
		bool endgloop(bool stop);

#endif
