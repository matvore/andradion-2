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

// constants having to do with how many sounds we have and
//  what the indices of the sounds are
const int WAV_OKGOTIT = 6;
const int WAV_STEP = 7;
const int WAV_GUNNOISE = 8;
const int WAVSET_WEAPONS = 9;
const int WAVSET_ALIENHIT = 12;
const int WAVSET_ALIENDEATH = 15;
const int WAVSET_FIRSTNONALIEN = 18;
const int WAVSET_POINTLESS = 42;
const int WAV_BING = 45;
const int WAVSINASET = 3;

const DWORD SOUNDRESOURCEFREQ = 11025;
const DWORD SOUNDRESOURCEBPS = 8; // bits per sample of sounds

void SndInitialize(HWND hWnd);
void SndRelease();

enum {SNDLOAD_NONE, SNDLOAD_SP, SNDLOAD_MP};
void SndLoad(int load_type);

AutoComPtr<IDirectSoundBuffer> SndSound(int index);
AutoComPtr<IDirectSound> SndDirectSound();
void SndPlay(int index, FIXEDNUM freq_factor, bool reverse);
void SndPlay(int index, FIXEDNUM x_dist, FIXEDNUM y_dist);
void SndSetPlaybackFrequency(FIXEDNUM factor);
