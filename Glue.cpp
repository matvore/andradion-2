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
#include "Buffer.h"
#include "Graphics.h"
#include "CompactMap.h"
#include "Fixed.h"
#include "Logger.h"
#include "Profiler.h"
#include "MusicLib.h"
#include "Timer.h"
#include "Menu.h"
#include "RawLoad.h"
#include "Glue.h"
#include "Weather.h"
#include "Fire.h"
#include "Character.h"
#include "Deeds.h"
#include "LevelEnd.h"
#include "PowerUp.h"
#include "Net.h"
#include "GammaEffects.h"
#include "BitMatrix.h"

using std::bitset;
using std::pair;
using std::max;
using std::min;
using std::swap;
using std::list;
using std::string;
using std::queue;
using std::multiset;
using std::set;
using std::auto_ptr;
using std::endl;

const DWORD SOUNDRESOURCEFREQ = 11025;
const DWORD SOUNDRESOURCEBPS = 8; // bits per sample of sounds
const int BMP_SPCOUNT = 51;
const int BMP_MPCOUNT = 91;
const int BITMAP_COUNT[] = {0, BMP_SPCOUNT, BMP_MPCOUNT};
const int XCOOR_ACCOMPLISHMENTTEXT = 5;
const int YCOOR_ACCOMPLISHMENTTEXT = 183;
const COLORREF COLOR_ACCOMPLISHMENTTEXT = RGB(0,128,0);
const char *const SYNCRATE_FORMAT = "%d";
const int MAX_TIMERCHAR = 9;
const int MAX_TIMERSECONDS =  Fixed(99 * 60 + 59);
const int FRAMERATE_BUFFERLEN = 10;
const int NUM_PRESOUNDS = 1;

const int MAX_LONG_SOUNDS = 3;
const int MAX_SHORT_SOUNDS = 12;
const int MAX_SOUNDS = MAX_LONG_SOUNDS + MAX_SHORT_SOUNDS;

// INTRODUCTION-RELATED CONSTANTS
const int MAXSTARDIMNESS = 10;
const int NUMSTARS = 1500;
const float TIMETOTHROWBACK = 2.25f;
const float TIMETOSCROLL = 100.0f;
const int   TRANSITIONSQUARESIZE = 350;
const float TRANSITIONSECSPERSQUARE = 1.0f/80.0f;
const float MAXFLASHTIME = 5.0f; 
	
const RECT  UPPERBLACKAREA = {0,0,800,75}; // left,top,right,bottom
const RECT  LOWERBLACKAREA = {0,525,800,600};
const RECT  DISPLAYAREA    = {0,75,800,525};
const int   MODEWIDTH = 800;
const int   MODEHEIGHT = 600;

const COLORREF FLASHCOLOR1 = RGB(255,0,0);
const COLORREF FLASHCOLOR2 = RGB(255,255,0);

// how fast to flash
const float FLASHCOLORPERSEC = 0.06f;

const int SCREENROWS = 450;
const float SCREENMINROW = -225;
const float VIEWERDISTANCE = 1000.0f;
const float SCREENWIDTHHALF = 400.0f;
const float STORYWIDTHHALFSQ = 390.0f * 390.0f; 
const float STORYANGLE = 3.14159f / 3.0f;
const float SCREENHEIGHTHALF = 225.0f;
const float STORYANGLECOS = cos(STORYANGLE);
const float STORYANGLESIN = sin(STORYANGLE);

const int STORY_WIDTH = 320, STORY_HEIGHT = 591;
// END OF INTRODUCTION-RELATED CONSTANTS

// constants for sound volume and balance calculation are not in floating-point,
//  because they wouldn't fit
const int MAX_XDIST = 100; // how far something can be on x axis before it is inaudible to one channel
const int MAX_DISTSQUARED = 71200; // how far something can be squared before it is totally inaudible
const int MIN_DISTSQUARED = 35600; // how far something has to be in order to have a lower volume
const int ALWAYSPUSH = ~(1 << 9); // use slightly different collision detection in ufo level
const int MAX_STRINGLEN = 500;
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

enum {LL_OKAY, LL_FLESH, LL_BONE, LL_ALL};
static int level_loaded; // indicates validity of loaded data

const int MENUACTION_ESCAPE = 0;
const int MENUACTION_RETURN = 1;
const int SCORE_X_OFFSET = -10;

enum {
  GLUESTATE_UNINITIALIZED,
  GLUESTATE_INTRODUCTION,
  GLUESTATE_MAINMENU,
  GLUESTATE_LEVELSELECT,
  GLUESTATE_DIFFICULTYSELECT,
  GLUESTATE_GAME,
  GLUESTATE_CONFIRMQUIT,
  GLUESTATE_ENTERNAME,
  GLUESTATE_PICKCHARACTER,
  GLUESTATE_SELECTCONNECTIONMETHOD,
  GLUESTATE_PICKGAME
};

enum {GLUEHSM_NOTHING, GLUEHSM_NEWPLAYER, GLUEHSM_SESSIONLOST};
enum {MAINMENU_SP, MAINMENU_MP, MAINMENU_QUIT, NUM_MAINMENUITEMS};
enum {NO, YES, NUM_YESNOMENUITEMS};
enum {RESOURCELOAD_NONE,
      RESOURCELOAD_SP,
      RESOURCELOAD_MP};

const COLORREF COLOR_HEADING = RGB(0,128,0);
const COLORREF COLOR_UNSELECTED = RGB(255,128,128);
const COLORREF COLOR_SELECTED = RGB(0,255,255);
const COLORREF COLOR_SHADOW = RGB(128,0,0);
const int SHADOW_OFFSET = 1;
const DWORD DEFAULT_SYNCRATE = 30;
const int MPDIFFICULTY = 2; // GLUdifficulty level used for multiplayer sessions
const int NUM_MPGAMESLOTS = 12; // number of game slots on multiplayer game selects
const int MAX_PLAYERS = 16; // maximum amount of players in a network game
const int LOADINGMETER_MINHEIGHT = 9;
const int LOADINGMETER_MAXHEIGHT = 15;
const FIXEDNUM FREQFACTOR_OKGOTITBACKWARDS = Fixed(1.2f);
const FIXEDNUM FREQFACTOR_OKGOTITNORMAL = Fixed(0.80f);
const char *const MIDI_RESOURCE_TYPE = "MIDI";
const char *const CMP_RESOURCE_TYPE = "CMP";
const char *const ONE_NUMBER_FORMAT = "%d";
const char *const TWO_NUMBERS_FORMAT = "%d/%d";
const char *const LEVELS_LIB_FILE = "LevelsLib.dat";
const char *const SCREENSHOTFN = "Level %d-%dmin%dsec.bmp";
const int SCORE_AND_TIMER_Y = 184;
const int WAV_PAUSE = 18;
const float SECONDS_BETWEEN_PAUSE_FLIPS = 0.5f;
const BYTE PAUSE_KEY = 'P';
const int FRAMESTOFLASHSCORE = 150;
const int MAX_CHARS_PER_LINE = 39;
const int CACHED_SECTORS = 9;
const int CACHED_SECTORS_HIGH = 3;
const int CACHED_SECTORS_WIDE = 3;
const double YOUWINTUNE_LENGTH = 4.0f;
const int NUM_SPSOUNDS = 22;
const int NUM_SOUNDS = 46;
const RECT SECTOR_AREA = {0, 0, SECTOR_WIDTH, SECTOR_HEIGHT};

const int FONTHEIGHT = 16;
const int FONTWIDTH = 8;
const int FONTDATA_SIZE = 1504;
const int FIRST_FONTCHAR = '!';
const int LAST_FONTCHAR = '~';

const FIXEDNUM TIMER_INC_PER_FRAME = Fixed(0.04f);

// variables accessable from other modules:
int                    GLUdifficulty;
BYTE                   GLUkeyb[KBBUFFERSIZE];
vector<CPowerUp>       GLUpowerups;
FIXEDNUM               GLUcenter_screen_y, GLUcenter_screen_x;

struct TSector {
  TSector() : upperCell(0), lowerCell(0) {}

  void ClearMaps() {
    delete upperCell;
    upperCell = 0;
    delete lowerCell;
    lowerCell = 0;
  }

  CCompactMap *upperCell, *lowerCell;
  set<int> powerups, enemies;
  list<CLevelEnd> levelEnds;
};

template<class c,class f> inline c Range(c minimum, c maximum,
                                         f progress) {
  // this function will simply take the progress var,
  //  and return a value from minimum to maximum, where it would
  //  return minimum if progress == 0, and maximum if progress == 1
  //  and anything inbetween would be a linear function derived
  //  from that range.
  return c(f(maximum-minimum) * progress) + minimum;
}

#ifdef _DEBUG
static HGDIOBJ profiler_font;
const int PROFILER_FONT_SIZE = 8;
#endif

static string last_music;
static bool disable_music = false;
static FIXEDNUM max_center_screen_x, max_center_screen_y;
static BYTE font_data[FONTDATA_SIZE];

// this is the score in multiplayer or singleplayer games.  If you are singleplayer and the score is at max, then the last character
//  is a countdown to when the flashing should stop
static string score;

// printing coordinate of score, negated when full score is obtained
// which signifies flashing colors
static int score_print_x; 

// x-coordinate of the current top-of-screen message
static int msg_x; 

// current message on  the screen
static string message; 

static HWND hWnd;
static HINSTANCE hInstance;

static int state = GLUESTATE_UNINITIALIZED;

static int bitmaps_loaded, sounds_loaded;

// array of boolean values which specify if the sounds are reversed or not
static bitset<NUM_SOUNDS> reversed;

// array of original sound buffers
static IDirectSoundBuffer *sounds[NUM_SOUNDS]; 

// array of duplicated sound buffers for sounds that are currently playing
static IDirectSoundBuffer *playing[MAX_SOUNDS];

static auto_ptr<CBitMatrix> walking_data; // on = inside, off = outside
static int width_in_tiles, height_in_tiles;

static vector<TSector> sectors;
static int sector_width, sector_height, total_sectors;
	
static LPDIRECTSOUND ds; // directsound
static LPDIRECTINPUT di; // directinput
static LPDIRECTINPUTDEVICE did; // keyboard
static CMenu *m;
static list<CLevelEnd> lends;

static int model; // character we are playing as

static CTimer char_demo_stepper;
static CTimer char_demo_direction_changer;

static DWORD frames_for_current_message; // how long the current message has been up

static string KILLEDYOURSELF;
static string KILLED;
static string YOU;
static string YOUKILLED;
static string KILLEDTHEMSELVES;
static string SPKILLED; // string displayed when killed in single player

static vector<POINT> possible_starting_spots;

static FIXEDNUM since_start;

static int std_powerups; // (used for mp) number of GLUpowerups there are when no backpacks are left

static int msg_and_score_color;
static int score_flash_color_1;
static int score_flash_color_2; 

// this array of strings contain data for the user to see
//  that concern his accomplishments
enum {DEEDS_SUMMARY, DEEDS_BESTTIME,
      DEEDS_BESTSCORE, DEEDS_LINECOUNT};
static string accomplishment_lines[DEEDS_LINECOUNT];

static queue<BYTE> key_presses;

static DWORD sync_rate;
static string player_name;
static int char_demo_direction;
static int level;

static surf_t bitmaps[BMP_MPCOUNT];

static void MenuFont(LOGFONT& lf);
static void Levels(vector<string>& target);
static void ShowMouseCursor();
static void HideMouseCursor();
static void SetupMenu();
static pair<const BYTE *, HGLOBAL> LoadLevelPaletteOnly();
static void LoadLevel();
static int  MenuLoop();
static void PrepareMenu();
static void LoadSounds(int type);
static void LoadBitmaps(int type);
static void Introduction();
static void EndGame();
static void Flip();
static void PrepareForMPGame();
static void GetLevelTimerMinSec(int& min, int& sec, int& hund);
static void Game();
static bool Menu();
static void FlushKeyPresses();
static void AddPossibleStartingSpot(FIXEDNUM x,FIXEDNUM y);
static void LoadCmps(int level_width,int level_height,bool skip_wd_resize); // level width and height are not specified as floating-point
static void ResetSinglePlayerScore(int possible_score);
static void CalculateScorePrintX();

// draws text that shows score and timer and message to screen
static void WriteInfo();

static void WriteChar(BYTE *surface, int pitch,
                      int c, int color, int back_color);
static void WriteString(BYTE *surface, int pitch,
                        const char *string, int color,
                        int back_color);
static void Recache(int flags);
static int NextSoundSlot();
static void PlayMusicAccordingly(int state_change_indicator);
static void FillAccomplishmentLines();
static void *GetResPtr(const char *res_name, const char *res_type,
                       HMODULE res_mod, WORD res_lang,
                       HRSRC& res_handle, HGLOBAL& data_handle);


// these statics make it easier to draw the compact maps faster
//  by caching them.  This way, the most that can be drawn each frame
//  is five, but usually none.  Those that are not "redrawn" are cached
//  into simple, un-compressed DirectDraw surfaces that are easy to render
static pair<surf_t, surf_t> cached[CACHED_SECTORS];

// index of upper most-left most sector in the cached array
static int upper_left_sector; 

// the column and row of the upper left sector that is cached
static int ul_cached_sector_x, ul_cached_sector_y; 

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
};

const DIDATAFORMAT c_dfDIKeyboard = { 24, 16, 0x2, 256, 256, c_rgodfDIKeyboard };

// profile names
enum {Main_Game_Loop,
      Recaching,
      Enemy_Logic,
      Hero_Logic,
      Draw_Things,
      Draw_Character,
      U_Cmp_Drawing_N_Weather,
      Weather_One_Frame,
      Draw_Extra_Stats_N_Msgs,
      L_Cmp_Drawing,
      Draw_Meters,
      NUM_PROFILES};

