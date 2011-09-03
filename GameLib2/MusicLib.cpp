// RobotMusic.cpp - simple library to play MIDI files
#include "StdAfx.h"
#include "MusicLib.h"
#include "LazyErrHandling.h"
#include "logger.h"

// Comment the next three lines to allow this module to log
#define WriteLog(x)
#undef LogArg
#define LogArg(x)
#undef TryAndReport
#define TryAndReport(x) x

namespace NGameLib2 {
	// here are the interfaces and objects:
	static IDirectMusicPerformance *dmusic_int = NULL;
	static IDirectMusicSegment *dmusic_seg = NULL;
	static IDirectMusicLoader *dmusic_loader = NULL;
	static double original_tempo;

	IDirectMusicPerformance *Performance(void) {return dmusic_int;} // returns the performance (ptr)
	IDirectMusicSegment *Segment(void) {return dmusic_seg;} // returns the segment (ptr)
	IDirectMusicLoader *Loader(void) {return dmusic_loader;} // returns the loader (ptr)
	double DefaultTempo() {return original_tempo;}

	int MusicInit(HWND w,LPDIRECTSOUND ds) { // opens up DirectMusic, needs the SoundLib to be init'd in order to work, starts com if necessary
		// make sure music isn't already initialized:
		assert(NULL == dmusic_int);
		assert(NULL == dmusic_seg);
		assert(NULL == dmusic_loader);

		HRESULT hr;

		// try to create the COM interface:
		CoInitialize(NULL);

		// now try to create the performance:
		if(FAILED(CoCreateInstance(
			CLSID_DirectMusicPerformance,
			NULL,
			CLSCTX_INPROC,
			IID_IDirectMusicPerformance,
			(void **)&dmusic_int)))
		{
			return MUSICLIBERR_DMUSICNOTAVAIL;
		}

		// now init it:
		MemoryAllocFunction(hr = dmusic_int->Init(NULL,ds,w),1,FAILED(hr));
			
		// now add the port:
		MemoryAllocFunction(hr = dmusic_int->AddPort(NULL),1,E_OUTOFMEMORY == hr);

		if(FAILED(hr))
		{
			dmusic_int->Release();
			dmusic_int = NULL;
			return MUSICLIBERR_PORTNOTAVAIL;
		}

		// finally, try to create the loader:
		if(FAILED(CoCreateInstance(
			CLSID_DirectMusicLoader,
			NULL,
			CLSCTX_INPROC,
			IID_IDirectMusicLoader,
			(void **)&dmusic_loader
		)))
		{
			dmusic_int->Release();
			dmusic_int = NULL;
			return MUSICLIBERR_DMUSICNOTAVAIL;
		}

		return 0;
	}
	
	void MusicUninit(void) { // this just releases and closes everything down
		MusicStop();

		if(NULL != dmusic_int)
		{ // the main interface (performance object)
			dmusic_int->CloseDown();
			dmusic_int->Release();
			dmusic_int = NULL;
		}

		if(NULL != dmusic_loader)
		{ // the loader object
			dmusic_loader->Release();
			dmusic_loader = NULL;
		}

		CoUninitialize(); // close COM
	}

