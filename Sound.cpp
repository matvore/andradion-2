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
#include "Sound.h"
#include "RawLoad.h"
#include "Logger.h"
#include "Resource.h"
#include "Fixed.h"

using namespace std;

// how far something can be on x axis before it is inaudible to one channel
const int MAX_XDIST = 100;

 // how far something can be squared before it is totally inaudible
const int MAX_DISTSQUARED = 71200;

 // how far something has to be in order to have a lower volume
const int MIN_DISTSQUARED = 35600;

const int NUM_SPSOUNDS = 22;
const int NUM_SOUNDS = 46;

const int SOUND_COUNT[] = {0, NUM_SPSOUNDS, NUM_SOUNDS};

const int MAX_LONG_SOUNDS = 3;
const int MAX_SHORT_SOUNDS = 12;
const int MAX_SOUNDS = MAX_LONG_SOUNDS + MAX_SHORT_SOUNDS;

const int NUM_PRESOUNDS = 1;

static AutoComPtr<IDirectSound> ds;
static AutoComPtr<IDirectSoundBuffer> sounds[NUM_SOUNDS];

// array of duplicated sound buffers for sounds that are currently playing
static AutoComPtr<IDirectSoundBuffer> playing[MAX_SOUNDS];

// array of boolean values which specify if the sounds are reversed or not
static bitset<NUM_SOUNDS> reversed;

static int sounds_loaded = SNDLOAD_NONE;

static FIXEDNUM sfxfreq;

static void LoadSounds(int type);
static int NextSoundSlot();

static int NextSoundSlot() {
  static int next_slot = 0;
  const int new_slot = next_slot++;

  next_slot %= MAX_SHORT_SOUNDS;

  if (playing[new_slot]) {
    DWORD sound_status;
    if (SUCCEEDED(playing[new_slot]->GetStatus(&sound_status))
        && (DSBSTATUS_PLAYING & sound_status)) {
      // we must use one of the long slots
      static int next_long_slot = MAX_SHORT_SOUNDS;
      const int new_long_slot = next_long_slot++;

      if (next_long_slot >= MAX_SOUNDS) {
        next_long_slot = MAX_SHORT_SOUNDS;
      }

      playing[new_long_slot] = playing[new_slot];
    }

    playing[new_slot].Reset();
  }

  return new_slot;
}



void SndInitialize(HWND hWnd) {
  AutoComPtr<IDirectSound> new_ds(CLSID_DirectSound, IID_IDirectSound);

  assert(!ds);

  if (SUCCEEDED(LogResult("Init DSound", new_ds->Initialize(0)))
      && SUCCEEDED(LogResult("Set DSound normal cooperative level",
          new_ds->SetCooperativeLevel(hWnd, DSSCL_NORMAL)))) {
    ds = new_ds;
  }

  sfxfreq = Fixed(1.0f);
}

void SndRelease() {
  logger << "Releasing Sound module" << endl;

  for (int i = 0; i < NUM_SOUNDS; i++) {
    logger << "Releasing sound " << i << endl;
    sounds[i].Reset();
  }

  for (int i = 0; i < MAX_SOUNDS; i++) {
    logger << "Releasing playing sound " << i << endl;
    playing[i].Reset();
  }

  ds.Reset();
  
  logger << "Done releasing Sound module" << endl;
}

void SndLoad(int type) {
  int next_num_sounds = SOUND_COUNT[type];
  int prev_num_sounds = SOUND_COUNT[sounds_loaded];
  logger << "LoadSounds called w/" << prev_num_sounds << " sounds loaded, ";
  logger << "caller wants " << next_num_sounds << " loaded" << endl;

  if (LogResult("Requires more sounds than loaded",
      next_num_sounds > prev_num_sounds)) {
    Resource sound_resource("DAT", "SFX");
    DWORD *sound_data = (DWORD *)sound_resource.GetPtr();

    for(int i = 0; i < NUM_PRESOUNDS+prev_num_sounds; i++) {
      // skip the first seven sounds; they aren't ours
      sound_data = (DWORD *)((BYTE *)sound_data+*sound_data)+1;
    }

    for(int i = prev_num_sounds; i < next_num_sounds; i++) {
      logger << "Loading sound " << i << endl;
      sounds[i] = CreateSBFromRawData(
          ds.Get(), (void *)(sound_data + 1), *sound_data,
          DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY,
          SOUNDRESOURCEFREQ, SOUNDRESOURCEBPS, 1);
      logger << "Clearing reversed bit for sound " << i << endl;
      reversed.reset(i);
      // skip the first seven sounds; they aren't ours
      sound_data = (DWORD *)((BYTE *)sound_data+*sound_data)+1;
    }
  } else if (LogResult("Requires fewer sounds than loaded",
      prev_num_sounds > next_num_sounds)) {
    for(int i = next_num_sounds; i < prev_num_sounds; i++) {
      logger << "Releasing sound " << i << endl;
      sounds[i].Reset();
    }
  }

  sounds_loaded = type;

  logger << "LoadSounds exitting" << endl;
}

AutoComPtr<IDirectSoundBuffer> SndSound(int index) {
  assert(index >= 0 && index < NUM_SOUNDS);
  return sounds[index];
}

