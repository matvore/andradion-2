#include "StdAfx.h"
#include "MusicLib.h"
#include "Logger.h"
#include "Resource.h"

using namespace std;

// here are the interfaces and objects:
static AutoComPtr<IDirectMusicPerformance> performance;
static AutoComPtr<IDirectMusicSegment> segment;
static AutoComPtr<IDirectMusicLoader> loader;
static auto_ptr<Com> com;

static double original_tempo;

IDirectMusicPerformance *Performance() {return performance.Get();} 
IDirectMusicSegment *Segment() {return segment.Get();} 
IDirectMusicLoader *Loader() {return loader.Get();} 
double DefaultTempo() {return original_tempo;}

int MusicInit(HWND w, IDirectSound *ds) throw(std::bad_alloc) {
  HRESULT hr;
  AutoComPtr<IDirectMusicPerformance> new_performance;
  auto_ptr<Com> new_com;
  int error = 0;

  assert(!performance);
  assert(!segment);
  assert(!loader);

  WriteLog("MusicInit called");

  new_com.reset(new Com());

  // create performance:
  if(FAILED(CoCreateInstance(CLSID_DirectMusicPerformance,
                             NULL,
                             CLSCTX_INPROC,
                             IID_IDirectMusicPerformance,
                             (void **)new_performance.GetPtrToPtr()))) {
    error = MUSICLIBERR_DMUSICNOTAVAIL;
  } else if (FAILED(new_performance->Init(0, ds, w))) {
    error = MUSICLIBERR_OUTOFMEMORY;
  } else if (FAILED(new_performance->AddPort(0))) {
    error =  MUSICLIBERR_PORTNOTAVAIL;
  } else if (FAILED(CoCreateInstance(CLSID_DirectMusicLoader, 0,
                                     CLSCTX_INPROC,
                                     IID_IDirectMusicLoader,
                                     (void **)loader.GetPtrToPtr()))) {
    error = MUSICLIBERR_DMUSICNOTAVAIL;
  }

  if (!error) {
    com = new_com;
    performance = new_performance;
  } else if (new_performance) {
    new_performance->CloseDown();
  }
  
  return error;
}
	
void MusicUninit(void) {
  assert(!performance == !loader);

  WriteLog("MusicUninit called");
  
  MusicStop();

  if (performance) {
    WriteLog("DMusic int not released");
    performance->CloseDown();
    performance.Reset();
  }

  loader.Reset();
  com.reset();
}

//plays the specified midi resource
int MusicPlay(bool loop, const char *res_type, const char *res_name) {
  DMUS_OBJECTDESC ObjDesc;
  AutoComPtr<IDirectMusicSegment> new_segment;
  DMUS_TEMPO_PARAM tp;
  auto_ptr<Resource> res;
  
  WriteLog("MusicPlay called");

  if (!performance) {
    assert (!loader);
    WriteLog("MusicPlay returning; DirectMusic not initiated");
    return 0; // return success, because their was no interface anyway
  }

  assert(loader);

  WriteLog("Calling MusicStop");
  MusicStop();

  InitDXStruct(&ObjDesc);
  ObjDesc.guidClass = CLSID_DirectMusicSegment; // set guid class
  ObjDesc.dwValidData  = DMUS_OBJ_CLASS | DMUS_OBJ_MEMORY;

  res.reset(new Resource(res_type, res_name));

  ObjDesc.llMemLength = res->GetSize();
  ObjDesc.pbMemData = const_cast<BYTE *>(res->GetPtr());

  // getting a MusicSegment interface
  if(FAILED(TryAndReport(loader->GetObject
                         (&ObjDesc, IID_IDirectMusicSegment,
                          (void **)new_segment.GetPtrToPtr())))) {
    return 4;
  }

  res.reset();

  // setting band track parameter
  if(FAILED(TryAndReport
            (new_segment->SetParam(GUID_StandardMIDIFile, (DWORD)-1,
                                   0,0, (void *)performance.Get())))) {
    return 3; 
  } else if(FAILED(TryAndReport
                   (new_segment->SetParam(GUID_Download,(DWORD)-1,0,0,
                                          (void *)performance.Get())))) {
    return 2; 
  }

  // enable the tempo track
  TryAndReport(new_segment->SetParam(GUID_EnableTempo, (DWORD)-1, 0, 0, 0));

  if(loop) {
    TryAndReport(new_segment->SetRepeats(0-1));
  }

  WriteLog("Done taking care of looping");

  // playing the new music
  if(FAILED(TryAndReport(performance->PlaySegment(new_segment.Get(),
						 0,0, NULL)))) {
    return 1;
  } else if (FAILED(TryAndReport
                    (new_segment->GetParam(GUID_TempoParam, 0-1, 0, 0, 0,
                                           (void *)&tp)))) {
    original_tempo = 0.0f;
  } else {
    original_tempo = tp.dblTempo;
  }
  
  WriteLog("MusicPlay succeeded");

  segment = new_segment;

  return 0; 
}

void MusicStop(void) { // stops music if it is playing
  assert (!performance == !loader);

  // we stop the music here; if there is an error, then it doesn't matter,
  //  because we will release it right afterwards
  if (segment) {
    assert(performance);
    TryAndReport(performance->Stop(0, 0, 0, 0));

    TryAndReport(segment->SetParam(GUID_Unload, (DWORD)-1, 0, 0,
                                   (void*)performance.Get()));

    segment.Reset();
  }
}

void SetTempo(double tempo) {
  WriteLog("Call to set tempo to %g" LogArg(tempo));

  if(segment) {
    DMUS_TEMPO_PMSG* pTempo;

    WriteLog("About to Disable tempo track in segment"
             "so that it does not reset the tempo");
    TryAndReport(segment->SetParam(GUID_DisableTempo, 0xFFFF, 0, 0, 0));
 
    if (SUCCEEDED(TryAndReport(performance->AllocPMsg(sizeof(DMUS_TEMPO_PMSG),
                                                      (DMUS_PMSG**)&pTempo)))) {
      WriteLog("About to queue the tempo event.");
      InitDXStruct(pTempo);
      pTempo->dblTempo = tempo;
      pTempo->dwFlags = DMUS_PMSGF_REFTIME;
      pTempo->dwType = DMUS_PMSGT_TEMPO;
      TryAndReport(performance->SendPMsg((DMUS_PMSG*)pTempo));
    }
  }
  
  WriteLog("SetTempo returning");
}
