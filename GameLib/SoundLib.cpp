// RobotSound.cpp
//  All this does is play sounds and interfaces simply with DirectSound.
//   Doesn't rely on other libraries, however, of course, the other libraries
//   and screen classes are using it
#include "SoundLib.h" // this file includes all necessary headers

// direct music uses sound, so start this system first and then dm, passing
//  this so it knows where direct sound is:
static LPDIRECTSOUND dsound_int=NULL; // our interface with direct sound

namespace SoundLib {

	// BORROWED FUNCTIONS USED ONLY IN THIS MODULE /////////////////////////////////

	static HRESULT ReadMMIO(HMMIO hmmioIn, MMCKINFO* pckInRIFF, WAVEFORMATEX** ppwfxInfo ) {
		MMCKINFO        ckIn;           // chunk info. for general use.
		PCMWAVEFORMAT   pcmWaveFormat;  // Temp PCM structure to load in.       

		*ppwfxInfo = NULL;

		if( ( 0 != mmioDescend( hmmioIn, pckInRIFF, NULL, 0 ) ) )
			return E_FAIL;

		if( (pckInRIFF->ckid != FOURCC_RIFF) ||
			(pckInRIFF->fccType != mmioFOURCC('W', 'A', 'V', 'E') ) )
			return E_FAIL;

		// Search the input file for for the 'fmt ' chunk.
		ckIn.ckid = mmioFOURCC('f', 'm', 't', ' ');
		if( 0 != mmioDescend(hmmioIn, &ckIn, pckInRIFF, MMIO_FINDCHUNK) )
			return E_FAIL;

		// Expect the 'fmt' chunk to be at least as large as <PCMWAVEFORMAT>;
		// if there are extra parameters at the end, we'll ignore them
		if( ckIn.cksize < (LONG) sizeof(PCMWAVEFORMAT) )
			return E_FAIL;

	    // Read the 'fmt ' chunk into <pcmWaveFormat>.
		if( mmioRead( hmmioIn, (HPSTR) &pcmWaveFormat, 
			          sizeof(pcmWaveFormat)) != sizeof(pcmWaveFormat) )
			return E_FAIL;

		// Allocate the waveformatex, but if its not pcm format, read the next
		// word, and thats how many extra bytes to allocate.
		if( pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM )
		{
			if( NULL == ( *ppwfxInfo = new WAVEFORMATEX ) )
				return E_FAIL;

	        // Copy the bytes from the pcm structure to the waveformatex structure
		    memcpy( *ppwfxInfo, &pcmWaveFormat, sizeof(pcmWaveFormat) );
			(*ppwfxInfo)->cbSize = 0;
		}
		else
		{
	        // Read in length of extra bytes.
		    WORD cbExtraBytes = 0L;
			if( mmioRead( hmmioIn, (CHAR*)&cbExtraBytes, sizeof(WORD)) != sizeof(WORD) )
				return E_FAIL;

	        *ppwfxInfo = (WAVEFORMATEX*)new CHAR[ sizeof(WAVEFORMATEX) + cbExtraBytes ];
		    if( NULL == *ppwfxInfo )
			    return E_FAIL;

			// Copy the bytes from the pcm structure to the waveformatex structure
			memcpy( *ppwfxInfo, &pcmWaveFormat, sizeof(pcmWaveFormat) );
			(*ppwfxInfo)->cbSize = cbExtraBytes;

	        // Now, read those extra bytes into the structure, if cbExtraAlloc != 0.
		    if( mmioRead( hmmioIn, (CHAR*)(((BYTE*)&((*ppwfxInfo)->cbSize))+sizeof(WORD)),
			              cbExtraBytes ) != cbExtraBytes )
			{
				delete *ppwfxInfo;
				*ppwfxInfo = NULL;
				return E_FAIL;
			}
		}

	    // Ascend the input file out of the 'fmt ' chunk.
		if( 0 != mmioAscend( hmmioIn, &ckIn, 0 ) )
		{
			delete *ppwfxInfo;
			*ppwfxInfo = NULL;
			return E_FAIL;
		}

	    return S_OK;
	}