AutoComPtr<IDirectSound> SndDirectSound() {return ds;}

void SndPlay(int index, FIXEDNUM freq_factor, bool reverse) {
  // will play a sound and changes its frequency based on freq_factor
  AutoComPtr<IDirectSoundBuffer> b2; // the duplicate of the sound
  int my_slot = NextSoundSlot();

  assert(index >= 0 && index < NUM_SOUNDS);

  // make sure we ever loaded this sound
  if (!sounds[index]) {
    return;
  }

  freq_factor = FixedMul(freq_factor, sfxfreq);

  if (SUCCEEDED(ds->DuplicateSoundBuffer(sounds[index].Get(),
                                         b2.GetPtrToPtr()))) {
    DWORD old_freq;

    if (SUCCEEDED(b2->GetFrequency(&old_freq))) {
      DWORD new_freq = FixedCnvFrom<long>(old_freq * freq_factor);

      // set our new frequency
      b2->SetFrequency(new_freq);
    }

    if (reverse != reversed.test(index)) {
      // reverse the contents of the sound buffer in order to play it
      //  backwards
      void *ptr1, *ptr2;
      DWORD ptr1_size, ptr2_size;
      DSBCAPS buffer;
      memset(&buffer, 0, sizeof(buffer));
      buffer.dwSize = sizeof(buffer);
      b2->GetCaps(&buffer);

      if (SUCCEEDED(b2->Lock(0, buffer.dwBufferBytes,
                             &ptr1, &ptr1_size,
                             &ptr2, &ptr2_size, 0))) {
        BYTE *p1, *p1_end, *p2, *p2_start;
        
        reversed.flip(index);

        // we have to reverse data that is in both ptr1 and ptr2
        // do this in four steps
        // if ptr1_size >= ptr2_size
        //  1. swap from end of ptr2 and beginning of ptr1 inward
        //     until one pointer is at the beginning of ptr2
        //  2. continue swapping, only within the first block
        // if ptr1_size < ptr2_size
        //  1. swap from end of ptr2 and the beginning of ptr1 inward
        //     until one pointer is at the end of ptr1
        //  2. continue swapping, only within the second block

        // get a pointer to the first byte in pointer one

        if (!ptr1) {
          p1 = (BYTE *)1;
          p1_end = 0;
        } else {
          p1 = (BYTE *)ptr1;
          p1_end = ((BYTE *)ptr1) + ptr1_size - 1;
        }

        // get a pointer to the last byte in pointer two

        if (!ptr2) {
          p2_start = (BYTE *)1;
          p2 = 0;
        } else {
          p2  = ((BYTE *)ptr2) + ptr2_size - 1;
          p2_start  = (BYTE *)ptr2;
        }

        // step one is very similar in both methods
        while (p2 >= p2_start && p1 <= p1_end) {
          swap (*p2, *p1);
          p1++;
          p2--;
        }

        // now branch off depending on size of ptr1 compared to ptr2
        if (ptr1_size < ptr2_size) {
          while (p2_start < p2) {
            swap(*p2_start, *p2);
            p2--;
            p2_start++;
          }
        } else {
          while (p1_end > p1) {
            swap(*p1, *p1_end);
            p1++;
            p1_end--;
          }
        }

        // now unlock the sound buffer
        b2->Unlock(ptr1,ptr1_size,ptr2,ptr2_size);
        b2->Play(0,0,0);
      }
    } else {
      b2->Play(0,0,0);
    }

    playing[my_slot] = b2;
  }
}

void SndPlay(int index, FIXEDNUM x_dist, FIXEDNUM y_dist) {
  int my_slot = NextSoundSlot();
  AutoComPtr<IDirectSoundBuffer> b2; // the duplicate of the sound
  long xd, yd;
  float factor;

  assert(index >= 0 && index < NUM_SOUNDS);

  // make sure we ever loaded this sound
  if (!sounds[index]) {
    return;
  }

  xd = FixedCnvFrom<long>(x_dist);
  yd = FixedCnvFrom<long>(y_dist);

  if (SUCCEEDED(ds->DuplicateSoundBuffer(sounds[index].Get(),
                                         b2.GetPtrToPtr()))) {
    DWORD freq;

    if (SUCCEEDED(b2->GetFrequency(&freq))) {
      freq = FixedCnvFrom<long>(freq * sfxfreq);

      // set our new frequency
      b2->SetFrequency(freq);
    }

    factor = (float)xd / (float)MAX_XDIST;
    if (factor > 1.0f) { // we were too far away
      factor = 1.0f;
    }
    factor *= 10000.0f;
    b2->SetPan((long)factor);

    factor = xd*xd + yd*yd - MIN_DISTSQUARED;
    if (factor < 0) {
      factor = 0;
    }
    factor /= (float)(MAX_DISTSQUARED);
    if (factor > 1.0f) { // we were too far away
      factor = 1.0f;
    }
    factor *= -10000.0f;

    b2->SetVolume((long)factor);

    b2->Play(0,0,0);

    // add playing sound to the list
    playing[my_slot] = b2;
  }
}

void SndSetPlaybackFrequency(FIXEDNUM factor) {sfxfreq = factor;}
