// Glue.cpp: implementation of the CGlue class.
//  for thorough summarizations of any function in this
//  module, see the CGlue header file
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Fixed.h"
#include "SharedConstants.h"
#include "Weather.h"
#include "Palette.h"
#include "Fire.h"
#include "Character.h"
#include "Glue.h"
#include "Deeds.h"
#include "LevelEnd.h"
#include "PowerUp.h"
#include "Net.h"
#include "GammaEffects.h"
#include "resource.h"
#include "BitArray2D.h"

// Comment the next two lines to allow this module to log
#define WriteLog(x)
#undef LogArg
#define LogArg(x)
#undef TryAndReport
#define TryAndReport(x) x

// define WINDOWEDMODE to make Andradion 2 take up only a small amount
// of space in the corner of the screen.  For debugging purposes
// only.  Use only under 8 bit desktop mode
// #define WINDOWEDMODE

// here is a long list of all the external
//  stuff we are using.  Instead of typing
//  using namespace NGameLib2,
//  each symbol is specified individually
//  this is so we can keep track of what
//  symbols are used in this module
using NGameLib2::NUM_TRANSITIONS;
using NGameLib2::MusicPlay;
using NGameLib2::MusicInit;
using NGameLib2::MusicUninit;
using NGameLib2::MusicStop;
using NGameLib2::CreateSBFromRawData;
using NGameLib2::CCompactMap;
using NGameLib2::matrix;
using NGameLib2::SCAPS_NONE;
using NGameLib2::SCAPS_ALL;
using NGameLib2::CTimer;
using NGameLib2::CColor256;
using NGameLib2::DefaultTempo;
using NGameLib2::SetTempo;
using NGameLib2::tstring;
using NGameLib2::CMenu;
using std::pair;

#ifdef BORLAND
#define __max max
#define __min min
using std::max;
using std::min;
#endif

// this template returns the number of bitmaps for any type of loading: 0->0, 1->51, 2->91
template<DWORD x> struct BitmapCountT
{
  enum {Val = (-11 * x + 113) * x / 2};
};

static int BitmapCount(int x)
{
  return (-11 * x + 113) * x / 2;
}


// constants used in this module
//  these constants should be self-explanatory
//  some of the most mundane things are defined so
//  that we can have powerful control over small things
//  we may want to change, while it makes the code more
//  constant and containing less literals
const int XCOOR_ACCOMPLISHMENTTEXT = 5;
const int YCOOR_ACCOMPLISHMENTTEXT = 183;
const COLORREF COLOR_ACCOMPLISHMENTTEXT = RGB(0,128,0);
const TCHAR SYNCRATE_FORMAT[] = TEXT("%d");
const int MAX_TIMERCHAR = 9;
const int MAX_TIMERSECONDS =  Fixed(99 * 60 + 59);
const int FRAMERATE_BUFFERLEN = 10;
const int NUM_PRESOUNDS = 7;

// constants for sound volume and balance calculation are not in floating-point,
//  because they wouldn't fit
const	int MAX_XDIST = 100; // how far something can be on x axis before it is inaudible to one channel
const int MAX_DISTSQUARED = 71200; // how far something can be squared before it is totally inaudible
const int MIN_DISTSQUARED = 35600; // how far something has to be in order to have a lower volume
const int ALWAYSPUSH = ~(1 << 9); // use slightly different collision detection in ufo level
const int MAX_STRINGLEN = 150;
const int LONG_STRINGLEN = 500;
const int LEVEL_NONE = -1;
const int HERO = -1;
const int POINTSFORSUICIDE = -1;
const int POINTSFORHOMICIDE = 1;
const int MAXSCORE = 9999;
const int MINSCORE = -9999;
const int SCORELEN = 11;
const DWORD MINFRAMESTOKEEPMESSAGE = 20;
const DWORD FRAMESTODISPLAYMSG = 120;
const int DEMOCHAR_X = 288; // coordinates of the character to display durring selection
const int DEMOCHAR_Y = 165;
const float DEMOCHAR_SECSTOCHANGEDIR = 1.0f;
const float DEMOCHAR_SECSTOSTEP = 0.1f;
const int DEMOCHAR_X2 = 288; // coordinates to display after selection
const int DEMOCHAR_Y2 = 125;
const DWORD LEVELLOAD_OKAY = 0;
const DWORD LEVELLOAD_RELOADFLESH = 1; // character positions, states, powerup states, etc
const DWORD LEVELLOAD_RELOADBONE = 2; // GLUbitmaps reload
const DWORD LEVELLOAD_RELOADALL = 3; // everything needs to be reloaded
const int MENUACTION_ESCAPE = 0;
const int MENUACTION_RETURN = 1;
const int SCORE_X_OFFSET = -10;

// some enumerations:
enum
{
  GLUESTATE_UNINITIALIZED,GLUESTATE_INTRODUCTION,GLUESTATE_MAINMENU,GLUESTATE_LEVELSELECT,GLUESTATE_DIFFICULTYSELECT,GLUESTATE_GAME,
  GLUESTATE_CONFIRMQUIT,GLUESTATE_ENTERNAME,GLUESTATE_PICKCHARACTER,GLUESTATE_SELECTCONNECTIONMETHOD,GLUESTATE_PICKGAME
};
enum {GLUEHSM_NOTHING,GLUEHSM_NEWPLAYER,GLUEHSM_SESSIONLOST};
enum {MAINMENU_SP,MAINMENU_MP,MAINMENU_QUIT,NUM_MAINMENUITEMS};
enum {NO,YES,NUM_YESNOMENUITEMS};
enum {RESOURCELOAD_NONE,RESOURCELOAD_SP,RESOURCELOAD_MP,RESOURCELOAD_RELOAD};

const COLORREF COLOR_HEADING = RGB(0,128,0);
const COLORREF COLOR_UNSELECTED = RGB(255,128,128);
const COLORREF COLOR_SELECTED = RGB(0,255,255);
const COLORREF COLOR_SHADOW = RGB(128,0,0);
const int SHADOW_OFFSET = 1;
const DWORD DEFAULT_SYNCRATE = 30;
const int MPDIFFICULTY = 2; // GLUdifficulty GLUlevel used for multiplayer sessions
const int NUM_MPGAMESLOTS = 12; // number of game slots on multiplayer game selects
const int MAX_PLAYERS = 16; // maximum amount of players in a network game
const int LOADINGMETER_MINHEIGHT = 9;
const int LOADINGMETER_MAXHEIGHT = 15;
const FIXEDNUM FREQFACTOR_OKGOTITBACKWARDS = Fixed(1.2f);
const FIXEDNUM FREQFACTOR_OKGOTITNORMAL = Fixed(0.80f);
const int MAXLEN_SECTORCOOR = 5;
const TCHAR MIDI_RESOURCE_TYPE[] = TEXT("MIDI");
const TCHAR CMP_RESOURCE_TYPE[] = TEXT("CMP");
const TCHAR ONE_NUMBER_FORMAT[] = TEXT("%d");
const TCHAR TWO_NUMBERS_FORMAT[] = TEXT("%d/%d");
const TCHAR LEVELS_LIB_FILE[] = TEXT("LevelsLib.dat");
const TCHAR LEVELS_LIB_FILE_2[] = TEXT("LevelsLib2.dat");
const TCHAR SCREENSHOTFN[] = TEXT("Level %d-%dmin%dsec.bmp");
const int FIRSTLIB2LEVEL = 7;
const int SCORE_AND_TIMER_Y = 184;
const int WAV_PAUSE = 12;
const float SECONDS_BETWEEN_PAUSE_FLIPS = 0.5f;
const BYTE PAUSE_KEY = 'P';
const int FRAMESTOFLASHSCORE = 150;
const int FONTWIDTH = 8;
const int CACHED_SECTORS = 9;
const int CACHED_SECTORS_HIGH = 3;
const int CACHED_SECTORS_WIDE = 3;
const int FONTHEIGHT = 16;
const double YOUWINTUNE_LENGTH = 4.0f;

const FIXEDNUM TIMER_INC_PER_FRAME = Fixed(0.04f);

// variables accessable from other modules:
int                    GLUdifficulty;
int                    GLUchar_demo_direction;
CBob                  *GLUbitmaps[BitmapCountT<RESOURCELOAD_MP>::Val];
LPDIRECTDRAWCLIPPER    GLUclipper = NULL;
int                    GLUlevel;
VCTR_CHARACTER         GLUenemies; // remote players or local aliens
CFire                  GLUfires[MAX_FIRES];
CCharacter             GLUhero; // us
BYTE                   GLUkeyb[KBBUFFERSIZE];
tstring                GLUname; // our GLUname (only used in mp)
vector<CPowerUp>       GLUpowerups;
DWORD                  GLUsync_rate;
FIXEDNUM               GLUcenter_screen_y;
FIXEDNUM               GLUcenter_screen_x;
MSET_PCHAR             GLUdrawing_order;

struct TSector
{
  CCompactMap upper_cell;
  CCompactMap lower_cell;

  SET_INT GLUpowerups;
  SET_INT GLUenemies;
  SET_INT level_ends;
};

template<class c,class f> inline c Range(c minimum,c maximum,f progress)
{
  // this function will simply take the progress var,
  //  and return a value from minimum to maximum, where it would
  //  return minimum if progress == 0, and maximum if progress == 1
  //  and anything inbetween would be a linear function derived
  //  from that range, or however you'd say something like that . . .
  return c(f(maximum-minimum) * progress) + minimum;
}

#ifdef _DEBUG
static HGDIOBJ profiler_font;
const int PROFILER_FONT_SIZE = 8;
#endif

static HMODULE level_lib_mod;
static HMODULE level_lib_mod_2;
static tstring last_music;
static bool disable_music = false;
static FIXEDNUM max_center_screen_y;
static FIXEDNUM max_center_screen_x;
static BYTE font_data[4096];

// this is the score in multiplayer or singleplayer games.  If you are singleplayer and the score is at max, then the last character
//  is a countdown to when the flashing should stop
static tstring score;

static int score_print_x; // coordinates of score (negated when full score is obtained which signifies flashing colors)
static int msg_x; // x-coordinate of the current message (used so we don't have to recalc each time)
static HWND hWnd;
static HINSTANCE hInstance;
static bool do_transition = false; // is true if the next time we flip we should do a transition

static int state = GLUESTATE_UNINITIALIZED;

static int bitmaps_loaded;
static int sounds_loaded;

// array of boolean values which specify if the sounds are reversed or not
static BitArray            reversed(NUM_SOUNDS);

// array of original sound buffers
static IDirectSoundBuffer *sounds  [NUM_SOUNDS]; 

// array of duplicated sound buffers for sounds that are currently playing
static IDirectSoundBuffer *playing [MAX_SOUNDS]; 

static CGraphics *gr; // graphics class
	
static BitArray2D *walking_data = NULL; // on = inside, off = outside
static matrix<TSector> sectors;
	
static LPDIRECTSOUND ds; // directsound
static LPDIRECTINPUT di; // directinput
static LPDIRECTINPUTDEVICE did; // keyboard
static CMenu *m;
static vector<CLevelEnd> lends;

static int model; // character we are playing as

static CTimer char_demo_stepper;
static CTimer char_demo_direction_changer;

static DWORD level_loaded; // indicates validity of loaded data

static tstring message; // current message on  the screen
static DWORD frames_for_current_message; // how long the current message has been up



// kind-of constant strings (only initialized, never changed, but loaded with tstring table
//  so cannot be declared as static const or const)
static tstring KILLEDYOURSELF;
static tstring KILLED;
static tstring YOU;
static tstring YOUKILLED;
static tstring KILLEDTHEMSELVES;
static tstring SPKILLED; // tstring displayed when killed in single player

static CSurfaceLock256 back_buffer_lock;

static vector<POINT> possible_starting_spots;

static FIXEDNUM since_start;

static int std_powerups; // (used for mp) number of GLUpowerups there are when no backpacks are left

static int msg_and_score_color;
static int score_flash_color_1;
static int score_flash_color_2; 

// this array of strings contain data for the user to see
//  that concern his accomplishments
static tstring accomplishment_lines[3];

static queue<BYTE> key_presses;

static void MenuFont(LOGFONT& lf);
static void Levels(VCTR_STRING& target);
static void ShowMouseCursor();
static void HideMouseCursor();
static void SetupMenu();
static void LoadLevel();
static int  MenuLoop();
static void PrepareMenu();
static void SoundLoop();
static void LoadSounds(int type);
static void LoadBitmaps(int type);
static void Introduction();
static void EndGame();
static bool Flip();
static void PrepareForMPGame();
static void GetLevelTimerMinSec(int& min,int& sec);
static void Game();
static bool Menu();
static void FlushKeyPresses();
static void AddPossibleStartingSpot(FIXEDNUM x,FIXEDNUM y);
static void LoadCmps(int level_width,int level_height,bool skip_wd_resize); // GLUlevel width and height are not specified as floating-point
static void ResetSinglePlayerScore(int possible_score);
static void CalculateScorePrintX();
static void WriteInfo(); // draws text that shows score and timer and message to screen

static void WriteChar(int x,int y,int c,int color,int back_color);
static void WriteString(int x,int y,const TCHAR *tstring,int color,int back_color);

static void Recache(int flags);
// these statics make it easier to draw the compact maps faster
//  by caching them.  This way, the most that can be drawn each frame
//  is five, but usually none.  Those that are not "redrawn" are cached
//  into simple, un-compressed DirectDraw surfaces that are easy to render
static CBob lower_cached[CACHED_SECTORS];
static CBob upper_cached[CACHED_SECTORS];
static CBob *lower_cached_grid[CACHED_SECTORS_HIGH][CACHED_SECTORS_WIDE];
static CBob *upper_cached_grid[CACHED_SECTORS_HIGH][CACHED_SECTORS_WIDE];

static int ul_cached_sector_x; // the column of the upper left sector that is cached
static int ul_cached_sector_y; // the row " " "

const int CACHE_ULEFT = 1;
const int CACHE_UMIDDLE = 2;
const int CACHE_URIGHT = 4;
const int CACHE_CLEFT = 8;
const int CACHE_CMIDDLE = 16;
const int CACHE_CRIGHT = 32;
const int CACHE_BLEFT = 64;
const int CACHE_BMIDDLE = 128;
const int CACHE_BRIGHT = 256;

const int CACHE_BOTTOMROW = 64 | 128 | 256;
const int CACHE_TOPROW = 1 | 2 | 4;
const int CACHE_LEFTCOLUMN = 1 | 8 | 64;
const int CACHE_RIGHTCOLUMN = 4 | 32 | 256;
const int CACHE_EVERYTHING = 511;

