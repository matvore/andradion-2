#include <windows.h>
#include <mmsystem.h>
#include <iostream>
using std::cin;
using std::cout;

	int CreateRawFromWaveFile(char *dest,char *src)
	{
		// STEP 2: SETUP CHUNK INFO STRUCTURE
		
		MMCKINFO parent; // parent chunk
		parent.ckid = (FOURCC)0;
		parent.cksize = 0;
		parent.fccType = (FOURCC)0;
		parent.dwDataOffset = 0;
		parent.dwFlags = 0;
		
		MMCKINFO child = parent; // child chunk

		// STEP 3: OPEN THE WAVE FILE

		HMMIO hwav = // handle of the wave file
			mmioOpen(src,NULL,MMIO_READ | MMIO_ALLOCBUF);

		if(NULL == hwav)
		{
			return 9; // failed
		}

		// STEP 4: DESCEND INTO THE RIFF

		parent.fccType = mmioFOURCC('W','A','V','E');

		if(0 != mmioDescend(hwav,&parent,NULL,MMIO_FINDRIFF))
		{
			// failed
			mmioClose(hwav,0);

			return 8; // failed
		}

		// STEP 5: DESCEND INTO THE WAVEFMT

		child.ckid = mmioFOURCC('f','m','t',' ');

		if(0 != mmioDescend(hwav,&child,&parent,0))
		{
			// failed
			mmioClose(hwav,0);

			return 7; // failed
		}

		// STEP 6: READ WAVEFORMATEX
		WAVEFORMATEX wfmtx; // wave format struct

		if(sizeof(wfmtx) != mmioRead(hwav,(HPSTR)&wfmtx,sizeof(wfmtx)))
		{
			// failed
			mmioClose(hwav,0);

			return 6; // failed
		}

		// STEP 7: VERIFY CORRECT FORMAT
		if(WAVE_FORMAT_PCM != wfmtx.wFormatTag)
		{
			// failed
			mmioClose(hwav,0);

			return 5; // failed
		}
		
		// STEP 8: ASCEND TO THE DATA CHUNK
		if(0 != mmioAscend(hwav,&child,0))
		{
			// failed
			mmioClose(hwav,0);

			return 4;
		}

		// STEP 9: DESCEND TO THE DATA CHUNK
		child.ckid = mmioFOURCC('d','a','t','a');

		if(0 != mmioDescend(hwav,&child,&parent,MMIO_FINDCHUNK))
		{
			// failed
			mmioClose(hwav,0);

			return 3; // failed
		}
		
		// STEP 10: READ THE SOUND DATA

		BYTE *snd_buffer = new BYTE[child.cksize];
		if(NULL == snd_buffer)
		{
			// failed
			mmioClose(hwav,0);

			return 2; // failed
		}

		mmioRead(hwav,(HPSTR)snd_buffer,child.cksize);

		// STEP 11: CLOSE THE WAV FILE

		mmioClose(hwav,0);
		
		// at this point, the raw data is in snd_buffer, and the size is child.cksize
		HANDLE raw_file = CreateFile(dest,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

		if(NULL == raw_file || INVALID_HANDLE_VALUE == raw_file)
		{
			delete snd_buffer;
			return 1; // failed
		}

		DWORD written;
		WriteFile(raw_file,(const void *)snd_buffer,child.cksize,&written,NULL);

		CloseHandle(raw_file);

		// all done, free sound data buffer
		delete snd_buffer;

		return 0;
}

int main(int argc,char *argv[])
{
	if(3 == argc)
	{
		cout << "Returned with code: " <<  CreateRawFromWaveFile(argv[1],argv[2])<< "\n";
		cout << "Zero means no error\n";
		return 0;
	}

	cout << "Enter .WAV source file (enter '*' to quit): ";
	char source[256];
	cin.getline(source,256);
	if(0 == strcmp(source,"*"))
	{
		return 0;
	}

	cout << "Enter RAW destination file (enter '*' to cancel): ";
	char dest[256];
	cin.getline(dest,256);
	if(0 != strcmp(dest,"*"))
	{
		cout << "Returned with code: " << CreateRawFromWaveFile(dest,source) << "\n";
		cout << "Zero means no error\n";
	}

	return main(0,NULL);
}