class NetGameBehavior : public NetFeedback {
public:
  NetGameBehavior() : NetFeedback() {}

  virtual void SetWeatherState(unsigned int new_state) throw() {
    WtrSetState(new_state);
  }

  virtual void EnemyFiresBazooka(unsigned int index,
                                 unsigned short x_hit,
                                 unsigned short y_hit) throw() {
    for (int i = 0; i < MAX_FIRES; i++) {
      if (fires[i].OkayToDelete()) {
        fires[i].Setup(enemies[index].X(), enemies[index].Y(),
                       FixedCnvTo<long>(x_hit), FixedCnvTo(y_hit));
        break;
      }
    }
  }

  virtual void EnemyFiresPistol(unsigned int index) throw() {
    for (int i = 0; i < MAX_FIRES; i++) {
      if (fires[i].OkayToDelete()) {
        fires[i].Setup(enemies[index].X(), enemies[index].Y(),
                       enemies[index].Direction(), WEAPON_PISTOL, true);
        break;
      }
    }
  }

  virtual void EnemyFiresMachineGun(unsigned int index) throw() {
    for (int i = 0; i < MAX_FIRES; i++) {
      if (fires[i].OkayToDelete()) {
        fires[i].Setup(enemies[index].X(), enemies[index].Y(),
                       enemies[index].Direction(), WEAPON_MACHINEGUN,
                       true);
        break;
      }
    }
  }

  virtual void PickUpPowerUp(unsigned short index) throw() {
    if (index < GLUpowerups.size()) {
      vector<CPowerUp>::iterator p = GLUpowerups.begin() + index;
      if (p->Regenerates()) {
        p->PickUp();
      } else {
        *p = GLUpowerups[GLUpowerups.size() - 1];
        GLUpowerups.resize(GLUpowerups.size() - 1);
      }
    }
  }

  virtual void ClearEnemyArray() throw() {
    logger << "Call to clear enemy array" << endl;
    enemies.clear();
  }

  virtual void CreateEnemy(unsigned int model) throw() {
    unsigned int index = enemies.size();
    enemies.resize(index + 1);
    enemies[index].Setup(model);
  }

  virtual void SetEnemyWeapon(unsigned int index, unsigned int weapon)
    throw() {
    enemies[index].SetWeapon(weapon);
  }

  virtual void SetEnemyPosition(unsigned int index,
                                unsigned short x,
                                unsigned short y) throw() {
    enemies[index].SetPosition(FixedCnvTo<long>(x),
                               FixedCnvTo<long>(y));
  }

  virtual void SetEnemyDirection(unsigned int index,
                                 unsigned int direction) throw() {
    enemies[index].SetDirection(direction);
  }

  virtual void WalkEnemy(unsigned int index) throw() {
    enemies[index].Walk(false);
    enemies[index].TryToMove();
  }

  virtual void KillEnemy(unsigned int index, WORD *ammo) throw() {
    vector<CPowerUp>::iterator p;
    FIXEDNUM got_data[NUM_WEAPONS];

    GLUpowerups.resize(GLUpowerups.size() + 1);
    p = GLUpowerups.end() - 1;

    for (int i = 0; i < NUM_WEAPONS; i++) {
      got_data[i] = (FIXEDNUM)ammo[i];
    }

    p->Setup(enemies[index].X(),
             enemies[index].Y(),
             got_data);
  }

  virtual void HurtEnemy(unsigned int index) throw() {
    enemies[index].Hurt();
  }

  virtual void HurtHero(unsigned int weapon_type) throw() {
    hero.SubtractHealth(weapon_type);
  }

  virtual void PlayerLeaving(const char *name) throw() {
    try {
      int name_strlen = strlen(name);
      string format;
      
      GluStrLoad(IDS_OLDPLAYER, format);
      Buffer buf(name_strlen + format.length() + 1);
      
      sprintf((char *)buf.Get(), format.c_str(), name);
				
      logger << "Posting message of player leaving to screen" << endl;
      GluPostMessage((const char *)buf.Get());
    } catch (std::bad_alloc& ba) { }
  }

  virtual void PlayerJoining(const char *name) throw() {
    try {
      int name_strlen = strlen(name);
      string format;
      
      GluStrLoad(IDS_NEWPLAYER, format);
      Buffer buf(name_strlen + format.length() + 1);
      
      sprintf((char *)buf.Get(), format.c_str(), name);
				
      logger << "Posting player join message to screen" << endl;
      GluPostMessage((const char *)buf.Get());
    } catch (std::bad_alloc& ba) { }
  }
};

static void ClearSectors() {
  logger << "Clearing sector data grid" << endl;
  
  for (vector<TSector>::iterator itr = sectors.begin();
       itr != sectors.end(); itr++) {
    itr->ClearMaps();
  }

  sectors.clear();

  logger << "Finished clearing" << endl;
}

// dialog proc.  This is a callback function that
//  sends messages to the code entity that defines
//  how the intro box works.  This box's controls
//  and the locations of them are defined in the
//  resource
static BOOL CALLBACK CfgDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM) {
  switch(uMsg) {
  case WM_COMMAND: {
    // get the identifier of the control, which is always passed
    //  through the lower word of the wParam parameter
    WORD ctrl = LOWORD(wParam);
    if(IDLAUNCH == ctrl || IDQUIT == ctrl || IDLAUNCHWITHOUTMUSIC == ctrl) {
      // the launch or quit or launch w/o music btn was pressed 
      char txt[MAX_STRINGLEN];
        
      // get the text of the sync rate edit box, where the user specifies
      //  how often andradion 2 peers communicate their location
      GetDlgItemText(hwndDlg, IDC_SYNC, txt, MAX_STRINGLEN);

      // set sync rate to a value which will be detected
      //  as invalid.  If the scanf function does not convert
      //  the string because it is invalid or something,
      //  and the sync_rate var isn't touched, then the if()
      //  block will see the error
      sync_rate = 0;

      // scan the sync rate from txt using the mapped tchar function
      //  of scanf
      sscanf(txt, SYNCRATE_FORMAT, &sync_rate);
				
      // see if the sync_rate was invalid
      if(sync_rate < 1 || sync_rate > 100) {
        // invalid sync rate was entered
        //  display error
        // load two strings that make up the message box of the error
        string error_msg, dialog_caption;
        GluStrLoad(IDS_INVALIDSYNCRATE, error_msg);
        GluStrLoad(IDS_WINDOWCAPTION, dialog_caption);

        MessageBox(hwndDlg, error_msg.c_str(), dialog_caption.c_str(), MB_ICONSTOP);
      } else {
        // a valid sync rate was entered
        if(IDLAUNCHWITHOUTMUSIC == ctrl) {
          MusicStop();
          GluDisableMusic(); // nullifies all GluSetMusic calls
        }
        
        EndDialog(hwndDlg, LOWORD(wParam));
      }
    }
    return FALSE;
  }
  case WM_SHOWWINDOW: {
    // use this opportunity to do somethings to initialize

    // set the hyper welcome box music
    GluSetMusic(false,IDR_WELCOMEBOXMUSIC);
			
    // convert a number to from DWORD to char
    //  then char to tchar by using a simple while loop
    char number_buffer[MAX_STRINGLEN];
    itoa(sync_rate,number_buffer,10);

    // set the text in the sync rate edit box to what we just got
    //  from the sprintf function
    SetDlgItemText(hwndDlg,IDC_SYNC,number_buffer);
    return FALSE;
  }
  case WM_INITDIALOG:
    return TRUE; // we want the currently-in-focus control to be chosen automatically
  default:
    return FALSE;
  }
}

static void ResetSinglePlayerScore(int possible_score) {
  char str_score[SCORELEN];
  sprintf(str_score, TWO_NUMBERS_FORMAT, 0, possible_score);
  score = str_score;

  CalculateScorePrintX();
}

static void CalculateScorePrintX() {
  score_print_x = GAME_MODEWIDTH - score.length() * (FONTWIDTH+1)
    + SCORE_X_OFFSET;
}

static void *GetResPtr(const char *res_name, const char *res_type,
                       HMODULE res_mod, WORD res_lang,
                       HRSRC& res_handle, HGLOBAL& data_handle) {
  res_handle = FindResourceEx(res_mod, res_type, res_name, res_lang);
  data_handle = LoadResource(res_mod, res_handle);
  return LockResource(data_handle);
}

static float spf;
static DWORD fps;
static FIXEDNUM sfxfreq;

static int GetSpeed() {return fps / 25;}

static void SetSpeed(int index) {
  logger << "SetSpeed(" << index << ") called" << endl;

  switch(NetInGame() ? 1 : index) {
  case 0:
     // slow the game down
     spf = 0.08f; // more seconds per frame
     fps = 12; // fewer frames per second
     sfxfreq = Fixed(0.5f); // lower sound frequency
     SetTempo(DefaultTempo()/2.0f);
     WtrSetSoundPlaybackFrequency(FixedCnvFrom<long>(Fixed(0.5f)
                                                     * SOUNDRESOURCEFREQ));
     break;
  case 1:
     // normal game speed
     spf = 0.04f;
     fps = 25;
     sfxfreq = Fixed(1);
     SetTempo(DefaultTempo());
     WtrSetSoundPlaybackFrequency(FixedCnvFrom<long>(Fixed(1.0f)
                                                     * SOUNDRESOURCEFREQ));
     break;
  case 2:
     // speed the game up
     spf = 0.02f;
     fps = 50;
     sfxfreq = Fixed(2);
     SetTempo(DefaultTempo()*2.0f);
     WtrSetSoundPlaybackFrequency(FixedCnvFrom<long>(Fixed(2.0f)
                                                     * SOUNDRESOURCEFREQ));
  }
}

