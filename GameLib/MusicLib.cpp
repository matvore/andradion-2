// RobotMusic.cpp - simple library to play MIDI files
#include "MusicLib.h"

// here are the interfaces and objects:
static IDirectMusicPerformance *dmusic_int = NULL;
static IDirectMusicSegment *dmusic_seg = NULL;
static IDirectMusicLoader *dmusic_loader = NULL;
static bool we_made_com;

namespace MusicLib {
	IDirectMusicPerformance *Performance(void) {return dmusic_int;} // returns the performance (ptr)
	IDirectMusicSegment *Segment(void) {return dmusic_seg;} // returns the segment (ptr)
	IDirectMusicLoader *Loader(void) {return dmusic_loader;} // returns the loader (ptr)

	Error_Ready Init(HWND w) { // opens up DirectMusic, needs the SoundLib to be init'd in order to work, starts com if necessary
		Init_Error_Ready_System();

		// check directsound (it must be initialized)
		if(!SoundLib::Direct_Sound()) Return_With_Error(8);

		// make sure music isn't already initialized:
		if(dmusic_int) Return_With_Error(7); // return error if it is so
		if(dmusic_loader) Return_With_Error(6);

		// try to create the COM interface:
		switch(CoInitialize(NULL)) {
		case S_OK: // we initialized the com interface
			we_made_com = true;
			break;
		case S_FALSE:
		case RPC_E_CHANGED_MODE:
			we_made_com = false; // we didn't initialize it
			break;
		default: // com was never initialized and we couldn't do it either:
			Return_With_Error(5); // return with error
		}

		// now try to create the performance:
		if(FAILED(
			CoCreateInstance(CLSID_DirectMusicPerformance,NULL,CLSCTX_INPROC,IID_IDirectMusicPerformance,(void **)&dmusic_int)
		)) Return_With_Error(4);

		// now init it:
		if(FAILED(dmusic_int->Init(NULL,SoundLib::Direct_Sound(),w)))
			Return_With_Error(3);
	
		// now add the port:
		if(FAILED(dmusic_int->AddPort(NULL)))
			Return_With_Error(2);

		// finally, try to create the loader:
		if(FAILED(CoCreateInstance(CLSID_DirectMusicLoader,NULL,CLSCTX_INPROC,IID_IDirectMusicLoader,(void **)&dmusic_loader)))
			Return_With_Error(1);
	
		Begin_Error_Table
			Define_Error(1,"Could not create DirectMusic Loader object") dmusic_loader = NULL;
			Define_Error(2,"Could not add the music port to the DMusic Interface") dmusic_int->CloseDown();
			Define_Error(3,"Could not initialize DirectMusic Interface") dmusic_int->Release();
			Define_Error(4,"Could not open DirectMusic Interface") dmusic_int = NULL;
			Define_Error(5,"Could not initialize COM")
			Define_Error(6,"DirectMusic Loader object tried to initialize when it was already initialized")
			Define_Error(7,"DirectMusic tried to initialize when it already was initialized")
			Define_Error(8,"DirectSound was not properly initialized, so DirectMusic could not be initialized")
		End_Error_Table(w);
	}
	
	void Uninit(void) { // this just releases and closes everything down
		Stop();

		if(dmusic_int) { // the main interface (performance object)
			dmusic_int->CloseDown();
			dmusic_int->Release();
			dmusic_int = NULL;
		}

		if(dmusic_loader) { // the loader object
			dmusic_loader->Release();
			dmusic_loader = NULL;
		}

		// if we made com:
		if(we_made_com)
			CoUninitialize(); // then we have to get rid of it
	}

	Error_Ready Play(const char *file_name) { // plays the specified midi file
		// we must load a MIDI segment and then play it
		DMUS_OBJECTDESC ObjDesc;
	
		char work_dir_multi[DMUS_MAX_FILENAME];
		WCHAR work_dir_wide[DMUS_MAX_FILENAME]; // work directory in WIDE format
		WCHAR file_name_wide[DMUS_MAX_FILENAME]; // file name converted to wide format

		// make sure direct music is initiated
		if(!dmusic_int)
			return 0; // return error

		if(!file_name) { // if file_name is null, we have a special job:
			if(!dmusic_seg) return 3;
			switch(dmusic_int->IsPlaying(dmusic_seg,NULL)) {
			case S_FALSE: // not playing, without errors
				if(FAILED(dmusic_int->PlaySegment(dmusic_seg,0,0,NULL)))
					return 2;
			case S_OK: // still playing, without errors
				return 0;
			default: // we have errors
				return 1;
			};
		}

		Stop();

		if( !dmusic_loader) // if either isn't init'd
			return 7;

		// try to get the working directory:
		if(_getcwd(work_dir_multi,DMUS_MAX_FILENAME) == NULL) // if we fail
			return 6; // return error

		// convert:
		MULTI_TO_WIDE(work_dir_wide, work_dir_multi);

		// tell the loader where to look for files:
		if(FAILED( // if we fail in
			dmusic_loader->SetSearchDirectory( // setting the search directory
				GUID_DirectMusicAllTypes,
				work_dir_wide,
				FALSE)))
			return 5; // return error

		// convert file_name to wide string
		MULTI_TO_WIDE(file_name_wide,file_name);

		// setup object description struct
		memset(&ObjDesc,0,sizeof(ObjDesc)); // zero memory
		ObjDesc.dwSize = sizeof(ObjDesc); // set size structure
		ObjDesc.guidClass = CLSID_DirectMusicSegment; // set guid class
		wcscpy(ObjDesc.wszFileName,file_name_wide);
		ObjDesc.dwValidData  = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;
	
		if(FAILED( // if we fail in
			dmusic_loader->GetObject( // getting a MusicSegment interface
				&ObjDesc,
				IID_IDirectMusicSegment,
				(void **)&dmusic_seg))) {
			dmusic_seg = NULL; // make sure this is null
			return 4; // return error 
		}

		if(FAILED( // if we fail in
			dmusic_seg->SetParam( // setting band track parameter
			GUID_StandardMIDIFile,-1,0,0,(void *)dmusic_int))) {
			dmusic_seg->Release(); // release the new object
			dmusic_seg = NULL; // make sure this is null
			return 3; // return error
		}

		if(FAILED( // if we fail in
			dmusic_seg->SetParam( // downloading
			GUID_Download,-1,0,0,(void *)dmusic_int))) {
			dmusic_seg->Release(); // release the new object
			dmusic_seg = NULL; // make sure this is null
			return 2; // return error
		}

		if(FAILED( // if we fail in
			dmusic_int->PlaySegment( // playing the new music
				dmusic_seg,
				0,0,
				NULL))) {
			dmusic_seg->Release(); // release the new object
			dmusic_seg = NULL; // make sure this is null
			return 1; // return with error
		}

		// we made it here, so we won!
		return 0; // return success
	}

	void Stop(void) { // stops music if it is playing
		// we stop the music here; if there is an error, then it doesn't matter,
		//  because we will release it right afterwards
		if(!dmusic_seg) return; // make sure one exists

		dmusic_int->Stop(
			NULL,NULL,0,0
		); // stop everything

		dmusic_seg->SetParam(GUID_Unload, -1, 0, 0, (void*)dmusic_int);

		dmusic_seg->Release(); // release the segment

		dmusic_seg = NULL;
	}
}