	static HRESULT WaveOpenFile( CHAR* strFileName, HMMIO* phmmioIn, WAVEFORMATEX** ppwfxInfo,
		MMCKINFO* pckInRIFF ) {
		HRESULT hr;
		HMMIO   hmmioIn = NULL;
    
		if( NULL == ( hmmioIn = mmioOpen( strFileName, NULL, MMIO_ALLOCBUF|MMIO_READ ) ) )
			return E_FAIL;

		if( FAILED( hr = ReadMMIO( hmmioIn, pckInRIFF, ppwfxInfo ) ) )
		{
			mmioClose( hmmioIn, 0 );
			return hr;
		}

		*phmmioIn = hmmioIn;
	
		return S_OK;
	}

	static HRESULT WaveStartDataRead( HMMIO* phmmioIn, MMCKINFO* pckIn,
		MMCKINFO* pckInRIFF ) {
		// Seek to the data
		if( -1 == mmioSeek( *phmmioIn, pckInRIFF->dwDataOffset + sizeof(FOURCC),
			                SEEK_SET ) )
			return E_FAIL;

	    // Search the input file for for the 'data' chunk.
		pckIn->ckid = mmioFOURCC('d', 'a', 't', 'a');
		if( 0 != mmioDescend( *phmmioIn, pckIn, pckInRIFF, MMIO_FINDCHUNK ) )
			return E_FAIL;

	    return S_OK;
	}

	static HRESULT WaveReadFile( HMMIO hmmioIn, UINT cbRead, BYTE* pbDest,
		                  MMCKINFO* pckIn, UINT* cbActualRead )
	{
		MMIOINFO mmioinfoIn;         // current status of <hmmioIn>

		*cbActualRead = 0;

		if( 0 != mmioGetInfo( hmmioIn, &mmioinfoIn, 0 ) )
			return E_FAIL;
                
	    UINT cbDataIn = cbRead;
		if( cbDataIn > pckIn->cksize ) 
			cbDataIn = pckIn->cksize;       

	    pckIn->cksize -= cbDataIn;
    
		for( DWORD cT = 0; cT < cbDataIn; cT++ )
		{
	        // Copy the bytes from the io to the buffer.
		    if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead )
			{
	            if( 0 != mmioAdvance( hmmioIn, &mmioinfoIn, MMIO_READ ) )
		            return E_FAIL;

			    if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead )
				    return E_FAIL;
	        }

		    // Actual copy.
			*((BYTE*)pbDest+cT) = *((BYTE*)mmioinfoIn.pchNext);
			mmioinfoIn.pchNext++;
		}

		if( 0 != mmioSetInfo( hmmioIn, &mmioinfoIn, 0 ) )
			return E_FAIL;

		*cbActualRead = cbDataIn;
		return S_OK;
	}

