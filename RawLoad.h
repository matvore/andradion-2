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

// the desc_flags parameter = DSBUFFERDESC::dwFlags passed to the sound buffer
//  creation function
IDirectSoundBuffer *CreateSBFromRawData
(IDirectSound *ds, void *data, DWORD size, DWORD desc_flags,
 DWORD frequency, DWORD bits_per_sample, DWORD channels);