// we have to define the data format of c_dfDIKeyboard ourselves,
// instead of relying on the directinput static lib to define it.
// This is because we aren't using any DirectX static libraries for
// Andradion 2 because getting them to work with the Borland compiler
// is harder than just copying this code snippet from a website
DIOBJECTDATAFORMAT c_rgodfDIKeyboard[256] = {
	{ &GUID_Key, 0, 0x8000000C, 0 }, 	{ &GUID_Key, 1, 0x8000010C, 0 },
	{ &GUID_Key, 2, 0x8000020C, 0 }, 	{ &GUID_Key, 3, 0x8000030C, 0 },
	{ &GUID_Key, 4, 0x8000040C, 0 }, 	{ &GUID_Key, 5, 0x8000050C, 0 },
	{ &GUID_Key, 6, 0x8000060C, 0 }, 	{ &GUID_Key, 7, 0x8000070C, 0 },
	{ &GUID_Key, 8, 0x8000080C, 0 }, 	{ &GUID_Key, 9, 0x8000090C, 0 },
	{ &GUID_Key, 10, 0x80000A0C, 0 }, 	{ &GUID_Key, 11, 0x80000B0C, 0 },
	{ &GUID_Key, 12, 0x80000C0C, 0 }, 	{ &GUID_Key, 13, 0x80000D0C, 0 },
	{ &GUID_Key, 14, 0x80000E0C, 0 }, 	{ &GUID_Key, 15, 0x80000F0C, 0 },
	{ &GUID_Key, 16, 0x8000100C, 0 }, 	{ &GUID_Key, 17, 0x8000110C, 0 },
	{ &GUID_Key, 18, 0x8000120C, 0 }, 	{ &GUID_Key, 19, 0x8000130C, 0 },
	{ &GUID_Key, 20, 0x8000140C, 0 }, 	{ &GUID_Key, 21, 0x8000150C, 0 },
	{ &GUID_Key, 22, 0x8000160C, 0 }, 	{ &GUID_Key, 23, 0x8000170C, 0 },
	{ &GUID_Key, 24, 0x8000180C, 0 }, 	{ &GUID_Key, 25, 0x8000190C, 0 },
	{ &GUID_Key, 26, 0x80001A0C, 0 }, 	{ &GUID_Key, 27, 0x80001B0C, 0 },
	{ &GUID_Key, 28, 0x80001C0C, 0 }, 	{ &GUID_Key, 29, 0x80001D0C, 0 },
	{ &GUID_Key, 30, 0x80001E0C, 0 }, 	{ &GUID_Key, 31, 0x80001F0C, 0 },
	{ &GUID_Key, 32, 0x8000200C, 0 }, 	{ &GUID_Key, 33, 0x8000210C, 0 },
	{ &GUID_Key, 34, 0x8000220C, 0 }, 	{ &GUID_Key, 35, 0x8000230C, 0 },
	{ &GUID_Key, 36, 0x8000240C, 0 }, 	{ &GUID_Key, 37, 0x8000250C, 0 },
	{ &GUID_Key, 38, 0x8000260C, 0 }, 	{ &GUID_Key, 39, 0x8000270C, 0 },
	{ &GUID_Key, 40, 0x8000280C, 0 }, 	{ &GUID_Key, 41, 0x8000290C, 0 },
	{ &GUID_Key, 42, 0x80002A0C, 0 }, 	{ &GUID_Key, 43, 0x80002B0C, 0 },
	{ &GUID_Key, 44, 0x80002C0C, 0 }, 	{ &GUID_Key, 45, 0x80002D0C, 0 },
	{ &GUID_Key, 46, 0x80002E0C, 0 }, 	{ &GUID_Key, 47, 0x80002F0C, 0 },
	{ &GUID_Key, 48, 0x8000300C, 0 }, 	{ &GUID_Key, 49, 0x8000310C, 0 },
	{ &GUID_Key, 50, 0x8000320C, 0 }, 	{ &GUID_Key, 51, 0x8000330C, 0 },
	{ &GUID_Key, 52, 0x8000340C, 0 }, 	{ &GUID_Key, 53, 0x8000350C, 0 },
	{ &GUID_Key, 54, 0x8000360C, 0 }, 	{ &GUID_Key, 55, 0x8000370C, 0 },
	{ &GUID_Key, 56, 0x8000380C, 0 }, 	{ &GUID_Key, 57, 0x8000390C, 0 },
	{ &GUID_Key, 58, 0x80003A0C, 0 }, 	{ &GUID_Key, 59, 0x80003B0C, 0 },
	{ &GUID_Key, 60, 0x80003C0C, 0 }, 	{ &GUID_Key, 61, 0x80003D0C, 0 },
	{ &GUID_Key, 62, 0x80003E0C, 0 }, 	{ &GUID_Key, 63, 0x80003F0C, 0 },
	{ &GUID_Key, 64, 0x8000400C, 0 }, 	{ &GUID_Key, 65, 0x8000410C, 0 },
	{ &GUID_Key, 66, 0x8000420C, 0 }, 	{ &GUID_Key, 67, 0x8000430C, 0 },
	{ &GUID_Key, 68, 0x8000440C, 0 }, 	{ &GUID_Key, 69, 0x8000450C, 0 },
	{ &GUID_Key, 70, 0x8000460C, 0 }, 	{ &GUID_Key, 71, 0x8000470C, 0 },
	{ &GUID_Key, 72, 0x8000480C, 0 }, 	{ &GUID_Key, 73, 0x8000490C, 0 },
	{ &GUID_Key, 74, 0x80004A0C, 0 }, 	{ &GUID_Key, 75, 0x80004B0C, 0 },
	{ &GUID_Key, 76, 0x80004C0C, 0 }, 	{ &GUID_Key, 77, 0x80004D0C, 0 },
	{ &GUID_Key, 78, 0x80004E0C, 0 }, 	{ &GUID_Key, 79, 0x80004F0C, 0 },
	{ &GUID_Key, 80, 0x8000500C, 0 }, 	{ &GUID_Key, 81, 0x8000510C, 0 },
	{ &GUID_Key, 82, 0x8000520C, 0 }, 	{ &GUID_Key, 83, 0x8000530C, 0 },
	{ &GUID_Key, 84, 0x8000540C, 0 }, 	{ &GUID_Key, 85, 0x8000550C, 0 },
	{ &GUID_Key, 86, 0x8000560C, 0 }, 	{ &GUID_Key, 87, 0x8000570C, 0 },
	{ &GUID_Key, 88, 0x8000580C, 0 }, 	{ &GUID_Key, 89, 0x8000590C, 0 },
	{ &GUID_Key, 90, 0x80005A0C, 0 }, 	{ &GUID_Key, 91, 0x80005B0C, 0 },
	{ &GUID_Key, 92, 0x80005C0C, 0 }, 	{ &GUID_Key, 93, 0x80005D0C, 0 },
	{ &GUID_Key, 94, 0x80005E0C, 0 }, 	{ &GUID_Key, 95, 0x80005F0C, 0 },
	{ &GUID_Key, 96, 0x8000600C, 0 }, 	{ &GUID_Key, 97, 0x8000610C, 0 },
	{ &GUID_Key, 98, 0x8000620C, 0 }, 	{ &GUID_Key, 99, 0x8000630C, 0 },
	{ &GUID_Key, 100, 0x8000640C, 0 }, 	{ &GUID_Key, 101, 0x8000650C, 0 },
	{ &GUID_Key, 102, 0x8000660C, 0 }, 	{ &GUID_Key, 103, 0x8000670C, 0 },
	{ &GUID_Key, 104, 0x8000680C, 0 }, 	{ &GUID_Key, 105, 0x8000690C, 0 },
	{ &GUID_Key, 106, 0x80006A0C, 0 }, 	{ &GUID_Key, 107, 0x80006B0C, 0 },
	{ &GUID_Key, 108, 0x80006C0C, 0 }, 	{ &GUID_Key, 109, 0x80006D0C, 0 },
	{ &GUID_Key, 110, 0x80006E0C, 0 }, 	{ &GUID_Key, 111, 0x80006F0C, 0 },
	{ &GUID_Key, 112, 0x8000700C, 0 }, 	{ &GUID_Key, 113, 0x8000710C, 0 },
	{ &GUID_Key, 114, 0x8000720C, 0 }, 	{ &GUID_Key, 115, 0x8000730C, 0 },
	{ &GUID_Key, 116, 0x8000740C, 0 }, 	{ &GUID_Key, 117, 0x8000750C, 0 },
	{ &GUID_Key, 118, 0x8000760C, 0 }, 	{ &GUID_Key, 119, 0x8000770C, 0 },
	{ &GUID_Key, 120, 0x8000780C, 0 }, 	{ &GUID_Key, 121, 0x8000790C, 0 },
	{ &GUID_Key, 122, 0x80007A0C, 0 }, 	{ &GUID_Key, 123, 0x80007B0C, 0 },
	{ &GUID_Key, 124, 0x80007C0C, 0 }, 	{ &GUID_Key, 125, 0x80007D0C, 0 },
	{ &GUID_Key, 126, 0x80007E0C, 0 }, 	{ &GUID_Key, 127, 0x80007F0C, 0 },
	{ &GUID_Key, 128, 0x8000800C, 0 }, 	{ &GUID_Key, 129, 0x8000810C, 0 },
	{ &GUID_Key, 130, 0x8000820C, 0 }, 	{ &GUID_Key, 131, 0x8000830C, 0 },
	{ &GUID_Key, 132, 0x8000840C, 0 }, 	{ &GUID_Key, 133, 0x8000850C, 0 },
	{ &GUID_Key, 134, 0x8000860C, 0 }, 	{ &GUID_Key, 135, 0x8000870C, 0 },
	{ &GUID_Key, 136, 0x8000880C, 0 }, 	{ &GUID_Key, 137, 0x8000890C, 0 },
	{ &GUID_Key, 138, 0x80008A0C, 0 }, 	{ &GUID_Key, 139, 0x80008B0C, 0 },
	{ &GUID_Key, 140, 0x80008C0C, 0 }, 	{ &GUID_Key, 141, 0x80008D0C, 0 },
	{ &GUID_Key, 142, 0x80008E0C, 0 }, 	{ &GUID_Key, 143, 0x80008F0C, 0 },
	{ &GUID_Key, 144, 0x8000900C, 0 }, 	{ &GUID_Key, 145, 0x8000910C, 0 },
	{ &GUID_Key, 146, 0x8000920C, 0 }, 	{ &GUID_Key, 147, 0x8000930C, 0 },
	{ &GUID_Key, 148, 0x8000940C, 0 }, 	{ &GUID_Key, 149, 0x8000950C, 0 },
	{ &GUID_Key, 150, 0x8000960C, 0 }, 	{ &GUID_Key, 151, 0x8000970C, 0 },
	{ &GUID_Key, 152, 0x8000980C, 0 }, 	{ &GUID_Key, 153, 0x8000990C, 0 },
	{ &GUID_Key, 154, 0x80009A0C, 0 }, 	{ &GUID_Key, 155, 0x80009B0C, 0 },
	{ &GUID_Key, 156, 0x80009C0C, 0 }, 	{ &GUID_Key, 157, 0x80009D0C, 0 },
	{ &GUID_Key, 158, 0x80009E0C, 0 }, 	{ &GUID_Key, 159, 0x80009F0C, 0 },
	{ &GUID_Key, 160, 0x8000A00C, 0 }, 	{ &GUID_Key, 161, 0x8000A10C, 0 },
	{ &GUID_Key, 162, 0x8000A20C, 0 }, 	{ &GUID_Key, 163, 0x8000A30C, 0 },
	{ &GUID_Key, 164, 0x8000A40C, 0 }, 	{ &GUID_Key, 165, 0x8000A50C, 0 },
	{ &GUID_Key, 166, 0x8000A60C, 0 }, 	{ &GUID_Key, 167, 0x8000A70C, 0 },
	{ &GUID_Key, 168, 0x8000A80C, 0 }, 	{ &GUID_Key, 169, 0x8000A90C, 0 },
	{ &GUID_Key, 170, 0x8000AA0C, 0 }, 	{ &GUID_Key, 171, 0x8000AB0C, 0 },
	{ &GUID_Key, 172, 0x8000AC0C, 0 }, 	{ &GUID_Key, 173, 0x8000AD0C, 0 },
	{ &GUID_Key, 174, 0x8000AE0C, 0 }, 	{ &GUID_Key, 175, 0x8000AF0C, 0 },
	{ &GUID_Key, 176, 0x8000B00C, 0 }, 	{ &GUID_Key, 177, 0x8000B10C, 0 },
	{ &GUID_Key, 178, 0x8000B20C, 0 }, 	{ &GUID_Key, 179, 0x8000B30C, 0 },
	{ &GUID_Key, 180, 0x8000B40C, 0 }, 	{ &GUID_Key, 181, 0x8000B50C, 0 },
	{ &GUID_Key, 182, 0x8000B60C, 0 }, 	{ &GUID_Key, 183, 0x8000B70C, 0 },
	{ &GUID_Key, 184, 0x8000B80C, 0 }, 	{ &GUID_Key, 185, 0x8000B90C, 0 },
	{ &GUID_Key, 186, 0x8000BA0C, 0 }, 	{ &GUID_Key, 187, 0x8000BB0C, 0 },
	{ &GUID_Key, 188, 0x8000BC0C, 0 }, 	{ &GUID_Key, 189, 0x8000BD0C, 0 },
	{ &GUID_Key, 190, 0x8000BE0C, 0 }, 	{ &GUID_Key, 191, 0x8000BF0C, 0 },
	{ &GUID_Key, 192, 0x8000C00C, 0 }, 	{ &GUID_Key, 193, 0x8000C10C, 0 },
	{ &GUID_Key, 194, 0x8000C20C, 0 }, 	{ &GUID_Key, 195, 0x8000C30C, 0 },
	{ &GUID_Key, 196, 0x8000C40C, 0 }, 	{ &GUID_Key, 197, 0x8000C50C, 0 },
	{ &GUID_Key, 198, 0x8000C60C, 0 }, 	{ &GUID_Key, 199, 0x8000C70C, 0 },
	{ &GUID_Key, 200, 0x8000C80C, 0 }, 	{ &GUID_Key, 201, 0x8000C90C, 0 },
	{ &GUID_Key, 202, 0x8000CA0C, 0 }, 	{ &GUID_Key, 203, 0x8000CB0C, 0 },
	{ &GUID_Key, 204, 0x8000CC0C, 0 }, 	{ &GUID_Key, 205, 0x8000CD0C, 0 },
	{ &GUID_Key, 206, 0x8000CE0C, 0 }, 	{ &GUID_Key, 207, 0x8000CF0C, 0 },
	{ &GUID_Key, 208, 0x8000D00C, 0 }, 	{ &GUID_Key, 209, 0x8000D10C, 0 },
	{ &GUID_Key, 210, 0x8000D20C, 0 }, 	{ &GUID_Key, 211, 0x8000D30C, 0 },
	{ &GUID_Key, 212, 0x8000D40C, 0 }, 	{ &GUID_Key, 213, 0x8000D50C, 0 },
	{ &GUID_Key, 214, 0x8000D60C, 0 }, 	{ &GUID_Key, 215, 0x8000D70C, 0 },
	{ &GUID_Key, 216, 0x8000D80C, 0 }, 	{ &GUID_Key, 217, 0x8000D90C, 0 },
	{ &GUID_Key, 218, 0x8000DA0C, 0 }, 	{ &GUID_Key, 219, 0x8000DB0C, 0 },
	{ &GUID_Key, 220, 0x8000DC0C, 0 }, 	{ &GUID_Key, 221, 0x8000DD0C, 0 },
	{ &GUID_Key, 222, 0x8000DE0C, 0 }, 	{ &GUID_Key, 223, 0x8000DF0C, 0 },
	{ &GUID_Key, 224, 0x8000E00C, 0 }, 	{ &GUID_Key, 225, 0x8000E10C, 0 },
	{ &GUID_Key, 226, 0x8000E20C, 0 }, 	{ &GUID_Key, 227, 0x8000E30C, 0 },
	{ &GUID_Key, 228, 0x8000E40C, 0 }, 	{ &GUID_Key, 229, 0x8000E50C, 0 },
	{ &GUID_Key, 230, 0x8000E60C, 0 }, 	{ &GUID_Key, 231, 0x8000E70C, 0 },
	{ &GUID_Key, 232, 0x8000E80C, 0 }, 	{ &GUID_Key, 233, 0x8000E90C, 0 },
	{ &GUID_Key, 234, 0x8000EA0C, 0 }, 	{ &GUID_Key, 235, 0x8000EB0C, 0 },
	{ &GUID_Key, 236, 0x8000EC0C, 0 }, 	{ &GUID_Key, 237, 0x8000ED0C, 0 },
	{ &GUID_Key, 238, 0x8000EE0C, 0 }, 	{ &GUID_Key, 239, 0x8000EF0C, 0 },
	{ &GUID_Key, 240, 0x8000F00C, 0 }, 	{ &GUID_Key, 241, 0x8000F10C, 0 },
	{ &GUID_Key, 242, 0x8000F20C, 0 }, 	{ &GUID_Key, 243, 0x8000F30C, 0 },
	{ &GUID_Key, 244, 0x8000F40C, 0 }, 	{ &GUID_Key, 245, 0x8000F50C, 0 },
	{ &GUID_Key, 246, 0x8000F60C, 0 }, 	{ &GUID_Key, 247, 0x8000F70C, 0 },
	{ &GUID_Key, 248, 0x8000F80C, 0 }, 	{ &GUID_Key, 249, 0x8000F90C, 0 },
	{ &GUID_Key, 250, 0x8000FA0C, 0 }, 	{ &GUID_Key, 251, 0x8000FB0C, 0 },
	{ &GUID_Key, 252, 0x8000FC0C, 0 }, 	{ &GUID_Key, 253, 0x8000FD0C, 0 },
	{ &GUID_Key, 254, 0x8000FE0C, 0 }, 	{ &GUID_Key, 255, 0x8000FF0C, 0 }
}; const DIDATAFORMAT c_dfDIKeyboard = { 24, 16, 0x2, 256, 256, c_rgodfDIKeyboard };
// end of c_dfDIKeyboard definition

// dialog proc.  This is a callback function that
//  sends messages to the code entity that defines
//  how the intro box works.  This box's controls
//  and the locations of them are defined in the
//  resource
static BOOL CALLBACK CfgDlgProc(
				HWND hwndDlg,  // handle to dialog box
				UINT uMsg,     // message
				WPARAM wParam, // first message parameter
				LPARAM   // second message parameter
				)
{
  switch(uMsg)
    {
    case WM_COMMAND:
      {
	// get the identifier of the control, which is always passed
	//  through the lower word of the wParam parameter
	WORD ctrl = LOWORD(wParam);
	if(IDLAUNCH == ctrl || IDQUIT == ctrl || IDLAUNCHWITHOUTMUSIC == ctrl)
	  {
				// the launch or quit or launch w/o music btn was pressed 
				// in either case, we need a text buffer, defined here:
	    TCHAR txt[MAX_STRINGLEN];
				// get the text of the sync rate edit box, where the user specifies
				//  how often andradion 2 peers communicate their location
	    GetDlgItemText(hwndDlg,IDC_SYNC,txt,MAX_STRINGLEN);

				// set sync rate to a value which will be detected
				//  as invalid.  If the scanf function does not convert
				//  the tstring because it is invalid or something,
				//  and the GLUsync_rate var isn't touched, then the if()
				//  block will see the error
	    GLUsync_rate = 0;

				// scan the sync rate from txt using the mapped tchar function
				//  of scanf
	    _stscanf(txt,SYNCRATE_FORMAT,&GLUsync_rate);
				
				// see if the GLUsync_rate was invalid
	    if(GLUsync_rate < 1 || GLUsync_rate > 100)
	      {
		// invalid sync rate was entered
		//  display error
		// load two strings that make up the message box of the error
		tstring error_msg;
		tstring dialog_caption;
		GluStrLoad(IDS_INVALIDSYNCRATE,error_msg);
		GluStrLoad(IDS_WINDOWCAPTION,dialog_caption);

		// show the error
		MessageBox(hwndDlg,error_msg.c_str(),dialog_caption.c_str(),MB_ICONSTOP);
	      }
	    else
	      {
		// a valid sync rate was entered
		if(IDLAUNCHWITHOUTMUSIC == ctrl)
		  {
		    // the launch w/o music button was pressed, so
		    //  stop the music already playing,
		    //  and call the CGlue function which will flag a member
		    //  and make sure that no music is ever played again
		    MusicStop();
		    GluDisableMusic(); // simple function that nullifies all GluSetMusic calls
		  }
		// this function will, well, uh...
		EndDialog(hwndDlg,LOWORD(wParam));
	      }
	  }
	return FALSE;
      }
    case WM_SHOWWINDOW:
      {
	// use this opportunity to do somethings to initialize

	// set the hyper welcome box music
	GluSetMusic(false,IDR_WELCOMEBOXMUSIC);
			
	// convert a number to from DWORD to char
	//  then char to tchar by using a simple while loop
	TCHAR number_buffer[MAX_STRINGLEN];
	_itot(GLUsync_rate,number_buffer,10);

	// set the text in the sync rate edit box to what we just got
	//  from the sprintf function
	SetDlgItemText(hwndDlg,IDC_SYNC,number_buffer);
	return FALSE;
      }
    case WM_INITDIALOG:
      {
	return TRUE; // we want the currently-in-focus control to be chosen automatically
      }
    default:
      return FALSE;
    }
}

static void ResetSinglePlayerScore(int possible_score)
{
  TCHAR str_score[SCORELEN];
  wsprintf(str_score,TWO_NUMBERS_FORMAT,0,possible_score);
  score = str_score;

  CalculateScorePrintX();
}

static void CalculateScorePrintX()
{
  score_print_x = GAME_MODEWIDTH - score.length() * (FONTWIDTH+1) + SCORE_X_OFFSET;
}

static float spf;
static DWORD fps;
static FIXEDNUM sfxfreq;

static int GetSpeed()
{
  if(25 == fps)
    {
      return 1;
    }
  else if(12 == fps)
    {
      return 0;
    }
  else
    {
      return 2;
    }
}