bool GluInitialize(HINSTANCE hInstance_, HWND hWnd_) { 
  for(int i = 0; i < MAX_SOUNDS; i++) {
    playing[i] = NULL;
  }
  
  CoInitialize(NULL); 
  hInstance = hInstance_;
  hWnd = hWnd_;

  string ini_file;
  GluStrLoad(IDS_INIFILE,ini_file);

  // level completion data
  HANDLE lc = CreateFile(ini_file.c_str(),
                         GENERIC_READ, 0, 0,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL, 0);

  if(INVALID_HANDLE_VALUE != lc) {
    DWORD read;
    ReadFile(lc,&sync_rate,sizeof(sync_rate),&read,NULL);
    DeeInitialize(lc);
    // and finally close the file
    CloseHandle(lc);
  } else {
    sync_rate = DEFAULT_SYNCRATE;
    DeeInitialize();
    string msg;
    string dlg_caption;
    GluStrLoad(IDS_BUDGETCUTS,msg);
    GluStrLoad(IDS_WINDOWCAPTION,dlg_caption);
    MessageBox(hWnd, msg.c_str(), dlg_caption.c_str(), MB_ICONINFORMATION);
  }

  if(FAILED(CoCreateInstance(CLSID_DirectSound, 0, CLSCTX_INPROC_SERVER,
			     IID_IDirectSound, (void **)&ds)) ||
     FAILED(ds->Initialize(0))) {
    ds = 0;
    GluDisableMusic();
  } else {
    ds->SetCooperativeLevel(hWnd_, DSSCL_NORMAL);
    // load sounds after intro . . .
    TryAndReport(MusicInit(hWnd_, ds));
  }

  NetInitialize();

  // show welcome dialog (aka cfg dialog)
  if(IDQUIT ==
     DialogBox(hInstance,MAKEINTRESOURCE(IDD_CFG),hWnd,CfgDlgProc)) {
    return true; // return true to quit
  }

  HideMouseCursor();

  // create direct input and setup the device state change event
  CoCreateInstance(CLSID_DirectInput,
                   NULL,
                   CLSCTX_INPROC_SERVER,
                   IID_IDirectInput,
                   (void **)&di);
  di->Initialize(hInstance_, DIRECTINPUT_VERSION);
  di->CreateDevice(GUID_SysKeyboard, &did, NULL);
  did->SetDataFormat(&c_dfDIKeyboard);
  did->SetCooperativeLevel(hWnd_, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

  // the device will be acquired automatically the first we try to get
  //  the device state.  The GetDeviceState function will fail, and we
  //  will try and acquire it then
#ifdef _DEBUG
  profiler_font = (HGDIOBJ)CreateFont(PROFILER_FONT_SIZE,
                                      0, 0, 0, 400, 0, 0, 0,
                                      ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                                      CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                      FF_MODERN | FIXED_PITCH, NULL);
#endif

  // this will just copy the font data resource into a global array
  memcpy(font_data, LockResource(LoadResource
                                 (0, FindResource
                                  (0, MAKEINTRESOURCE(IDR_FONT), "DAT"))),
         FONTDATA_SIZE);

  state = GLUESTATE_INTRODUCTION;

  ClearSectors();

  level = LEVEL_NONE;

  memset(GLUkeyb, 0, KBBUFFERSIZE);
  memset(bitmaps, 0, BITMAP_COUNT[RESOURCELOAD_MP] * sizeof(surf_t));
  memset(sounds, 0, NUM_SOUNDS * sizeof(IDirectSoundBuffer *));
  memset(playing, 0, MAX_SOUNDS * sizeof(IDirectSoundBuffer *));

  bitmaps_loaded = 0;
  sounds_loaded = RESOURCELOAD_NONE;

  return false;
}

void GluRelease() {
  if(GLUESTATE_UNINITIALIZED == state) {
    logger << "Glue is already released" << endl;
    return;
  }

  logger << "calling NetRelease()" << endl;
  NetRelease();
  logger << "deleting menu" << endl;
  delete m;
  m = 0;
  logger << "calling MusicUninit()" << endl;
  MusicUninit();
  logger << "calling LoadSounds()" << endl;
  LoadSounds(RESOURCELOAD_NONE);

#ifdef _DEBUG
  TryAndReport(DeleteObject(profiler_font));
#endif

  TryAndReport(did->Unacquire());
  TryAndReport(did->Release());
  TryAndReport(di->Release());

  logger << "Unload all bitmaps" << endl;
  LoadBitmaps(RESOURCELOAD_NONE);
  
  ClearSectors();
  
  WtrRelease();
  logger << "Releasing cached map surfaces" << endl;
  int i;
  for(i = 0; i < CACHED_SECTORS; i++) {
    GfxDestroySurface(cached[i].first);
    GfxDestroySurface(cached[i].second);
  }

  logger << "Uncertifying CGraphics" << endl;
  GfxUninit();
	
  logger << "About to release all sounds" << endl;
  for(i = 0; i < MAX_SOUNDS; i++) {
    if(playing[i]) {
      TryAndReport(playing[i]->Release());
      playing[i] = 0;
    }
  }

  logger << "About to close DirectSound interface" << endl;
  if(NULL != ds) {
    TryAndReport(ds->Release());
    ds = NULL;
  }
	
  logger << "Saving level completion data" << endl;
  HANDLE lc;
  string ini_file;
  GluStrLoad(IDS_INIFILE,ini_file);
  logger << "About to write to " << ini_file << endl;
  lc = TryAndReport(CreateFile(ini_file.c_str(),GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL));
  DWORD written;
  TryAndReport(WriteFile(lc,(const void *)&sync_rate,sizeof(sync_rate),&written,NULL));
  logger << written << " bytes written" << endl;
  DeeRelease(lc);
  TryAndReport(CloseHandle(lc));
  logger << "Closing COM" << endl;
  CoUninitialize();
  logger << "Calling ShowMouseCursor to show the mouse" << endl;
  ShowMouseCursor();

  state = GLUESTATE_UNINITIALIZED;

  logger << "GluRelease returning" << endl;
}

class StarFiller : public SurfaceFiller {
public:
  virtual void FillSurface(BYTE *starsb, int p, int inx, int iny) throw() {
    BYTE *clearing_point = starsb;
    for (int y = 0; y < iny; y++) {
      memset(clearing_point, 0, inx);
      clearing_point += p;
    }
    
    for (int i = 0; i < NUMSTARS; i++) {
      // plot a bunch of stars
      // make a small plus sign for each star

      int x = (rand()%(inx-2))+1;
      int y = (rand()%(iny-2))+1;
      BYTE c = 255-(rand()%MAXSTARDIMNESS);
      starsb[y*p+x] = c;
      starsb[(y+1)*p+x] = starsb[(y-1)*p+x] = starsb[y*p+x+1] =
        starsb[y*p+x-1] = c/2;
    }
  }
};

static void Introduction() {
  short STORYCOOR[SCREENROWS], STORYWIDTH[SCREENROWS];

  // calculate STORYCOOR and STORYWIDTH values
  for (int i = 0; i < SCREENROWS; i++) {
    float Y = SCREENMINROW + (float)i;
    float MplusY = Y + SCREENHEIGHTHALF;
    float S = MplusY
      * (STORYANGLECOS + STORYANGLESIN
         * tan(STORYANGLE + atan(Y / VIEWERDISTANCE)));
    float Z = STORYANGLESIN * S;

    STORYCOOR[i] = (short)(S / 4.0f);
    STORYWIDTH[i] = (short)(VIEWERDISTANCE * STORYWIDTHHALFSQ
                            / SCREENWIDTHHALF
                            / (Z + VIEWERDISTANCE));
  }

  surf_t stars, story, turner; 
  CTimer timer, inc_or_dec;

  DDBLTFX fx;
  RECT source;

  int inx = DISPLAYAREA.right - DISPLAYAREA.left;
  int iny = DISPLAYAREA.bottom - DISPLAYAREA.top;

  state = GLUESTATE_INTRODUCTION; // we are in the intro right now

  // load the intro

  GfxInit(hWnd, MODEWIDTH, MODEHEIGHT, false);

  RECT dest;

  // setup the palette
  GamInitializeWithIntroPalette();

  // create starscape surface
  stars = GfxCreateSurface(inx, iny,
                           auto_ptr<SurfaceFiller>(new StarFiller()));
  // load story bmp
  story = BitmapLoadingSurfaceFiller::CreateSurfaceFromBitmap
    (hInstance, MAKEINTRESOURCE(IDB_STORY));

  turner = BitmapLoadingSurfaceFiller::CreateSurfaceFromBitmap
    (hInstance, MAKEINTRESOURCE(IDB_TURNER));

  IDirectSoundBuffer *warpout;
  DWORD warp_status; // playing or not of the above sound

  // load the warpout sound
  HRSRC res_handle;
  HGLOBAL data_handle;
  void *warpout_data = TryAndReport(GetResPtr(TEXT("SFX"), TEXT("DAT"), NULL,
					      MAKELANGID(LANG_NEUTRAL,
							 SUBLANG_NEUTRAL),
					      res_handle,data_handle));
  
  if(NULL == warpout_data) {
    warpout = NULL;
  } else {
    // we locked it successfully
    DWORD warpout_size = *(DWORD *)warpout_data;
    warpout_data = (void *)((DWORD *)warpout_data + 1);
    warpout = CreateSBFromRawData(ds, warpout_data, warpout_size,
                                  0, SOUNDRESOURCEFREQ, SOUNDRESOURCEBPS,
                                  1);
  }
  
  if(data_handle) {
    TryAndReport(FreeResource(data_handle));
  }

  logger << "Play Intro Music" << endl;
  GluSetMusic(false, IDR_INTROMUSIC);

  // load polygon data for splash screen
  int polygon_count, *vertex_counts, *polygon_vertices;

  BYTE *locked
    = (BYTE *)GetResPtr(MAKEINTRESOURCE(IDR_SPLASH), TEXT("DAT"), NULL,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                        res_handle, data_handle);

  if(NULL == locked) {
    polygon_count = 0;
    vertex_counts = NULL;
    polygon_vertices = NULL;
  } else {
    // load the meaningful data
    polygon_count = (int)*locked++;
    // get the vertex counts
    int total_vertex_count = 0;
    vertex_counts = new int[polygon_count];
    for(int i = 0; i < polygon_count; i++) {
        vertex_counts[i] = (int)*locked++;
        total_vertex_count += vertex_counts[i];
      }
    // get each vertex coordinate
    polygon_vertices = new int[total_vertex_count * 2];
    for(int i = 0; i < total_vertex_count; i++) {
      polygon_vertices[i * 2] = (int)*locked++;
      polygon_vertices[i * 2+1] = (int)*locked++;

      polygon_vertices[i * 2] *= MODEWIDTH;
      polygon_vertices[i * 2] /= 0x100;

      polygon_vertices[i * 2+1] *= MODEHEIGHT;
      polygon_vertices[i * 2+1] /= 0x100;
    }

    FreeResource(data_handle);
  }

  if (warpout) {
    logger << "Starting to play warpout sound" << endl;
    warpout->Play(0,0,0);
    logger << "Done with playing call." << endl;
  }

  CTimer total_flash;

  // we need a red brush and a red pen, and a yellow brush and a yellow pen
  HBRUSH brush1 = CreateSolidBrush(FLASHCOLOR1);
  HBRUSH brush2 = CreateSolidBrush(FLASHCOLOR2);
  HPEN pen1 = CreatePen(PS_SOLID, 1, FLASHCOLOR1);
  HPEN pen2 = CreatePen(PS_SOLID, 1, FLASHCOLOR2);

  do {
      CTimer frame; // this makes sure we don't flip too quick
      HDC dc;
      if(SUCCEEDED(GfxBackBuffer()->GetDC(&dc))) {
	swap(brush1, brush2);
	swap(pen1, pen2);
	
	// blit the background
	HBRUSH old_brush = (HBRUSH)SelectObject(dc, (HGDIOBJ)brush1);
	Rectangle(dc, 0, 0, MODEWIDTH, MODEHEIGHT);
	// select the objects for the text
	SelectObject(dc, (HGDIOBJ)brush2);
	HPEN old_pen = (HPEN)SelectObject(dc,(HGDIOBJ)pen2);
	// blit the text
	PolyPolygon(dc, (const POINT *)polygon_vertices,
                    vertex_counts, polygon_count);
	// Select back the old objects
	SelectObject(dc, (HGDIOBJ)old_brush);
	SelectObject(dc, (HGDIOBJ)old_pen);
	// release dc
	GfxBackBuffer()->ReleaseDC(dc);
      } while(frame.SecondsPassed32() < FLASHCOLORPERSEC);
      GfxFlip();
  } while((!warpout || SUCCEEDED(warpout->GetStatus(&warp_status)))
          && (warp_status & DSBSTATUS_PLAYING)
	  && (total_flash.SecondsPassed32() < MAXFLASHTIME));

  // get rid of those extra brushes we made
  DeleteObject((HGDIOBJ)brush1);
  DeleteObject((HGDIOBJ)brush2);
  DeleteObject((HGDIOBJ)pen1);
  DeleteObject((HGDIOBJ)pen2);

  if(warpout) {
    TryAndReport(warpout->Release());
    warpout = 0;
  }

  delete polygon_vertices;
  delete vertex_counts;

  // done loading!
  timer.Restart();

  FlushKeyPresses();
  inc_or_dec.Restart();
  do {
    // put the stars and black spots on the back buffer
    GfxPut(stars, DISPLAYAREA.left, DISPLAYAREA.top, false);
    GfxRectangle(0, &UPPERBLACKAREA);
    GfxRectangle(0, &LOWERBLACKAREA);

    float progress = timer.SecondsPassed32() / TIMETOTHROWBACK;
    dest.left = Range(0,iny/2,progress) + (inx - iny) / 2;
    dest.top = Range(0,iny/2,progress) + DISPLAYAREA.top;
    dest.right = Range(iny,iny/2,progress) + (inx - iny) / 2;
    dest.bottom = Range(iny,iny/2,progress) + DISPLAYAREA.top;

    if(dest.left >= dest.right || dest.top >= dest.bottom) {
      break;
    }

    // put the turner photo on the back buffer
    GfxPutScale(turner, &dest, true);
    Flip();
  } while(timer.SecondsPassed32() <= TIMETOTHROWBACK
          && key_presses.empty());
  inc_or_dec.Pause();

  // we don't need the turner photo no more
  GfxDestroySurface(turner);

  // now show the story
  FlushKeyPresses();
  timer.Restart();
  IDirectDrawSurface *backbuffer = GfxBackBuffer();
  RECT entire_screen = {0, 0, MODEWIDTH, MODEHEIGHT};
  int max_storyy = (int)(STORY_HEIGHT + iny);
  bool key_pressed = false;
  do {
    // put the stars and black spots on the back buffer
    GfxPut(stars, DISPLAYAREA.left, DISPLAYAREA.top, false);
    GfxRectangle(0, &UPPERBLACKAREA);
    GfxRectangle(0, &LOWERBLACKAREA);

    int storyy=Range(0,max_storyy,timer.SecondsPassed32()/TIMETOSCROLL);
    int index = 0;

    source.left = 0;
    source.right = STORY_WIDTH;
    source.top = storyy;
    source.bottom = storyy+1;
    dest.top = DISPLAYAREA.bottom - 1;
    dest.bottom = DISPLAYAREA.bottom;

    for(int index = 0; index < SCREENROWS; index++) {
      dest.left = MODEWIDTH / 2 - STORYWIDTH[index];
      dest.right = MODEWIDTH - dest.left;
      dest.top--;
      dest.bottom--;
      
      source.top = storyy - STORYCOOR[index];

      if(source.top < STORY_HEIGHT && source.top >= 0
         && dest.right > dest.left) {
        source.bottom = source.top + 1;

        GfxPutScale(story, &dest, true, &source);
      }
    }

    while(!key_pressed && !key_presses.empty()) {
      switch(key_presses.front()) {
      case VK_NEXT:
        logger << "PGDN" << endl;
        timer += inc_or_dec;
        key_presses.pop();
        if(timer.SecondsPassed32() >= TIMETOSCROLL) {
          timer -= inc_or_dec;
        }
        break;
      case VK_PRIOR:
        logger << "PGUP" << endl;
        timer -= inc_or_dec;
        key_presses.pop();
        if(0 > timer.SecondsPassedInt()) {
          timer.Restart();
        }
        break;
      case VK_SPACE:
        logger << "SPACE" << endl;
        if(timer.Paused()) {
          timer.Resume();
        } else {
          timer.Pause();
        }
        key_presses.pop();
        break;
      default:
        key_pressed = true;
      }
    }

    Flip();
  } while(!key_pressed);

  logger << "Introduction finished" << endl;

  logger << "Deleting stars and story" << endl;
  GfxDestroySurface(stars);
  GfxDestroySurface(story);

  logger << "Doing gray block screen transition" << endl;
    
  for(int c = 255; c >= 0; c--) {
    int x = rand()%(MODEWIDTH-TRANSITIONSQUARESIZE);
    int y0 = rand()%(MODEHEIGHT-TRANSITIONSQUARESIZE);
    int y1 = y0 + TRANSITIONSQUARESIZE;
    GfxLock lock(GfxLock::Front());

    BYTE *write_to = lock(x, y0);

    for (int yn = y0; yn < y1; yn++) {
      memset(write_to, GfxGetPaletteEntry(RGB(c, c, c)),
             TRANSITIONSQUARESIZE);
      write_to += lock.Pitch();
    }

    while(timer.SecondsPassed32() < TRANSITIONSECSPERSQUARE);
    timer.Restart();
  }

  logger << "Going into Mode 13h..." << endl;

  HANDLE reconfig_file = CreateFile("43.hz", FILE_SHARE_READ, 0, NULL,
				    OPEN_EXISTING, 0, NULL);
  GfxUninit();
  GfxInit(hWnd, GAME_MODEWIDTH, GAME_MODEHEIGHT,
          INVALID_HANDLE_VALUE != reconfig_file);

  CloseHandle(reconfig_file);

  logger << "Now in 13h" << endl;

  logger << "Creating surfaces for cached maps" << endl;
  for(int i = 0; i < CACHED_SECTORS; i++) {
    cached[i].first = GfxCreateSurface(SECTOR_WIDTH, SECTOR_HEIGHT);
    cached[i].second = GfxCreateSurface(SECTOR_WIDTH, SECTOR_HEIGHT);
  }
  upper_left_sector = 0;

  logger << "Initializing Pal with menu palette" << endl;
  GamInitializeWithMenuPalette();
  GfxRefillSurfaces();
	
  logger << "Loading single player and multiplayer bitmaps" << endl;
  LoadBitmaps(RESOURCELOAD_MP);
  logger << "Loading single player and multiplayer sounds" << endl;
  LoadSounds(RESOURCELOAD_MP);

  WtrInitialize(sounds);

  logger << "Loading important strings from string table" << endl;
  GluStrLoad(IDS_KILLED,KILLED);
  GluStrLoad(IDS_KILLEDYOURSELF,KILLEDYOURSELF);
  GluStrLoad(IDS_KILLEDTHEMSELVES,KILLEDTHEMSELVES);
  GluStrLoad(IDS_YOU,YOU);
  GluStrLoad(IDS_YOUKILLED,YOUKILLED);
  GluStrLoad(IDS_SPKILLED,SPKILLED);

  logger << "Calling SetupMenu()" << endl;
  SetupMenu();

  logger << "Flushing key presses and setting glue state" << endl;
  FlushKeyPresses();
  state = GLUESTATE_MAINMENU;

  logger << "Calling PrepareMenu() to prepare for main menu" << endl;
  PrepareMenu();

  logger << "Loading Chattahoochee main menu music" << endl;
  GluSetMusic(true, IDR_MENUMUSIC);

  logger << "Now leaving Introduction() function" << endl;
}

static void LoadBitmaps(int type) {
  logger << "LoadBitmaps called with load type " << type << endl;
  
  // first take care of the special case
  //  of reloading bitmaps in case of surface loss
  //  or palette change
  int prev_bmp_count = bitmaps_loaded;
  bitmaps_loaded = BITMAP_COUNT[type];

  logger << "prev_bmp_count: " << prev_bmp_count <<
      ", bitmaps_loaded: " << bitmaps_loaded << endl;

  if(bitmaps_loaded > prev_bmp_count) {
    logger << "we have to load more bitmaps" << endl;

    for(int i = prev_bmp_count; i < bitmaps_loaded; i++) {
      if(!bitmaps[i]) {
        logger << "Bitmap " << i << " not already loaded" << endl;
        bitmaps[i] = BitmapLoadingSurfaceFiller::CreateSurfaceFromBitmap
          (hInstance, MAKEINTRESOURCE(IDB_BITMAP2 + i));
      }
    }
  } else if(bitmaps_loaded < prev_bmp_count) {
    logger << "we have to release bitmaps" << endl;

    for(int i = bitmaps_loaded; i < prev_bmp_count; i++) {
      logger << "Releasing bitmap " << i << endl;
      GfxDestroySurface(bitmaps[i]);
      bitmaps[i] = 0;
    }
  }

  logger << "LoadBitmaps() finished" << endl;
}

static void LoadSounds(int type)
{
  const int SOUND_COUNT[] = {0,NUM_SPSOUNDS,NUM_SOUNDS};
  int next_num_sounds = SOUND_COUNT[type];
  int prev_num_sounds = SOUND_COUNT[sounds_loaded];
  logger << "LoadSounds called w/" << prev_num_sounds << " sounds loaded, "
      "caller wants " << next_num_sounds << " loaded" << endl;
  if(next_num_sounds > prev_num_sounds) {
      logger << "We have to load more sounds" << endl;
      HRSRC res_handle;
      HGLOBAL data_handle;
      DWORD *sound_data
        = (DWORD *)GetResPtr("SFX", "DAT", 0,
                             MAKELANGID(LANG_NEUTRAL,
                                        SUBLANG_NEUTRAL),
                             res_handle, data_handle);
      if(!sound_data) {
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
	  logger << "Loading sound " << i << endl;
          sounds[i]
            = CreateSBFromRawData(ds, (void *)(sound_data + 1),
                                  *sound_data,
                                  DSBCAPS_CTRLPAN
                                  | DSBCAPS_CTRLVOLUME
                                  | DSBCAPS_CTRLFREQUENCY,
                                  SOUNDRESOURCEFREQ,
                                  SOUNDRESOURCEBPS, 1);
	  logger << "Clearing reversed bit for sound " << i << endl;
	  reversed.reset(i);
	  // skip the first seven sounds; they aren't ours
	  sound_data = (DWORD *)((BYTE *)sound_data+*sound_data)+1;
	}
      FreeResource(data_handle);
    }
  else if(prev_num_sounds > next_num_sounds)
    {
      logger << "We have to release some sounds" << endl;
      for(int i = next_num_sounds; i < prev_num_sounds; i++)
	{
	  logger << "Releasing sound " << i << endl;
	  if(NULL != sounds[i])
	    {
	      TryAndReport(sounds[i]->Release());
	      sounds[i] = NULL;
	    }
	}
    }

  sounds_loaded = type;

  logger << "LoadSounds exitting" << endl;
}

void GluPlaySound(int i,FIXEDNUM x_dist,FIXEDNUM y_dist) {
  const int MY_SLOT = NextSoundSlot();
  LPDIRECTSOUNDBUFFER b2; // the duplicate of the sound

  x_dist = FixedCnvFrom<long>(x_dist);
  y_dist = FixedCnvFrom<long>(y_dist);

  float factor; // used in intermediate calculations

  // make sure we ever loaded this sound
  if(NULL == sounds[i]) {return;}
	
  if(SUCCEEDED(ds->DuplicateSoundBuffer(sounds[i], &b2))) {
    // set frequency
    DWORD freq;
    if(SUCCEEDED(b2->GetFrequency(&freq))) {
      freq = FixedCnvFrom<long>(freq * sfxfreq);

      // set our new frequency
      b2->SetFrequency(freq);
    }

    factor = (float)x_dist/(float)MAX_XDIST;
    if(factor > 1.0f) { // we were too far away
      factor = 1.0f;
    }
    factor *= 10000.0f;
    b2->SetPan((long)factor);

    factor = x_dist*x_dist+y_dist*y_dist - MIN_DISTSQUARED;
    if(factor < 0) {
      factor = 0;
    }
    factor /= (float)(MAX_DISTSQUARED);
    if(factor > 1.0f) { // we were too far away
      factor = 1.0f;
    }
    factor *= -10000.0f;
		
    b2->SetVolume((long)factor);

    b2->Play(0,0,0);

    // add playing sound to the list
    playing[MY_SLOT] = b2;
  }
}

static void PrepareMenu()
{
  SetSpeed(1);
  vector<string> strings;
  string header;

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
      strings.resize(DeeLevelAvailability(level));
      GluStrVctrLoad(IDS_DIFFICULTYLEVEL1,strings);
      GluStrLoad(IDS_LEVELNAME1 + level * 2,header);
      m->SetStrings(header,strings,strings.size()-1);

      // load bitmaps without mp
      LoadBitmaps(RESOURCELOAD_SP);

      // load sounds without mp
      LoadSounds(RESOURCELOAD_SP);

      // figure out accomplishment text lines
      GLUdifficulty = m->GetSelectionIndex();
      
      FillAccomplishmentLines();
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
      strings[0] = player_name;
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

      // load extra multiplayer bitmaps
      LoadBitmaps(RESOURCELOAD_MP);

      break;
    case GLUESTATE_SELECTCONNECTIONMETHOD:{
      vector<string> con_names(NetProtocolCount());
      string con_header;
	
      GluStrLoad(IDS_SELECTCONNECTIONMETHODCAPTION,con_header);

      if(0 == con_names.size()) {
	con_names.resize(1);
	GluStrLoad(IDS_NOCONNECTIONMETHODAVAILABLE, con_names[0]);
      } else {
	for (int i = 0; i < con_names.size(); i++) {
	  con_names[i] = NetProtocolName(i);
	}
      }

      m->SetStrings(con_header,con_names,0);
    }
    }
  key_presses.push(0);
}