	static int MusicPlayGeneric(bool loop,DMUS_OBJECTDESC& ObjDesc)
	{
		// carries out second half of playing music when a new file is being loaded
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

		// enable the tempo track
		dmusic_seg->SetParam( GUID_EnableTempo, 0xFFFF,0,0, NULL );

		if(true == loop)
		{
			dmusic_seg->SetRepeats(0-1);
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

		// try to get original tempo
		DMUS_TEMPO_PARAM tp;
		if(SUCCEEDED(dmusic_seg->GetParam(GUID_TempoParam,0-1,0,0,NULL,(void *)&tp)))
		{
			//SetTempo(original_tempo = tp.dblTempo);
			original_tempo = tp.dblTempo;
		}
		else
		{
			original_tempo = 0.0f;
		}

		// we made it here, so we won!
		return 0; // return success
	}

	int MusicPlay(bool loop,const TCHAR *res_name,const TCHAR *res_type,HMODULE res_mod,WORD res_lang)
	{
		//plays the specified midi resource.  To continue it, call the other MusicPlay function MusicPlay(NULL)

		// make sure direct music is initiated
		if(NULL == dmusic_int || NULL == dmusic_loader)
		{
			return 0; // return success, because their was no interface anyway
		}

		MusicStop();

		DMUS_OBJECTDESC ObjDesc;

		memset((void *)&ObjDesc,0,sizeof(ObjDesc));
		ObjDesc.dwSize = sizeof(ObjDesc);
		ObjDesc.guidClass = CLSID_DirectMusicSegment; // set guid class
		ObjDesc.dwValidData  = DMUS_OBJ_CLASS | DMUS_OBJ_MEMORY;

		// try to Find, Lock, and Load the resource
		HRSRC res_handle = FindResourceEx(res_mod,(LPCTSTR)res_type,(LPCTSTR)res_name,res_lang);

		if(NULL == res_handle)
		{
			return 7;
		}

		HGLOBAL res_data = LoadResource(res_mod,res_handle);

		if(NULL == res_data)
		{
			return 6;
		}

		ObjDesc.llMemLength = SizeofResource(res_mod,res_handle);
		ObjDesc.pbMemData = (PBYTE)LockResource(res_data);

		if(NULL == ObjDesc.pbMemData)
		{
			return 5;
		}

		int res = MusicPlayGeneric(loop,ObjDesc);

		FreeResource(res_data);

		return res;
	}

	int MusicPlay(bool loop,const TCHAR *file_name)
	{ // plays the specified midi file, unless file_name is NULL, in which case, any midi file playing is continued

		// we must load a MIDI segment and then play it

		// make sure direct music is initiated
		if(NULL == dmusic_int || NULL == dmusic_loader)
		{
			return 0; // return success, because their was no interface anyway
		}

		if(NULL == file_name)
		{ // if file_name is null, we have a special job:
			if(NULL == dmusic_seg) 
			{
				return 0;
			}
			switch(dmusic_int->IsPlaying(dmusic_seg,NULL))
			{
			case S_FALSE: // not playing, without errors
				if(FAILED(dmusic_int->Stop(NULL,NULL,0,0)))
				{
					return 3;
				}
				if(FAILED(dmusic_int->PlaySegment(dmusic_seg,0,0,NULL)))
				{
					return 2;
				}
			case S_OK: // still playing, without errors
				return 0;
			case DMUS_E_NO_MASTER_CLOCK: // we have errors
				assert(false); // this error wouldn't make any sense
			default:
				assert(false);
			};
		}

		MusicStop();

		DMUS_OBJECTDESC ObjDesc;
	
		WCHAR work_dir_wide[DMUS_MAX_FILENAME]; // work directory in WIDE format
		WCHAR file_name_wide[DMUS_MAX_FILENAME]; // file name converted to wide format

		// try to get the working directory:
		if(_wgetcwd(work_dir_wide,DMUS_MAX_FILENAME) == NULL) // if we fail
			return 6; // return error

		// tell the loader where to look for files:
		if(FAILED( // if we fail in
			dmusic_loader->SetSearchDirectory( // setting the search directory
				GUID_DirectMusicAllTypes,
				work_dir_wide,
				FALSE)))
			return 5; // return error

		// setup object description struct
		memset(&ObjDesc,0,sizeof(ObjDesc)); // zero memory
		ObjDesc.dwSize = sizeof(ObjDesc); // set size structure
		ObjDesc.guidClass = CLSID_DirectMusicSegment; // set guid class
		// convert file_name to wide string if necessary before copying it into ObjDesc
#ifdef _UNICODE
		wcscpy(ObjDesc.wszFileName,file_name);
#else
		MultiByteToWideChar( CP_ACP,MB_PRECOMPOSED, file_name,-1,file_name_wide,DMUS_MAX_FILENAME);
		wcscpy(ObjDesc.wszFileName,file_name_wide);
#endif
		ObjDesc.dwValidData  = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;

		return MusicPlayGeneric(loop,ObjDesc);
	}

	void MusicStop(void) { // stops music if it is playing
		// we stop the music here; if there is an error, then it doesn't matter,
		//  because we will release it right afterwards
		if(NULL == dmusic_seg || NULL == dmusic_int || NULL == dmusic_loader)
		{
			return; // make sure one exists
		}

		dmusic_int->Stop(
			NULL,NULL,0,0
		); // stop everything

		dmusic_seg->SetParam(GUID_Unload, -1, 0, 0, (void*)dmusic_int);

		dmusic_seg->Release(); // release the segment

		dmusic_seg = NULL;
	}

	void SetTempo(double tempo)
	{
		WriteLog("Call to set tempo to %g" LogArg(tempo));
		if(NULL != dmusic_seg)
		{
			WriteLog("About to Disable tempo track in segment so that it does not reset the tempo");
			TryAndReport(dmusic_seg->SetParam( GUID_DisableTempo, 0xFFFF,0,0, NULL ));
 
			DMUS_TEMPO_PMSG* pTempo;
 
			if( SUCCEEDED(TryAndReport(dmusic_int->AllocPMsg(sizeof(DMUS_TEMPO_PMSG), (DMUS_PMSG**)&pTempo))))
			{
				WriteLog("About to queue the tempo event.");
				ZeroMemory(pTempo, sizeof(DMUS_TEMPO_PMSG));
				pTempo->dwSize = sizeof(DMUS_TEMPO_PMSG);
				pTempo->dblTempo = tempo;
				pTempo->dwFlags = DMUS_PMSGF_REFTIME;
				pTempo->dwType = DMUS_PMSGT_TEMPO;
				TryAndReport(dmusic_int->SendPMsg((DMUS_PMSG*)pTempo));
			}
		}
		WriteLog("SetTempo returning");
	}
}
