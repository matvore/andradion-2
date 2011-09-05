/*
Copyright 2011 Matt DeVore

Licensed under the Apache License, Version 2.0 (the "License");
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
#include "Logger.h"

using std::endl;

// module that loads raw sound data into sound buffers

static IDirectSoundBuffer *FillSoundBuffer(IDirectSoundBuffer *target,
                                           const void *data,
                                           DWORD bytes) {
		
  BYTE *audio_ptr_1 = 0, *audio_ptr_2 = 0;

  DWORD audio_length_1 = 0, audio_length_2 = 0;

  const BYTE *snd_buffer = (const BYTE *)data;

  if(FAILED((*target).Lock(0, bytes,
                           (void **)&audio_ptr_1,
                           &audio_length_1,
                           (void **)&audio_ptr_2,
                           &audio_length_2,
                           DSBLOCK_FROMWRITECURSOR))) {
    (*target).Release();
    logger << "Could not lock sound buffer" << endl;
    return 0;
  }

  memcpy(audio_ptr_1, snd_buffer, audio_length_1);

  memcpy(audio_ptr_2, snd_buffer + audio_length_1, audio_length_2);

  (*target).Unlock(audio_ptr_1, audio_length_1,
                   audio_ptr_2, audio_length_2);

  return target;
}

// this function returns zero on success
static IDirectSoundBuffer *CreateSB(IDirectSound *ds,
                                    DWORD desc_flags,
                                    DWORD frequency,
                                    DWORD bits_per_sample,
                                    DWORD channels,
                                    DWORD buffer_size) {
  IDirectSoundBuffer *new_buff;
  DSBUFFERDESC dsbd;
  WAVEFORMATEX wfm;
  
  if(!ds) {
    logger << "Direct sound is not available; could not create sound buffer" <<
        endl;
    return 0;
  }

  assert(channels && bits_per_sample && frequency);

  // copy wave format structure
  wfm.cbSize = 0;
  wfm.nAvgBytesPerSec = DWORD(wfm.nBlockAlign = WORD(channels * bits_per_sample / 8));
  wfm.nAvgBytesPerSec *= frequency;
  wfm.nChannels = (WORD)channels;
  wfm.nSamplesPerSec = frequency;
  wfm.wBitsPerSample = (WORD)bits_per_sample;
  wfm.wFormatTag=WAVE_FORMAT_PCM;

  dsbd.dwBufferBytes = buffer_size;
  dsbd.dwFlags = desc_flags;
  dsbd.dwReserved = 0;
  dsbd.dwSize = sizeof(dsbd);
  dsbd.lpwfxFormat = &wfm;
		
  if (FAILED(TryAndReport(ds->CreateSoundBuffer(&dsbd, &new_buff, 0)))) {
    logger << "Could not create sound buffer" << endl;
    return 0;
  } else {
    return new_buff;
  }
}

IDirectSoundBuffer *CreateSBFromRawData
(IDirectSound *ds, void *data, DWORD size, DWORD desc_flags,
 DWORD frequency, DWORD bits_per_sample, DWORD channels) {
  IDirectSoundBuffer *new_buff = CreateSB(ds, desc_flags,
                                          frequency, bits_per_sample,
                                          channels, size);

  return new_buff ? FillSoundBuffer(new_buff, data, size) : 0;
}
