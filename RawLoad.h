// returns zero on success
int CreateSBFromRawFile(
 LPDIRECTSOUND ds,
 LPDIRECTSOUNDBUFFER *target,
 const TCHAR *path,
 DWORD desc_flags,
 DWORD frequency,
 DWORD bits_per_sample,
 DWORD channels);

// returns zero on success
int CreateSBFromRawResource(
 LPDIRECTSOUND ds,
 LPDIRECTSOUNDBUFFER *target,
 const TCHAR *res_name,
 const TCHAR *res_type,
 HMODULE res_mod,
 WORD res_lang,
 DWORD desc_flags,
 DWORD frequency,
 DWORD bits_per_sample,
 DWORD channels);

int CreateSBFromRawData(
 LPDIRECTSOUND ds,
 LPDIRECTSOUNDBUFFER *target,
 void *data,
 DWORD size,
 DWORD desc_flags,
 DWORD frequency,
 DWORD bits_per_sample,
 DWORD channels);

// the desc_flags parameter = DSBUFFERDESC::dwFlags passed to the sound buffer
//  creation function