static void SetSpeed(int index)
{
  WriteLog("SetSpeed(%d) called" LogArg(index));
  switch(index)
    {
    case 0:
      // slow the game down
      spf = 0.08f; // more seconds per frame
      fps = 12; // less frames per second
      sfxfreq = Fixed(0.5f); // lower sound frequency
      SetTempo(DefaultTempo()/2.0f);
      WtrSetSoundPlaybackFrequency(Fixed(0.5f));
      break;
    case 1:
      // normal game speed
      spf = 0.04f;
      fps = 25;
      sfxfreq = Fixed(1);
      SetTempo(DefaultTempo());
      WtrSetSoundPlaybackFrequency(Fixed(1.0f));
      break;
    case 2:
      // speed the game up
      spf = 0.02f;
      fps = 50;
      sfxfreq = Fixed(2);
      SetTempo(DefaultTempo()*2.0f);
      WtrSetSoundPlaybackFrequency(Fixed(2.0f));
    }

  if(NetInGame() == true && 1 != index)
    { 
      SetSpeed(1); // return it to the right speed
    }
}

bool GluInitialize(HINSTANCE hInstance_,HWND hWnd_)
{ 
  for(int i = 0; i < MAX_SOUNDS; i++)
    {
      playing[i] = NULL;
    }
  CoInitialize(NULL); // open com
  hInstance = hInstance_; // save this for later so we now how to load strings from the table
  hWnd = hWnd_;

  // load the levels dll
  level_lib_mod = LoadLibrary(LEVELS_LIB_FILE);
  level_lib_mod_2 = LoadLibrary(LEVELS_LIB_FILE_2);
  if(NULL == level_lib_mod || NULL == level_lib_mod_2)
    {
      // unload both GLUlevel dlls
      FreeLibrary(level_lib_mod);
      FreeLibrary(level_lib_mod_2);

      // something went wrong; we couldn't load the dll
      //  so display an error message
      tstring went_wrong;
      tstring went_wrong_cap;
      GluStrLoad(IDS_NODLL,went_wrong);
      GluStrLoad(IDS_WINDOWCAPTION,went_wrong_cap);

      MessageBox(hWnd,went_wrong.c_str(),went_wrong_cap.c_str(),MB_ICONSTOP);

      return true;
    }
	
  tstring ini_file;
  GluStrLoad(IDS_INIFILE,ini_file);

  // GLUlevel completion data
  HANDLE lc = CreateFile(ini_file.c_str(),GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

  if(INVALID_HANDLE_VALUE != lc) // if we didn't fail . . .
    {
      DWORD read;
      ReadFile(lc,&GLUsync_rate,sizeof(GLUsync_rate),&read,NULL);
      DeeInitialize(lc);
      // and finally close the file
      CloseHandle(lc);
    }
  else
    {
      GLUsync_rate = DEFAULT_SYNCRATE;
      DeeInitialize();
      tstring msg;
      tstring dlg_caption;
      GluStrLoad(IDS_BUDGETCUTS,msg);
      GluStrLoad(IDS_WINDOWCAPTION,dlg_caption);
      MessageBox(hWnd,msg.c_str(),dlg_caption.c_str(),MB_ICONINFORMATION);
    }

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
       (void **)&ds
       )
      )
     ||
     FAILED
     (
      ds->Initialize
      (
       NULL
       )
      )
     )
    {
      ds = NULL;
      GluDisableMusic();
    }
  else
    {
      ds->SetCooperativeLevel(hWnd_,DSSCL_NORMAL);
      // load sounds after intro . . .
      TryAndReport(MusicInit(hWnd_,ds));
    }

  NetInitialize();

  // show welcome dialog (aka cfg dialog)
  if(IDQUIT == DialogBox(hInstance,MAKEINTRESOURCE(IDD_CFG),hWnd,CfgDlgProc))
    {
      // nevermind, we need to leave . . .
      return true; // return true to quit
    }

  HideMouseCursor();

  // create direct input and setup the device state change event
  CoCreateInstance
    (
     CLSID_DirectInput,
     NULL,
     CLSCTX_INPROC_SERVER,
     IID_IDirectInput,
     (void **)&di
     );
  di->Initialize(hInstance_,DIRECTINPUT_VERSION);
  di->CreateDevice(GUID_SysKeyboard,&did,NULL);
  did->SetDataFormat(&c_dfDIKeyboard);
  did->SetCooperativeLevel(hWnd_,DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

  // the device will be acquired automatically the first we try to get
  //  the device state.  The GetDeviceState function will fail, and we
  //  will try and acquire it then
#ifdef _DEBUG
  profiler_font = (HGDIOBJ)CreateFont(PROFILER_FONT_SIZE,0,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_MODERN | FIXED_PITCH,NULL);
#endif

  // this will just copy the font data resource into a global array
  memcpy((void *)font_data,LockResource(LoadResource(NULL,FindResource(NULL,MAKEINTRESOURCE(IDR_FONT),TEXT("DAT")))),4096);

  state = GLUESTATE_INTRODUCTION;
  sectors.resize(0,0);
  GLUlevel = LEVEL_NONE;

  memset((void *)GLUkeyb,0,KBBUFFERSIZE);

  // so we know that no GLUbitmaps are loaded, let's set each pointer to NULL
  memset((void *)GLUbitmaps,0,BitmapCountT<RESOURCELOAD_MP>::Val * sizeof(CBob *));
  bitmaps_loaded = 0;
  // so we know that no sounds are loaded, let's set each pointer to NULL
  memset((void *)sounds,0,NUM_SOUNDS * sizeof(LPDIRECTSOUNDBUFFER));
  sounds_loaded = RESOURCELOAD_NONE;

  return false;
}

void GluRelease()
{
  WriteLog("GluReleease() called");

  if(GLUESTATE_UNINITIALIZED == state)
    {
      WriteLog("Glue is already released");
      return;
    }

  WriteLog("Calling NetRelease() from GluRelease()"); NetRelease();
  WriteLog("Deleting menu");	delete m;
  WriteLog("Calling MusicUninit() from GluRelease() to stop any music if it is playing and close DirectMusic interface"); MusicUninit();
  WriteLog("Calling LoadSounds() from GluRelease to unload all sounds"); LoadSounds(RESOURCELOAD_NONE);

#ifdef _DEBUG
  TryAndReport(DeleteObject(profiler_font));
#endif

  TryAndReport(did->Unacquire());
  TryAndReport(did->Release());
  TryAndReport(di->Release());
  WriteLog("Calling PalRelease() from GluRelease() to close DirectDraw interface to palette"); PalRelease();
  WriteLog("Calling LoadBitmaps() from GluRelease() to unload all bitmaps"); LoadBitmaps(RESOURCELOAD_NONE);
  WriteLog("Clearing out sector data matrix"); sectors.resize(0,0);
  if(NULL != GLUclipper)
    {
      WriteLog("Need to release clipper");
      TryAndReport(gr->Buffer(1)->SetClipper(NULL));
      TryAndReport(GLUclipper->Release());
    }
  GamRelease();
  WtrRelease();
  WriteLog("Releasing cached map surfaces");
  int i;
  for(i = 0; i < CACHED_SECTORS; i++)
    {
      lower_cached[i].Destroy();
      upper_cached[i].Destroy();
    }

  WriteLog("Uncertifying CGraphics"); gr->Uncertify();
  WriteLog("Deleting CGraphics to get releasa all memory"); delete gr;
	
  WriteLog("About to release all sounds");
  for(i = 0; i < MAX_SOUNDS; i++)
    {
      if(NULL != playing[i])
	{
	  TryAndReport(playing[i]->Release());
	  playing[i] = NULL;
	}
    }

  WriteLog("About to close DirectSound interface");
  if(NULL != ds)
    {
      TryAndReport(ds->Release());
      ds = NULL;
    }
	
  WriteLog("Saving level completion data");
  HANDLE lc;
  tstring ini_file;
  GluStrLoad(IDS_INIFILE,ini_file);
  WriteLog("About to write to %s" LogArg(ini_file.c_str()));
  lc = TryAndReport(CreateFile(ini_file.c_str(),GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL));
  DWORD written;
  TryAndReport(WriteFile(lc,(const void *)&GLUsync_rate,sizeof(GLUsync_rate),&written,NULL));
  WriteLog("%d bytes written"LogArg((int)written));
  DeeRelease(lc);
  TryAndReport(CloseHandle(lc));
  WriteLog("Finished saving level completion data");
  WriteLog("Closing COM"); CoUninitialize();
  WriteLog("Calling ShowMouseCursor to show the mouse"); ShowMouseCursor();

  WriteLog("Freeing the levelsLib dll's");
  TryAndReport(FreeLibrary(level_lib_mod));
  TryAndReport(FreeLibrary(level_lib_mod_2));

  WriteLog("Deleting walking data bit matrix");
  delete walking_data; // delete bit array that specifies where is outside and inside
  walking_data = NULL;

  state = GLUESTATE_UNINITIALIZED;

  WriteLog("GluRelease returning");
}

static void Introduction()
{
  const float TIMETOTHROWBACK = 2.25f;
  const float STORYSLANT = 1.20f;
  const float TIMETOSCROLL = 70.0f;
  const float STORYVSCALEMAXY = 2.0f;
  const float STORYVSCALEMINY = 0.5f;
  const int   NUMSTARS = 1500;
  const int   TRANSITIONSQUARESIZE = 350; // squares of this size
  const float TRANSITIONSECSPERSQUARE = 1.0f/80.0f;
  const int   MAXSTARDIMNESS = 10;
  const float MAXFLASHTIME = 5.0f; // if the warpout sound is still playing after five seconds, stop anyway
	
  const RECT  UPPERBLACKAREA = {0,0,800,75}; // left,top,right,bottom
  const RECT  LOWERBLACKAREA = {0,525,800,600};
  const RECT  DISPLAYAREA    = {0,75,800,525};
  const int   MODEWIDTH = 800; // width and height of the mode
  const int   MODEHEIGHT = 600;

  // red
  const COLORREF FLASHCOLOR1 = RGB(255,0,0);
  // yellow
  const COLORREF FLASHCOLOR2 = RGB(255,255,0);

  // how fast to flash
  const float FLASHCOLORPERSEC = 0.06f;

  CBob *stars; // the stars in the background
  CBob *story;
  CBob *turner;
  CTimer timer;
  CTimer inc_or_dec;
  int story_width;
  int story_height;

  DDBLTFX fx;
  int i; // looping
  RECT source,dest;

  int inx = DISPLAYAREA.right - DISPLAYAREA.left;
  int iny = DISPLAYAREA.bottom - DISPLAYAREA.top;

  state = GLUESTATE_INTRODUCTION; // we are in the intro right now

  // load the intro

  // change video mode
#ifndef WINDOWEDMODE
  gr = new CGraphics(
		     hWnd,false,
		     MODEWIDTH,MODEHEIGHT,
		     GAME_MODEBPP,
		     GAME_MODEBUFFERS
		     );
#else
  gr = new CGraphics(NULL,
		     CGraphics::MXS_NONE,
		     MODEWIDTH,
		     MODEHEIGHT,
		     0,
		     GAME_MODEBUFFERS);
#endif

  gr->Certify();

  // we will need this pointer various times
  RECT *target = &gr->TargetScreenArea();

  // setup the palette
  PalInitializeWithIntroPalette(*gr);

  // create starscape surface
  stars = new CBob(inx,iny);

  HBITMAP a_bmp;

  // load story bmp
  a_bmp = (HBITMAP)LoadImage(hInstance,MAKEINTRESOURCE(IDB_STORY),IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
  story = new CBob(a_bmp);
  DeleteObject((HGDIOBJ)a_bmp);
  story->GetSize(story_width,story_height);

  // load turner bmp
  a_bmp = (HBITMAP)LoadImage(hInstance,MAKEINTRESOURCE(IDB_TURNER),IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
  turner = new CBob(a_bmp);
  DeleteObject((HGDIOBJ)a_bmp);
	
  // clear the starscape
  fx.dwFillColor = 0;
  fx.dwSize = sizeof(fx);

  // fill with black
  while
    (
     DDERR_WASSTILLDRAWING ==
     stars->Data()->Blt(NULL,NULL,NULL,DDBLT_COLORFILL,&fx)
     );
	
  DDSURFACEDESC starsd;

  // init the starsd struct
  memset((void *)&starsd,0,sizeof(starsd));
  starsd.dwSize = sizeof(starsd);

  // lock the stars
  if(SUCCEEDED(stars->Data()->Lock(NULL,&starsd,DDLOCK_WAIT | DDLOCK_WRITEONLY,NULL)))
    {
      BYTE *starsb = (BYTE *)starsd.lpSurface;

      // figure out pitch now
      int p = starsd.lPitch;

      for(i = 0; i < NUMSTARS; i++)
	{ // plot a bunch of stars
	  // make a small plus sign for the stars instead of just dots!

	  int x = (rand()%(inx-2))+1;
	  int y = (rand()%(iny-2))+1;
	  BYTE c = (BYTE)(255 - rand()%MAXSTARDIMNESS);
	  starsb[y*p+x] = c;
	  starsb[(y+1)*p+x] = starsb[(y-1)*p+x] = starsb[y*p+x+1] =
	    starsb[y*p+x-1] = c/2;
	}

      // unlock the stars
      stars->Data()->Unlock(NULL);
    }

  LPDIRECTSOUNDBUFFER warpout;
  DWORD warp_status; // playing or not of the above sound

  // load the warpout sound
  HRSRC res_handle;
  HGLOBAL data_handle;
  void *warpout_data = GetResPtr(TEXT("SFX"),TEXT("DAT"),NULL,MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),res_handle,data_handle);
  DWORD warpout_size = *(DWORD *)warpout_data;
  warpout_data = (void *)((DWORD *)warpout_data + 1);
  if(NULL == warpout_data)
    {
      warpout = NULL;
    }
  else
    {
      // we locked it successfully
      CreateSBFromRawData(ds,&warpout,warpout_data,warpout_size,0,SOUNDRESOURCEFREQ,SOUNDRESOURCEBPS,1);
    }
  FreeResource(data_handle);

  // play intro music
  GluSetMusic(false,IDR_INTROMUSIC);

  // load polygon data for splash screen
  int polygon_count;
  int *vertex_counts;
  int *polygon_vertices;

  BYTE *locked = (BYTE *)GetResPtr(MAKEINTRESOURCE(IDR_SPLASH),TEXT("DAT"),NULL,MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),res_handle,data_handle);

  if(NULL == locked)
    {
      polygon_count = 0;
      vertex_counts = NULL;
      polygon_vertices = NULL;
    }
  else
    {
      // load the meaningful data
      polygon_count = (int)*locked++;
      // get the vertex counts
      int total_vertex_count = 0;
      vertex_counts = new int[polygon_count];
      for(int i = 0; i < polygon_count; i++)
	{
	  vertex_counts[i] = (int)*locked++;
	  total_vertex_count += vertex_counts[i];
	}
      // get each vertex coordinate
      polygon_vertices = new int[total_vertex_count * 2];
      for(i = 0; i < total_vertex_count; i++)
	{
	  polygon_vertices[i * 2] = (int)*locked++;
	  polygon_vertices[i * 2+1] = (int)*locked++;

	  polygon_vertices[i * 2] *= MODEWIDTH;
	  polygon_vertices[i * 2] /= 0x100;

	  polygon_vertices[i * 2+1] *= MODEHEIGHT;
	  polygon_vertices[i * 2+1] /= 0x100;
	}

      FreeResource(data_handle);
    }

  // play the warpout sound synchronously, that is,
  //  don't continue until the sound is through
  if(NULL != warpout)
    {
      warpout->Play(0,0,0); // play our warpout sound
    }

  CTimer total_flash;
  bool flash_colors = false; // keeps track of what flash colors to use

  // we need a red brush and a red pen, and a yellow brush and a yellow pen
  HBRUSH brush1 = CreateSolidBrush(FLASHCOLOR1);
  HBRUSH brush2 = CreateSolidBrush(FLASHCOLOR2);
  HPEN pen1 = CreatePen(PS_SOLID,1,FLASHCOLOR1);
  HPEN pen2 = CreatePen(PS_SOLID,1,FLASHCOLOR2);
	
  do
    {
      CTimer frame; // this makes sure we don't flip to quick
      HDC dc;
      if(SUCCEEDED(gr->Buffer(1)->GetDC(&dc)))
	{
	  HBRUSH back_brush;
	  HBRUSH text_brush;
	  HPEN   text_pen;
	  if(true == flash_colors)
	    {
	      back_brush = brush1;
	      text_brush = brush2;
	      text_pen = pen2;
	    }
	  else
	    {
	      back_brush = brush2;
	      text_brush = brush1;
	      text_pen = pen1;
	    }
	  // blit the background
	  HBRUSH old_brush = (HBRUSH)SelectObject(dc,(HGDIOBJ)back_brush);
	  Rectangle(dc,0,0,MODEWIDTH,MODEHEIGHT);
	  // select the objects for the text
	  SelectObject(dc,(HGDIOBJ)text_brush);
	  HPEN old_pen = (HPEN)SelectObject(dc,(HGDIOBJ)text_pen);
	  // blit the text
	  PolyPolygon(dc,(const POINT *)polygon_vertices,vertex_counts,polygon_count);
	  // Select back the old objects
	  SelectObject(dc,(HGDIOBJ)old_brush);
	  SelectObject(dc,(HGDIOBJ)old_pen);
	  // release dc
	  gr->Buffer(1)->ReleaseDC(dc);
	}
      while(frame.SecondsPassed32() < FLASHCOLORPERSEC);
#ifndef WINDOWEDMODE
      gr->Flip(DDFLIP_NOVSYNC);
#else
      gr->Buffer(0)->BltFast(0,0,gr->Buffer(1),NULL,FALSE);
#endif
      flash_colors = !flash_colors;
    }
  while( // pause until the sound has been played
	(NULL == warpout || SUCCEEDED(warpout->GetStatus(&warp_status))) &&
	(warp_status & DSBSTATUS_PLAYING) &&
	(total_flash.SecondsPassed32() < MAXFLASHTIME)
	);
  // get rid of those extra brushes we made
  DeleteObject((HGDIOBJ)brush1);
  DeleteObject((HGDIOBJ)brush2);
  DeleteObject((HGDIOBJ)pen1);
  DeleteObject((HGDIOBJ)pen2);

  if(NULL != warpout)
    {
      TryAndReport(warpout->Release());
      warpout = NULL;
    }
	

  delete polygon_vertices;
  delete vertex_counts;

  // done loading!
  timer.Restart();

  FlushKeyPresses();
  inc_or_dec.Restart();
  do
    {
      // put the stars and black spots on the back buffer
      *target = DISPLAYAREA;
      gr->PutFast(*stars,false);
      *target = UPPERBLACKAREA;
      gr->Rectangle(0);
      *target = LOWERBLACKAREA;
      gr->Rectangle(0);

      float progress = timer.SecondsPassed32() / TIMETOTHROWBACK;
      target->left = Range(0,iny/2,progress) + (inx - iny) / 2;
      target->top = Range(0,iny/2,progress) + DISPLAYAREA.top;
      target->right =	Range(iny,iny/2,progress) + (inx - iny) / 2;
      target->bottom = Range(iny,iny/2,progress) + DISPLAYAREA.top;

      if(target->left >= target->right || target->top >= target->bottom)
	{
	  break;
	}

      // put the turner photo on the back buffer
      gr->Put(*turner,DDBLT_KEYSRC,NULL,true);
    }
  while(timer.SecondsPassed32() <= TIMETOTHROWBACK && true == key_presses.empty() && false == Flip());
  inc_or_dec.Pause();

  // we don't need the turner photo no more
  delete turner;

  // now show the story
  FlushKeyPresses();
  timer.Restart();
  LPDIRECTDRAWSURFACE2 backbuffer = gr->Buffer(1);
  int max_storyy = story_height + iny / STORYVSCALEMAXY;
  do
    {
      // put the stars and black spots on the back buffer
      *target = DISPLAYAREA;
      gr->PutFast(*stars,false);
      *target = UPPERBLACKAREA;
      gr->Rectangle(0);
      *target = LOWERBLACKAREA;
      gr->Rectangle(0);

      int storyy=Range(0,max_storyy,timer.SecondsPassed32()/TIMETOSCROLL);

      source.left = 0; source.right = story_width;
      source.top = storyy;
      source.bottom = storyy+1;
      dest.left = 0;
      dest.right = MODEWIDTH;
      float dest_top = float(iny-STORYVSCALEMAXY+DISPLAYAREA.top);
      float dest_bottom = float(iny+DISPLAYAREA.top);

      float story_scale;

      for
	(
	 float error= 0.0f;
	 dest.left < dest.right && source.top >= 0 && (int)dest_top >= DISPLAYAREA.top;
	 dest_top-=story_scale,dest_bottom-=story_scale,source.top--,source.bottom--
	 ) 
	{
	  error += STORYSLANT;
	  while(error >= 1.00) 
	    {
	      dest.left+=1;
	      dest.right-=1;
	      error -= 1.00;
	    }
	  dest.top = (int)dest_top;
	  dest.bottom = (int)dest_bottom;
	  if(source.bottom < story_height)
	    {
	      while
		(
		 DDERR_WASSTILLDRAWING ==
		 backbuffer->Blt(&dest,story->Data(),&source,DDBLT_KEYSRC,NULL)
		 );
	    }

	  story_scale = Range(STORYVSCALEMINY,STORYVSCALEMAXY,float(dest.top-DISPLAYAREA.top)/float(DISPLAYAREA.bottom-DISPLAYAREA.top));
	}
      if(false == key_presses.empty())
	{
	  switch(key_presses.front())
	    {
	    case VK_NEXT:
	      timer += inc_or_dec;
	      key_presses.pop();
	      if(timer.SecondsPassed32() >= TIMETOSCROLL)
		{
		  timer -= inc_or_dec;
		}
	      break;
	    case VK_PRIOR:
	      timer -= inc_or_dec;
	      key_presses.pop();
	      if(0 > timer.SecondsPassedInt())
		{
		  timer.Restart();
		}
	      break;
	    case VK_SPACE:
	      if(true == timer.Paused())
		{
		  timer.Resume();
		}
	      else
		{
		  timer.Pause();
		}
	      key_presses.pop();
	    }
	}
    }
  while(true == key_presses.empty() && false == Flip());

  WriteLog("Introduction finished");

  WriteLog("Deleting stars and story");
  delete stars;
  delete story;

  WriteLog("Doing gray blockscreen transition");
  gr->SetTargetBuffer(0); // we want to put a bunch of gray rectangles on the front buffer directly
  for(BYTE c = 255; c >= INTROPALETTE_BASECOLORS; c--)
    {
      timer.Restart();
      target->left = rand()%(MODEWIDTH-TRANSITIONSQUARESIZE);
      target->top = rand()%(MODEHEIGHT-TRANSITIONSQUARESIZE);
      target->bottom = target->top+TRANSITIONSQUARESIZE;
      target->right = target->left+TRANSITIONSQUARESIZE;

      gr->Rectangle(c);

      while(timer.SecondsPassed32() < TRANSITIONSECSPERSQUARE);
    }

  WriteLog("Done screen transition, releasing Pal");
  PalRelease();

  WriteLog("Going into Mode 13h...");
#ifndef WINDOWEDMODE
  gr->Recertify(CGraphics::MXS_13,GAME_MODEWIDTH,GAME_MODEHEIGHT,GAME_MODEBPP,GAME_MODEBUFFERS);
#else
  gr->Uncertify();
  gr->SetWindow(NULL);
  gr->ModeDim().cx = GAME_MODEWIDTH;
  gr->ModeDim().cy = GAME_MODEHEIGHT;
  gr->SetNumBuffers(GAME_MODEBUFFERS);
  gr->Certify();
#endif
  WriteLog("Now in 13h");

  WriteLog("Creating surfaces for cached maps");
  for(i = 0; i < CACHED_SECTORS; i++)
    {
      lower_cached[i].Create(SECTOR_WIDTH,SECTOR_HEIGHT);
      lower_cached_grid[i/CACHED_SECTORS_WIDE][i%CACHED_SECTORS_HIGH] = &lower_cached[i];
      upper_cached[i].Create(SECTOR_WIDTH,SECTOR_HEIGHT);
      upper_cached_grid[i/CACHED_SECTORS_WIDE][i%CACHED_SECTORS_HIGH] = &upper_cached[i];
    }
  WriteLog("Finished creating cached map surfaces");

  WriteLog("Presetting parameters for locking surface");
  static RECT lock_rectangle;
  lock_rectangle.left = 0;
  lock_rectangle.right = GAME_MODEWIDTH;
  lock_rectangle.top = 0;
  lock_rectangle.bottom = GAME_PORTHEIGHT;
  back_buffer_lock.SetArea(&lock_rectangle);
  back_buffer_lock.SetLockBehaviorFlags(DDLOCK_WAIT);
  back_buffer_lock.SetTargetSurface(*gr);

  WriteLog("Initializing Pal with menu palette");
  PalInitializeWithMenuPalette(*gr);
	
  WriteLog("Loading single player and multiplayer bitmaps");
  LoadBitmaps(RESOURCELOAD_MP);
  WriteLog("Loading single player and multiplayer sounds");
  LoadSounds(RESOURCELOAD_MP);

  WriteLog("Creating DirectDraw clipper used for bazooka explosions");
  RECT clip_rect = {0,0,GAME_MODEWIDTH,GAME_PORTHEIGHT}; 
  GLUclipper = gr->CreateClipper(vector<RECT>(1,clip_rect));

  WriteLog("Initializing Gamma");
  GamInitialize(*gr);

  WriteLog("Initializing Weather");
  WtrInitialize(ds);

  WriteLog("Loading important strings from tstring table");
  GluStrLoad(IDS_KILLED,KILLED);
  GluStrLoad(IDS_KILLEDYOURSELF,KILLEDYOURSELF);
  GluStrLoad(IDS_KILLEDTHEMSELVES,KILLEDTHEMSELVES);
  GluStrLoad(IDS_YOU,YOU);
  GluStrLoad(IDS_YOUKILLED,YOUKILLED);
  GluStrLoad(IDS_SPKILLED,SPKILLED);

  WriteLog("Calling SetupMenu()");
  SetupMenu();

  WriteLog("Flushing key presses and setting glue state");
  FlushKeyPresses();
  state = GLUESTATE_MAINMENU;

  WriteLog("Calling PrepareMenu() to prepare for main menu");
  PrepareMenu();

  WriteLog("Loading Chattahoochee main menu music");
  GluSetMusic(true,IDR_MENUMUSIC);

  WriteLog("Now leaving Introduction() function");
}

