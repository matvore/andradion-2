#ifndef __KEYBLIB_H__
#define __KEYBLIB_H__

// The RobotKeyboard library!
//  This is a very simple and cool little library for receiving keyboard input
#include <dinput.h> // we need direct input!
#include "GameLib.h"

namespace KeybLib {
	const int KEYBOARD_BUFFER_SIZE = 256;

	LPDIRECTINPUTDEVICE7 Keyboard(void);
	LPDIRECTINPUT7 Direct_Input(void);

	// This function is for initializing the keyboard
	Error_Ready Init(HINSTANCE i, HWND w);
	// return value: 0 on success, non-zero on failure
	// i: handle to the instance it will be used in
	// w: handle to the window it will be used in

	// Next function will uninit this library
	void Uninit(void);

	// this will get the keyboard state
	void Get_State(unsigned char to_put_it[KEYBOARD_BUFFER_SIZE]);
}

#endif