// END OF BORROWED FUNCTIONS USED ONLY IN THIS MODULE //////////////////////////

	LPDIRECTSOUND Direct_Sound(void) {return dsound_int;}

	int Init(HWND w) {
		// make sure dsound isn't already up and running:
		if(dsound_int) return 3;

		// let's try to make the dsound object:
		CoInitialize(NULL);
		if
		(
			FAILED
			(
				CoCreateInstance
				(
					CLSID_DirectSound,
					NULL, 
	                CLSCTX_INPROC_SERVER,
		            IID_IDirectSound,
			        (void **)&dsound_int
				)
			)
			||
			FAILED(dsound_int->Initialize(NULL))
		)
		{
			CoUninitialize();
			if(NULL != dsound_int)
			{
				dsound_int->Release();
				dsound_int = NULL;
			}
			return 2;
		}

		// now set the coop level:
		if(FAILED(dsound_int->SetCooperativeLevel(w,DSSCL_NORMAL))) {
			// on error release the object and return with error:
			CoUninitialize();
			dsound_int->Release(); // fancy-like, reset the interface pointer
			dsound_int = NULL; 
			return 1;
		}

		// we made it here so everything is initialized and ready to go so . . .
		return 0; // return success
	}

	void Uninit(void) {
		// we just gotta release the interface here:
		if(dsound_int) {
			dsound_int->Release();
			dsound_int = NULL;
		}
		CoUninitialize();
	}

	int Create_SOB(SOB *s,const char* file_name) {
		const int MAXFILELEN = 256;

		// this function, which does nothing but load an 8-bit mono wave file
		//  into a secondary sound buffer (named 's'), is the most complicated in 
		//  the sound library

		HMMIO hwav; // handle to the .wav file
		// parent and child chunks, whatever the hell that means, I'm just copying
		//  a function out of this book I have here . . .
		MMCKINFO pckInRIFF,pckIn;
		UINT x,y;

		WAVEFORMATEX *wfmtx; // wave format structure
		DSBUFFERDESC dsound_buffer_desc_sob; // buffer desc to help create the SOB

		UCHAR *snd_buffer, // temporary sound buffer to hold voc data
			  *audio_ptr_one=NULL, // pointers into the non-linear sound buffer
			  *audio_ptr_two=NULL;

		// length of the first and second write buffers:
		DWORD audio_length_one=0,audio_length_two=0;

		char file_name_2[MAXFILELEN+1]; // used to pass non-const strings
		assert(strlen(file_name) <= MAXFILELEN);
		memcpy((void *)file_name_2,(const void *)file_name,strlen(file_name)+1);

		// first of all, make sure the dsound interface is
		//  already on:
		if(!dsound_int) return 7; // return if it isn't on

		// open the sound file:
		if(WaveOpenFile(file_name_2,&hwav,&wfmtx,&pckInRIFF) != S_OK)
			// on error,
			return 6; // return failure

		// prepare for reading:
		if(WaveStartDataRead(&hwav,&pckIn,&pckInRIFF) != S_OK) {
			// on error, close the newly openned file and return with error
			mmioClose( hwav, 0 );
			return 5;
		}

		// now all we gotta do is read the data in and set up the directsound buffer
		snd_buffer = (UCHAR *)malloc(y=pckIn.cksize);
	
		if(snd_buffer == NULL) { // we couldn't allocate enough memory , so
			mmioClose(hwav,0); // close the file
			return 4; // return with error
		}

		// now read the wave data
		if(WaveReadFile(hwav,pckIn.cksize,snd_buffer,&pckIn,&x) != S_OK) {
			// on failure, do this:
			mmioClose(hwav, 0);
			free(snd_buffer);
			return 3;
		}

		// and close the file:
		mmioClose( hwav, 0 );

		// let's fill up the description of the new SOB buffer
		memset((void *)&dsound_buffer_desc_sob,0,sizeof(dsound_buffer_desc_sob));
		dsound_buffer_desc_sob.dwSize = sizeof(dsound_buffer_desc_sob);
		dsound_buffer_desc_sob.dwFlags = DSBCAPS_STATIC | DSBCAPS_LOCSOFTWARE;
		dsound_buffer_desc_sob.dwBufferBytes = y;//;pckIn.cksize;
		dsound_buffer_desc_sob.lpwfxFormat = wfmtx;

		// now we will try to create the sound buffer:
		if(FAILED(dsound_int->CreateSoundBuffer(&dsound_buffer_desc_sob,s,NULL))) {
			free(snd_buffer);
			return 2;
		}

		delete wfmtx;

		// now we gotta lock the new buffer in order to copy the data into it:
		if(FAILED((**s).Lock(
			0,
			y,
			(void **)&audio_ptr_one,
			&audio_length_one,
			(void **)&audio_ptr_two,
			&audio_length_two,
			DSBLOCK_FROMWRITECURSOR))) {
			// we got an error, so release the new buffer
			(**s).Release();
		
			// and return with error
			return 1;
		}
 
		// copy first section of circular buffer
		memcpy(audio_ptr_one, snd_buffer, audio_length_one);
	
		// and the second section
		memcpy(audio_ptr_two, snd_buffer+audio_length_one, audio_length_two);

		// unlock the buffer
		while(FAILED((**s).Unlock(
			audio_ptr_one,
			audio_length_one,
			audio_ptr_two,
			audio_length_two)));

		// release the temporary buffer
		free(snd_buffer);

		// return success
		return 0;
	}

	int Create_SOB(SOB *dest_s,SOB source_s) {
		// first of all, make sure the dsound interface is
		//  already on:
		if(!dsound_int) return 2; // return with error if it isn't on

		if(FAILED(dsound_int->DuplicateSoundBuffer(source_s,dest_s)))
			return 1;

		// if we got here, then we won, so return zero for no error
		return 0;
	}

	void Destroy_SOB(SOB s) {
		s->Release(); // just release it
	}
}