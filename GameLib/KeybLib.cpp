// ROBOTKEYBOARD.CPP
//  Library of RobotFighters for interface with the keyboard through
//  DirectInput 7.0
#include "KeybLib.h"

// here are our necessary interface/object pointers
static LPDIRECTINPUTDEVICE7 dinput_obj_keyb = NULL;
static LPDIRECTINPUT7 dinput_int = NULL;

namespace KeybLib {
	LPDIRECTINPUTDEVICE7 Keyboard(void) {return dinput_obj_keyb;}
	LPDIRECTINPUT7 Direct_Input(void) {return dinput_int;}

	// note that the next function note only allows the keyboard to be used,
	//  but also gives functionality to the RobotMouse library
	Error_Ready Init(HINSTANCE i, HWND w) {
		Init_Error_Ready_System();

		// first make sure the DInput system is really inactive:
		if(dinput_int) Return_With_Error(6);
	
		// the following would be odd, but it can't do harm to check to make sure:
		if(dinput_obj_keyb) Return_With_Error(5);

		// alright, now let's try to create the interface:
		CoInitialize(NULL);
		CoCreateInstance
		(
			CLSID_DirectInput,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IDirectInput7W,
			(void **)&dinput_int
		);
		if(FAILED(dinput_int->Initialize(i,DIRECTINPUT_VERSION)))
		{
			Return_With_Error(4);
		}
	
		// now let's try to create the keyboard device:
		if(FAILED(dinput_int->CreateDeviceEx(GUID_SysKeyboard,IID_IDirectInputDevice7,(void **)&dinput_obj_keyb,NULL)))
			Return_With_Error(3);

		// now we set the coop level:
		if(FAILED(dinput_obj_keyb->SetCooperativeLevel(w,DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))) Return_With_Error(2);
	
		if(FAILED(dinput_obj_keyb->SetDataFormat(&c_dfDIKeyboard))) Return_With_Error(1); // set the data format
	
		Begin_Error_Table
			Define_Error(1,"Could not set keyboard data format")
			Define_Error(2,"Could not set keyboard cooperative level") dinput_obj_keyb->Release();
			Define_Error(3,"Keyboard Interface could not be created") dinput_obj_keyb = NULL; dinput_int->Release();
			Define_Error(4,"DirectInput Interface could not be created") dinput_int = NULL;
			Define_Error(5,"Keyboard Interface has been opened twice")
			Define_Error(6,"DirectInput Interface has been opened twice")
		End_Error_Table(w);
	}

	void Uninit(void) {
		// get rid of the keyboard interface
		if(dinput_obj_keyb) { // make sure it is allocated memory
			dinput_obj_keyb->Unacquire(); // unacquire it
			dinput_obj_keyb->Release(); // get rid of the memory
			dinput_obj_keyb = NULL; // point it away
		}

		// get rid of the direct input interface
		if(dinput_int) { // make sure it is allocated memory
			dinput_int->Release(); // get rid of the memory
			dinput_int = NULL; // point it away
		}

		CoUninitialize();
	}
	void Get_State(
		unsigned char to_put_it[KEYBOARD_BUFFER_SIZE] // where the keyboard data is to be placed
	) {
		while( // while
			FAILED( // we fail in
				dinput_obj_keyb->GetDeviceState( // getin the keyboard device state
					256, // into 256 bytes of memory
					to_put_it // in the passed array
					))) 
			// just try to reacqure it:
			dinput_obj_keyb->Acquire();
	}
}