static void LoadBitmaps(int type)
{
  WriteLog("LoadBitmaps called with load type %d" LogArg(type));
  // first take care of the special case
  //  of reloading GLUbitmaps in case of surface loss
  //  or palette change
  int prev_bmp_count;
  if(RESOURCELOAD_RELOAD == type)
    {
      WriteLog("type is to reload, using recursion to get the job done");
      prev_bmp_count = bitmaps_loaded;
      LoadBitmaps(RESOURCELOAD_NONE);
      bitmaps_loaded = prev_bmp_count;
      prev_bmp_count = 0;
      WriteLog("Back from recursive call- proceeding as normal");
    }
  else
    {
      prev_bmp_count = bitmaps_loaded;
      bitmaps_loaded = BitmapCount(type);
    }

  WriteLog("prev_bmp_count: %d, bitmaps_loaded: %d" LogArg(prev_bmp_count) LogArg(bitmaps_loaded));

  if(bitmaps_loaded > prev_bmp_count)
    {
      WriteLog("we have to load more GLUbitmaps");
      for(int i = prev_bmp_count; i < bitmaps_loaded; i++)
	{
	  WriteLog("Loading bitmap %d" LogArg(i));
	  if(NULL == GLUbitmaps[i])
	    {
	      WriteLog("Bitmap is not already loaded; loading now");
	      HBITMAP loading = (HBITMAP)TryAndReport(LoadImage(hInstance,MAKEINTRESOURCE(IDB_BITMAP2+i),IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION));
	      GLUbitmaps[i] = TryAndReport(new CBob(loading));
	      TryAndReport(DeleteObject((HGDIOBJ)loading));
	    }
	}
    }
  else if(bitmaps_loaded < prev_bmp_count)
    {
      WriteLog("we have to release GLUbitmaps");
      for(int i = bitmaps_loaded; i < prev_bmp_count; i++)
	{
	  WriteLog("Releasing bitmap %d" LogArg(i));
	  delete GLUbitmaps[i];
	  GLUbitmaps[i] = NULL;
	}
    }
  WriteLog("LoadBitmaps() finished");
}

static void LoadSounds(int type)
{
  const int SOUND_COUNT[] = {0,NUM_SPSOUNDS,NUM_SOUNDS};
  int next_num_sounds = SOUND_COUNT[type];
  int prev_num_sounds = SOUND_COUNT[sounds_loaded];
  WriteLog("LoadSounds called w/%d sounds loaded, caller wants %d loaded" LogArg(prev_num_sounds) LogArg(next_num_sounds));
  if(next_num_sounds > prev_num_sounds)
    {
      WriteLog("We have to load more sounds");
      HRSRC res_handle;
      HGLOBAL data_handle;
      DWORD *sound_data = (DWORD *)GetResPtr(TEXT("SFX"),TEXT("DAT"),NULL,MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),res_handle,data_handle);
      if(NULL == sound_data)
	{
	  LoadSounds(0);
	  return;
	}
      int i;
      for(i = 0; i < NUM_PRESOUNDS+prev_num_sounds; i++)
	{
	  // skip the first seven sounds; they aren't ours
	  sound_data = (DWORD *)((BYTE *)sound_data+*sound_data)+1;
	}
      for(int i = prev_num_sounds; i < next_num_sounds; i++)
	{
	  WriteLog("Loading sound %d" LogArg(i));
	  CreateSBFromRawData(ds,&sounds[i],(void *)(sound_data + 1),*sound_data,DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY,SOUNDRESOURCEFREQ,SOUNDRESOURCEBPS,1);
	  WriteLog("Clearing reversed bit for sound %d" LogArg(i));
	  reversed.ClearBit(i);
	  // skip the first seven sounds; they aren't ours
	  sound_data = (DWORD *)((BYTE *)sound_data+*sound_data)+1;
	}
      FreeResource(data_handle);
    }
  else if(prev_num_sounds > next_num_sounds)
    {
      WriteLog("We have to release some sounds");
      for(int i = next_num_sounds; i < prev_num_sounds; i++)
	{
	  WriteLog("Releasing sound %d" LogArg(i));
	  if(NULL != sounds[i])
	    {
	      TryAndReport(sounds[i]->Release());
	      sounds[i] = NULL;
	    }
	}
    }

  sounds_loaded = type;

  WriteLog("LoadSounds exitting");
}

void GluPlaySound(int i,FIXEDNUM x_dist,FIXEDNUM y_dist)
{
  x_dist = FixedCnvFrom<long>(x_dist);
  y_dist = FixedCnvFrom<long>(y_dist);

  LPDIRECTSOUNDBUFFER b = sounds[i];
	
  LPDIRECTSOUNDBUFFER b2; // the duplicate of the sound

  float factor; // used in intermediate calculations

  if(NULL == b)
    {
      return; // we never loaded this sound
    }
	
  for(int slot = 0; slot < MAX_SOUNDS; slot++) // use loop to find empty sound slot
    {
      if(NULL != playing[slot])
	{
	  // this slot is used, leave . . .
	  continue;
	}

      if(SUCCEEDED(ds->DuplicateSoundBuffer(b,&b2)))
	{
	  // set frequency
	  DWORD freq;
	  if(SUCCEEDED(b2->GetFrequency(&freq)))
	    {
	      freq = FixedCnvFrom<long>(freq * sfxfreq);

				// set our new frequency
	      b2->SetFrequency(freq);
	    }

	  factor = (float)x_dist/(float)MAX_XDIST;
	  if(factor > 1.0f) // we were too far away
	    {
	      factor = 1.0f;
	    }
	  factor *= (float)DSBPAN_RIGHT;
	  b2->SetPan((long)factor);

	  factor = x_dist*x_dist+y_dist*y_dist - MIN_DISTSQUARED;
	  if(factor < 0)
	    {
	      factor = 0;
	    }
	  factor /= (float)(MAX_DISTSQUARED);
	  if(factor > 1.0f) // we were too far away
	    {
	      factor = 1.0f;
	    }
	  factor *= (float)(DSBVOLUME_MIN);
		
	  b2->SetVolume((long)factor);

	  b2->Play(0,0,0);

	  // add playing sound to the list
	  playing[slot] = b2;
	}

      break;
    }
}

static void SoundLoop()
{
  // loop through the sounds, release whichever ones are not playing
  for(int i = 0; i < MAX_SOUNDS; i++)
    {
      if(NULL == playing[i])
	{
	  return; // we already have an open slot, no need to free any
	}
      // get status of sound buffer
      DWORD s;
      playing[i]->GetStatus(&s);
      if(!(s & DSBSTATUS_PLAYING))
	{
	  // this sound is no longer playing, so release it
	  playing[i]->Release();
	  playing[i] = NULL;
	}

    }
}

static void PrepareMenu()
{
  SetSpeed(1);
  VCTR_STRING strings;
  tstring header;

  // fills menu with strings appropriate for the current state
  switch(state)
    {
    case GLUESTATE_INTRODUCTION:
    case GLUESTATE_GAME:
      break;
    case GLUESTATE_MAINMENU:
      HideMouseCursor(); // hide mouse in case it has been showing
      GluStrLoad(IDS_MAINMENUCAPTION,header);
      strings.resize(NUM_MAINMENUITEMS);
      GluStrVctrLoad(IDS_MAINMENUITEM1,strings);
      m->SetStrings(header,strings,0);
      break;
    case GLUESTATE_LEVELSELECT:
      Levels(strings);
      GluStrLoad(IDS_LEVELSELECTCAPTION,header);
      m->SetStrings(header,strings,strings.size()-1);
      break;
    case GLUESTATE_DIFFICULTYSELECT:
      strings.resize(DeeLevelAvailability(GLUlevel));
      GluStrVctrLoad(IDS_DIFFICULTYLEVEL1,strings);
      GluStrLoad(IDS_LEVELNAME1 + GLUlevel * 2,header);
      m->SetStrings(header,strings,strings.size()-1);

      // load GLUbitmaps without mp
      LoadBitmaps(RESOURCELOAD_SP);

      // load sounds without mp
      LoadSounds(RESOURCELOAD_SP);

      // figure out accomplishment text lines
      GLUdifficulty = m->GetSelectionIndex();
      DeeGetLevelAccomplishments(accomplishment_lines);
      break;
    case GLUESTATE_CONFIRMQUIT:
      strings.resize(NUM_YESNOMENUITEMS);
      GluStrVctrLoad(IDS_NO,strings);
      GluStrLoad(IDS_CONFIRMATIONCAPTION,header);
      m->SetStrings(header,strings,NO);
      break;
    case GLUESTATE_PICKGAME:
      ShowMouseCursor(); // show cursor in case we need connection info (user needs mouse!)
      strings.resize(NUM_MPGAMESLOTS);
      GluStrVctrLoad(IDS_GAMESLOTNAME1,strings);
      GluStrLoad(IDS_SELECTGAMECAPTION,header);
      m->SetStrings(header,strings,rand()%NUM_MPGAMESLOTS);
      break;
    case GLUESTATE_ENTERNAME:
      strings.resize(1);
      strings[0] = GLUname;
      GluStrLoad(IDS_ENTERNAMECAPTION,header);
      m->SetStrings(header,strings,0);

      // load extra multiplayer sounds
      LoadSounds(RESOURCELOAD_MP);

      break;
    case GLUESTATE_PICKCHARACTER:
      strings.resize(NUM_CHARACTERS);
      GluStrVctrLoad(IDS_CHARNAME1,strings);
      GluStrLoad(IDS_SELECTCHARACTERCAPTION,header);
      m->SetStrings(header,strings,CHAR_TURNER);

      // load extra multiplayer GLUbitmaps
      LoadBitmaps(RESOURCELOAD_MP);

      break;
    case GLUESTATE_SELECTCONNECTIONMETHOD:{
      VCTR_STRING con_names;
      tstring con_header;
	
      GluStrLoad(IDS_SELECTCONNECTIONMETHODCAPTION,con_header);

      if(0 == NetProtocols().size())
	{
	  con_names.resize(1);
	  GluStrLoad(IDS_NOCONNECTIONMETHODAVAILABLE,con_names[0]);
	}
      else
	{			
	  con_names = NetProtocols();
	}

      m->SetStrings(con_header,con_names,0);
    }
    }
  key_presses.push(0);
}

