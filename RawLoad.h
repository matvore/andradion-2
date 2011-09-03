// the desc_flags parameter = DSBUFFERDESC::dwFlags passed to the sound buffer
//  creation function
IDirectSoundBuffer *CreateSBFromRawData
(IDirectSound *ds, void *data, DWORD size, DWORD desc_flags,
 DWORD frequency, DWORD bits_per_sample, DWORD channels);

