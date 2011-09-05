/*
Copyright 2011 Matt DeVore

Licensed under the Apache License, Version 2.0 (the "License" << endl;
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "StdAfx.h"
#include "MusicLib.h"
#include "Logger.h"

using std::endl;

// here are the interfaces and objects:
static IDirectMusicPerformance *performance = 0;
static IDirectMusicSegment *segment = 0;
static IDirectMusicLoader *loader = 0;
static double original_tempo;

IDirectMusicPerformance *Performance() {return performance;} 
IDirectMusicSegment *Segment() {return segment;} 
IDirectMusicLoader *Loader() {return loader;} 
double DefaultTempo() {return original_tempo;}

int MusicInit(HWND w, IDirectSound *ds) throw(std::bad_alloc) {
  HRESULT hr;

  assert(!performance);
  assert(!segment);
  assert(!loader);

  logger << "MusicInit called" << endl;
  
  CoInitialize(0);

  // create performance:
  if(FAILED(CoCreateInstance(CLSID_DirectMusicPerformance,
			     NULL,
			     CLSCTX_INPROC,
			     IID_IDirectMusicPerformance,
			     (void **)&performance))) {
    return MUSICLIBERR_DMUSICNOTAVAIL;
  }

  if (FAILED(performance->Init(NULL, ds, w))) {
    performance->CloseDown();
    performance->Release();
    performance = 0;
    return MUSICLIBERR_OUTOFMEMORY;
  }
  
  if (FAILED(performance->AddPort(NULL))) {
    performance->CloseDown();
    performance->Release();
    performance = 0;
    return MUSICLIBERR_PORTNOTAVAIL;
  }

  // finally, try to create the loader:
  if(FAILED(CoCreateInstance(CLSID_DirectMusicLoader,
			     NULL,
			     CLSCTX_INPROC,
			     IID_IDirectMusicLoader,
			     (void **)&loader))) {
    performance->CloseDown();
    performance->Release();
    performance = NULL;
    return MUSICLIBERR_DMUSICNOTAVAIL;
  }

  return 0;
}
	
void MusicUninit(void) {
  assert ((NULL == performance) == (NULL == loader));

  logger << "MusicUninit called" << endl;
  
  MusicStop();

  if(NULL != performance) {
    logger << "DMusic int not released" << endl;
    TryAndReport(performance->CloseDown());
    TryAndReport(performance->Release());
    performance = NULL;
  }

  if(NULL != loader) {
    logger << "DMusic loader not released" << endl;
    TryAndReport(loader->Release());
    loader = NULL;
  }

  logger << "MusicUninit closing COM" << endl;
  CoUninitialize(); 
}

//plays the specified midi resource
int MusicPlay(bool loop,
	      const TCHAR *res_name,
	      const TCHAR *res_type,
	      HMODULE res_mod,
	      WORD res_lang) {
  DMUS_OBJECTDESC ObjDesc;
  
  logger << "MusicPlay called" << endl;

  if(NULL == performance) {
    assert (NULL == loader);
    logger << "MusicPlay returning; DirectMusic not initiated" << endl;
    return 0; // return success, because their was no interface anyway
  }

  assert (NULL != loader);

  logger << "Calling MusicStop" << endl;
  MusicStop();

  memset((void *)&ObjDesc,0,sizeof(ObjDesc));
  ObjDesc.dwSize = sizeof(ObjDesc);
  ObjDesc.guidClass = CLSID_DirectMusicSegment; // set guid class
  ObjDesc.dwValidData  = DMUS_OBJ_CLASS | DMUS_OBJ_MEMORY;

  HRSRC res_handle = TryAndReport(FindResourceEx(res_mod,(LPCTSTR)res_type,
						 (LPCTSTR)res_name,res_lang));

  if(NULL == res_handle) {
    logger << "MusicPlay returning; could not find resource" << endl;
    return 7;
  }

  HGLOBAL res_data = TryAndReport(LoadResource(res_mod,res_handle));

  if(NULL == res_data)
    {
      logger << "MusicPlay returning; could not load resource" << endl;
      return 6;
    }

  ObjDesc.llMemLength = TryAndReport(SizeofResource(res_mod, res_handle));
  ObjDesc.pbMemData = (PBYTE)TryAndReport(LockResource(res_data));

  if(NULL == ObjDesc.pbMemData) {
    logger << "MusicPlay returning; could not lock resource" << endl;
    return 5;
  }

  // getting a MusicSegment interface
  if(FAILED(TryAndReport(loader->GetObject(&ObjDesc,
					   IID_IDirectMusicSegment,
					   (void **)&segment)))) {
    TryAndReport(FreeResource(res_data));
    segment = NULL; // make sure this is null
    return 4; // return error 
  }

  TryAndReport(FreeResource(res_data));

  // setting band track parameter
  if(FAILED(TryAndReport(segment->SetParam(GUID_StandardMIDIFile,(DWORD)-1,
					      0,0,(void *)performance)))) {
    TryAndReport(segment->Release());
    segment = NULL; 
    return 3; 
  }

  // downloading
  if(FAILED(TryAndReport(segment->SetParam(GUID_Download,(DWORD)-1,0,0,
					      (void *)performance)))) {
    TryAndReport(segment->Release());
    segment = NULL; 
    return 2; 
  }

  // enable the tempo track
  TryAndReport(segment->SetParam( GUID_EnableTempo, (DWORD)-1,0,0, NULL ));

  if(loop) {TryAndReport(segment->SetRepeats(0-1));}

  logger << "Done taking care of looping" << endl;

  // playing the new music
  if(FAILED(TryAndReport(performance->PlaySegment(segment,
						 0,0, NULL)))) {
    TryAndReport(segment->Release()); 
    segment = NULL; 
    return 1;
  }

  // try to get original tempo
  DMUS_TEMPO_PARAM tp;
  if(SUCCEEDED(TryAndReport(segment->GetParam(GUID_TempoParam,0-1,0,0,NULL,
						 (void *)&tp)))) {
    original_tempo = tp.dblTempo;
  } else {
    original_tempo = 0.0f;
  }
  
  logger << "MusicPlay succeeded" << endl;

  return 0; 
}

void MusicStop(void) { // stops music if it is playing

  assert ((NULL == performance) == (NULL == loader));

  // we stop the music here; if there is an error, then it doesn't matter,
  //  because we will release it right afterwards
  if(NULL == segment) {
    logger << "MusicStop called without music being played" << endl;
    return;
  }

  assert(NULL != performance);
  
  TryAndReport(performance->Stop(NULL, NULL, 0, 0));

  TryAndReport(segment->SetParam(GUID_Unload, (DWORD)-1, 0, 0,
				    (void*)performance));

  TryAndReport(segment->Release());

  segment = NULL;
}

void SetTempo(double tempo) {
  logger << "Call to set tempo to " << tempo << endl;
  if(NULL != segment) {
    logger << "About to Disable tempo track in segment so that it does not reset the tempo" << endl;
    TryAndReport(segment->SetParam( GUID_DisableTempo, 0xFFFF,0,0, NULL ));
 
    DMUS_TEMPO_PMSG* pTempo;
 
    if( SUCCEEDED(TryAndReport(performance->AllocPMsg(sizeof(DMUS_TEMPO_PMSG), (DMUS_PMSG**)&pTempo)))) {
      logger << "About to queue the tempo event." << endl;
      ZeroMemory(pTempo, sizeof(DMUS_TEMPO_PMSG));
      pTempo->dwSize = sizeof(DMUS_TEMPO_PMSG);
      pTempo->dblTempo = tempo;
      pTempo->dwFlags = DMUS_PMSGF_REFTIME;
      pTempo->dwType = DMUS_PMSGT_TEMPO;
      TryAndReport(performance->SendPMsg((DMUS_PMSG*)pTempo));
    }
  }
  logger << "SetTempo returning" << endl;
}