static int MenuLoop() {
  BYTE last_key_pressed;
  bool show_demo_character =
    GLUESTATE_PICKCHARACTER == state ||
    (GLUESTATE_LEVELSELECT == state && NetProtocolInitialized()) ||
    GLUESTATE_SELECTCONNECTIONMETHOD == state ||
    GLUESTATE_PICKGAME == state;
  
  do {
      GamOneFrame(Fixed(1.0f));

      if(key_presses.empty()) {
	last_key_pressed = 0;
      } else {
	last_key_pressed = key_presses.front();
	key_presses.pop();
      }
		
      bool has_changed_position = false;

      if(VK_DOWN == last_key_pressed) {
	  // going up in the selections
	  if(m->MoveDown()) {
            GluPlaySound(WAV_STEP,Fixed(1),false);
            has_changed_position = true;
          }
      } else if(VK_UP == last_key_pressed) {
        // going down in the selections
        if(m->MoveUp()) {
          GluPlaySound(WAV_STEP,Fixed(1),true);
          has_changed_position = true;
        }
      }

      m->FillSurface();

      // show GLUdifficulty level accomplishments
      if(GLUESTATE_DIFFICULTYSELECT == state) {
        // if we have changed our position, and we are selecting GLUdifficulty,
        //  then we have to update our accomplishment data
        if(has_changed_position) {
          GLUdifficulty = m->GetSelectionIndex();
          FillAccomplishmentLines();
        }
			
        HDC dc;
        if(SUCCEEDED(GfxBackBuffer()->GetDC(&dc))) {
          // select new parameters in, saving the old ones
          int old_bk_mode = SetBkMode(dc,TRANSPARENT);
          COLORREF old_text_color = SetTextColor(dc,COLOR_ACCOMPLISHMENTTEXT);

          // print the text
          TextOut(dc,
                  XCOOR_ACCOMPLISHMENTTEXT,
                  YCOOR_ACCOMPLISHMENTTEXT,
                  accomplishment_lines[DEEDS_SUMMARY].c_str(),
                  accomplishment_lines[DEEDS_SUMMARY].length());

          // restore old text-drawing parameters
          SetTextColor(dc,old_text_color);
          SetBkMode(dc,old_bk_mode);
          GfxBackBuffer()->ReleaseDC(dc);
        }
      } else if(show_demo_character) {
        // make sure we don't have any whacked-out values in char_demo_* vars
        if(char_demo_direction < 0
           || char_demo_direction >= RENDERED_DIRECTIONS) {
          char_demo_direction = DSOUTH;
        }

        if(char_demo_direction_changer.SecondsPassed32()
           >= DEMOCHAR_SECSTOCHANGEDIR) {	
          char_demo_direction = rand()%RENDERED_DIRECTIONS;
          char_demo_direction_changer.Restart();
        }

        int bmp = BMPSET_CHARACTERS;
		
        if(char_demo_stepper.SecondsPassed32() > DEMOCHAR_SECSTOSTEP) {
          bmp += RENDERED_DIRECTIONS;
          if(char_demo_stepper.SecondsPassed32()
             > DEMOCHAR_SECSTOSTEP*2.0f) {
            char_demo_stepper.Restart();
          }
        }

        bmp += char_demo_direction;

        int target_x, target_y;

        if(GLUESTATE_PICKCHARACTER == state) {
          target_x = DEMOCHAR_X;
          target_y = DEMOCHAR_Y;
          bmp+=m->GetSelectionIndex()*ANIMATIONFRAMESPERCHARACTER;
        } else {
          target_x = DEMOCHAR_X2;
          target_y = DEMOCHAR_Y2;
          bmp+=model*ANIMATIONFRAMESPERCHARACTER;
        }

        GfxPut(bitmaps[bmp], target_x, target_y);
      }
      
      Flip();
    } while(VK_RETURN != last_key_pressed &&
            VK_ESCAPE != last_key_pressed);

  if(VK_RETURN == last_key_pressed) {
    GluPlaySound(WAV_OKGOTIT, FREQFACTOR_OKGOTITNORMAL, false); 
    return MENUACTION_RETURN;
  } else {
    GluPlaySound(WAV_OKGOTIT,FREQFACTOR_OKGOTITBACKWARDS, true);
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

static pair<const BYTE *, HGLOBAL> LoadLevelPaletteOnly() {
  string path;

  assert(level >= 0);

  GluStrLoad(IDS_LEVELFILE1 + level * 2, path);

  HRSRC res_handle
    = FindResourceEx(0, TEXT("LVL"), path.c_str(),
                     MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
  HGLOBAL data_handle = LoadResource(0, res_handle);
  const BYTE *data_ptr = (const BYTE *)LockResource(data_handle);

  return pair<const BYTE *, HGLOBAL>(GamInitialize(data_ptr), data_handle);
}

static void LoadLevel() {
  int i, j, k, l, m, n, o, p;

  logger << "LoadLevel has been called" << endl;
  RECT target;
  target.left= 0;
  target.top = 0;
  target.bottom = GAME_MODEHEIGHT;
  target.right = GAME_MODEWIDTH;

  logger << "Making a blank screen" << endl;
  GfxRectangle(0, &target);

  assert(level >= 0);

  string path;
	
  GluStrLoad(IDS_LEVELFILE1+level*2,path);

  if (!NetInGame()) {
    FillAccomplishmentLines();
  }

  logger << "User is loading level with string id: " << path << endl;

  // load the file by getting a pointer to the resource

  // GET A POINTER TO RESOURCE DATA
  pair<const BYTE *, HGLOBAL> level_resource = LoadLevelPaletteOnly();
  const BYTE *data_ptr = level_resource.first;

  // load the palettes
  if(LL_BONE & level_loaded) {
    CFire::PickBestBulletTrailColor();
    WtrAnalyzePalette();
    // reload bitmaps since the palette has changed
    GfxRefillSurfaces();
    GluFindTextColors();

    // get level width and height
    int level_width, level_height;
    data_ptr = ExtractWord(data_ptr, level_width);
    data_ptr = ExtractWord(data_ptr, level_height);
    LoadCmps(level_width, level_height, !(LL_FLESH & level_loaded));
  } else {
    // rush past level width and height (four bytes)
    data_ptr += sizeof(WORD) * 2 ;
  }

  // skip the rest if we don't have to load item data and positions
  if(!(LL_FLESH & level_loaded)) {
    FreeResource(level_resource.second);
    level_loaded = LL_OKAY;
    return;
  }

  // load weather script index
  int script_index;
  data_ptr = ExtractByte(data_ptr,script_index);
  if(!NetInGame()) {
     WtrBeginScript(script_index);
  }

  possible_starting_spots.clear();

  // get hero data

  // get turner's coordinates
  data_ptr = ExtractWord(data_ptr,i);
  data_ptr = ExtractWord(data_ptr,j);
  i = FixedCnvTo<long>(i);
  j = FixedCnvTo<long>(j);
	
  AddPossibleStartingSpot(i,j);

  if(!NetProtocolInitialized()) {
     // single-player behaviour
     hero.Setup(i, j, CHAR_TURNER, false);
  } else {
     hero.Setup(-1, -1, model, true);
  }

  ul_cached_sector_x = ul_cached_sector_y = -1;

  assert(i >= 0);
  assert(j >= 0);

  walking_data = CBitMatrix::forDimensions(width_in_tiles, height_in_tiles);

  // loop through each rectangle which defines indoor regions

  data_ptr = ExtractByte(data_ptr,j);

  for(i = 0; i < j; i++) {
    data_ptr = ExtractWord(data_ptr, m);
    data_ptr = ExtractWord(data_ptr, n);
    data_ptr = ExtractWord(data_ptr, o);
    data_ptr = ExtractWord(data_ptr, p);

    for(k = n; k < p; k+= TILE_HEIGHT) {
      assert(k/TILE_HEIGHT >= 0);
      assert(k >= 0);
      for(l = m; l < o; l+=TILE_WIDTH) {
        walking_data->set(l / TILE_WIDTH, k / TILE_HEIGHT);
      }
    }
  }

  for(i = 0; i < total_sectors; i++) {
     sectors[i].enemies.clear();
     sectors[i].powerups.clear();
     sectors[i].levelEnds.clear();
  }

  // so now let's do the level ends

  data_ptr = ExtractByte(data_ptr, j);

  lends.clear();

  for(i = 0; i < j; i++) {
     data_ptr = ExtractByte(data_ptr,k);
     data_ptr = ExtractWord(data_ptr,l);
     data_ptr = ExtractWord(data_ptr,m);

     if(!NetInGame()) {
        const CLevelEnd cle(FixedCnvTo<long>(l), FixedCnvTo<long>(m), k);
        lends.push_back(cle);
        sectors[(m/SECTOR_HEIGHT) * sector_width + (l/SECTOR_WIDTH)]
          .levelEnds.push_back(cle);
     }
  }

  int possible_score = 0; // reset the potential score counter

  // do the enemies

  if(!NetInGame()) {
     enemies.clear();
  }

  for(p = 0; p < 3; p++) { // p is the current enemy type
    data_ptr = ExtractByte(data_ptr,j); // need to know how many there are
    if(!NetInGame()) {
       enemies.resize(enemies.size()+j);
    }

    for(i = 0; i < j; i++) {
      data_ptr = ExtractWord(data_ptr,k); // get x position
      data_ptr = ExtractWord(data_ptr,l); // get y position
      if(!NetInGame()) {
	int magic_index = enemies.size()-i-1;
	enemies[magic_index].Setup(FixedCnvTo<long>(k),
                                      FixedCnvTo<long>(l),
                                      p, false);
	int sec_row, sec_col;
	enemies[magic_index].CalculateSector(sec_row, sec_col);
	sectors[sec_row * sector_width + sec_col].enemies.insert(magic_index);
      } else {
	AddPossibleStartingSpot(FixedCnvTo<long>(k),FixedCnvTo<long>(l));
      }

      // increment possible score
      if(CHAR_EVILTURNER == p) {
	possible_score += GluScoreDiffKill(CHAR_SALLY);
      }
      possible_score +=GluScoreDiffKill(p);			
    }
  }

  // do the ammo and health like we did the enemies
  GLUpowerups.clear();

  for(p = 0; p < 4; p++) {
     data_ptr = ExtractByte(data_ptr,j);
     GLUpowerups.resize(GLUpowerups.size()+j);

     for(i = 0; i < j; i++) {
        data_ptr = ExtractWord(data_ptr,k);
        data_ptr = ExtractWord(data_ptr,l);
        sectors[(l/SECTOR_HEIGHT) * sector_width
                + (k/SECTOR_WIDTH)]
          .powerups.insert(GLUpowerups.size()-i-1);

        k = FixedCnvTo<long>(k);
        l = FixedCnvTo<long>(l);

        GLUpowerups[GLUpowerups.size()-i-1].Setup(k,l,p);
			
        AddPossibleStartingSpot(k,l);

        possible_score += GluScoreDiffPickup(p);
     }
  }

  std_powerups = GLUpowerups.size();

  // reset score and calculate its coordinates
  ResetSinglePlayerScore(possible_score);

  // reset timer shown in bottom left of screen
  since_start = 0;

  // and now we've finished
  FreeResource(level_resource.second);

  level_loaded = LL_OKAY;
}

static void PrepareForMPGame() {
   GLUdifficulty = MPDIFFICULTY;

   // reset score and calculate its coordinates
   GluChangeScore(0);
   
   HideMouseCursor(); 
   NetChangeWeather(WtrCurrentState());
   NetSetLevelIndex(level);
}

static void EndGame() {
  // TODO: ADD END-GAME CODE HERE
}

bool GluWalkingData(FIXEDNUM x,FIXEDNUM y) {
   x /= TILE_WIDTH; 
   y /= TILE_HEIGHT;

   return walking_data->get(FixedCnvFrom<long>(x), FixedCnvFrom<long>(y));
}

static void Flip() {
  bool lost_focus = false;

  while(true) {
    MSG msg;

    FlushKeyPresses();
    if (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    if (!GfxInFocus(SUCCEEDED(did->Acquire()))) {
      lost_focus = true;
    } else {
      break;
    }
  }

  if (lost_focus) {
    if (GLUESTATE_GAME == state) {
      FreeResource(LoadLevelPaletteOnly().second);
    } else if (GLUESTATE_INTRODUCTION == state) {
      GamInitializeWithIntroPalette();
    } else {
      GamInitializeWithMenuPalette();
    }

    GfxRefillSurfaces();
  }

  // display frame rate/Choppiness factor if the
  //  user is holding down the C key
  if(GLUkeyb[DIK_C] & EIGHTHBIT) {
     static int frames = 0;
     static CTimer counter;
     
     frames++;
     if(counter.SecondsPassed32() >= 1.0f) {
        char buffer[FRAMERATE_BUFFERLEN];
        frames = fps - frames;
        sprintf(buffer, ONE_NUMBER_FORMAT, frames);
        GluPostMessage(buffer);
        frames = 0;
        counter.Restart();
     }
  }

  if(GLUESTATE_GAME == state) {
     GamOneFrame(WtrBrightness());
  }

#ifdef _DEBUG
  if(GLUESTATE_GAME == state) {
     // show profiler results
     vector<string> profiler_results;
     GetProfileData(profiler_results);
     HDC dc;
     if(SUCCEEDED(GfxBackBuffer()->GetDC(&dc))) {
        int old_bk_mode = SetBkMode(dc,TRANSPARENT);
        HGDIOBJ old_font = SelectObject(dc,profiler_font);
        COLORREF old_text_color = SetTextColor(dc,RGB(255,255,255));
        int y = 0;
        for(vector<string>::iterator i = profiler_results.begin();
            i != profiler_results.end(); i++) {
           TextOut(dc,0,y,i->c_str(),i->length());
           y += PROFILER_FONT_SIZE;
        }
        SetBkMode(dc,old_bk_mode);
        SetTextColor(dc,old_text_color);
        SelectObject(dc,old_font);
        GfxBackBuffer()->ReleaseDC(dc);
     }
  }
#endif

  GfxFlip();

  if(GLUESTATE_GAME == state) {
     static CTimer syncer;

     while(syncer.SecondsPassed32() < spf);
     syncer.Restart();

     // a complete frame has passed
     since_start += TIMER_INC_PER_FRAME;
     if(since_start > MAX_TIMERSECONDS) {
        since_start = MAX_TIMERSECONDS;
     }
  }
}

void GluPostMessage(const char *str) {
  // make sure we are done with the current message
  if(frames_for_current_message >= MINFRAMESTOKEEPMESSAGE) {
    message = str;

    if (message.length() > MAX_CHARS_PER_LINE) {
      message = message.substr(0, MAX_CHARS_PER_LINE);
    }
    
    msg_x = (GAME_MODEWIDTH  - (message.length() * (FONTWIDTH+1))) / 2;

    // reset the timer
    frames_for_current_message = 0;
  }
}

void GluChangeScore(int diff) {
  int new_score, max_score;

  sscanf(score.c_str(), TWO_NUMBERS_FORMAT, &new_score, &max_score);

	
  if(0 == diff)                 { new_score = 0        ;}
  if(new_score > MAXSCORE)      { new_score = MAXSCORE ;}
  else if(new_score < MINSCORE) { new_score = MINSCORE ;}
  else                          { new_score += diff    ;}

  char str_score[SCORELEN]; 
  if(NetInGame()) {
    // don't care about maximum score
    sprintf(str_score, ONE_NUMBER_FORMAT, new_score);
  } else {
    sprintf(str_score, TWO_NUMBERS_FORMAT, new_score, max_score);
  }

  score = str_score;

  // now calculate printing coordinates
  CalculateScorePrintX();

  if(!NetInGame() && max_score == new_score) {
    // colors should be flashing because we have the highest score,
    //  so flag it by making score_print_x negative
    score_print_x = -score_print_x;

    string max_score_message;
    GluStrLoad(IDS_MAXSCORE,max_score_message);
    GluPostMessage(max_score_message.c_str());
  }
}

void GluKeyPress(BYTE scan_code) {key_presses.push(scan_code);}

void GluCharPress(char c) {
  if(GLUESTATE_ENTERNAME == state) {
      switch(c)	{
	case '\b':
	  // backspace was pressed
	  if(player_name.length() > 0) {
            player_name = player_name.substr(0, player_name.length()-1);
          }
	  GluPlaySound(WAV_BING);
	  break;
      case '\r': case '\n': case '\t': case '\a':
      case '\f': case '\v': case 27:
        // a key we don't care about was pressed
        break;
      default:
        GluPlaySound(WAVSET_POINTLESS+(rand()%WAVSINASET),Fixed(1),rand()&1 ? true : false);
        player_name += c;
      }
      // reset the menu
      string header; 
      vector<string> strings;
      GluStrLoad(IDS_ENTERNAMECAPTION,header);
      strings.resize(1);
      strings[0] = player_name;
      m->SetStrings(header,strings,0);
      return;
  } else if(GLUESTATE_GAME == state && !NetInGame()) {
    // we are doing single-player, we may want to show the player some
    //  best time/ best score data
    switch(c) {
    case '8':
      SetSpeed(0);
      break;
    case '9':
      SetSpeed(1);
      break;
    case '0':
      SetSpeed(2);
      break;
    case 't':
    case 'T':
      // show best time
      GluPostMessage(accomplishment_lines[1].c_str());
      break;
    case 's':
    case 'S':
      // show best score
      GluPostMessage(accomplishment_lines[2].c_str());
      break;
    }
  }
}

static void SetupMenu() {
  // setup the menu
  LOGFONT mf; // font to use
  MenuFont(mf);
  // setup the menu
  m = new CMenu(mf, COLOR_UNSELECTED,
                COLOR_SHADOW, COLOR_SELECTED,
                COLOR_SHADOW, COLOR_HEADING,
                COLOR_SHADOW, SHADOW_OFFSET,
                bitmaps[BMP_MENU]);
}

void GluPostSPKilledMessage() {GluPostMessage(SPKILLED.c_str());}

static void MenuFont(LOGFONT& lf) {
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
  lf.lfFaceName[0] = '\0';
}

static void Levels(vector<string>& target)
{
  // find the first available level from the end
  DWORD i;
  for(i = NUM_LEVELS -1; i >= 0; i--) {
    if(LEVELAVAIL_NONE != DeeLevelAvailability(i)) {
      break;
    }
  }
	
  // fill up every element
  target.resize(NUM_LEVELS); // assume player has every level
  char buffer[MAX_STRINGLEN];
  int i3 = 0;
  for(int i2 = IDS_LEVELNAME1; i2 < IDS_LEVELNAME1+NUM_LEVELS*2; i2+=2)
    {
      LoadString(hInstance,i2,buffer,MAX_STRINGLEN);
      target[i3++] = buffer;
    }
  target.resize(i+1); // shrink down to chop off all trailing AVAIL_NONE's
	
	// add unavailable string to the end of every level that's unavailable
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

void GluStrLoad(unsigned int id, string& target) {
  char buffer[MAX_STRINGLEN];
  LoadString(hInstance, id, buffer, MAX_STRINGLEN);
  target = buffer;
}

void GluInterpretDirection(BYTE d,FIXEDNUM& xf,FIXEDNUM& yf) {
  // fix the xf
  switch(d) {
  case DNORTH: case DSOUTH:           xf = Fixed( 0); break;
  case DWEST : case DNW   : case DSW: xf = Fixed(-1); break;
  default:                            xf = Fixed( 1);
  }

  // fix up the yf
  switch(d) {
  case DWEST : case DEAST:           yf = Fixed( 0); break;
  case DNORTH: case DNE  : case DNW: yf = Fixed(-1); break;
  default:                           yf = Fixed( 1); 
  }
}

void GluStrVctrLoad(unsigned int id, vector<string>& target) {
  char buffer[MAX_STRINGLEN];
	
  for(vector<string>::iterator iterate = target.begin();
      iterate != target.end(); iterate++) {
    LoadString(hInstance, id++, buffer, MAX_STRINGLEN);

    *iterate = buffer;
  }
}

static void LoadCmps(int level_width, int level_height,
                     bool skip_wd_resize) {
  // calculate these members so there is never
  //  any dead space at the edges of the screen when
  //  the hero is near the edge of the level
  max_center_screen_x = FixedCnvTo<long>(level_width  - GAME_MODEWIDTH/2);
  max_center_screen_y = FixedCnvTo<long>(level_height - GAME_PORTHEIGHT/2);

  int tw, th; // tile width and height

  tw = level_width / TILE_WIDTH;
  if(0 != level_width % TILE_WIDTH || level_width < TILE_WIDTH) {
    tw++;
  }
  th = level_height / TILE_HEIGHT;
  if(0 != level_height % TILE_HEIGHT || level_height < TILE_HEIGHT) {
    th++;
  }

  sector_width = level_width / SECTOR_WIDTH;
  if(0 != level_width % SECTOR_WIDTH || level_width < SECTOR_WIDTH) {
    sector_width++;
  }
  sector_height = level_height / SECTOR_HEIGHT;
  if(0 != level_height % SECTOR_HEIGHT || level_height < SECTOR_HEIGHT) {
    sector_height++;
  }

  logger << "Loading level cmps -- size is " << level_width << "x" <<
      level_height << endl;

  string path;

  if(!skip_wd_resize) {
    total_sectors = sector_width * sector_height;
    ClearSectors();
    sectors.resize(total_sectors);
    width_in_tiles = tw;
    height_in_tiles = th;
    walking_data = CBitMatrix::forDimensions(tw, th);
  }
				
  GluStrLoad(IDS_LEVELPATH, path);

  int r;

  string level_name;

  // name of the level is loaded here
  GluStrLoad(IDS_LEVELFILE1 + level * 2, level_name);

  auto_ptr<vector<CCompactMap *> > cmp_set = CCompactMap::LoadMapSet
    ((level_name + "_").c_str(), CMP_RESOURCE_TYPE, 0,
     MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));

  vector<TSector>::iterator current_sector = sectors.begin();
  vector<CCompactMap *>::iterator citr = cmp_set->begin();

  for(r = 0; r < sector_height; r++) {
    for(int c = 0; c < sector_width; c++, current_sector++) {
      delete current_sector->lowerCell;
      current_sector->lowerCell = *citr++;
    }
  }

  current_sector = sectors.begin();
  cmp_set = CCompactMap::LoadMapSet
    ((level_name + "u").c_str(), CMP_RESOURCE_TYPE, 0,
     MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
  citr = cmp_set->begin();

  for(r = 0; r < sector_height; r++) {
    for(int c = 0; c < sector_width; c++, current_sector++) {
      delete current_sector->upperCell;
      current_sector->upperCell = *citr++;
    }
  }
}



void GluFilterMovement(const POINT *start, POINT *end) {
  // this method is public because the CFire class needs to use it
  //  the plans parameter is passed as a non-const reference because
  //  we will change the second part of the pair to tell the mover where
  //  they can go which is closest to where they wanted to go

  // first check to make sure they are on the screen, and that we have 
  //  the back buffer successfully locked

  if(abs(GLUcenter_screen_x - start->x) >= Fixed(GAME_MODEWIDTH/2) ||
     abs(GLUcenter_screen_y - start->y) >= Fixed(GAME_PORTHEIGHT/2)) {
    // this guy isn't on the screen
    *end = *start;
    return;
  }

  // clip the second part of the plans pair to the edge of the screen
  if(end->x - GLUcenter_screen_x >= Fixed(GAME_MODEWIDTH/2)) {
    end->x = GLUcenter_screen_x + Fixed(GAME_MODEWIDTH/2 - 1);
  } else if(GLUcenter_screen_x - end->x >= Fixed(GAME_MODEWIDTH/2)) {
    end->x = GLUcenter_screen_x - Fixed(GAME_MODEWIDTH/2 - 1);
  }

  if(end->y - GLUcenter_screen_y >= Fixed(GAME_PORTHEIGHT/2)) {
    end->y = GLUcenter_screen_y + Fixed(GAME_PORTHEIGHT/2 - 1);
  } else if(GLUcenter_screen_y - end->y >= Fixed(GAME_PORTHEIGHT/2)) {
    end->y = GLUcenter_screen_y - Fixed(GAME_PORTHEIGHT/2 - 1);
  }

  int x_change = FixedCnvFrom<long>(end->x - start->x);
  int y_change = FixedCnvFrom<long>(end->y - start->y);

  int gen_change, inc;
  LONG *axis, axis_inc;
	
  GfxLock lock(GfxLock::Back());

  if(x_change) {
    if(y_change) {
      // moving diagonally (never use rerouting in this case)

      if(y_change < 0) {
        y_change = -y_change;
      }

      if(x_change < 0) {
        x_change = -x_change;
      }

      inc = lock.Pitch() - x_change - 1;

      BYTE *surface_data
        = lock(FixedCnvFrom<long>(min(start->x, end->x)-GLUcenter_screen_x
                                  + Fixed(GAME_MODEWIDTH /2)),
               FixedCnvFrom<long>(min(start->y, end->y)-GLUcenter_screen_y
                                  + Fixed(GAME_PORTHEIGHT/2)));

      for(int y = 0; y <= y_change; y++) {
        for(int x = 0; x <= x_change; x++) {
          if(!*surface_data++) {
            *end = *start;
            return;
          }
        }

        surface_data += inc;
      }

      return;
    } else {
      // moving horizontally

      if(x_change < 0) {	
        inc = -1;
        gen_change = -x_change;
        axis_inc = -Fixed(1);
      } else {
        inc = 1;
        gen_change = x_change;
        axis_inc = Fixed(1);
      }
	
      axis = &end->x;
    }
  } else if(y_change) {
    // moving vertically

    if(y_change < 0) {
      inc = -lock.Pitch();
      gen_change = -y_change;
      axis_inc = -Fixed(1);
    } else {
      inc = lock.Pitch();
      gen_change = y_change;
      axis_inc = Fixed(1);
    }

    axis = &end->y;
  } else {
    return;
  }

  BYTE *surface_data
    = lock(FixedCnvFrom<long>(start->x - GLUcenter_screen_x
                              + Fixed(GAME_MODEWIDTH / 2)),
           FixedCnvFrom<long>(start->y - GLUcenter_screen_y
                              + Fixed(GAME_PORTHEIGHT / 2)));

  *end = *start;

  *axis += axis_inc;
  surface_data += inc;

  // moves is automatically non-zero if we push
  //  Turner when he moves unsuccessully
  bool moves = bool((ALWAYSPUSH >> level) & 1);

  while (gen_change-- > 0) {
    if(!*surface_data) {
      *axis -= axis_inc;
      break;
    }
    
    *axis += axis_inc;
    surface_data += inc;
    moves = true;
  }
	
  if(moves) {
    *axis -= axis_inc;
  }
}

void GluGetRandomStartingSpot(POINT& p)
{
  p = possible_starting_spots[rand()%possible_starting_spots.size()];
  ul_cached_sector_x = ul_cached_sector_y = -1;
}

static void AddPossibleStartingSpot(FIXEDNUM x, FIXEDNUM y) {
  if(NetProtocolInitialized()) {
    // we are doing mp, so we need to store these extra
    //  coordinates
    int magic_i = possible_starting_spots.size();

    possible_starting_spots.resize(magic_i+1);

    possible_starting_spots[magic_i].x = x;
    possible_starting_spots[magic_i].y = y;
  }
}

void GluSetMusic(bool loop, const char *music_resource) {
  // only play music if it was not disabled by
  //  the intro/welcome dialog, and make sure we
  //  don't play music that's already going
  logger << "SetMusic type A called to use music resource " << music_resource <<
      endl;
  
  if(!disable_music && last_music != music_resource) {
    TryAndReport(MusicPlay(loop,music_resource, MIDI_RESOURCE_TYPE));
    last_music = music_resource;
  }

  if(GLUESTATE_GAME == state) {
    SetSpeed(GetSpeed());
  } else {
    SetSpeed(1);
  }
  
  logger << "SetMusic finished" << endl;
}

void GluSetMusic(bool loop, WORD music_resource) {
  logger << "SetMusic type B called to use music resource " <<
      (int)music_resource << endl;

  if(!disable_music) {
    TryAndReport(MusicPlay(loop, MAKEINTRESOURCE(music_resource),
                           MIDI_RESOURCE_TYPE));
    last_music = "";
  }

  SetSpeed(1);

  logger << "SetMusic finished." << endl;
}

static int NextSoundSlot() {
  static int next_slot = 0;
  const int NEW_SLOT = next_slot++;

  if (next_slot >= MAX_SHORT_SOUNDS) {next_slot = 0;}

  if (playing[NEW_SLOT]) {
    DWORD sound_status;
    if (SUCCEEDED(playing[NEW_SLOT]->GetStatus(&sound_status))
	&& (0 != (DSBSTATUS_PLAYING & sound_status))) {
      // we must use one of the long slots
      static int next_long_slot = MAX_SHORT_SOUNDS;
      const int NEW_LONG_SLOT = next_long_slot++;

      if (next_long_slot >= MAX_SOUNDS) {
	next_long_slot = MAX_SHORT_SOUNDS;
      }

      if (NULL != playing[NEW_LONG_SLOT]) {
	playing[NEW_LONG_SLOT]->Release();
      }

      playing[NEW_LONG_SLOT] = playing[NEW_SLOT];
    } else {
      playing[NEW_SLOT]->Release();
    }

    playing[NEW_SLOT] = 0;
  }

  return NEW_SLOT;
}

void GluPlaySound(int i,FIXEDNUM freq_factor, bool reverse) {
  // will play a sound and changes its frequency based on freq_factor
  LPDIRECTSOUNDBUFFER b2; // the duplicate of the sound
  const int MY_SLOT = NextSoundSlot();

  // make sure we ever loaded this sound
  if(NULL == sounds[i]) {return;}

  freq_factor = FixedMul(freq_factor,sfxfreq);

  if(SUCCEEDED(ds->DuplicateSoundBuffer(sounds[i], &b2))) {
    DWORD old_freq;

    if(SUCCEEDED(b2->GetFrequency(&old_freq))) {
      DWORD new_freq = FixedCnvFrom<long>(old_freq * freq_factor);

      // set our new frequency
      b2->SetFrequency(new_freq);
    }

    if(reverse != reversed.test(i)) {
      // reverse the contents of the sound buffer in order to play it
      //  backwards
      void *ptr1, *ptr2;
      DWORD ptr1_size, ptr2_size;
      DSBCAPS buffer;
      memset(&buffer, 0, sizeof(buffer));
      buffer.dwSize = sizeof(buffer);
      b2->GetCaps(&buffer);

      if(SUCCEEDED(b2->Lock(0, buffer.dwBufferBytes,
			    &ptr1, &ptr1_size,
			    &ptr2, &ptr2_size, 0))) {
	BYTE *p1, *p1_end, *p2, *p2_start;

	reversed.flip(i);

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
	      
	if(NULL == ptr1) {
	  p1 = (BYTE *)1;
	  p1_end = NULL;
	} else {
	  p1 = (BYTE *)ptr1;
	  p1_end = ((BYTE *)ptr1) + ptr1_size - 1;
	}

	// get a pointer to the last byte in pointer two

	if(NULL == ptr2) {
	  p2_start = (BYTE *)1;
	  p2 = (BYTE *)NULL;
	} else {
	  p2  = ((BYTE *)ptr2) + ptr2_size - 1;
	  p2_start  = (BYTE *)ptr2;
	}

	// step one is very similar in both methods
	while(p2 >= p2_start && p1 <= p1_end) {
	  swap (*p2, *p1);
	  p1++;
	  p2--;
	}

	// now branch off depending on size of ptr1 compared to ptr2
	if(ptr1_size < ptr2_size) {
	  while(p2_start < p2) {
	    swap(*p2_start, *p2);
	    p2--;
	    p2_start++;
	  }
	} else {
	  while(p1_end > p1) {
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

    playing[MY_SLOT] = b2;
  }
}

void GluDisableMusic() {disable_music = true;}

void GluStopMusic(){
  // stops whatever music has been playing
  MusicStop();
  // the last music playing was . . . nothing!
  last_music = "";
}

void GluPostForcePickupMessage() {
  static bool shown_message_already = false;

  // only display the "hold p" message if
  //  we have never showed it before,
  //  no other message is showing, the hero
  //  cares about the score, and we are not
  //  in multiplayer
  if
    (
     false == shown_message_already
     &&
     frames_for_current_message > FRAMESTODISPLAYMSG
     )
    {
      string msg;
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

HWND GluMain() {
  logger << "Calling Introduction() to display intro screen" << endl;

  Introduction();

  logger << "Starting Menu()" << endl;
  
  while(Menu()) {
    logger << "Menu() terminated" << endl;
    Game();
    logger << "Game() terminated; Starting Menu()" << endl;
  }

  logger << "Menu() returned false" << endl;

  return hWnd;
}

static void FlushKeyPresses() {
  while (!key_presses.empty()) {
    key_presses.pop();
  }
}

bool GluCanQuit() {return bool(GLUESTATE_UNINITIALIZED == state);}

void GluPlayLevelMusic() {
  string music_res;
  GluStrLoad(IDS_LEVELFILE1+level*2,music_res);
  GluSetMusic(true, music_res.c_str());
}

// returns true if the game should be started, false to quit
static bool Menu() {
  logger << "Now running Menu() function" << endl;
  while(true) {
      logger << "Right now we are at the main menu, where you can't press Escape" << endl;
      logger << "Calling MenuLoop until the user presses Enter and not Escape..." << endl;
      while(MENUACTION_ESCAPE == MenuLoop());
      logger << "The user pressed Enter, analyzing selection" << endl;

      switch(m->GetSelectionIndex()) {
      case MAINMENU_SP: {
        // do single-player game
      level_select:
        logger << "The user picked Single player" << endl;
        state = GLUESTATE_LEVELSELECT;
        logger << "PrepareMenu()'ing for level select" << endl;
        PrepareMenu();
        logger << "Done preparing menu, now waiting for user to press Enter or Escape" << endl;
        int menu_action;
			
        do {
          menu_action = MenuLoop();
          level = m->GetSelectionIndex();
        } while(MENUACTION_RETURN == menu_action && LEVELAVAIL_NONE == DeeLevelAvailability(level));

        if (MENUACTION_ESCAPE == menu_action) {
          logger << "User pressed Escape at level select, anyway.  PrepareMenu()'ing for main menu" << endl;
          state = GLUESTATE_MAINMENU;
          PrepareMenu();
          logger << "Done preparing.  About to enter main menu again" << endl;
          continue;
        }

        logger << "User selected level " << level <<
            ", with availability of " << DeeLevelAvailability(level) << endl;

        state = GLUESTATE_DIFFICULTYSELECT;
        logger << "PrepareMenu()'ing for difficulty select" << endl;
        PrepareMenu();

        if(MENUACTION_ESCAPE == MenuLoop()) {
          logger << "User pressed Escape at difficulty select" << endl;
          goto level_select;
        }

        state = GLUESTATE_GAME;
        level_loaded = LL_ALL;
        GLUdifficulty = m->GetSelectionIndex();

        logger << "User picked difficulty " << GLUdifficulty << endl;
        logger << "Menu() returning 'true'" << endl;
        return true;
      } case MAINMENU_MP:
	  // multiplayer game
      enter_name:
          state = GLUESTATE_ENTERNAME;
          if(0 == player_name.size()) {
            // the user has never entered a player_name before, so pick one
            GluStrLoad(IDS_CHARNAME1 + CHAR_TURNER,player_name);
          }
          PrepareMenu();

          // now entering player_name:
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

          if(MENUACTION_ESCAPE == MenuLoop()) {
            goto pick_character;
          }

          assert(m->GetSelectionIndex() < NetProtocolCount());

          NetInitializeProtocol(m->GetSelectionIndex());

      pick_game:
          state = GLUESTATE_PICKGAME;
          PrepareMenu();

          while(true) {
            if(MENUACTION_ESCAPE == MenuLoop()) {
              NetReleaseProtocol();
              goto select_connection_method;
            }

            WtrBeginScript(m->GetSelectionIndex());
            GfxFlipToGDISurface();

            while(FAILED(did->GetDeviceState(KBBUFFERSIZE,(void *)GLUkeyb))) {
              did->Acquire();
            }

            if((GLUkeyb[DIK_LSHIFT] & EIGHTHBIT) ||
               (GLUkeyb[DIK_RSHIFT] & EIGHTHBIT)) {
              logger << "try to host" << endl;
              try {
                NetCreateGame(m->GetSelectionIndex(), sync_rate,
                              WtrCurrentState(),
                              model, player_name.c_str(),
                              auto_ptr<NetFeedback>
                              (new NetGameBehavior()));
                state = GLUESTATE_LEVELSELECT;
                PrepareMenu();
                break;
              } catch (NetCreateFailure& ncf) { }
            } else {
              logger << "try to join" << endl;
              try {
                level = NetJoinGame(m->GetSelectionIndex(),
                                    model, player_name.c_str(),
                                    auto_ptr<NetFeedback>
                                    (new NetGameBehavior()));
                state = GLUESTATE_GAME;
                PrepareForMPGame();
                level_loaded = LL_ALL;
                return true;
              } catch (NetJoinFailure& njf) { }
            }
          }

          // now we can select a level
          while(true) {
            if(MENUACTION_ESCAPE == MenuLoop()) {
              NetLeaveGame();
              goto pick_game;
            }

            if(LEVELAVAIL_NONE != DeeLevelAvailability(m->GetSelectionIndex())) {
              break;
            }
          }

          state = GLUESTATE_GAME;
          level = m->GetSelectionIndex();
          PrepareForMPGame();
          level_loaded = LL_ALL;

          return true;
      case MAINMENU_QUIT:
	state =GLUESTATE_CONFIRMQUIT;
	PrepareMenu();

	while(MENUACTION_ESCAPE == MenuLoop());
			
	if(YES == m->GetSelectionIndex()) {
	  // the user wants to quit
	  GluRelease();
	  return false;
	} else {
	  // the user wants to stay longer
	  state = GLUESTATE_MAINMENU;
	  PrepareMenu();
	  continue;
	}
      } // end of switch(main_menu_selection)
  } // end of while(true) loop of Menu()
} // end of Menu()

static void Game() {
  logger << "Entering Game() loop" << endl;
  InitializeProfiler(NUM_PROFILES);
  while(true) {
    StartProfileFrame();
    BeginProfile(Main_Game_Loop);
    int i;

    // check for new input
    did->Acquire();
    if (FAILED(did->GetDeviceState(KBBUFFERSIZE, (void *)GLUkeyb))) {
      logger << "Can't get keyb data" << endl;
      memset(GLUkeyb, 0, KBBUFFERSIZE);
    }
	
    // STEP 0: RELOAD LEVEL IF APPROPRIATE or RUN THE END GAME
    if(GLUkeyb[DIK_RETURN] & EIGHTHBIT && !NetProtocolInitialized()) {
      logger << "User pressed Return, reloading level" << endl;
      level_loaded |= LL_FLESH;
    }
	
    if(LL_OKAY != level_loaded) {
      LoadLevel();
      logger << "LoadLevel finished" << endl;
    } else if(NUM_LEVELS <= level) {
      logger << "NUM_LEVELS <= level, the ending sequence will be shown" << endl;
      EndGame();
      break;
    }
	
    // STEP 2: CHECK FOR QUITTER
    if(GLUkeyb[DIK_ESCAPE] & EIGHTHBIT) {
      logger << "User pressed escape, quitting game" << endl;
      break;
    }

    // check for pauser
    if(!key_presses.empty() && PAUSE_KEY == key_presses.front()) {
      if(!NetInGame()) {
        logger << "User pressed pause" << endl;
	
        // play the sound for pausing the game, using the original
        // sound buffer 
        LPDIRECTSOUNDBUFFER s = sounds[WAV_PAUSE];

        if(s && !hero.Dead()) {
          s->Play(0,0,0);
          CTimer::Wait(0.10);
          s->SetCurrentPosition(0);
          CTimer::Wait(0.20);
          s->SetCurrentPosition(0);
          CTimer::Wait(0.95);
        } else {
          CTimer::Wait(0.95+0.20+0.10);
        }

        // copy front buffer to back
        GfxBackBuffer()->BltFast(0, 0, GfxFrontBuffer(), 0, DDBLTFAST_WAIT);

        // show health
        BeginProfile(Draw_Meters);
        hero.DrawMeters(SHOWHEALTH_YES);
        EndProfile();

        // put the "paused" text on the back buffer
        // load the string we'll need
        string paused;
        GluStrLoad(IDS_PAUSE,paused);

        // put the text down
        {
          GfxLock lock(GfxLock::Back());
          WriteString
            (lock((GAME_MODEWIDTH-(FONTWIDTH+1)*paused.length())/2,
                  (GAME_MODEHEIGHT-FONTHEIGHT)/2),
             lock.Pitch(), paused.c_str(),
             msg_and_score_color, msg_and_score_color);
        }

        // now flip between the two surfaces
        FlushKeyPresses();
        while(true) {
          Flip();
          CTimer::Wait(SECONDS_BETWEEN_PAUSE_FLIPS);
          if(!key_presses.empty()) {
            if(PAUSE_KEY == key_presses.front()) {
              key_presses.pop();
              break;
            } else {
              key_presses.pop();
            }
          }
          FlushKeyPresses();
          logger << "User has resumed game" << endl;
        }
      }             
    }
          
    // STEP 4: LOGIC POWERUPS
    CPowerUp::Rotate();

    // powerup logic is only used to see if regeneration is
    //  necessary.  There is not regeneration in a single-player game
    for(vector<CPowerUp>::iterator iterate = GLUpowerups.begin();
        NetInGame() && iterate != GLUpowerups.end();
        (*iterate++).Logic()) {}

    // STEP 5: DRAW EVERYTHING, INCLUDING POSTED MESSAGE, AND FLIP

    // draw lower-level compact map
    // figure center of screen coordinates

    hero.GetLocation(GLUcenter_screen_x,GLUcenter_screen_y);
    if(GLUcenter_screen_x < Fixed(GAME_MODEWIDTH/2)) {
      GLUcenter_screen_x = Fixed(GAME_MODEWIDTH/2);
    } else if(GLUcenter_screen_x > max_center_screen_x) {
      GLUcenter_screen_x = max_center_screen_x;
    }
    if(GLUcenter_screen_y < Fixed(GAME_PORTHEIGHT/2)) {
      GLUcenter_screen_y = Fixed(GAME_PORTHEIGHT/2);
    } else if(GLUcenter_screen_y > max_center_screen_y) {
      GLUcenter_screen_y = max_center_screen_y;
    }

    int level_blit_x = GAME_MODEWIDTH/2
      - FixedCnvFrom<long>(GLUcenter_screen_x);
    int level_blit_y = GAME_PORTHEIGHT/2
      - FixedCnvFrom<long>(GLUcenter_screen_y);
    int column1 = max(0,abs(level_blit_x)/SECTOR_WIDTH);
    int row1 = max(0,abs(level_blit_y)/SECTOR_HEIGHT);
    int column3 = min(column1+3, sector_width);
    int row3 = min(row1+3, sector_height);
    int r, c;

    column1 = column3 - 3;
    row1 = row3 - 3;

    level_blit_x += column1 * SECTOR_WIDTH;
    level_blit_y += row1    * SECTOR_HEIGHT;

    BeginProfile(Recaching);
    int reload;
    if(-1 == ul_cached_sector_x || -1 == ul_cached_sector_y) {
      // we need to recache everything
      reload = CACHE_EVERYTHING;
    } else {
      reload = 0;
      if(ul_cached_sector_x < column1) {
        reload |= CACHE_RIGHTCOLUMN;
        upper_left_sector++;
      } else if(ul_cached_sector_x > column1) {
        reload |= CACHE_LEFTCOLUMN;
        upper_left_sector = (0 == upper_left_sector)
          ? CACHED_SECTORS - 1 : upper_left_sector - 1;
      }

      if(ul_cached_sector_y < row1) {
        reload |= CACHE_BOTTOMROW;
        upper_left_sector += CACHED_SECTORS_WIDE;
      } else if(ul_cached_sector_y > row1) {
        reload |= CACHE_TOPROW;
        upper_left_sector = (upper_left_sector < CACHED_SECTORS_WIDE)
          ? upper_left_sector + CACHED_SECTORS - CACHED_SECTORS_WIDE
          : upper_left_sector - CACHED_SECTORS_WIDE;
      }
    }

    ul_cached_sector_y = row1;
    ul_cached_sector_x = column1;

    Recache(reload);

    upper_left_sector %= CACHED_SECTORS;
		
    EndProfile(); // recaching

    // draw lower-level CMP
    //  all of its calculations (or most of it, whatever) will
    //  be used later when the upper-level cmp set is drawn
    int target_x, target_y;
    target_y = level_blit_y;
    BeginProfile(L_Cmp_Drawing);
    for(r = row1; r < row3; r++) {
      target_x = level_blit_x;
      for(c = column1; c < column3; c++) {
        GfxPut(cached[upper_left_sector++ % CACHED_SECTORS].first,
               target_x, target_y, false);

        if(LL_ALL != level_loaded) {
          // see if we collided with the end of a level in this sector
          // CHECK FOR WINNER
          for(list<CLevelEnd>::const_iterator iterate = sectors[r * sector_width + c].levelEnds.begin();
              iterate != sectors[r * sector_width + c].levelEnds.end();
              iterate++) {
            if(iterate->Collides(hero.X(),
                                 hero.Y())) {
              GluSetMusic(false, IDR_YOUWINTUNE);

              if (!disable_music) {
                CTimer::Wait(YOUWINTUNE_LENGTH);
              } 

              int next_level = iterate->Reference();
				
              // update level availability variable
              DeeLevelComplete(level, GLUdifficulty,
                               since_start, atoi(score.c_str()),
                               next_level);
	
              level = next_level;

              if(NUM_LEVELS > next_level) {
                level_loaded = LL_ALL;
              }
              break;
            } // end if collides with level end
          } // end for level ends in this sector
        }
        
        target_x += SECTOR_WIDTH;
      } // end for column
      target_y += SECTOR_HEIGHT;
    } // end for row
    EndProfile(); // lower-level-cmp drawing
	
    // create a drawing order for this frame
    multiset<TCharacterPointer> drawing_order;

    auto_ptr<GfxLock> lock(new GfxLock(GfxLock::Back()));

    // allow the hero to try moving
    //  with respect to black lines
    // allow hero to do logic
    BeginProfile(Hero_Logic);
    hero.Logic();
    drawing_order.insert(TCharacterPointer(&hero));
    EndProfile();

    // drive the enemies based on AI or network messages
    BeginProfile(Enemy_Logic);
    
    if (NetInGame()) {
      NetLogic();

      for (vector<CCharacter>::iterator i = enemies.begin();
           i != enemies.end(); i++) {
        drawing_order.insert(TCharacterPointer(&(*i)));
      }
    } else {
      for(r = row1; r < row3; r++) {
        for(c = column1; c < column3; c++) {
          set<int>::const_iterator iterate;
          set<int> *const vctr = &sectors[r * sector_width + c].enemies;
	    
          for(iterate = vctr->begin(); iterate != vctr->end(); iterate++) {
            if (enemies[*iterate].Logic(hero)) {
              drawing_order.insert(TCharacterPointer(&enemies[*iterate]));
            }
          }
        }
      }
    }
    
    EndProfile();

    // now we can logic with the projectiles
    for(i = 0; i < MAX_FIRES; i++) {
      fires[i].Logic();
    }
	
    lock.reset();
	
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
    BeginProfile(Draw_Character);
    for(multiset<TCharacterPointer>::iterator i = drawing_order.begin();
        i != drawing_order.end(); i++) {
      if(i->ch->DrawCharacter()
         && i->ch != &hero && !NetInGame()) {
        // need to update sectors
        // the character may have left the current sector
        //  so remove the enemy from one sector and put it
        //  in another
        int character_row, character_column;
        i->ch->GetSector(character_row,character_column);
        int index = i->ch - &enemies[0];
        sectors[character_row * sector_width + character_column].enemies.erase(index);
        i->ch->CalculateSector(character_row, character_column);
        sectors[character_row * sector_width + character_column].enemies.insert(index);
      }
    }
    EndProfile();

    BeginProfile(Draw_Things);

    // draw power ups
    //  draw ones of sectors
    for(r = row1; r < row3; r++) {
      for(c = column1; c < column3; c++) {
        set<int>::const_iterator iterate;
        set<int> *const powerups = &sectors[r * sector_width + c].powerups;
	  
        for(iterate = powerups->begin(); iterate != powerups->end(); iterate++) {
          GLUpowerups[*iterate].Draw();
        }
      }
    }

    // draw extra ones
    for(vector<CPowerUp>::iterator iterate
          = GLUpowerups.begin() + std_powerups;
        iterate < GLUpowerups.end(); iterate++) {
      iterate->Draw();
    }

    // draw bullets
    for(int bullet_i = 0; bullet_i < MAX_FIRES; bullet_i++) {
      fires[bullet_i].Draw();
    }
    EndProfile(); // powerup and bullets drawing

    // draw upper-level bitmap if outdoors
    BeginProfile(U_Cmp_Drawing_N_Weather);
    if(!GluWalkingData(hero.X(),hero.Y()))
      {
        target_y = level_blit_y;
        for(r = row1; r < row3; r++) {
          target_x = level_blit_x;
          for(c = column1;c < column3; c++) {
            TSector *const current_sector = &sectors[r * sector_width + c];
            if(current_sector->upperCell->NoStep2()) {
              //  just draw step one normally
              current_sector->upperCell->RenderStep1
                (GfxBackBuffer(), target_x, target_y,
                 GfxSimpleClipperRect());
            } else {
              // this cell is complex!  draw the uncompressed bitmap form
              GfxPut(cached[(upper_left_sector + (r - row1)
                             * CACHED_SECTORS_WIDE + (c - column1))
                            % CACHED_SECTORS].second, target_x, target_y);
            }
            target_x += SECTOR_WIDTH;
          }
          target_y += SECTOR_HEIGHT;
        }

        // keep back buffer locked for WriteInfo and WtrOneFrame
        GfxLock weatherAndMsgLock(GfxLock::Back());

        BeginProfile(Weather_One_Frame);
        PlayMusicAccordingly(WtrOneFrame(GLUcenter_screen_x,
                                         GLUcenter_screen_y));
        WriteInfo();
        EndProfile();
      } else {
        BeginProfile(Weather_One_Frame);
        PlayMusicAccordingly(WtrOneFrame());
        EndProfile();

        WriteInfo();
      }

    if ((!NetInGame() || NetIsHost())
        && WtrPermitStateChange()) {
      NetChangeWeather(WtrCurrentState());
    }
      
    EndProfile(); // upper cmp drawing and weather

    // draw meters of health and ammo
    BeginProfile(Draw_Meters);
    hero.DrawMeters((GLUkeyb[DIK_H] & EIGHTHBIT)
                    ? SHOWHEALTH_YES : SHOWHEALTH_IFHURT);
    EndProfile();

    EndProfile(); // main game loop

    Flip();
  }

  logger << "Main game loop has terminated" << endl;

  // cleanup after the game
  if(NetProtocolInitialized()) {
    if(NetInGame()) {
      NetLeaveGame();
    }
    NetReleaseProtocol();
  }

  GamInitializeWithMenuPalette();
  GfxRefillSurfaces();
  state = GLUESTATE_MAINMENU;

  FlushKeyPresses();

  PrepareMenu();
			
  WtrEndScript();
  GluSetMusic(true, IDR_MENUMUSIC);
}

static void GetLevelTimerMinSec(int &min, int &sec, int &hund) {
  // get how many second we have been playing from the CGlue timer that
  //  was restarted when the level was loaded
  sec = FixedCnvFrom<long>(since_start);
  min = sec / SECONDSPERMINUTE;
  sec %= SECONDSPERMINUTE;
  hund = FixedCnvFrom<unsigned long>(since_start * 100) % 100;
}

static void WriteChar(BYTE *surface, int pitch,
                      int c, int color, int back_color) {
  // find the offset to the right
  // character
  BYTE *d = font_data + (c - FIRST_FONTCHAR) * 16;
  pitch -= FONTWIDTH;

  if (c >= FIRST_FONTCHAR && c <= LAST_FONTCHAR) {
    for(int i = 0; i < FONTHEIGHT; i++) {
      BYTE row=*d++; // get the data for the current row
      for(int j = 0; j < FONTWIDTH; j++) {
        if(row & 0x80) {
          // draw the current pixel
          *surface = color;
        } else if(back_color != color) {
          *surface = back_color;
        }

        surface++;
        row<<=1;
      }
      surface += pitch;
    }
  }
}

static void WriteString(BYTE *surface, int pitch,
                        const char *string, int color, int back_color)
{
  while(*string) {
    WriteChar(surface, pitch, *((BYTE *)string), color, back_color);
    surface += FONTWIDTH+1;
    string++; // go to next character in the string
  }
}

void GluFindTextColors() {
  msg_and_score_color = GfxGetPaletteEntry(RGB(0xff, 0xff, 0xff));
  score_flash_color_1 = GfxGetPaletteEntry(RGB(0xff, 0, 0));
  score_flash_color_2 = GfxGetPaletteEntry(RGB(0xff, 0xff, 0));
}

static void WriteInfo() {
  BeginProfile(Draw_Extra_Stats_N_Msgs);

  GfxLock lock(GfxLock::Back());

  if(score_print_x > 0) {
    // write score's shadow
    WriteString(lock(score_print_x-1, (SCORE_AND_TIMER_Y-1)),
                lock.Pitch(), score.c_str(), 0, 0);

    // write score
    WriteString(lock(score_print_x, SCORE_AND_TIMER_Y),
                lock.Pitch(), score.c_str(),
                msg_and_score_color, msg_and_score_color);
  } else {
    // score_print_x is negative, which means we should be printing it
    //  in flashing colors also, the last character in score string
    //  contains countdown to stop flashing

    // print score in flashing red/yellow
    int color = frames_for_current_message & 1
      ? score_flash_color_1 : score_flash_color_2;

    // call this function to put the score on the back buffer
    WriteString(lock(-score_print_x, SCORE_AND_TIMER_Y),
                lock.Pitch(), score.c_str(), color, color);

    // countdown to stop flashing
    if(FRAMESTODISPLAYMSG == frames_for_current_message) {
      score_print_x = -score_print_x;
    }
  }

  // this array will contain the text buffer which holds the time
  char time[MAX_TIMERCHAR];

  int minutes, seconds, hund;

  // call a member function that will fill these two
  //  integers with the numbers to be displayed in the timer
  GetLevelTimerMinSec(minutes, seconds, hund);
	
  sprintf(time, "%02d:%02d.%02d", minutes, seconds, hund);

  // write the timer's shadow
  WriteString(lock(1, SCORE_AND_TIMER_Y-1),
              lock.Pitch(), time, 0, 0);

  // write the timer itself
  WriteString(lock(0, SCORE_AND_TIMER_Y),
              lock.Pitch(), time,
              msg_and_score_color, msg_and_score_color);

  // print current top-of-screen message
  if(++frames_for_current_message < FRAMESTODISPLAYMSG) {
    // print shadow
    WriteString(lock(msg_x+1, 1),
                lock.Pitch(), message.c_str(),
                0, 0);

    // print message itself
    WriteString(lock(msg_x, 0),
                lock.Pitch(), message.c_str(),
                msg_and_score_color, msg_and_score_color);
  }

  EndProfile(); // extra stats and messages
}

static void Recache(int flags) {
  if(0 == flags) {
    return; // already cached everything
  }
  
  int bit = 1, cached_sector = upper_left_sector;
  
  for(int y = 0; y < CACHED_SECTORS_HIGH; y++) {
    // find the sector coordinates at this height
    int sector_y = ul_cached_sector_y + y;
    if(sector_y >= sector_height) {
      return; // all done, we are out of range
    }
    
    for(int x = 0; x < CACHED_SECTORS_WIDE; x++, bit <<= 1) {
      if(bit & flags) {
	// we have to recache the map at x,y
	int sector_x = ul_cached_sector_x + x;

	logger << "Reload sector " << sector_x << "x" << sector_y << endl;

	// now make sure this fits into the sector grid of the whole level
	if(sector_x < sector_width) {
	  logger << "Sector coor is in range, loading" << endl;
	  
	  cached_sector %= CACHED_SECTORS;
	  surf_t targetA = cached[cached_sector].first;
	  surf_t targetB = cached[cached_sector].second;
	  CCompactMap *sourceA
            = sectors[sector_y * sector_width + sector_x].lowerCell;
	  CCompactMap *sourceB
            = sectors[sector_y * sector_width + sector_x].upperCell;

          GfxChangeSurfaceFiller(targetA, sourceA->Filler());
          GfxChangeSurfaceFiller(targetB, sourceB->Filler());
	}

	// all done rendering to surface
	logger << "Reloaded sector " << sector_x << "x" << sector_y << endl;
				
	flags &= ~bit; // turn off the bit, like on a checklist
	if(0 == flags) {
	  logger << "Recache finished" << endl;
	  return;
	}
      }
      cached_sector++;
    }
  }

  logger << "Recache finished" << endl;
}

int GluScoreDiffPickup(int x) {return (2 == x) ? 2 : 1;}
int GluScoreDiffKill(int x) {return x * 2 + 2;}

static void PlayMusicAccordingly(int state_change_indicator) {
  switch (state_change_indicator) {
  case WTROF_TURNMUSICON:
    GluPlayLevelMusic();
    break;
  case WTROF_TURNMUSICOFF:
    GluStopMusic();
  }
}

static void FillAccomplishmentLines() {
  if (DeeHasBeatenLevel(level, GLUdifficulty)) {
    char buffer[MAX_STRINGLEN];
    pair<FIXEDNUM, int> best_time_and_score
      = DeeGetBestTimeAndScore(level, GLUdifficulty);
    FIXEDNUM timeAtBestScore
      = DeeGetTimeAtBestScore(level, GLUdifficulty);

    GluStrLoad(IDS_RECORDSUMMARY,
               accomplishment_lines[DEEDS_SUMMARY]);
    
    GluStrLoad(IDS_BESTTIMEFORMAT,
               accomplishment_lines[DEEDS_BESTTIME]);
    
    GluStrLoad(IDS_BESTSCOREFORMAT,
               accomplishment_lines[DEEDS_BESTSCORE]);

    sprintf(buffer, accomplishment_lines[DEEDS_SUMMARY].c_str(),
            best_time_and_score.second,
            best_time_and_score.first/SECONDSPERMINUTE,
            best_time_and_score.first%SECONDSPERMINUTE);
    accomplishment_lines[DEEDS_SUMMARY] = buffer;

    sprintf(buffer, accomplishment_lines[DEEDS_BESTTIME].c_str(),
            best_time_and_score.first/SECONDSPERMINUTE,
            best_time_and_score.first%SECONDSPERMINUTE,
            DeeGetScoreAtBestTime(level, GLUdifficulty));
    accomplishment_lines[DEEDS_BESTTIME] = buffer;

    sprintf(buffer, accomplishment_lines[DEEDS_BESTSCORE].c_str(),
            best_time_and_score.second,
            timeAtBestScore/SECONDSPERMINUTE,
            timeAtBestScore%SECONDSPERMINUTE);
    accomplishment_lines[DEEDS_BESTSCORE] = buffer;
  } else {
    for (int i = 0; i < DEEDS_LINECOUNT; i++) {
      GluStrLoad(IDS_NORECORDS, accomplishment_lines[i]);
    }
  }
}

void GluDraw(unsigned int bmp, int x, int y) {
  GfxPut(bitmaps[bmp], x, y);
}

void GluDrawScale(unsigned int bmp, RECT *target) {
  GfxAttachScalingClipper();
  GfxPutScale(bitmaps[bmp], target);
  GfxDetachScalingClipper();
}