static int MenuLoop()
{
  BYTE last_key_pressed;
  bool show_demo_character =
    bool
    (
     GLUESTATE_PICKCHARACTER == state ||
     (GLUESTATE_LEVELSELECT == state && true == NetProtocolInitialized()) ||
     GLUESTATE_SELECTCONNECTIONMETHOD == state ||
     GLUESTATE_PICKGAME == state
     );
  do
    {
      GamOneFrame();
      SoundLoop(); // must be called each frame

      if(true == key_presses.empty())
	{
	  last_key_pressed = 0;
	}
      else
	{
	  last_key_pressed = key_presses.front();
	  key_presses.pop();
	}
		
      bool has_changed_position = false;

      if(VK_DOWN == last_key_pressed)
	{
	  // going up in the selections
	  if(true == m->MoveDown())
	    {
	      GluPlaySound(WAV_STEP,Fixed(1),false);
	      has_changed_position = true;
	    }
	}
      else if(VK_UP == last_key_pressed)
	{
	  // going down in the selections
	  if(true == m->MoveUp())
	    {
	      GluPlaySound(WAV_STEP,Fixed(1),true);
	      has_changed_position = true;
	    }
	}

      // put the menu background on the screen
      RECT *r = &gr->TargetScreenArea();
      r->left = 0;
      r->top = 0;
      r->bottom = CGraphics::ModeHeight();
      r->right = CGraphics::ModeWidth();

      if(FAILED(TryAndReport(gr->PutFast(*GLUbitmaps[BMP_MENU],false))))
      {
	WriteLog("Failed to blit menu backdrop with PutFast, now using just regular Put");
	
	if(FAILED(TryAndReport(gr->Put(*GLUbitmaps[BMP_MENU],0))))
	  {
	    WriteLog("Failed to blit menu backdrop with regular Put too!  "
		     "Filling the screen with black so at least the text "
		     "can be read.");

	    TryAndReport(gr->Rectangle(0));
	  } // end if failed to blit with regular Put
      } // end if failed to blit with PutFast

      m->FillSurface(*gr);

      // show GLUdifficulty GLUlevel accomplishments
      if(GLUESTATE_DIFFICULTYSELECT == state)
	{
	  // if we have changed our position, and we are selecting GLUdifficulty,
	  //  then we have to update our accomplishment data
	  if(true == has_changed_position)
	    {
	      GLUdifficulty = m->GetSelectionIndex();
	      DeeGetLevelAccomplishments(accomplishment_lines);
	    }
			
	  HDC dc;
	  if(SUCCEEDED(gr->Buffer(1)->GetDC(&dc)))
	    {
				// select new parameters in, saving the old ones
	      int old_bk_mode = SetBkMode(dc,TRANSPARENT);
	      COLORREF old_text_color = SetTextColor(dc,COLOR_ACCOMPLISHMENTTEXT);

				// print the text
	      TextOut
		(
		 dc,
		 XCOOR_ACCOMPLISHMENTTEXT,YCOOR_ACCOMPLISHMENTTEXT,
		 accomplishment_lines[0].c_str(),accomplishment_lines[0].length()
		 );

				// restore old text-drawing parameters
	      SetTextColor(dc,old_text_color);
	      SetBkMode(dc,old_bk_mode);
	      gr->Buffer(1)->ReleaseDC(dc);
	    }
	}

      // display character if selecting character
      else if(true == show_demo_character)
	{
	  // make sure we don't have any whacked-out values in char_demo_* vars
	  if(GLUchar_demo_direction < 0 || GLUchar_demo_direction >= RENDERED_DIRECTIONS)
	    {
	      GLUchar_demo_direction = DSOUTH;
	    }

	  if(char_demo_direction_changer.SecondsPassed32() >= DEMOCHAR_SECSTOCHANGEDIR)
	    {	
	      GLUchar_demo_direction = rand()%RENDERED_DIRECTIONS;
	      char_demo_direction_changer.Restart();
	    }

	  int bmp=BMPSET_CHARACTERS;
		
	  if(char_demo_stepper.SecondsPassed32() > DEMOCHAR_SECSTOSTEP)
	    {
	      bmp += RENDERED_DIRECTIONS;
	      if(char_demo_stepper.SecondsPassed32() > DEMOCHAR_SECSTOSTEP*2.0f)
		{
		  char_demo_stepper.Restart();
		}
	    }

	  bmp+=GLUchar_demo_direction;

	  RECT *target = &gr->TargetScreenArea();

	  if(GLUESTATE_PICKCHARACTER == state)
	    {
	      target->left = DEMOCHAR_X;
	      target->top = DEMOCHAR_Y;
	      bmp+=m->GetSelectionIndex()*ANIMATIONFRAMESPERCHARACTER;
	    }
	  else
	    {
	      target->left = DEMOCHAR_X2;
	      target->top = DEMOCHAR_Y2;
	      bmp+=model*ANIMATIONFRAMESPERCHARACTER;
	    }

	  target->right = target->left + TILE_WIDTH;
	  target->bottom = target->top + TILE_HEIGHT;

	  gr->PutFast(*GLUbitmaps[bmp]);
	}

      Flip();
    }
  while
    (
     VK_RETURN != last_key_pressed &&
     VK_ESCAPE != last_key_pressed
     );

  if(VK_RETURN == last_key_pressed)
    {
      GluPlaySound(WAV_OKGOTIT,FREQFACTOR_OKGOTITNORMAL,false); // Play the okay got it sound
      return MENUACTION_RETURN;
    }
  else
    {
      GluPlaySound(WAV_OKGOTIT,FREQFACTOR_OKGOTITBACKWARDS,true); // play the okay got it sound backwards
      return MENUACTION_ESCAPE;
    }
}

static const BYTE *ExtractByte(const BYTE *ptr,int& target)
{
  BYTE in = *ptr;
  ptr+=sizeof(BYTE);
  target = in;
  return ptr;
}

static const BYTE *ExtractWord(const BYTE *ptr,int& target)
{
  unsigned short in = *((unsigned short *)ptr);
  ptr+=sizeof(WORD);
  target = in;
  return ptr;
}

static void LoadLevel()
{
  int i;
  int j;
  int k;
  int l;
  int m;
  int n;
  int o;
  int p;

  WriteLog("LoadLevel has been called");
  RECT *target = &gr->TargetScreenArea();
  target->left= 0;
  target->top = 0;
  target->bottom = GAME_MODEHEIGHT;
  target->right = GAME_MODEWIDTH;

  WriteLog("Making a blank screen with screen transitions");
  gr->Rectangle(0,true);
  gr->Transition(rand()%NUM_TRANSITIONS);

  WriteLog("Making sure the GLUlevel index is valid.");
  assert(GLUlevel >= 0);

  tstring path;
	
  GluStrLoad(IDS_LEVELFILE1+GLUlevel*2,path);

  WriteLog("User is loading level with tstring id: %s" LogArg(path.c_str()));

  // load the file by getting a pointer to the resource

  // GET A POINTER TO RESOURCE DATA 
  //  do this by calling FindResourceEx, LoadResource, and then LockResource
  HRSRC res_handle = FindResourceEx(NULL,TEXT("LVL"),path.c_str(),MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL));
  HGLOBAL data_handle = LoadResource(NULL,res_handle);
  const BYTE *data_ptr = (const BYTE *)LockResource(data_handle);

  // load the palettes
  if(LEVELLOAD_RELOADBONE&level_loaded)
    {
      PalRelease();
      data_ptr = PalInitialize
	(
	 *gr,data_ptr,NetProtocolInitialized()
	 );
      data_ptr = PalInitialize
	(
	 *gr,data_ptr,!NetProtocolInitialized()
	 );

      // get GLUlevel width and height
      int level_width;
      int level_height;
      data_ptr = ExtractWord(data_ptr,level_width);
      data_ptr = ExtractWord(data_ptr,level_height);
      LoadCmps(level_width,level_height,!(LEVELLOAD_RELOADFLESH & level_loaded));
    }
  else
    {
      data_ptr = PalInitialize(*gr,data_ptr,true);
      data_ptr = PalInitialize(*gr,data_ptr,true);
      // rush past GLUlevel width and height (four bytes)
      data_ptr += sizeof(WORD) * 2 ;
    }

  // skip the rest if we don't have to load item data and positions
  if(!(LEVELLOAD_RELOADFLESH & level_loaded))
    {
      FreeResource(data_handle);
      // make sure weather is working on right palette
      WtrNextState();
      level_loaded = LEVELLOAD_OKAY;
      return;
    }

  // load weather script index
  int script_index;
  data_ptr = ExtractByte(data_ptr,script_index);
  if(false == NetInGame())
    {
      WtrSetScript(script_index);
    }

  possible_starting_spots.clear();

  // get GLUhero data

  // get turner's coordinates
  data_ptr = ExtractWord(data_ptr,i);
  data_ptr = ExtractWord(data_ptr,j);
  i = FixedCnvTo<long>(i);
  j = FixedCnvTo<long>(j);
	
  AddPossibleStartingSpot(i,j);

  if(false == NetProtocolInitialized())
    {
      // single-player behaviour
      GLUhero.Setup(i,j,CHAR_TURNER,false);
    }
  else
    {
      GLUhero.Setup(-1,-1,model,true);
    }

  ul_cached_sector_x = ul_cached_sector_y = -1;

  assert(i >= 0);
  assert(j >= 0);

  walking_data->Clear();

  // loop through each rectangle which defines indoor regions

  data_ptr = ExtractByte(data_ptr,j);

  for(i = 0; i < j; i++)
    {
      data_ptr = ExtractWord(ExtractWord(ExtractWord(ExtractWord(data_ptr,m),n),o),p);

      for(k = n; k < p; k+= TILE_HEIGHT)
	{
	  assert(k/TILE_HEIGHT >= 0);
	  assert(k >= 0);
	  for(l = m; l < o; l+=TILE_WIDTH)
	    {
	      walking_data->SetBit(k/TILE_HEIGHT,l/TILE_WIDTH);
	    }
	}
    }

  // clear sectors
  for(i = 0; i < sectors.cols(); i++)
    {
      for(j = 0; j < sectors.rows(); j++)
	{
	  sectors[j][i].GLUenemies.clear();
	  sectors[j][i].GLUpowerups.clear();
	  sectors[j][i].level_ends.clear();
	}
    }

  // so now let's do the GLUlevel ends

  data_ptr = ExtractByte(data_ptr,j);
		
  if(false == NetInGame())
    {
      lends.resize(j);
    }
  else
    {
      lends.clear();
    }

  for(i = 0; i < j; i++)
    {
      data_ptr = ExtractByte(data_ptr,k);
      data_ptr = ExtractWord(data_ptr,l);
      data_ptr = ExtractWord(data_ptr,m);
	
      if(false == NetInGame())
	{
	  lends[i].Setup(FixedCnvTo<long>(l),FixedCnvTo<long>(m),k);
	  sectors[m/SECTOR_HEIGHT][l/SECTOR_WIDTH].level_ends.insert(i);
	}
    }

  int possible_score = 0; // reset the potential score counter

  // do the GLUenemies

  if(false == NetInGame())
    {
      GLUenemies.clear();
    }

  for(p = 0; p < 3; p++) // p is the current enemy type
    {
      data_ptr = ExtractByte(data_ptr,j); // need to know how many there are
      if(false == NetInGame())
	{
	  GLUenemies.resize(GLUenemies.size()+j);
	}
      for(i = 0; i < j; i++)
	{
	  data_ptr = ExtractWord(data_ptr,k); // get x position
	  data_ptr = ExtractWord(data_ptr,l); // get y position
	  if(false == NetInGame())
	    {
	      int magic_index = GLUenemies.size()-i-1;
	      GLUenemies[magic_index].Setup(FixedCnvTo<long>(k),FixedCnvTo<long>(l),p,false);
	      int sec_row;
	      int sec_col;
	      GLUenemies[magic_index].CalculateSector(sec_row,sec_col);
	      sectors[sec_row][sec_col].GLUenemies.insert(magic_index);
	    }
	  else
	    {
	      AddPossibleStartingSpot(FixedCnvTo<long>(k),FixedCnvTo<long>(l));
	    }

	  // increment possible score
	  if(CHAR_EVILTURNER == p)
	    {
	      possible_score += GluScoreDiffKill(CHAR_SALLY);
	    }
	  possible_score +=GluScoreDiffKill(p);			
	}
    }

  // do the ammo and health like we did the GLUenemies
  GLUpowerups.clear();

  for(p = 0; p < 4; p++)
    {
      data_ptr = ExtractByte(data_ptr,j);
      GLUpowerups.resize(GLUpowerups.size()+j);

      for(i = 0; i < j; i++)
	{
	  data_ptr = ExtractWord(data_ptr,k);
	  data_ptr = ExtractWord(data_ptr,l);
	  sectors[l/SECTOR_HEIGHT][k/SECTOR_WIDTH].GLUpowerups.insert(GLUpowerups.size()-i-1);

	  k = FixedCnvTo<long>(k);
	  l = FixedCnvTo<long>(l);

	  GLUpowerups[GLUpowerups.size()-i-1].Setup(k,l,p);
			
	  AddPossibleStartingSpot(k,l);

	  possible_score += GluScoreDiffPickup(p);
	}
    }

  std_powerups = GLUpowerups.size();

  // start weather script
  WtrBeginScript();

  // reset score and calculate its coordinates
  ResetSinglePlayerScore(possible_score);

  // reset timer shown in bottom left of screen
  since_start = 0;

  // and now we've finished
  FreeResource(data_handle);

  level_loaded = LEVELLOAD_OKAY;

  // announce our model if we are multiplayer
  if(true == NetInGame())
    {
      WriteLog("Setting player data from Glue module");
      NetSetPlayerData();
    } // end if we are in a game
}

static void PrepareForMPGame()
{
  GLUdifficulty = MPDIFFICULTY;

  // reset score and calculate its coordinates
  GluChangeScore(0);

  HideMouseCursor(); // hide cursor (no longer needed)

  WtrNextState();
  NetWelcome();	
}

static void EndGame()
{
  // TODO: ADD END-GAME CODE HERE
}

bool GluWalkingData(FIXEDNUM x,FIXEDNUM y)
{

  // of course, parameters that were not passed by reference won't change the caller's copy
  x/=TILE_WIDTH; 
  y/=TILE_HEIGHT;

  return walking_data->IsBitSet(FixedCnvFrom<long>(y),FixedCnvFrom<long>(x));
}

