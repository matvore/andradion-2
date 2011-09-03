IDirectMusicPerformance *Performance();
IDirectMusicSegment *Segment();
IDirectMusicLoader *Loader();
double DefaultTempo(); // returns original tempo of current segment

const int MUSICLIBERR_OUTOFMEMORY = 4;
const int MUSICLIBERR_DMUSICNOTAVAIL = 3;
const int MUSICLIBERR_COMNOTAVAIL  = 2;
const int MUSICLIBERR_PORTNOTAVAIL = 1;

// returns non-zero on failure
int MusicInit(HWND w, IDirectSound *ds) throw(std::bad_alloc); 

void MusicUninit();

// allows looping.  Call this function to start playing a midi from resource
int MusicPlay(bool loop,
	      const char *res_name,
	      const char *res_type,
	      HMODULE res_mod = NULL,
	      WORD res_lang = MAKELANGID(LANG_NEUTRAL,
                                         SUBLANG_NEUTRAL));

void MusicStop();
void SetTempo(double tempo);

