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
#include "Resource.h"

using namespace std;

using std::endl;

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

  logger << "MusicInit called" << endl;

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

  logger << "MusicUninit called" << endl;

  MusicStop();

  if (performance) {
    logger << "DMusic int not released" << endl;
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

  logger << "MusicPlay called" << endl;

  if (!performance) {
    assert (!loader);
    logger << "MusicPlay returning; DirectMusic not initiated" << endl;
    return 0; // return success, because their was no interface anyway
  }

  assert(loader);

  logger << "Calling MusicStop" << endl;
  MusicStop();

  InitDXStruct(&ObjDesc);
  ObjDesc.guidClass = CLSID_DirectMusicSegment; // set guid class
  ObjDesc.dwValidData  = DMUS_OBJ_CLASS | DMUS_OBJ_MEMORY;

  res.reset(new Resource(res_type, res_name));

  ObjDesc.llMemLength = res->GetSize();
  ObjDesc.pbMemData = const_cast<BYTE *>(res->GetPtr());

  if (FAILED(LogResult("Getting MusicSegment Interface",
      loader->GetObject(
          &ObjDesc, IID_IDirectMusicSegment,
          (void **)new_segment.GetPtrToPtr())))) {
    return 4;
  }

  res.reset();

  // setting band track parameter
  if (FAILED(LogResult("Setting band track parameter step 1/2",
       new_segment->SetParam(
           GUID_StandardMIDIFile, (DWORD)-1,
           0, 0, (void *)performance.Get())))) {
    return 3;
  } else if (FAILED(LogResult("Setting band track parameter step 2/2",
      new_segment->SetParam(
          GUID_Download,(DWORD) - 1, 0 , 0,
          (void *)performance.Get())))) {
    return 2;
  }

  LogResult("Enabling the tempo track",
      new_segment->SetParam(GUID_EnableTempo, (DWORD)-1, 0, 0, 0));

  if(loop) {
    LogResult("Set segment to infinitely repeat",
        new_segment->SetRepeats(0 - 1));
  }

  logger << "Done taking care of looping" << endl;

  if (FAILED(LogResult("Play the new music",
      performance->PlaySegment(new_segment.Get(), 0, 0, NULL)))) {
    return 1;
  }

  if (SUCCEEDED(LogResult("Get original tempo",
      new_segment->GetParam(GUID_TempoParam, 0 - 1, 0, 0, 0, (void *)&tp)))) {
    original_tempo = tp.dblTempo;
  } else {
    original_tempo = 0.0f;
  }

  logger << "MusicPlay succeeded" << endl;

  segment = new_segment;

  return 0;
}

void MusicStop(void) { // stops music if it is playing
  assert (!performance == !loader);

  // we stop the music here; if there is an error, then it doesn't matter,
  //  because we will release it right afterwards
  if (segment) {
    assert(performance);
    LogResult("Stop performance", performance->Stop(0, 0, 0, 0));

    LogResult("Unload performance", segment->SetParam(
        GUID_Unload, (DWORD)-1, 0, 0, (void*)performance.Get()));

    segment.Reset();
  }
}

void SetTempo(double tempo) {
  logger << "Call to set tempo to " << tempo << endl;

  if(segment) {
    DMUS_TEMPO_PMSG* pTempo;

    LogResult("Disable tempo track so it does not reset the tempo",
        segment->SetParam(GUID_DisableTempo, 0xFFFF, 0, 0, 0));

    if (SUCCEEDED(LogResult("Allocate tempo event",
        performance->AllocPMsg(
            sizeof(DMUS_TEMPO_PMSG), (DMUS_PMSG**)&pTempo)))) {
      InitDXStruct(pTempo);
      pTempo->dblTempo = tempo;
      pTempo->dwFlags = DMUS_PMSGF_REFTIME;
      pTempo->dwType = DMUS_PMSGT_TEMPO;
      LogResult("Queue tempo event", performance->SendPMsg((DMUS_PMSG*)pTempo));
    }
  }

  logger << "SetTempo returning" << endl;
}