// this function flips and returns true if the direct draw surfaces need to be reloaded
static bool Flip()
{
  // first see if we need to back off for a second due to alt-tabbing
  MSG msg;
  while(PeekMessage(&msg,hWnd,0,0,PM_REMOVE))
    {
      // check if we are being minimized/deactivated
      if(WM_ACTIVATEAPP == msg.message && FALSE == msg.wParam)
	{
	  WriteLog("User is alt-tabbing away from Andradion 2.  Now entering suspended game mode . . .");
	  // go into the GetMessage mode
	  while(true)
	    {
	      DispatchMessage(&msg);
	      GetMessage(&msg,hWnd,0,0);

	      if(WM_ACTIVATEAPP == msg.message && TRUE == msg.wParam)
		{
		  DispatchMessage(&msg);
		  GluRestoreSurfaces();
		  WriteLog("User has restored Andradion 2.");
		  return true;
		}
	    }
	}
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

  if(true == do_transition)
    {
      WriteLog("Starting a screen transition . . .");

      // do a cool screen transition
      gr->Transition(rand()%NUM_TRANSITIONS);

      do_transition = false;
      WriteLog("Finished!");
    }
  else
    {
      // display frame rate/Choppiness factor if the user is holding down the C key
      if(GLUkeyb[DIK_C] & EIGHTHBIT)
	{
	  static int frames = 0;
	  static NGameLib2::CTimer counter;

	  frames++;
	  if(counter.SecondsPassed32() >= 1.0f)
	    {
	      TCHAR buffer[FRAMERATE_BUFFERLEN];
	      frames = fps - frames;
	      wsprintf(buffer,ONE_NUMBER_FORMAT,frames);
	      GluPostMessage(buffer);
	      frames = 0;
	      counter.Restart();
	    }
	}

      if(GLUESTATE_GAME == state)
	{
	  GamOneFrame();
	}

#ifdef _DEBUG
      if(GLUESTATE_GAME == state)
	{
	  // show profiler results
	  VCTR_STRING profiler_results;
	  GetProfileData(profiler_results);
	  HDC dc;
	  if(SUCCEEDED(gr->GetTargetBufferInterface()->GetDC(&dc)))
	    {
	      int old_bk_mode = SetBkMode(dc,TRANSPARENT);
	      HGDIOBJ old_font = SelectObject(dc,profiler_font);
	      COLORREF old_text_color = SetTextColor(dc,RGB(255,255,255));
	      int y = 0;
	      for(VCTR_STRING::iterator i = profiler_results.begin(); i != profiler_results.end(); i++)
		{
		  TextOut(dc,0,y,i->c_str(),i->length());
		  y += PROFILER_FONT_SIZE;
		}
	      SetBkMode(dc,old_bk_mode);
	      SetTextColor(dc,old_text_color);
	      SelectObject(dc,old_font);
	      gr->GetTargetBufferInterface()->ReleaseDC(dc);
	    }
	}

#endif

#ifndef WINDOWEDMODE
      gr->Flip(DDFLIP_WAIT);
#else
      gr->Buffer(0)->BltFast(0,0,gr->Buffer(1),NULL,FALSE);
#endif


      if(GLUESTATE_GAME == state)
	{
	  static CTimer syncer;
	  while(syncer.SecondsPassed32() < spf);
	  syncer.Restart();

	  // a complete frame has passed
	  since_start += TIMER_INC_PER_FRAME;
	  if(since_start > MAX_TIMERSECONDS)
	    {
	      since_start = MAX_TIMERSECONDS;
	    }
	}
    }

  return false;
}

void GluPostMessage(const TCHAR *str)
{
  // make sure we are done with the current message
  if(frames_for_current_message < MINFRAMESTOKEEPMESSAGE)
    {
      return;
    }

  // put a message on the screen
  message = str;
	
  // calculate the x-coordinate of our message
  msg_x = (GAME_MODEWIDTH  - (message.length() * (FONTWIDTH+1))) / 2;

  // reset the timer
  frames_for_current_message = 0;

  // the end
}

void GluChangeScore(int diff)
{
  int new_score;
  int max_score;

  _stscanf(score.c_str(),TWO_NUMBERS_FORMAT,&new_score,&max_score);

	
  if(0 == diff)                 { new_score = 0        ;}
  if(new_score > MAXSCORE)      { new_score = MAXSCORE ;}
  else if(new_score < MINSCORE) { new_score = MINSCORE ;}
  else                          { new_score += diff    ;}

  TCHAR str_score[SCORELEN]; 
  if(true == NetInGame())
    {
      // don't care about maximum score
      wsprintf(str_score,ONE_NUMBER_FORMAT,new_score);
    }
  else
    {
      wsprintf(str_score,TWO_NUMBERS_FORMAT,new_score,max_score);
    }

  score = str_score;

  // now calculate printing coordinates
  CalculateScorePrintX();

  if(false == NetInGame() && (const)max_score == (const)new_score)
    {
      // colors should be flashing because we have the highest score,
      //  so flag it by making score_print_x negative
      score_print_x = -score_print_x;

      tstring max_score_message;
      GluStrLoad(IDS_MAXSCORE,max_score_message);
      GluPostMessage(max_score_message.c_str());
    }
}

void GluKeyPress(int scan_code)
{
  if(VK_F1 == scan_code)
    {
      // do a print screen

      // figure what file GLUname to use
      TCHAR fn[MAX_STRINGLEN];
      int timer_sec;
      int timer_min;
      GetLevelTimerMinSec(timer_min,timer_sec);
      wsprintf(fn,SCREENSHOTFN,GLUlevel,timer_min,timer_sec);

      // get a pointer to the target screen area specification
      RECT *ta = &gr->TargetScreenArea();

      // backup old target screen area
      RECT old_target_area = *ta;

      // specify the target area as the entire screen:
      ta->left = 0;
      ta->top = 0;
      ta->right = CGraphics::ModeWidth();
      ta->bottom = CGraphics::ModeHeight();

      // take a screenshot
      HANDLE file = CreateFile(fn,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
      gr->Screenshot(file);
      CloseHandle(file);

      *ta = old_target_area;
    }
  else if(GLUESTATE_GAME != state || PAUSE_KEY == scan_code)
    {
      key_presses.push(scan_code);
    }
}

void GluCharPress(TCHAR c)
{
  if(GLUESTATE_ENTERNAME == state)
    {
      switch(c)
	{
	case TCHAR('\b'):
	  // backspace was pressed
	  if(GLUname.length() > 0)
	    {
	      GLUname = GLUname.substr(0,GLUname.length()-1);
	    }
	  GluPlaySound(WAV_BING);
	  break;
	case (TCHAR)'\r':
	case (TCHAR)'\n':
	case (TCHAR)'\t':
	case (TCHAR)'\a':
	case (TCHAR)'\f':
	case (TCHAR)'\v':
	case (TCHAR)27: // escape
	  // a key we don't care about was pressed
	  break;
	default:
	  GluPlaySound(WAVSET_POINTLESS+(rand()%WAVSINASET),Fixed(1),rand()&1 ? true : false);
	  GLUname += c;
	}
      // reset the menu
      tstring header; 
      VCTR_STRING strings;
      GluStrLoad(IDS_ENTERNAMECAPTION,header);
      strings.resize(1);
      strings[0] = GLUname;
      m->SetStrings(header,strings,0);
      return;
    }
  else if(GLUESTATE_GAME == state && false == NetInGame())
    {
      // we are doing single-player, we may want to show the player some
      //  best time/ best score data
      switch(c)
	{
	case TCHAR('8'):
	  SetSpeed(0);
	  break;
	case TCHAR('9'):
	  SetSpeed(1);
	  break;
	case TCHAR('0'):
	  SetSpeed(2);
	  break;
	case TCHAR('t'):
	case TCHAR('T'):
	  // show best time
	  GluPostMessage(accomplishment_lines[1].c_str());
	  break;
	case TCHAR('s'):
	case TCHAR('S'):
	  // show best score
	  GluPostMessage(accomplishment_lines[2].c_str());
	  break;
	}
    }
}

void GluRestoreSurfaces()
{
  // restores surfaces and gets the GLUbitmaps loaded back into them
  if(GLUESTATE_GAME == state)
    {
      level_loaded |= LEVELLOAD_RELOADBONE;
    }
  else if(GLUESTATE_UNINITIALIZED == state)
    {
      return;
    }
  else if(GLUESTATE_INTRODUCTION == state)
    {
      //gr->DirectDraw()->RestoreAllSurfaces();
      return;
    }
	
  PalRelease();

	// when we initialize with the menu palette, the GLUbitmaps
	//  for the characters will be reloaded
  PalInitializeWithMenuPalette(*gr);
	
  delete m;
  SetupMenu();
  PrepareMenu();
}

static void SetupMenu()
{
  // setup the menu
  LOGFONT mf; // font to use
  MenuFont(mf);
  // setup the menu
  m = new CMenu(mf,COLOR_UNSELECTED,COLOR_SHADOW,COLOR_SELECTED,COLOR_SHADOW,COLOR_HEADING,COLOR_SHADOW,SHADOW_OFFSET);
}

void GluPostSPKilledMessage() 
{
  GluPostMessage(SPKILLED.c_str());
}

static void MenuFont(LOGFONT& lf)
{
  lf.lfHeight = 16;
  lf.lfWidth = 0;
  lf.lfEscapement = 0;
  lf.lfOrientation = 0;
  lf.lfWeight = 600;
  lf.lfUnderline = FALSE;
  lf.lfItalic = FALSE;
  lf.lfStrikeOut = FALSE;
  lf.lfCharSet = DEFAULT_CHARSET;
  lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
  lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
  lf.lfQuality = DRAFT_QUALITY;
  lf.lfPitchAndFamily = FF_ROMAN | VARIABLE_PITCH ;
  lf.lfFaceName[0] = TCHAR('\0');
}

static void Levels(VCTR_STRING& target)
{
  // find the first available GLUlevel from the end
  DWORD i;
  for(i = NUM_LEVELS -1; i >= 0; i--)
    {
      if(LEVELAVAIL_NONE != DeeLevelAvailability(i))
	{
	  break;
	}
    }
	
  // fill up every element
  target.resize(NUM_LEVELS); // assume player has every GLUlevel
  char buffer[MAX_STRINGLEN];
  int i3 = 0;
  for(int i2 = IDS_LEVELNAME1; i2 < IDS_LEVELNAME1+NUM_LEVELS*2; i2+=2)
    {
      LoadString(hInstance,i2,buffer,MAX_STRINGLEN);
      target[i3++] = buffer;
    }
  target.resize(i+1); // shrink down to chop off all trailing AVAIL_NONE's
	
	// add unavailable tstring to the end of every GLUlevel that's unavailable
  LoadString(hInstance,IDS_UNAVAILABLELEVEL,buffer,MAX_STRINGLEN);

  for(i = 0; i < target.size(); i++)
    {
      if(LEVELAVAIL_NONE == DeeLevelAvailability(i))
	{
	  target[i] = buffer;
	}
    }
}

static void ShowMouseCursor()
{
  while(ShowCursor(TRUE) < 0);
}

static void HideMouseCursor()
{
  while(ShowCursor(FALSE) >= 0);
}

void GluStrLoad(unsigned int id,tstring& target)
{
  int str_len = (IDS_BUDGETCUTS == id || IDS_OLDDX == id) ? LONG_STRINGLEN : MAX_STRINGLEN;

  TCHAR *buffer = new TCHAR[str_len];

  LoadString(hInstance,id,buffer,str_len);
	
  target = buffer;

  delete buffer;
}

void GluInterpretDirection(int d,FIXEDNUM& xf,FIXEDNUM& yf)
{
  // fix the xf
  switch(d)
    {
    case DNORTH: case DSOUTH:           xf = Fixed( 0); break;
    case DWEST : case DNW   : case DSW: xf = Fixed(-1); break;
    default:                            xf = Fixed( 1);
    }

  // fix up the yf
  switch(d)
    {
    case DWEST : case DEAST:           yf = Fixed( 0); break;
    case DNORTH: case DNE  : case DNW: yf = Fixed(-1); break;
    default:                           yf = Fixed( 1); 
    }
}

void GluStrVctrLoad(unsigned int id,VCTR_STRING& target)
{
  TCHAR buffer[MAX_STRINGLEN];
	
  for(VCTR_STRING::iterator iterate = target.begin(); iterate != target.end(); iterate++)
    {
      LoadString(hInstance,id++,buffer,MAX_STRINGLEN);

      *iterate = buffer;
    }
}

static void LoadCmps(int level_width,int level_height,bool skip_wd_resize)
{
  // calculate these members so there is never
  //  any dead space at the edges of the screen when
  //  the GLUhero is near the edge of the GLUlevel
  max_center_screen_x = FixedCnvTo<long>(level_width  - GAME_MODEWIDTH/2);
  max_center_screen_y = FixedCnvTo<long>(level_height - GAME_PORTHEIGHT/2);

  int tw; // tile width and height
  int th;

  tw = level_width / TILE_WIDTH;
  if(0 != level_width % TILE_WIDTH || level_width < TILE_WIDTH)
    {
      tw++;
    }
  th = level_height / TILE_HEIGHT;
  if(0 != level_height % TILE_HEIGHT || level_height < TILE_HEIGHT)
    {
      th++;
    }

  int sw; // sector width and height
  int sh;

  sw = level_width / SECTOR_WIDTH;
  if(0 != level_width % SECTOR_WIDTH || level_width < SECTOR_WIDTH)
    {
      sw++;
    }
  sh = level_height / SECTOR_HEIGHT;
  if(0 != level_height % SECTOR_HEIGHT || level_height < SECTOR_HEIGHT)
    {
      sh++;
    }

  tstring path;

  if(false == skip_wd_resize)
    {
      sectors.resize(sh,sw);
      delete walking_data;
      walking_data = new BitArray2D(th,tw);
    }
				
  GluStrLoad(IDS_LEVELPATH,path);

  int r;

  tstring level_name;

	// figure which module we need to load from
  HMODULE mod = (GLUlevel >= FIRSTLIB2LEVEL) ? level_lib_mod_2 : level_lib_mod;

  // GLUname of the GLUlevel is loaded here
  GluStrLoad(IDS_LEVELFILE1 + GLUlevel * 2,level_name);

  for(r = 0; r < sectors.rows(); r++)
    {
      char row_text[MAXLEN_SECTORCOOR];

      itoa(r,row_text,10);
		
      for(int c = 0; c < sectors.cols(); c++)
	{
	  char col_text[MAXLEN_SECTORCOOR];

	  itoa(c,col_text,10);

	  tstring fn;
			
	  fn = level_name;
	  fn += TCHAR('_');
	  fn += col_text;
	  fn += TCHAR('X');
	  fn += row_text;
			
	  if(true == sectors[r][c].lower_cell.Certified())
	    {
	      sectors[r][c].lower_cell.Uncertify();
	    }

	  sectors[r][c].lower_cell.SetLoadFromResource(true);
	  sectors[r][c].lower_cell.FileName() = fn;
	  sectors[r][c].lower_cell.SetResourceLanguage(MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
	  sectors[r][c].lower_cell.SetResourceModule(mod);
	  sectors[r][c].lower_cell.ResourceType() = CMP_RESOURCE_TYPE;
	  sectors[r][c].lower_cell.Certify();
	}
    }

  for(r = 0; r < sectors.rows(); r++)
    {
      char row_text[MAXLEN_SECTORCOOR];

      itoa(r,row_text,10);
		
      for(int c = 0; c < sectors.cols(); c++)
	{
	  char col_text[MAXLEN_SECTORCOOR];

	  itoa(c,col_text,10);

	  tstring fn = level_name + TCHAR('U') + col_text + TCHAR('X') + row_text;

	  if(true == sectors[r][c].upper_cell.Certified())
	    {
	      sectors[r][c].upper_cell.Uncertify();
	    }

	  sectors[r][c].upper_cell.SetLoadFromResource(true);
	  sectors[r][c].upper_cell.FileName() = fn;
	  sectors[r][c].upper_cell.SetResourceLanguage(MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
	  sectors[r][c].upper_cell.SetResourceModule(mod);
	  sectors[r][c].upper_cell.ResourceType() = CMP_RESOURCE_TYPE;
	  sectors[r][c].upper_cell.Certify();
	}
    }
}

void GluFilterMovement(pair<POINT,POINT>& plans)
{
  // this method is public because the CFire class needs to use it
  //  the plans parameter is passed as a non-const reference because
  //  we will change the second part of the pair to tell the mover where
  //  they can go which is closest to where they wanted to go

	// first check to make sure they are on the screen, and that we have 
	//  the back buffer successfully locked
  if(
     abs(GLUcenter_screen_x - plans.first.x) >= Fixed(GAME_MODEWIDTH/2) ||
     abs(GLUcenter_screen_y - plans.first.y) >= Fixed(GAME_PORTHEIGHT/2) ||
     false == back_buffer_lock.Certified()
     )
    {
      // this guy isn't even on the screen to begin with . . .
      plans.second = plans.first;
      return;
    }

  // now pre-clip the second part of the plans pair to the edge of the screen

	// check x-coor
  if(plans.second.x - GLUcenter_screen_x >= Fixed(GAME_MODEWIDTH/2))
    {
      // too far off to the right . . .
      plans.second.x = GLUcenter_screen_x + Fixed(GAME_MODEWIDTH/2 - 1);
    }
  else if(GLUcenter_screen_x - plans.second.x >= Fixed(GAME_MODEWIDTH/2))
    {
      // too far off to the left . . .
      plans.second.x = GLUcenter_screen_x - Fixed(GAME_MODEWIDTH/2 - 1);
    }

  // check y-coor
  if(plans.second.y - GLUcenter_screen_y >= Fixed(GAME_PORTHEIGHT/2))
    {
      // too far down . . .
      plans.second.y = GLUcenter_screen_y + Fixed(GAME_PORTHEIGHT/2 - 1);
    }
  else if(GLUcenter_screen_y - plans.second.y >= Fixed(GAME_PORTHEIGHT/2))
    {
      // too far up . . .
      plans.second.y = GLUcenter_screen_y - Fixed(GAME_PORTHEIGHT/2 - 1);
    }

  int x_change = FixedCnvFrom<long>(plans.second.x - plans.first.x);
  int y_change = FixedCnvFrom<long>(plans.second.y - plans.first.y);

  int gen_change; // generic change
  FIXEDNUM *axis;    // used by the generic part of the
  FIXEDNUM axis_inc; //  filtering code (the code used for any non-diagonal movement)
	
  int inc; // increments to use into surface memory

  int surface_pitch = back_buffer_lock.SurfaceDesc().lPitch;
  BYTE *surface_data = (BYTE *)back_buffer_lock.SurfaceDesc().lpSurface;

  if(0 != x_change)
    {
      if(0 != y_change)
	{
	  // moving diagonally (never use rerouting in this case)

	  // change to absolute value of these two . . .
	  if(y_change < 0)
	    {
	      y_change = -y_change;
	    }
	  if(x_change < 0)
	    {
	      x_change = -x_change;
	    }

	  inc = surface_pitch - x_change - 1;

	  // offset into surface memory:
	  int offset =
	    surface_pitch * FixedCnvFrom<long>(__min(plans.first.y,plans.second.y)-GLUcenter_screen_y+Fixed(GAME_PORTHEIGHT/2)) +
	    FixedCnvFrom<long>(__min(plans.first.x,plans.second.x)-GLUcenter_screen_x+Fixed(GAME_MODEWIDTH /2)) ;

	  for(int y = 0; y <= y_change; y++)
	    {
	      for(int x = 0; x <= x_change; x++)
		{
		  if(0 == surface_data[offset++])
		    {
		      // there was some black there . . .
		      plans.second = plans.first;
		      return;
		    }
		}
	      offset+=inc;
	    }

	  return; // no more to do
	}
      else
	{
	  // moving horizontally

	  if(x_change < 0)
	    {	
	      inc = -1;
	      gen_change = -x_change;
	      axis_inc = -Fixed(1);
	    }
	  else
	    {
	      inc = 1;
	      gen_change = x_change;
	      axis_inc = Fixed(1);
	    }
	
	  axis = (FIXEDNUM *)&plans.second.x;
	}
    }
  else if(0 != y_change)
    {
      // moving vertically

      if(y_change < 0)
	{
	  inc = -surface_pitch;
	  gen_change = -y_change;
	  axis_inc = Fixed(-1);
	}
      else
	{
	  inc = surface_pitch;
	  gen_change = y_change;
	  axis_inc = Fixed(1);
	}

      axis = (FIXEDNUM *)&plans.second.y;
    }
  else
    {
      return;
    }

  // offset into surface memory:
  int offset =
    surface_pitch * FixedCnvFrom<long>(plans.first.y-GLUcenter_screen_y+Fixed(GAME_PORTHEIGHT/2)) +
    FixedCnvFrom<long>(plans.first.x-GLUcenter_screen_x+Fixed(GAME_MODEWIDTH /2)) ;

  plans.second = plans.first;

  *axis += axis_inc;
  offset += inc;

	// moves is automatically non-zero if we push Turner when he moves unsuccessully
  int moves = (ALWAYSPUSH >> GLUlevel) & 1;

  for(int i = 0; i < gen_change; i++)
    {
      if(0 == surface_data[offset])
	{
	  *axis -= axis_inc;
	  break;
	}
      *axis += axis_inc;
      offset += inc;
      moves++;
    }
	
  if(0 != moves) {*axis -= axis_inc;}
}

void GluGetRandomStartingSpot(POINT& p)
{
  p = possible_starting_spots[rand()%possible_starting_spots.size()];
  ul_cached_sector_x = ul_cached_sector_y = -1;

  // this is a pretty simple function,
  //  see CGlue header for some more comments
}

static void AddPossibleStartingSpot(FIXEDNUM x, FIXEDNUM y)
{
  if(true == NetProtocolInitialized())
    {
      // we are doing mp, so we need to store these extra
      //  coordinates
      int magic_i = possible_starting_spots.size();

      possible_starting_spots.resize(magic_i+1);

      possible_starting_spots[magic_i].x = x;
      possible_starting_spots[magic_i].y = y;
    }

  // this is a simple internal function to make the LoadLevel function
  //  a little cleaner
}

void GluSetMusic(bool loop,const TCHAR *music_resource)
{
  // only play music if it was not disabled by
  //  the intro/welcome dialog, and make sure we
  //  don't play music that's already going
  WriteLog("SetMusic type A called to use music resource %s" LogArg(music_resource));
  if(false == disable_music && last_music != music_resource)
    {
      TryAndReport(MusicPlay(loop,music_resource,MIDI_RESOURCE_TYPE));
      last_music = music_resource;
    }
  if(GLUESTATE_GAME == state)
    {
      SetSpeed(GetSpeed());
    }
  else
    {
      SetSpeed(1);
    }
  WriteLog("SetMusic finished");
}

void GluSetMusic(bool loop,WORD music_resource)
{
  WriteLog("SetMusic type B called to use music resource %x" LogArg((DWORD)music_resource));

  if(false == disable_music)
    {
      TryAndReport(MusicPlay(loop,MAKEINTRESOURCE(music_resource),MIDI_RESOURCE_TYPE));
      last_music = "";
    }

  SetSpeed(1);

  WriteLog("SetMusic finished.");
}

void GluPlaySound(int i,FIXEDNUM freq_factor, bool reverse)
{
  freq_factor = FixedMul(freq_factor,sfxfreq);

  // will play a sound and changes its frequency based on freq_factor
  LPDIRECTSOUNDBUFFER b = sounds[i];
	
  LPDIRECTSOUNDBUFFER b2; // the duplicate of the sound

  if(NULL == b)
    {
      return; // we never loaded this sound
    }
	
  // find free slot
  int free_slot;
  for(free_slot = 0; free_slot < MAX_SOUNDS; free_slot++)
    {
      if(NULL == playing[free_slot])
	{
	  // we found a good spot
	  break;
	}
    }

  if(MAX_SOUNDS == free_slot)
    {
      // couldn't find a free slot, so leave . . .
      return;
    }

  if(SUCCEEDED(ds->DuplicateSoundBuffer(b,&b2)))
    {
      DWORD old_freq;

      if(SUCCEEDED(b2->GetFrequency(&old_freq)))
	{
	  DWORD new_freq = FixedCnvFrom<long>(old_freq * freq_factor);

	  // set our new frequency
	  b2->SetFrequency(new_freq);
	}

      if((const)reverse != (const)reversed.IsBitSet(i))
	{
	  // reverse the contents of the sound buffer in order to play it
	  //  backwards
	  void *ptr1;
	  void *ptr2;
	  DWORD ptr1_size;
	  DWORD ptr2_size;

	  if(SUCCEEDED(b2->Lock(0,0,&ptr1,&ptr1_size,&ptr2,&ptr2_size,DSBLOCK_ENTIREBUFFER)))
	    {
	      reversed.FlipBit(i);

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
	      BYTE *p1; 
	      BYTE *p1_end;
	      if(NULL == ptr1)
		{
		  p1 = (BYTE *)(NULL + 1);
		  p1_end = NULL;
		}
	      else
		{
		  p1 = (BYTE *)ptr1;
		  p1_end = ((BYTE *)ptr1) + ptr1_size - 1;
		}

				// get a pointer to the last byte in pointer two
	      BYTE *p2;
	      BYTE *p2_start;

	      if(NULL == ptr2)
		{
		  p2_start = (BYTE *)(NULL + 1);
		  p2 = (BYTE *)NULL;
		}
	      else
		{
		  p2  = ((BYTE *)ptr2) + ptr2_size - 1;
		  p2_start  = (BYTE *)ptr2;
		}

				// step one is very similar in both methods
	      while(p2 >= p2_start && p1 <= p1_end)
		{
		  // do the swapping
		  BYTE temp;
		  temp = *p2;
		  *p2  = *p1;
		  *p1  = temp;

		  // increment these two:
		  p1++;
		  p2--;
		}

				// now branch off depending on size of ptr1 compared to ptr2
	      if(ptr1_size < ptr2_size)
		{
		  // ptr1 is less than ptr2 in size
		  BYTE *p2_partner = p2_start;

		  while(p2_partner < p2)
		    {
		      BYTE temp;
		      temp = *p2_partner;
		      *p2_partner = *p2;
		      *p2 = temp;

		      // change the pointer values to next two bytes
		      p2--;
		      p2_partner++;
		    }
		}
	      else
		{
		  // ptr1 is greater than(or equal to) ptr2 in size
		  BYTE *p1_partner = p1_end;

		  while(p1_partner > p1)
		    {
		      BYTE temp;
		      temp = *p1_partner;
		      *p1_partner = *p1;
		      *p1 = temp;

		      // change ptr values to next two bytes
		      p1++;
		      p1_partner--;
		    }
		}

				// now unlock the sound buffer
	      b2->Unlock(ptr1,ptr1_size,ptr2,ptr2_size);
	      b2->Play(0,0,0);
	    }
	}
      else
	{
	  b2->Play(0,0,0);
	}

      playing[free_slot] = b2;
    }
}

void GluDisableMusic()
{
  disable_music = true;
}

void GluStopMusic()
{
  // stops whatever music has been playing
  MusicStop();
  // the last music playing was . . . nothing!
  last_music = TEXT("");
}

void GluPostForcePickupMessage()
{
  static bool shown_message_already = false;

  // only display the "hold p" message if
  //  we have never showed it before,
  //  no other message is showing, the GLUhero
  //  cares about the score, and we are not
  //  in multiplayer
  if
    (
     false == shown_message_already
     &&
     frames_for_current_message > FRAMESTODISPLAYMSG
     )
    {
      tstring msg;
      if(true == NetInGame())
	{
	  GluStrLoad(IDS_FORCEPICKUPMP,msg);
	}
      else
	{
	  GluStrLoad(IDS_FORCEPICKUPSP,msg);
	}
      GluPostMessage(msg.c_str());
      shown_message_already = true;
    }
}

HWND GluMain()
{

  WriteLog("Calling Introduction() to display intro screen");
  Introduction();
  WriteLog("Introduction() terminated.  Intro screen is over");

  WriteLog("Entering while(Menu()) Game(); loop");

  WriteLog("Starting Menu()");
	
  while(true == Menu())
    {
      WriteLog("Menu() terminated");
      WriteLog("Entering Game() function");
      Game();
      WriteLog("Game() terminated; Starting Menu()");
    }

  WriteLog("Menu() returned false, player must have picked Quit");
  WriteLog("About to leave GluMain()");

  return hWnd;
}

static void FlushKeyPresses()
{
  // flush out all key presses
  while(false == key_presses.empty())
    {
      key_presses.pop();
    }
}

bool GluCanQuit()
{
  return bool(GLUESTATE_UNINITIALIZED == state);
}

// returns true if the game should be started, false to quit
static bool Menu()
{
  WriteLog("Now running Menu() function");
  while(true)
    {
      WriteLog("Right now we are at the main menu, where you can't press Escape");
      WriteLog("Calling MenuLoop until the user presses Enter and not Escape . . .");
      while(MENUACTION_ESCAPE == MenuLoop());
      WriteLog("The user pressed Enter, analyzing selection");

      switch(m->GetSelectionIndex())
	{
	case MAINMENU_SP: {
	  // do single-player game
	level_select:
	  WriteLog("The user picked Single player");
	  state = GLUESTATE_LEVELSELECT;
	  WriteLog("PrepareMenu()'ing for level select");
	  PrepareMenu();
	  WriteLog("Done preparing menu, now waiting for user to press Enter or Escape");
	  int menu_action;
			
	  do
	    {
	      menu_action = MenuLoop();
	      GLUlevel = m->GetSelectionIndex();
	    }
	  while(MENUACTION_RETURN == menu_action && LEVELAVAIL_NONE == DeeLevelAvailability(GLUlevel));

	  if(MENUACTION_ESCAPE == menu_action)
	    {
	      WriteLog("User pressed Escape at level select, anyway.  PrepareMenu()'ing for main menu");
	      state = GLUESTATE_MAINMENU;
	      PrepareMenu();
	      WriteLog("Done preparing.  About to enter main menu again");
	      continue;
	    }

	  WriteLog("User selected level %d, with availability of %d" LogArg(GLUlevel) LogArg(DeeLevelAvailability(GLUlevel)));

	  state = GLUESTATE_DIFFICULTYSELECT;
	  WriteLog("PrepareMenu()'ing for difficulty select");
	  PrepareMenu();

	  if(MENUACTION_ESCAPE == MenuLoop())
	    {
	      WriteLog("User pressed Escape at difficulty select, now returning to level select");
	      goto level_select;
	    }

	  state = GLUESTATE_GAME;
	  level_loaded = LEVELLOAD_RELOADALL;
	  GLUdifficulty = m->GetSelectionIndex();

	  WriteLog("User picked difficulty of %d" LogArg(GLUdifficulty));
	  WriteLog("Menu() returning with value of 'true'");
	  return true;
	} case MAINMENU_MP:
	  // multiplayer game
	enter_name:
	state = GLUESTATE_ENTERNAME;
	if(0 == GLUname.size())
	  {
				// the user has never entered a GLUname before, so pick one
	    GluStrLoad(IDS_CHARNAME1 + CHAR_TURNER,GLUname);
	  }
	PrepareMenu();

	// now entering GLUname:
	if(MENUACTION_ESCAPE == MenuLoop())
	  {
				// canceled the mp game plans
	    state = GLUESTATE_MAINMENU;
	    PrepareMenu();
	    continue;
	  }

	pick_character:
	state = GLUESTATE_PICKCHARACTER;
	PrepareMenu();

	// picking character:
	if(MENUACTION_ESCAPE == MenuLoop())
	  {
				// canceled mp game plans
	    goto enter_name;
	  }
			
	model = m->GetSelectionIndex();

	select_connection_method:
	state = GLUESTATE_SELECTCONNECTIONMETHOD;
	PrepareMenu();

	while(true)
	  {
	    if(MENUACTION_ESCAPE == MenuLoop())
	      {
		goto pick_character;
	      }

	    if
	      (
	       INITIALIZEPROTOCOL_SUCCESS ==
	       NetInitializeProtocol(m->GetSelectionIndex())
	       )
	      {
		break;
	      }
	  }

	pick_game:
	state = GLUESTATE_PICKGAME;
	PrepareMenu();

	while(true)
	  {
	    if(MENUACTION_ESCAPE == MenuLoop())
	      {
		NetReleaseProtocol();
		goto select_connection_method;
	      }

	    WtrSetScript(m->GetSelectionIndex());
	    gr->DirectDraw()->FlipToGDISurface();

	    while(FAILED(did->GetDeviceState(KBBUFFERSIZE,(void *)GLUkeyb)))
	      {
		did->Acquire();
	      }

	    if(
	       !(GLUkeyb[DIK_LSHIFT] & EIGHTHBIT) &&
	       !(GLUkeyb[DIK_RSHIFT] & EIGHTHBIT)
	       )
	      {
		// not hosting
		int join_game_result = NetJoinGame(m->GetSelectionIndex());
		if(JOINGAME_FAILURE != join_game_result)
		  {
		    // joined game successfully, not hosting
		    state = GLUESTATE_GAME;
		    GLUlevel = join_game_result;
		    PrepareForMPGame();
		    level_loaded = LEVELLOAD_RELOADALL;
		    return true;
		  }
	      }
	    else
	      {
		// hosting
		// try to host
		if(CREATEGAME_SUCCESS == NetCreateGame(m->GetSelectionIndex()))
		  {
		    state = GLUESTATE_LEVELSELECT;
		    PrepareMenu();
		    break;
		  }
	      }
	  }

	// now we can select a GLUlevel
	while(true)
	  {
	    if(MENUACTION_ESCAPE == MenuLoop())
	      {
		NetLeaveGame();
		goto pick_game;
	      }

	    if(LEVELAVAIL_NONE != DeeLevelAvailability(m->GetSelectionIndex()))
	      {
		break;
	      }
	  }

	state = GLUESTATE_GAME;
	GLUlevel = m->GetSelectionIndex();
	PrepareForMPGame();
	level_loaded = LEVELLOAD_RELOADALL;

	return true;
	case MAINMENU_QUIT:
	  state =GLUESTATE_CONFIRMQUIT;
	  PrepareMenu();

	  while(MENUACTION_ESCAPE == MenuLoop());
			
	  if(YES == m->GetSelectionIndex())
	    {
				// the user wants to quit
	      GluRelease();
	      return false;
	    }
	  else
	    {
				// the user wants to stay longer
	      state = GLUESTATE_MAINMENU;
	      PrepareMenu();
	      continue;
	    }
	} // end of switch(main_menu_selection)
    } // end of while(true) loop of Menu()
} // end of Menu()

static void Game()
{
  WriteLog("Entering Game() loop.  Things that usually happen every frame are not logged.");
  WriteLog("Things that happen a lot may be logged, however");
  InitializeProfiler(NUM_PROFILES);
  while(true)
    {
      StartProfileFrame();
      BeginProfile(Main_Game_Loop);
      int i;

      BeginProfile(Sound_Loop);
      SoundLoop();
      EndProfile(); // sound loop

      // check for new input
      while(FAILED(did->GetDeviceState(KBBUFFERSIZE,(void *)GLUkeyb)))
	{
	  WriteLog("DirectInput GetDeviceState function failed . . . Attempting to reacquire.");
	  did->Acquire();
	}
	
      // STEP 0: RELOAD LEVEL IF APPROPRIATE or RUN THE END GAME
      if(GLUkeyb[DIK_RETURN] & EIGHTHBIT && false == NetProtocolInitialized())
	{
	  WriteLog("User pressed Return, reloading level");
	  level_loaded |= LEVELLOAD_RELOADFLESH;
	}
	
      if(LEVELLOAD_OKAY != level_loaded)
	{
	  WriteLog("Level has not been correctly loaded yet, calling LoadLevel() function");
	  // load the new GLUlevel
	  LoadLevel();

	  WriteLog("LoadLevel finished");
	
	  // the loadlevel function will update the level_loaded
	  //  variable itself
	
	  do_transition = true; // next time we flip, do a screen transition
	}
      else if(NUM_LEVELS <= GLUlevel)
	{
	  WriteLog("NUM_LEVELS <= GLUlevel, the ending sequence will be shown");
	  EndGame();
	  break;
	}
	
      // STEP 2: CHECK FOR QUITTER
      if(GLUkeyb[DIK_ESCAPE] & EIGHTHBIT)
	{
	  WriteLog("User pressed escape, quitting game");
	  break;
	}

      // check for pauser
      if(false == key_presses.empty() && PAUSE_KEY == key_presses.front())
	{
	  if(false == NetInGame())
	    {
				
	      WriteLog("User pressed pause");
	
				// play the sound for pausing the game, using the original sound buffer
	      LPDIRECTSOUNDBUFFER s = sounds[WAV_PAUSE];

	      if(NULL != s && false == GLUhero.Dead())
		{
		  s->Play(0,0,0);
		  CTimer::Wait(0.10);
		  s->SetCurrentPosition(0);
		  CTimer::Wait(0.20);
		  s->SetCurrentPosition(0);
		  CTimer::Wait(0.95);
		}
	      else
		{
		  CTimer::Wait(0.95+0.20+0.10);
		}

				// copy front buffer to back
	      while(DDERR_WASSTILLDRAWING == gr->Buffer(1)->BltFast(0,0,gr->Buffer(0),NULL,0));

				// show health
	      GLUhero.DrawMeters(*gr,SHOWHEALTH_YES);

				// put the "paused" text on the back buffer
				// load the tstring we'll need
	      tstring paused;
	      GluStrLoad(IDS_PAUSE,paused);

				// put the text down
	      if(0 == back_buffer_lock.Certify())
		{
		  WriteString((GAME_MODEWIDTH-(FONTWIDTH+1)*paused.length())/2,(GAME_MODEHEIGHT-FONTHEIGHT)/2,paused.c_str(),msg_and_score_color,msg_and_score_color);
		  back_buffer_lock.Uncertify();
		}

				// now flip between the two surfaces
	      FlushKeyPresses();
	      while(false == Flip())
		{
		  CTimer::Wait(SECONDS_BETWEEN_PAUSE_FLIPS);
		  if(false == key_presses.empty())
		    {
		      if(PAUSE_KEY == key_presses.front())
			{
			  key_presses.pop();
			  break;
			}
		      else
			{
			  key_presses.pop();
			}
		    }
		}
	      FlushKeyPresses();
	      WriteLog("User has resumed game");
	    }
	}
	
      // STEP 4: LOGIC POWERUPS
      CPowerUp::Rotate();

      // powerup logic is only used to see if regeneration is
      //  necessary.  There is not regeneration in a single-player game
      if(true == NetInGame())
	{
	  for
	    (
	     VCTR_POWERUP::iterator iterate = GLUpowerups.begin();
	     iterate != GLUpowerups.end();
	     iterate++
	     )
	    {
	      iterate->Logic();
	    }
	}

      // STEP 5: DRAW EVERYTHING, INCLUDING POSTED MESSAGE, AND FLIP

      // draw lower-GLUlevel compact map
      // figure center of screen coordinates

      GLUhero.GetLocation(GLUcenter_screen_x,GLUcenter_screen_y);
      if(GLUcenter_screen_x < Fixed(GAME_MODEWIDTH/2))
	{
	  GLUcenter_screen_x = Fixed(GAME_MODEWIDTH/2);
	}
      else if(GLUcenter_screen_x > max_center_screen_x)
	{
	  GLUcenter_screen_x = max_center_screen_x;
	}
      if(GLUcenter_screen_y < Fixed(GAME_PORTHEIGHT/2))
	{
	  GLUcenter_screen_y = Fixed(GAME_PORTHEIGHT/2);
	}
      else if(GLUcenter_screen_y > max_center_screen_y)
	{
	  GLUcenter_screen_y = max_center_screen_y;
	}

      int level_blit_x = GAME_MODEWIDTH/2 - FixedCnvFrom<long>(GLUcenter_screen_x);
      int level_blit_y = GAME_PORTHEIGHT/2 - FixedCnvFrom<long>(GLUcenter_screen_y);
      int column1 = __max(0,abs(level_blit_x)/SECTOR_WIDTH);
      int row1 = __max(0,abs(level_blit_y)/SECTOR_HEIGHT);
      int column3 = __min(column1+3,sectors.cols());
      int row3 = __min(row1+3,sectors.rows());
      int r;
      int c;

      level_blit_x += column1 * SECTOR_WIDTH;
      level_blit_y += row1    * SECTOR_HEIGHT;

      BeginProfile(Recaching);
      int reload;
      if(-1 == ul_cached_sector_x || -1 == ul_cached_sector_y)
	{
	  // we need to recache everything
	  reload = CACHE_EVERYTHING;
	}
      else
	{
	  reload = 0;
	  if(ul_cached_sector_x < column1)
	    {
				// we are moving to the right
	      reload |= CACHE_RIGHTCOLUMN;
	      for(int y = 0; y < CACHED_SECTORS_HIGH; y++)
		{
		  CBob *temp = lower_cached_grid[y][0];
		  lower_cached_grid[y][0] = lower_cached_grid[y][1];
		  lower_cached_grid[y][1] = lower_cached_grid[y][2];
		  lower_cached_grid[y][2] = temp;
		  temp = upper_cached_grid[y][0];
		  upper_cached_grid[y][0] = upper_cached_grid[y][1];
		  upper_cached_grid[y][1] = upper_cached_grid[y][2];
		  upper_cached_grid[y][2] = temp;
		}
	    }
	  else if(ul_cached_sector_x > column1)
	    {
	      reload |= CACHE_LEFTCOLUMN;
	      for(int y = 0; y < CACHED_SECTORS_HIGH; y++)
		{
		  CBob *temp = upper_cached_grid[y][2];
		  upper_cached_grid[y][2] = upper_cached_grid[y][1];
		  upper_cached_grid[y][1] = upper_cached_grid[y][0];
		  upper_cached_grid[y][0] = temp;
		  temp = lower_cached_grid[y][2];
		  lower_cached_grid[y][2] = lower_cached_grid[y][1];
		  lower_cached_grid[y][1] = lower_cached_grid[y][0];
		  lower_cached_grid[y][0] = temp;
		}
	    }

	  if(ul_cached_sector_y < row1)
	    {
				// we are moving down
	      reload |= CACHE_BOTTOMROW;
	      for(int x = 0; x < CACHED_SECTORS_WIDE; x++)
		{
		  CBob *temp = lower_cached_grid[0][x];
		  lower_cached_grid[0][x] = lower_cached_grid[1][x];
		  lower_cached_grid[1][x] = lower_cached_grid[2][x];
		  lower_cached_grid[2][x] = temp;
		  temp = upper_cached_grid[0][x];
		  upper_cached_grid[0][x] = upper_cached_grid[1][x];
		  upper_cached_grid[1][x] = upper_cached_grid[2][x];
		  upper_cached_grid[2][x] = temp;
		}
	    }
	  else if(ul_cached_sector_y > row1)
	    {
	      reload |= CACHE_TOPROW;
	      for(int x = 0; x < CACHED_SECTORS_WIDE; x++)
		{
		  CBob *temp = upper_cached_grid[2][x];
		  upper_cached_grid[2][x] = upper_cached_grid[1][x];
		  upper_cached_grid[1][x] = upper_cached_grid[0][x];
		  upper_cached_grid[0][x] = temp;
		  temp = lower_cached_grid[2][x];
		  lower_cached_grid[2][x] = lower_cached_grid[1][x];
		  lower_cached_grid[1][x] = lower_cached_grid[0][x];
		  lower_cached_grid[0][x] = temp;
		}
	    }
	}

      Recache(reload);
		
      EndProfile(); // recaching

      // draw lower-GLUlevel CMP
      //  all of its calculations (or most of it, whatever) will
      //  be used later when the upper-GLUlevel cmp set is drawn
      RECT *target = &gr->TargetScreenArea();
      target->top = level_blit_y;
      target->bottom = level_blit_y + SECTOR_HEIGHT;
      BeginProfile(L_Cmp_Drawing);
      for(r = row1; r < row3; r++)
	{
	  target->left = level_blit_x;
	  target->right = level_blit_x + SECTOR_WIDTH;
	  for(c = column1; c < column3; c++)
	    {
	      WriteLog("Blitting lower level cell r%dc%d" LogArg(r) LogArg(c));
	      gr->PutFastClip(*lower_cached_grid[r-row1][c-column1],false);

	      if(LEVELLOAD_RELOADALL != level_loaded)
		{
		  // see if we collided with the end of a GLUlevel in this sector
		  // CHECK FOR WINNER
		  for
		    (
		     SET_INT::const_iterator iterate = sectors[r][c].level_ends.begin();
		     iterate != sectors[r][c].level_ends.end();
		     iterate++
		     )
		    {
		      if(true == lends[*iterate].Collides(GLUhero))
			{
			  // play the "You've won" tune
			  GluSetMusic(false,IDR_YOUWINTUNE);

			  // let the tune play for four seconds if the music is enabled
			  if(false == disable_music)
			    {
			      // music is enabled; let's wait so the user can hear it
			      CTimer::Wait(YOUWINTUNE_LENGTH);
			    } // end if music is not disabled

			  int next_level = lends[*iterate].Reference();
				
			  // update GLUlevel availability variable
			  DeeLevelComplete
			    (
			     since_start, // pass the timer that keeps track of how long we've played
			     _ttoi(score.c_str()),
			     next_level
			     );
	
			  GLUlevel = next_level;

			  if(NUM_LEVELS > next_level)
			    {
			      level_loaded = LEVELLOAD_RELOADALL;
			    }
			  break;
			} // end if collides with GLUlevel end
		    } // end for GLUlevel ends in this sector
		}
	      target->left = target->right;
	      target->right += SECTOR_WIDTH;
				
	    } // end for column
	  target->top = target->bottom;
	  target->bottom += SECTOR_HEIGHT;
	} // end for row
      EndProfile(); // lower-level-cmp drawing
	
      // erase the drawing order, no longer needed, out of date
      WriteLog("Clearing drawing order");
      GLUdrawing_order.clear();

      CSurfaceLock256 *lock = &back_buffer_lock;
      TryAndReport(lock->Certify());

      // allow the GLUhero to try moving
      //  with respect to black lines
      // allow GLUhero to do logic
      WriteLog("Doing hero logic...");
      GLUhero.Logic();

      // drive the GLUenemies based on AI or network messages
      if(false == NetRemoteLogic())
	{
	  for(r = row1; r < row3; r++)
	    {
	      for(c = column1; c < column3; c++)
		{
		  SET_INT::const_iterator iterate;
		  for(iterate = sectors[r][c].GLUenemies.begin(); iterate != sectors[r][c].GLUenemies.end(); iterate++)
		    {
		      WriteLog("Doing enemy logic %d" LogArg(*iterate));
		      GLUenemies[*iterate].Logic(GLUhero);
		    } // end for GLUenemies in this sector
		}
	    }
	}

      // now we can logic with the projectiles
      for(i = 0; i < MAX_FIRES; i++)
	{
	  WriteLog("Projectile logic %d" LogArg(i));
	  GLUfires[i].Logic();
	}
	
      WriteLog("We can unlock the back buffer now");
      if(true == lock->Certified())
	{
	  WriteLog("Unlocking back buffer...");
	  lock->Uncertify();
	}
	
      // note that we picked a definite coordinate for the center of the
      //  screen before we allowed any movement.  Also note that the 
      //  coordinates decided on by the FilterMovement methods will not
      //  be applied until after the corresponding sprites have been
      //  drawn to the back buffer.  (look at the CCharacter class)
      //  This will prevent shakiness and other strange distortions,
      //  such as making turner's head move like a pigeon's.
      //	However, the coordinates of the characters will always be
      //  one frame old, while the pistol projectiles will
      //  be right on track.  This won't look too bad, or even
      //  noticible.

      // draw characters with respect to drawing order
      BeginProfile(Draw_Things);

      for(MSET_PCHAR::iterator i = GLUdrawing_order.begin(); i != GLUdrawing_order.end(); i++)
	{
	  WriteLog("Drawing character at %8x" LogArg((DWORD)i->ch));
	  if(true == i->ch->DrawCharacter(*gr) && (const)i->ch != (const)&GLUhero && false == NetInGame())
	    {
	      WriteLog("Character moved, updating sectors");
				// need to update sectors
				// the character may have left the current sector
				//  so remove the enemy from one sector and put it
				//  in another
	      int character_row;
	      int character_column;
	      i->ch->GetSector(character_row,character_column);
	      int index = i->ch - &GLUenemies[0];
	      sectors[character_row][character_column].GLUenemies.erase(index);
	      i->ch->CalculateSector(character_row,character_column);
	      sectors[character_row][character_column].GLUenemies.insert(index);
	    }
	}
      WriteLog("Done drawing characters");


      // draw power ups
      //  draw ones of sectors
      for(r = row1; r < row3; r++)
	{
	  for(c = column1; c < column3; c++)
	    {
	      SET_INT::const_iterator iterate;
	      for(iterate = sectors[r][c].GLUpowerups.begin(); iterate != sectors[r][c].GLUpowerups.end(); iterate++)
		{
		  WriteLog("Drawing powerup %d" LogArg(*iterate));
		  GLUpowerups[*iterate].Draw(*(gr));
		}
	    }
	}

      // draw extra ones
      for(VCTR_POWERUP::iterator iterate = GLUpowerups.begin()+std_powerups; iterate < GLUpowerups.end(); iterate++)
	{
	  WriteLog("Drawing extra powerup at %8x" LogArg(iterate));
	  iterate->Draw(*gr);
	}

      // draw bullets
      for(int bullet_i = 0; bullet_i < MAX_FIRES; bullet_i++)
	{
	  WriteLog("Drawing bullet %d" LogArg(bullet_i));
	  GLUfires[bullet_i].Draw(*gr);
	}
      EndProfile(); // powerup and bullets drawing

      // draw upper-GLUlevel bitmap if outdoors
      BeginProfile(U_Cmp_Drawing_N_Weather);
      if(false == GluWalkingData(GLUhero.X(),GLUhero.Y()))
	{
	  target->top = level_blit_y;
	  target->bottom = level_blit_y + SECTOR_HEIGHT;
	  for(r = row1; r < row3; r++)
	    {
	      target->left = level_blit_x;
	      target->right = level_blit_x + SECTOR_WIDTH;
	      for(c = column1;c < column3; c++)
		{
		  if(false == sectors[r][c].upper_cell.Empty())
		    {
		      if(true == sectors[r][c].upper_cell.NoStep2())
			{
			  // this cell is probably very simple graphically, so
			  //  just draw step one normally
			  sectors[r][c].upper_cell.RenderStep1(gr->GetTargetBufferInterface(),target->left,target->top,gr->SimpleClipperRect());
			}
		      else
			{
			  // this cell is complex!  draw the uncompressed bitmap form
			  gr->PutFastClip(*upper_cached_grid[r-row1][c-column1]);
			}
		    }
		  target->left = target->right;
		  target->right += SECTOR_WIDTH;
		}
	      target->top = target->bottom;
	      target->bottom += SECTOR_HEIGHT;
	    }
	  if(0 == lock->Certify())
	    {
	      WtrOneFrame(*lock);
	      WriteInfo();
	      lock->Uncertify();
	    }
	}
      else
	{
	  WtrOneFrame();
	  if(0 == lock->Certify())
	    {
	      WriteInfo();
	      lock->Uncertify();
	    }
	}
      EndProfile(); // upper cmp drawing and weather

      // draw meters of health and ammo
      GLUhero.DrawMeters(*(gr),(GLUkeyb[DIK_H] & EIGHTHBIT) ? SHOWHEALTH_YES : SHOWHEALTH_IFHURT);

      // we should check for system messages (or any message)
      //  if we are in a network game.
      if(true == NetInGame())
	{
	  // handle system messages
	  NetCheckForSystemMessages();
	}

      // set the brightness factor now, because it is
      //  set, but not applied, when the surface is locked
      //  this is the time to actually change the palette if needed
      PalApplyBrightnessFactor();

      EndProfile(); // main game loop

      // finally, flip the surfaces showing our wonderful
      //  rendering job
      Flip();
    }

  WriteLog("Main game loop has terminated");

  // cleanup after the game
  if(true == NetProtocolInitialized())
    {
      if(true == NetInGame())
	{
	  NetLeaveGame();
	}
      NetReleaseProtocol();
    }

  PalRelease();
  PalInitializeWithMenuPalette(*gr);
  state = GLUESTATE_MAINMENU;

  FlushKeyPresses();

  PrepareMenu();
			
  // change music and shut up the crickets/rain
  WtrSilence();
  GluSetMusic(true,IDR_MENUMUSIC);
}

static void GetLevelTimerMinSec(int &min, int &sec)
{
  // get how many second we have been playing from the CGlue timer that
  //  was restarted when the GLUlevel was loaded
  sec = FixedCnvFrom<long>(since_start);
  min = sec / SECONDSPERMINUTE;
  sec %= SECONDSPERMINUTE;
}

void GluReloadBitmaps()
{
  LoadBitmaps(RESOURCELOAD_RELOAD);
}

static void WriteChar(int x,int y,int c,int color,int back_color)
{
  BYTE *d = font_data + c * 16; // find the offset to the right characten
  BYTE *s = (BYTE *)back_buffer_lock.SurfaceDesc().lpSurface + y * back_buffer_lock.SurfaceDesc().lPitch + x;
  for(int i = 0; i < FONTHEIGHT; i++)
    {
      BYTE row=*d++; // get the data for the current row
      for(int j=0;j < FONTWIDTH; j++)
	{
	  if(row & 0x80)
	    {
				// draw the current pixel
	      *s = color;
	    }
	  else if((const)back_color != color)
	    {
	      *s = back_color;
	    }

	  s++;
	  row<<=1;
	}
      s += back_buffer_lock.SurfaceDesc().lPitch-FONTWIDTH;
    }
}

static void WriteString(int x,int y,const TCHAR *tstring,int color,int back_color)
{
  if(y < 0)
    {
      return;
    }

  while(x < 0) {tstring++; x+= FONTWIDTH + 1;}

  while(*tstring && GAME_MODEWIDTH - x >= FONTWIDTH && GAME_MODEHEIGHT - y >= FONTHEIGHT)
    {
      WriteChar(x,y,*((BYTE *)tstring),color,back_color);
      x += FONTWIDTH+1;
      tstring++; // go to next character in the tstring
    }
}

void GluFindTextColors()
{
  // get message color
  CColor256 c;
  c.SetColor(0-1,0-1,0-1);
  c.Certify();
  msg_and_score_color = c.Color();
  c.Uncertify();

	// get red flash color
  c.SetColor(0-1,0,0);
  c.Certify();
  score_flash_color_1 = c.Color();
  c.Uncertify();
	
	// get yellow flash color
  c.SetColor(0-1,0-1,0);
  c.Certify();
  score_flash_color_2 = c.Color();
}

static void WriteInfo()
{
  BeginProfile(Draw_Extra_Stats_N_Msgs);

  if(score_print_x > 0)
    {
      // now we should put the score's shadow on the back buffer
      WriteString(score_print_x-1,SCORE_AND_TIMER_Y-1,score.c_str(),0,0);

      // call this function to put the score on the back buffer
      WriteString(score_print_x,SCORE_AND_TIMER_Y,score.c_str(),msg_and_score_color,msg_and_score_color);
    } 
  else
    {
      // score_print_x is negative, which means we should be printing it in flashing colors
      //  also, the last character in score tstring contains countdown to stop flashing

      // print score in flashing red/yellow
      int color = frames_for_current_message & 1 ? score_flash_color_1 : score_flash_color_2;

      // call this function to put the score on the back buffer
      WriteString(-score_print_x,SCORE_AND_TIMER_Y,score.c_str(),color,color);

      // countdown to stop flashing
      if(FRAMESTODISPLAYMSG == frames_for_current_message)
	{
	  score_print_x = -score_print_x;
	}
    }

  // this array will contain the text buffer which holds the time
  TCHAR time[MAX_TIMERCHAR];

  int minutes;
  int seconds;

  // call a member function that will fill these two
  //  integers with the numbers to be displayed in the timer
  GetLevelTimerMinSec(minutes,seconds);
	
  // format the tstring so timers look like 15:22 or 01:54 or 00:13, etc . . .
  //  also add a random number from 0 to 99 at the end to look like
  //  we have a real accurate timer
  wsprintf(time,TEXT("%02d:%02d.%02d"),minutes,seconds,rand()%100);

  // now print the timer's shadow
  WriteString(1,SCORE_AND_TIMER_Y-1,time,0,0);

  // print timer out at same y-coor as score by calling this function
  WriteString(0,SCORE_AND_TIMER_Y,time,msg_and_score_color,msg_and_score_color);

  // print our message
  if(++frames_for_current_message < FRAMESTODISPLAYMSG)
    {
      // now print it's shadow
      WriteString(msg_x+1,0+1,message.c_str(),0,0);

      // print out our message (the message coordinates have already been calculated
      //  to be placed in the center of the top of the screen
      WriteString(msg_x,0,message.c_str(),msg_and_score_color,msg_and_score_color);
    }

  EndProfile(); // extra stats and messages
}

static void Recache(int flags)
{
  WriteLog("Recache called with flags %x" LogArg(flags));
  if(0 == flags)
    {
      return; // already cached everything
    }
  int bit = 1;
  DDBLTFX fx;
  ZeroMemory((void *)&fx,sizeof(fx));
  fx.dwSize = sizeof(fx);
  fx.dwFillColor = 0;
  ul_cached_sector_y = (FixedCnvFrom<long>(GLUcenter_screen_y) - GAME_PORTHEIGHT/2) / SECTOR_HEIGHT;
  ul_cached_sector_x = (FixedCnvFrom<long>(GLUcenter_screen_x) - GAME_MODEWIDTH/2)/ SECTOR_WIDTH;
  for(int y = 0; y < CACHED_SECTORS_HIGH; y++)
    {
      // find the sector coordinates at this height
      int sector_y = ul_cached_sector_y + y;
      if(sector_y >= sectors.rows())
	{
	  return; // all done, we are out of range
	}
      for(int x = 0; x < CACHED_SECTORS_WIDE; x++,bit <<= 1)
	{
	  if(bit & flags)
	    {
				// we have to recache the map at x,y
	      int sector_x = ul_cached_sector_x + x;

	      WriteLog("Reloading sector %dx%d" LogArg(sector_x) LogArg(sector_y));

				// now make sure this fits into the sector grid of the whole level
	      if(sector_x < sectors.cols())
		{
		  WriteLog("Sector coor is in range, loading . . . ");
		  // and it does!  let's get caching
		  IDirectDrawSurface2 *targetA = lower_cached_grid[y][x]->Data();
		  IDirectDrawSurface2 *targetB = upper_cached_grid[y][x]->Data();
		  CCompactMap *sourceA = &sectors[sector_y][sector_x].lower_cell;
		  CCompactMap *sourceB = &sectors[sector_y][sector_x].upper_cell;
					
		  WriteLog("Clearing out target surfaces . . .");
		  while(DDERR_WASSTILLDRAWING == targetA->Blt(NULL,NULL,NULL,DDBLT_COLORFILL,&fx)) {}
		  while(DDERR_WASSTILLDRAWING == targetB->Blt(NULL,NULL,NULL,DDBLT_COLORFILL,&fx)) {}

		  WriteLog("Rendering step 1 . . . ");
		  sourceA->RenderStep1(targetA,0,0,false);
		  sourceB->RenderStep1(targetB,0,0,false);

		  CSurfaceLock256 x;
		  x.SetTargetSurface(targetA);
		  x.SetArea(NULL);
		  if(0 == x.Certify())
		    {
		      WriteLog("Rendering step 2 . . . ");
		      sourceA->RenderStep2(x.SurfaceDesc().lpSurface,x.SurfaceDesc().lPitch,0,0,NULL);
		      x.Uncertify();
		    }
					
		  x.SetTargetSurface(targetB);
		  if(0 == x.Certify())
		    {
		      WriteLog("Rendering step 2 . . . ");
		      sourceB->RenderStep2(x.SurfaceDesc().lpSurface,x.SurfaceDesc().lPitch,0,0,NULL);
		      x.Uncertify();
		    }
		}

				// all done rendering to surface
	      WriteLog("Finished reloading sector %dx%d" LogArg(sector_x) LogArg(sector_y));
				
	      flags &= ~bit; // turn off the bit, like on a checklist
	      if(0 == flags)
		{
		  WriteLog("Recache finished");
		  return;
		}
	    }
	}
    }
  WriteLog("Recache finished");
}

int GluScoreDiffPickup(int x)
{
  return (2 == x) ? 2 : 1;
}

int GluScoreDiffKill(int x)
{
  return x * 2 + 2;
}
