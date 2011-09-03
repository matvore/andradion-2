#include "StdAfx.h"

// module that loads raw sound data into sound buffers

static int FillSoundBuffer(LPDIRECTSOUNDBUFFER& target,const void *data,DWORD bytes) {
		
  BYTE *audio_ptr_1 = NULL;
  BYTE *audio_ptr_2 = NULL;

  DWORD audio_length_1 = 0; // length of the first write buffer
  DWORD audio_length_2 = 0; // length of the second write buffer

  const BYTE *snd_buffer = (const BYTE *)data;

  if(FAILED((*target).Lock(
			   0,bytes,
			   (void **)&audio_ptr_1,
			   &audio_length_1,
			   (void **)&audio_ptr_2,
			   &audio_length_2,
			   DSBLOCK_FROMWRITECURSOR
			   )))
    {
      // failed
      target = NULL;

      return 1; // failed
    }

  memcpy(
	 (void *)audio_ptr_1,(const void *)snd_buffer,audio_length_1
	 );

  memcpy(
	 (void *)audio_ptr_2,(const void *)(snd_buffer+audio_length_1),audio_length_2
	 );

  // STEP 15: UNLOCK THE BUFFER AND DELETE THE TEMPORARY SOUND DATA
  (*target).Unlock(audio_ptr_1,audio_length_1,audio_ptr_2,audio_length_2);

  return 0;
}

// this function returns zero on success
static int CreateSB(LPDIRECTSOUND ds,LPDIRECTSOUNDBUFFER *target,DWORD desc_flags,DWORD frequency,DWORD bits_per_sample,DWORD channels,DWORD buffer_size)
{
  if(NULL == ds)
    {
      // direct sound is not available
      return 2;
    }
  if(0 == channels || 0 == bits_per_sample || 0 == frequency)
    {
      return 1; // invalid parameters
    }

  // copy wave format structure
  WAVEFORMATEX wfm;
  wfm.cbSize = 0;
  wfm.nAvgBytesPerSec = DWORD(wfm.nBlockAlign = WORD(channels * bits_per_sample / 8));
  wfm.nAvgBytesPerSec *= frequency;
  wfm.nChannels = (WORD)channels;
  wfm.nSamplesPerSec = frequency;
  wfm.wBitsPerSample = (WORD)bits_per_sample;
  wfm.wFormatTag=WAVE_FORMAT_PCM;

  DSBUFFERDESC dsbd;

  dsbd.dwBufferBytes = buffer_size;
  dsbd.dwFlags = desc_flags;
  dsbd.dwReserved = 0;
  dsbd.dwSize = sizeof(dsbd);
  dsbd.lpwfxFormat = &wfm;
		
  //  CREATE THE SOUND BUFFER
  HRESULT hr;

  MemoryAllocFunction(
		      hr = ds->CreateSoundBuffer(&dsbd,target,NULL),
		      buffer_size,
		      FAILED(hr)
		      );
  return 0;
}

int CreateSBFromRawFile
(
 LPDIRECTSOUND ds,
 LPDIRECTSOUNDBUFFER *target,
 const TCHAR *path,
 DWORD desc_flags,
 DWORD frequency,
 DWORD bits_per_sample,
 DWORD channels
 )
{
  *target = NULL;
  HANDLE raw_file = CreateFile(path,GENERIC_READ,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
  DWORD buffer_size = GetFileSize(raw_file,NULL);
  if(0 == buffer_size || NULL == raw_file || INVALID_HANDLE_VALUE == raw_file)
    {
      return 3; // failure to open file and/or get file size
    }

  int res = CreateSB(ds,target,desc_flags,frequency,bits_per_sample,channels,buffer_size);

  if(0 != res)
    {
      // return value was non-zero, there was an error
      CloseHandle(raw_file);
      return res;
    }

  BYTE *snd_buffer  = new BYTE[buffer_size];

  DWORD read;
  // read sound data
  ReadFile(raw_file,(void *)snd_buffer,buffer_size,&read,NULL);

  CloseHandle(raw_file);

  // COPY THE DATA TO THE SOUND BUFFER
  res = FillSoundBuffer(*target,(const void *)snd_buffer,buffer_size);

  delete snd_buffer;

  return res;
}

int CreateSBFromRawResource
(
 LPDIRECTSOUND ds,
 LPDIRECTSOUNDBUFFER *target,
 const TCHAR *res_name,
 const TCHAR *res_type,
 HMODULE res_mod,
 WORD res_lang,
 DWORD desc_flags,
 DWORD frequency,
 DWORD bits_per_sample,
 DWORD channels
 )
{
  *target = NULL;
  HRSRC res_handle;
  HGLOBAL data_handle;

  // here we get a pointer to resource data
  DWORD *data_ptr = (DWORD *)LockResource(data_handle = LoadResource(res_mod,res_handle = FindResourceEx(res_mod,res_type,res_name,res_lang)));
  if(NULL == data_ptr)
    {
      // there was a failure
      return 3;
    }

  DWORD buffer_size = SizeofResource(res_mod,res_handle);

  int res = CreateSB(ds,target,desc_flags,frequency,bits_per_sample,channels,buffer_size);
  if(0 != res)
    {
      // there was a failure
      return res;
    }

  // lock, fill, and unlock the sound buffer
  res = FillSoundBuffer(*target,(void *)data_ptr,buffer_size);

  // they say it's unnecessary, but I don't care . . .
  FreeResource(data_handle);

  return res;
}

int CreateSBFromRawData
(
 LPDIRECTSOUND ds,
 LPDIRECTSOUNDBUFFER *target,
 void *data,
 DWORD size,
 DWORD desc_flags,
 DWORD frequency,
 DWORD bits_per_sample,
 DWORD channels
 )
{
  *target = NULL;
  int res = CreateSB(ds,target,desc_flags,frequency,bits_per_sample,channels,size);

  if(0 != res)
    {
      return res;
    }

  return FillSoundBuffer(*target,data,size);	  
}
