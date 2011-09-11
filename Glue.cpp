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
#include "Gfx.h"
#include "GfxBasic.h"
#include "GfxPretty.h"
#include "CompactMap.h"
#include "Fixed.h"
#include "Logger.h"
#include "Profiler.h"
#include "MusicLib.h"
#include "Timer.h"
#include "Menu.h"
#include "RawLoad.h"
#include "Weather.h"
#include "Fire.h"
#include "Character.h"
#include "Glue.h"
#include "Deeds.h"
#include "LevelEnd.h"
#include "PowerUp.h"
#include "Net.h"
#include "GammaEffects.h"
#include "BitMatrix.h"
#include "Resource.h"
#include "Keyboard.h"
#include "Difficulty.h"
#include "Sound.h"

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
using std::vector;

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

 // use slightly different collision detection in ufo level
const int ALWAYSPUSH = ~(1 << 9);

const int MAX_STRINGLEN = 150;
const int LONG_STRINGLEN = 500;
const int LEVEL_NONE = -1;
const int POINTSFORSUICIDE = -1;
const int POINTSFORHOMICIDE = 1;
const int MAXSCORE = 9999;
const int MINSCORE = -9999;
const DWORD MINFRAMESTOKEEPMESSAGE = 20;
const DWORD FRAMESTODISPLAYMSG = 120;

 // coordinates of the character to display durring selection
const int DEMOCHAR_X = 288;
const int DEMOCHAR_Y = 165;

const float DEMOCHAR_SECSTOCHANGEDIR = 1.0f;
const float DEMOCHAR_SECSTOSTEP = 0.1f;
const int DEMOCHAR_X2 = 288; // coordinates to display after selection
const int DEMOCHAR_Y2 = 125;

const int MENUACTION_ESCAPE = 0;
const int MENUACTION_RETURN = 1;
const int SCORE_X_OFFSET = -10;

const unsigned short AMMOCAPACITY[] = {130, 500, 6};
const unsigned short AMMOSTARTING[] = {25, 0, 0};

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
  GLUESTATE_PICKGAME,
  
  GLUESTATE_PICKVIDEOMODE,
  GLUESTATE_PRESSESCAPEINSTRUCTIONS,
  GLUESTATE_CANSEENORMALLY
};

const char LEVEL_NAMES[][40] =
  {"The End of the Beginning",
   "Out of the Office and into the Fire",
   "Out of the Office and Viva Zepellin!",
   "What goes up must come SPLAT! Excuse me",
   "Super Mario Super Fun Zone",
   "Switzerland",
   "Gibs Rum",
   "¡Taco Taco!",
   "Border Hopping and Famous Landmarks",
   "Probe-mania",
   "Welcome to Hawaii and Die",
   "Tijuana: the Happiest Place on Earth"};

const char LEVEL_IDS[][3] =
  {"1_",
   "2A", "2B",
   "3A", "3B",
   "4_",
   "5_",
   "6A",
   "7A",
         "6B",
         "7B",
   "8A"};

const char *MENU_LEVELSELECTHEADER = "PICK A LEVEL";

const char *MENU_MAINMENU =
"ANDRADION 2\n"
"Single Player\n"
"Multiplayer\n"
"Video Options\n"
"Quit\n";

const char *MENU_PRESSESCAPEINSTRUCTIONS =
"PRESS ESC IF THE SCREEN GOES BLACK\n"
"Continue\n"
"Cancel\n";

const char *MENU_PICKVIDEOMODE =
"PICK NEW VIDEO MODE (currently #~)\n"
"1. Fast - 43Hz (for some flatscreens)\n"
"2. Fast normal\n"
"3. Pretty (modern CPUs only)\n";

const char *MENU_CANSEENORMALLY =
"CAN YOU SEE THE SCREEN NORMALLY?\n"
"No\n"
"Yes\n";

const char *MENU_CONFIRMATION =
"ARE YOU SURE?\n"
"No\n"
"Yes\n";

const char *MENU_GAMESLOT =
"PICK ROOM (holding down shift to host)\n"
"Happiness Village\n"
"Insurance Ranch\n"
"Summer School\n"
"Detour Den\n"
"DMV\n"
"Middle School\n"
"Gibbs' Class\n"
"Turnabout Shack\n"
"Joe Henderson\n"
"Robert Semple\n"
"Matthew Turner\n"
"Mary Farmer\n";

const char *MENU_PICKCHARACTER =
"PICK CHARACTER\n"
"Sally\n"
"Milton\n"
"Evil Mr. Turner\n"
"Mr. Turner\n"
"The Switz\n"
"Charmin\n"
"Pepsi One\n"
"Coca-Cola Classic\n"
"Kool-Aid Guy\n";

const char *UNAVAILABLE_LEVEL = "(Unavailable - keep playing!)";

const char *MAXSCORE_MESSAGE = "You got max score!";

enum {GLUEHSM_NOTHING, GLUEHSM_NEWPLAYER, GLUEHSM_SESSIONLOST};
enum {MAINMENU_SP, MAINMENU_MP, MAINMENU_VIDEOOPTIONS, MAINMENU_QUIT,
      NUM_MAINMENUITEMS};
enum {ESCAPEINSTRUCTIONS_CONTINUE, ESCAPEINSTRUCTIONS_CANCEL,
      NUM_ESCAPEINSTRUCTIONSITEMS};
enum {CONFIRMATION_NO, CONFIRMATION_YES, NUM_CONFIRMATIONITEMS};
enum {RESOURCELOAD_NONE,
      RESOURCELOAD_SP,
      RESOURCELOAD_MP};

const COLORREF COLOR_HEADING = RGB(0,128,0);
const COLORREF COLOR_UNSELECTED = RGB(255,128,128);
const COLORREF COLOR_SELECTED = RGB(0,255,255);
const COLORREF COLOR_SHADOW = RGB(128,0,0);
const int SHADOW_OFFSET = 1;

 // difficulty level used for multiplayer sessions
const int MPDIFFICULTY = 2;

// number of game slots on multiplayer game selects
const int NUM_MPGAMESLOTS = 12; 

const int LOADINGMETER_MINHEIGHT = 9;
const int LOADINGMETER_MAXHEIGHT = 15;
const FIXEDNUM FREQFACTOR_OKGOTITBACKWARDS = Fixed(1.2f);
const FIXEDNUM FREQFACTOR_OKGOTITNORMAL = Fixed(0.80f);
const char *const MIDI_RESOURCE_TYPE = "MIDI";
const char *const CMP_RESOURCE_TYPE = "CMP";
const char *const ONE_NUMBER_FORMAT = "%d";
const char *const TWO_NUMBERS_FORMAT = "%d/%d";
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

const int FONTHEIGHT = 16;
const int FONTWIDTH = 8;
const int FIRST_FONTCHAR = '!';
const int LAST_FONTCHAR = '~';

const FIXEDNUM TIMER_INC_PER_FRAME = Fixed(0.04f);

const char *WINDOW_CAPTION = "Andradion 2";

const int SECTOR_WIDTH = 160;
const int SECTOR_HEIGHT = 100;

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

struct Sector {
  auto_ptr<CompactMap> upperCell, lowerCell;
  set<int> powerups, enemies;
  list<CLevelEnd> levelEnds;
};

typedef char Score[11];

template<class c,class f> inline c Range(c minimum, c maximum, f progress) {
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
static auto_ptr<Gfx::Font> font;

// x-coordinate of the current top-of-screen message
static int msg_x; 

// current message on the screen
static string message;

static HWND hWnd;
static HINSTANCE hInstance;

static int state = GLUESTATE_UNINITIALIZED;

static auto_ptr<CBitMatrix> walking_data; // on = inside, off = outside
static int width_in_tiles, height_in_tiles;

static Array<Sector> sectors;
static int sector_width, sector_height;
	
static auto_ptr<CMenu> m;
static list<CLevelEnd> lends;

static int model; // character we are playing as

static CTimer char_demo_stepper;
static CTimer char_demo_direction_changer;

// how long the current message has been up
static int frames_for_current_message; 

static string KILLEDYOURSELF;
static string KILLED;
static string YOU;
static string YOUKILLED;
static string KILLEDTHEMSELVES;
static string SPKILLED; // string displayed when killed in single player

static vector<POINT> possible_starting_spots;

static FIXEDNUM since_start;

// (used for mp) number of powerups there are when no backpacks are left
static int std_powerups; 

static int border_color;
static int led_color;
static int msg_and_score_color;
static int score_flash_color_1;
static int score_flash_color_2; 

// these statics make it easier to draw the compact maps faster
//  by caching them.  This way, the most that can be drawn each frame
//  is five, but usually none.  Those that are not "redrawn" are cached
//  into un-compressed DirectDraw surfaces
static pair<Gfx::Surface *, Gfx::Surface *> cached[CACHED_SECTORS];

// index of upper most-left most sector in the cached array
static int upper_left_sector; 

// the column and row of the upper left sector that is cached
static int ul_cached_sector_x, ul_cached_sector_y; 

// this array of strings contain data for the user to see
//  that concern his accomplishments
enum {DEEDS_SUMMARY, DEEDS_BESTTIME,
      DEEDS_BESTSCORE, DEEDS_LINECOUNT};
static string accomplishment_lines[DEEDS_LINECOUNT];

static queue<BYTE> key_presses;

static string player_name;
static int char_demo_direction;
static int level;

static auto_ptr<Gfx::Surface> bitmaps[BMP_MPCOUNT];
static int bitmaps_loaded = 0;

static vector<BYTE> character_sectors;

static float spf;
static DWORD fps;

static vector<CPowerUp> powerups;

static Array<Context> contexts;
static int current_context;

static int max_score;

static auto_ptr<Com> com;

static void Levels(vector<string>& target);
static void ShowMouseCursor();
static void HideMouseCursor();
static pair<const BYTE *, HGLOBAL> LoadLevelPaletteOnly();
static void LoadLevel(bool load_flesh, bool load_bone);
static int  MenuLoop();
static void PrepareMenu();
static void LoadBitmaps(int type);
static void Introduction();
static void EndGame();
static void Flip();
static void PrepareForMPGame();
static void GetLevelTimerMinSec(int *min, int *sec, int *hund);
static void Game();
static bool Menu();
static void FlushKeyPresses();
static void AddPossibleStartingSpot(FIXEDNUM x, FIXEDNUM y);
static void LoadCmps(int level_width, int level_height, bool skip_wd_resize); 
// draws to the front buffer the current score
static void WriteScorePretty(Score *score, BYTE color);

static void WriteMessageTimerAndScore(Context *cxt);

static void WriteString(int x, int y, const char *str, int color,
                        int shadow_color = -1);
static void Recache(int flags);
static void PlayMusicAccordingly(int state_change_indicator);
static void FillAccomplishmentLines();
static void FilterMovement(const POINT *start, POINT *end);
static void SetGfx();
static void ReleaseGfxElements();
static void ExtractByte(const BYTE **, int *);
static void ExtractWord(const BYTE **, int *);
static int CalculateSector(Character::Ptr ch);
static Character::Ptr AddCharacter();
static void ClearCharacters(bool keep_first);
static void AnalyzePalette();
static void CleanUpAfterGame();

class NetGameBehavior : public NetFeedback {
public:
  NetGameBehavior() : NetFeedback() {}

  virtual void SetWeatherState(unsigned int new_state) throw() {
    WtrSetState(new_state);
  }

  virtual void EnemyFiresBazooka(unsigned int index,
                                 unsigned short x_hit,
                                 unsigned short y_hit) throw() {
    Fire *new_fire = Fire::UnusedSlot();

    if (new_fire) {
      Character::Ptr ch(Character::Get(index+1));
      new_fire->Setup(ch->X(), ch->Y(),
                      FixedCnvTo<long>(x_hit), FixedCnvTo(y_hit));
    }
  }

  virtual void EnemyFiresPistol(unsigned int index) throw() {
    Fire *new_fire = Fire::UnusedSlot();
    
    if (new_fire) {
      Character::Ptr ch(Character::Get(index+1));
      new_fire->Setup(ch->X(), ch->Y(), ch->Direction(), WEAPON_PISTOL, true);
    }
  }

  virtual void EnemyFiresMachineGun(unsigned int index) throw() {
    Fire *new_fire = Fire::UnusedSlot();

    if (new_fire) {
      Character::Ptr ch(Character::Get(index+1));

      new_fire->Setup(ch->X(), ch->Y(),
                      ch->Direction(), WEAPON_MACHINEGUN, true);
    }
  }

  virtual void PickUpPowerUp(unsigned short index) throw() {
    if (index < powerups.size()) {
      vector<CPowerUp>::iterator p = powerups.begin() + index;
      if (p->Regenerates()) {
        p->PickUp();
      } else {
        *p = powerups[powerups.size() - 1];
        powerups.resize(powerups.size() - 1);
      }
    }
  }

  virtual void ClearEnemyArray() throw() {
    logger << "Call to clear enemy array" << endl;
    ClearCharacters(true);
  }

  virtual void CreateEnemy(unsigned int model) throw() {
    AddCharacter()->Setup(model);
  }

  virtual void SetEnemyWeapon(unsigned int index, unsigned int weapon)
    throw() {
    Character::Get(index+1)->SetWeapon(weapon);
  }

  virtual void SetEnemyPosition(unsigned int index,
                                unsigned short x,
                                unsigned short y) throw() {
    Character::Get(index+1)->SetPosition
      (FixedCnvTo<long>(x), FixedCnvTo<long>(y));
  }

  virtual void SetEnemyDirection(unsigned int index,
                                 unsigned int direction) throw() {
    Character::Get(index+1)->SetDirection(direction);
  }

  virtual void WalkEnemy(unsigned int index) throw() {
    Character::Ptr ch(Character::Get(index+1));
    
    ch->Walk(false);
    ch->TryToMove();
  }

  virtual void KillEnemy(unsigned int index, WORD *ammo) throw() {
    vector<CPowerUp>::iterator p;
    FIXEDNUM got_data[WEAPON_COUNT];
    Character::Ptr ch(Character::Get(index+1));

    powerups.resize(powerups.size() + 1);
    p = powerups.end() - 1;

    for (int i = 0; i < WEAPON_COUNT; i++) {
      got_data[i] = (FIXEDNUM)ammo[i];
    }

    p->Setup(ch->X(), ch->Y(), got_data);
  }

  virtual void HurtEnemy(unsigned int index) throw() {
    Character::Get(index+1)->Hurt();
  }

  virtual void HurtHero(unsigned int weapon_type) throw() {
    Character::Get(0)->SubtractHealth(weapon_type);
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

// dialog proc.  This is a callback function that
//  sends messages to the code entity that defines
//  how the intro box works.  This box's controls
//  and the locations of them are defined in the
//  resource
static BOOL CALLBACK CfgDlgProc(HWND hwndDlg, UINT uMsg,
                                WPARAM wParam, LPARAM) {
  switch(uMsg) {
  case WM_COMMAND: {
    // get the identifier of the control, which is always passed
    //  through the lower word of the wParam parameter
    WORD ctrl = LOWORD(wParam);
    if(IDLAUNCH == ctrl || IDQUIT == ctrl || IDLAUNCHWITHOUTMUSIC == ctrl) {
      // the launch or quit or launch w/o music btn was pressed 
      char txt[MAX_STRINGLEN];
      int sync_rate;
        
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
      if(sync_rate < MIN_SYNCRATE || sync_rate > MAX_SYNCRATE) {
        // invalid sync rate was entered
        //  display error
        // load two strings that make up the message box of the error
        string error_msg;
        GluStrLoad(IDS_INVALIDSYNCRATE, error_msg);

        MessageBox(hwndDlg, error_msg.c_str(),
                   WINDOW_CAPTION, MB_ICONSTOP);
      } else {
        DeeSetSyncRate(sync_rate);

        // a valid sync rate was entered
        if(IDLAUNCHWITHOUTMUSIC == ctrl) {
          MusicStop();
          disable_music = true;
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
			
    char number_buffer[MAX_STRINGLEN];
    itoa(DeeSyncRate(), number_buffer, 10);

    // set the text in the sync rate edit box to what we just got
    //  from the sprintf function
    SetDlgItemText(hwndDlg,IDC_SYNC,number_buffer);
    return FALSE;
  }

  // we want the currently-in-focus control to be chosen automatically
  case WM_INITDIALOG: return TRUE; 

  default: return FALSE;
  }
}

static int GetSpeed() {return fps / 25;}

static void SetSpeed(int index) {
  logger << "SetSpeed(" << index << ") called" << endl;

  switch(NetInGame() ? 1 : index) {
  case 0:
     // slow the game down
     spf = 0.08f; // more seconds per frame
     fps = 12; // fewer frames per second
     SndSetPlaybackFrequency(Fixed(0.5f));
     SetTempo(DefaultTempo()/2.0f);
     WtrSetSoundPlaybackFrequency(FixedCnvFrom<long>(Fixed(0.5f)
                                                     * SOUNDRESOURCEFREQ));
     break;
  case 1:
     // normal game speed
     spf = 0.04f;
     fps = 25;
     SndSetPlaybackFrequency(Fixed(1.0f));
     SetTempo(DefaultTempo());
     WtrSetSoundPlaybackFrequency(FixedCnvFrom<long>(Fixed(1.0f)
                                                     * SOUNDRESOURCEFREQ));
     break;
  case 2:
     // speed the game up
     spf = 0.02f;
     fps = 50;
     SndSetPlaybackFrequency(Fixed(2.0f));
     SetTempo(DefaultTempo()*2.0f);
     WtrSetSoundPlaybackFrequency(FixedCnvFrom<long>(Fixed(2.0f)
                                                     * SOUNDRESOURCEFREQ));
  }

  logger << "SetSpeed() returning" << endl;
}

class StarFiller : public Gfx::SurfaceFiller {
public:
  virtual void Fill(BYTE *starsb, int p,
                    Gfx::Surface *surf) throw() {
    logger << "Filling star surface" << endl;
    
    BYTE *clearing_point = starsb;
    for (int y = 0; y < surf->GetHeight(); y++) {
      memset(clearing_point, 0, surf->GetWidth());
      clearing_point += p;
    }
    
    for (int i = 0; i < NUMSTARS; i++) {
      // plot a bunch of stars
      // make a small plus sign for each star

      int x = (rand()%(surf->GetWidth()-2))+1;
      int y = (rand()%(surf->GetHeight()-2))+1;
      BYTE c = 255-(rand()%MAXSTARDIMNESS);
      starsb[y*p+x] = c;
      starsb[(y+1)*p+x] = starsb[(y-1)*p+x] = starsb[y*p+x+1] =
        starsb[y*p+x-1] = c/2;
    }

    logger << "Done filling star surface" << endl;
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

  auto_ptr<Gfx::Surface> stars, story, turner; 
  CTimer timer, inc_or_dec;

  RECT dest, source;

  int inx = DISPLAYAREA.right - DISPLAYAREA.left;
  int iny = DISPLAYAREA.bottom - DISPLAYAREA.top;

  state = GLUESTATE_INTRODUCTION; // we are in the intro right now

  logger << "Entering graphics mode" << endl;
  assert(!Gfx::Get());
  GfxBasic::Initialize(hWnd, MODEWIDTH, MODEHEIGHT, 0, MODEWIDTH, MODEHEIGHT);

  logger << "Setup the Introduction palette" << endl;
  GamInitializeWithIntroPalette();

  logger << "Create starscape surface" << endl;
  stars = Gfx::Get()->CreateSurface
    (inx, iny, auto_ptr<Gfx::SurfaceFiller>(new StarFiller()));

  logger << "Load story bmp" << endl;
  story = Gfx::BitmapSurfaceFiller::CreateSurfaceFromBitmap
    (Gfx::Get(), hInstance, MAKEINTRESOURCE(IDB_STORY));

  logger << "Load Turner in crosshairs bmp" << endl;
  turner = Gfx::BitmapSurfaceFiller::CreateSurfaceFromBitmap
    (Gfx::Get(), hInstance, MAKEINTRESOURCE(IDB_TURNER));

  AutoComPtr<IDirectSoundBuffer> warpout;
  DWORD warp_status; // playing or not of the above sound

  // load the warpout sound
  auto_ptr<Resource> warpout_resource(new Resource("DAT", "SFX"));
  void *warpout_data = (void *)warpout_resource->GetPtr();
  
  // we locked it successfully
  int warpout_size = *(int *)warpout_data;
  warpout_data = (void *)(warpout_resource->GetPtr(sizeof(int)));
  warpout = CreateSBFromRawData
    (SndDirectSound().Get(), warpout_data, warpout_size, 0,
     SOUNDRESOURCEFREQ, SOUNDRESOURCEBPS, 1);
  
  warpout_resource.reset();

  logger << "Play Intro Music" << endl;
  GluSetMusic(false, IDR_INTROMUSIC);

  // load polygon data for splash screen
  int polygon_count, *vertex_counts, *polygon_vertices;

  auto_ptr<Resource> splash_resource
    (new Resource("DAT", MAKEINTRESOURCE(IDR_SPLASH)));
  const BYTE *locked = splash_resource->GetPtr();
  
  // load the meaningful data
  polygon_count = (int)*locked++;
  // get the vertex counts
  int total_vertex_count = 0;
  vertex_counts = new int[polygon_count];
  for (int i = 0; i < polygon_count; i++) {
    vertex_counts[i] = (int)*locked++;
    total_vertex_count += vertex_counts[i];
  }
  // get each vertex coordinate
  polygon_vertices = new int[total_vertex_count * 2];
  for (int i = 0; i < total_vertex_count; i++) {
    polygon_vertices[i * 2] = (int)*locked++;
    polygon_vertices[i * 2+1] = (int)*locked++;

    polygon_vertices[i * 2] *= MODEWIDTH;
    polygon_vertices[i * 2] /= 0x100;

    polygon_vertices[i * 2+1] *= MODEHEIGHT;
    polygon_vertices[i * 2+1] /= 0x100;
  }

  splash_resource.reset();

  if (TryAndReport(warpout)) {
    TryAndReport(warpout->Play(0,0,0));
  }

  CTimer total_flash;

  // we need a red brush and a red pen, and a yellow brush and a yellow pen
  HBRUSH brush1 = CreateSolidBrush(FLASHCOLOR1);
  HBRUSH brush2 = CreateSolidBrush(FLASHCOLOR2);
  HPEN pen1 = CreatePen(PS_SOLID, 1, FLASHCOLOR1);
  HPEN pen2 = CreatePen(PS_SOLID, 1, FLASHCOLOR2);

  do {
    CTimer frame; // this makes sure we don't flip too quick
    HDC dc = GfxBasic::Get()->GetDC();
    if(dc) {
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

      SelectObject(dc, (HGDIOBJ)old_brush);
      SelectObject(dc, (HGDIOBJ)old_pen);

      GfxBasic::Get()->ReleaseDC(dc);
    } while (frame.SecondsPassed32() < FLASHCOLORPERSEC);
    Gfx::Get()->Flip();
  } while ((!warpout || SUCCEEDED(warpout->GetStatus(&warp_status)))
          && (warp_status & DSBSTATUS_PLAYING)
	  && (total_flash.SecondsPassed32() < MAXFLASHTIME));

  // get rid of those extra brushes we made
  DeleteObject((HGDIOBJ)brush1);
  DeleteObject((HGDIOBJ)brush2);
  DeleteObject((HGDIOBJ)pen1);
  DeleteObject((HGDIOBJ)pen2);

  warpout.Reset();

  Delete(&polygon_vertices);
  Delete(&vertex_counts);

  // done loading!
  timer.Restart();

  FlushKeyPresses();
  inc_or_dec.Restart();
  do {
    // put the stars and black spots on the back buffer
    stars->Draw(DISPLAYAREA.left, DISPLAYAREA.top, false);
    Gfx::Get()->Rectangle(&UPPERBLACKAREA, 0, false);
    Gfx::Get()->Rectangle(&LOWERBLACKAREA, 0, false);

    float progress = timer.SecondsPassed32() / TIMETOTHROWBACK;
    dest.left = Range(0,iny/2,progress) + (inx - iny) / 2;
    dest.top = Range(0,iny/2,progress) + DISPLAYAREA.top;
    dest.right = Range(iny,iny/2,progress) + (inx - iny) / 2;
    dest.bottom = Range(iny,iny/2,progress) + DISPLAYAREA.top;

    if(dest.left >= dest.right || dest.top >= dest.bottom) {
      break;
    }

    // put the turner photo on the back buffer
    turner->DrawScale(&dest, 0, true);
    Flip();
  } while (timer.SecondsPassed32() <= TIMETOTHROWBACK
          && key_presses.empty());
  inc_or_dec.Pause();

  turner.reset();

  // now show the story
  FlushKeyPresses();
  timer.Restart();
  int max_storyy = (int)(STORY_HEIGHT + iny);
  bool key_pressed = false;
  do {
    // put the stars and black spots on the back buffer
    stars->Draw(DISPLAYAREA.left, DISPLAYAREA.top, false);
    Gfx::Get()->Rectangle(&UPPERBLACKAREA, 0, false);
    Gfx::Get()->Rectangle(&LOWERBLACKAREA, 0, false);

    int storyy=Range(0,max_storyy,timer.SecondsPassed32()/TIMETOSCROLL);

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

        story->DrawScale(&dest, &source, true); 
      }
    }

    while (!key_pressed && !key_presses.empty()) {
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
  } while (!key_pressed);

  logger << "Introduction finished" << endl;

  logger << "Deleting stars and story" << endl;
  stars.reset();
  story.reset();

  logger << "Doing gray block screen transition" << endl;

  for(int c = 255; c >= 0; c--) {
    RECT square;
    square.left = rand()%(MODEWIDTH-TRANSITIONSQUARESIZE);
    square.top = rand()%(MODEHEIGHT-TRANSITIONSQUARESIZE);
    square.bottom = square.top + TRANSITIONSQUARESIZE;
    square.right = square.left + TRANSITIONSQUARESIZE;

    Gfx::Get()->Rectangle
      (&square, Gfx::Get()->MatchingColor(RGB(c, c, c)), false);
    Gfx::Get()->Flip();
    Gfx::Get()->CopyFrontBufferToBackBuffer();

    while (timer.SecondsPassed32() < TRANSITIONSECSPERSQUARE);
    timer.Restart();
  }

  SetGfx();

  logger << "Loading single player and multiplayer sounds" << endl;
  SndLoad(SNDLOAD_MP);

  WtrInitialize();

  logger << "Loading important strings from string table" << endl;
  GluStrLoad(IDS_KILLED,KILLED);
  GluStrLoad(IDS_KILLEDYOURSELF,KILLEDYOURSELF);
  GluStrLoad(IDS_KILLEDTHEMSELVES,KILLEDTHEMSELVES);
  GluStrLoad(IDS_YOU,YOU);
  GluStrLoad(IDS_YOUKILLED,YOUKILLED);
  GluStrLoad(IDS_SPKILLED,SPKILLED);

  logger << "Initializing Menu" << endl;
  m.reset(new CMenu(font.get(),
                    Gfx::Get()->MatchingColor(COLOR_UNSELECTED),
                    Gfx::Get()->MatchingColor(COLOR_SHADOW),
                    Gfx::Get()->MatchingColor(COLOR_SELECTED),
                    Gfx::Get()->MatchingColor(COLOR_SHADOW),
                    Gfx::Get()->MatchingColor(COLOR_HEADING),
                    Gfx::Get()->MatchingColor(COLOR_SHADOW),
                    SHADOW_OFFSET, bitmaps[BMP_MENU].get()));

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
      if(!bitmaps[i].get()) {
        logger << "Bitmap #" << i << " not already loaded" << endl;
        bitmaps[i] = Gfx::BitmapSurfaceFiller::CreateSurfaceFromBitmap
          (Gfx::Get(), hInstance, MAKEINTRESOURCE(IDB_BITMAP2 + i));
      }
    }
  } else if(bitmaps_loaded < prev_bmp_count) {
    logger << "we have to release bitmaps" << endl;

    for(int i = bitmaps_loaded; i < prev_bmp_count; i++) {
      logger << "Releasing bitmap #" << i << endl;
      bitmaps[i].reset();
    }
  }

  logger << "LoadBitmaps() finished" << endl;
}

static void PrepareMenu() {
  SetSpeed(1);

  string data;

  vector<string> strings;
  string header;

  // fills menu with strings appropriate for the current state
  switch(state) {
  case GLUESTATE_INTRODUCTION:
  case GLUESTATE_GAME:
    break;
  case GLUESTATE_MAINMENU:
    HideMouseCursor(); // hide mouse in case it has been showing
    m->SetStrings(MENU_MAINMENU, 0);
    break;
  case GLUESTATE_LEVELSELECT:
    Levels(strings);
    header = MENU_LEVELSELECTHEADER;
    m->SetStrings(header,strings,strings.size()-1);
    break;
  case GLUESTATE_DIFFICULTYSELECT:
    switch (DeeLevelAvailability(level)) {
    case LEVELAVAIL_DANGERDANGER:
      strings.insert(strings.begin(), DifName(DIFFLEVEL_DANGERDANGER));
    case LEVELAVAIL_MYDEARCHILD:
      strings.insert(strings.begin(), DifName(DIFFLEVEL_MYDEARCHILD));
    case LEVELAVAIL_DANGNABIT:
      strings.insert(strings.begin(), DifName(DIFFLEVEL_DANGNABIT));
    }

    header = LEVEL_NAMES[level];

    m->SetStrings(header,strings,strings.size()-1);

    LoadBitmaps(RESOURCELOAD_SP);
    SndLoad(SNDLOAD_SP);

    DifSet(m->GetSelectionIndex());

    FillAccomplishmentLines();
    break;
  case GLUESTATE_CONFIRMQUIT:
    m->SetStrings(MENU_CONFIRMATION, CONFIRMATION_NO);
    break;
  case GLUESTATE_PICKGAME:
    // show cursor in case we need connection info (user would need  mouse)
    ShowMouseCursor(); 
    m->SetStrings(MENU_GAMESLOT, 0);
    break;
  case GLUESTATE_ENTERNAME:
    strings.resize(1);
    strings[0] = player_name;
    GluStrLoad(IDS_ENTERNAMECAPTION,header);
    m->SetStrings(header,strings,0);

    // load extra multiplayer sounds
    SndLoad(SNDLOAD_MP);

    break;
  case GLUESTATE_PICKCHARACTER:
    m->SetStrings(MENU_PICKCHARACTER, CHAR_TURNER);

    LoadBitmaps(RESOURCELOAD_MP);

    break;
  case GLUESTATE_SELECTCONNECTIONMETHOD: {
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

    break;
  }
  case GLUESTATE_PICKVIDEOMODE:
    data = MENU_PICKVIDEOMODE;

    data[data.find('~')] = (char)(DeeVideoMode() + '1');

    m->SetStrings(data.c_str(), DeeVideoMode());

    break;
  case GLUESTATE_PRESSESCAPEINSTRUCTIONS:
    m->SetStrings(MENU_PRESSESCAPEINSTRUCTIONS, 0);

    break;
  case GLUESTATE_CANSEENORMALLY:
    m->SetStrings(MENU_CANSEENORMALLY, 0);
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
          SndPlay(WAV_STEP, Fixed(1), false);
          has_changed_position = true;
        }
      } else if(VK_UP == last_key_pressed) {
        // going down in the selections
        if(m->MoveUp()) {
          SndPlay(WAV_STEP, Fixed(1), true);
          has_changed_position = true;
        }
      }

      m->FillSurface();

      // show difficulty level accomplishments
      if(GLUESTATE_DIFFICULTYSELECT == state) {
        // if we have changed our position, and we are selecting difficulty,
        //  then we have to update our accomplishment data
        if(has_changed_position) {
          DifSet(m->GetSelectionIndex());
          FillAccomplishmentLines();
        }

        WriteString(XCOOR_ACCOMPLISHMENTTEXT,
                    YCOOR_ACCOMPLISHMENTTEXT,
                    accomplishment_lines[DEEDS_SUMMARY].c_str(),
                    Gfx::Get()->MatchingColor(COLOR_ACCOMPLISHMENTTEXT), 0);
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

        bitmaps[bmp]->Draw(target_x, target_y, true);
      }
      
      if (GfxPretty::Get()) {
        GfxPretty::Get()->ClearBorderArea();
      }

      Flip();
    } while (VK_RETURN != last_key_pressed &&
            VK_ESCAPE != last_key_pressed);

  if(VK_RETURN == last_key_pressed) {
    SndPlay(WAV_OKGOTIT, FREQFACTOR_OKGOTITNORMAL, false); 
    return MENUACTION_RETURN;
  } else {
    SndPlay(WAV_OKGOTIT,FREQFACTOR_OKGOTITBACKWARDS, true);
    return MENUACTION_ESCAPE;
  }
}

static void ExtractByte(const BYTE **ptr, int *target) {*target = *(*ptr)++;}

static void ExtractWord(const BYTE **ptr, int *target) {
  int upper_byte;

  ExtractByte(ptr, target);
  ExtractByte(ptr, &upper_byte);

  *target |= upper_byte << 8;
}

static pair<const BYTE *, HGLOBAL> LoadLevelPaletteOnly() {
  assert(level >= 0);

  HRSRC res_handle
    = FindResourceEx(0, TEXT("LVL"), LEVEL_IDS[level],
                     MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
  HGLOBAL data_handle = LoadResource(0, res_handle);
  const BYTE *data_ptr = (const BYTE *)LockResource(data_handle);

  return pair<const BYTE *, HGLOBAL>(GamInitialize(data_ptr), data_handle);
}

static void LoadLevel(bool load_flesh, bool load_bone) {
  int i, j, k, l, m, n, o, p;
  RECT target = Rect(0, 0, GAME_MODEWIDTH, GAME_MODEHEIGHT);

  logger << "LoadLevel() has been called" << endl;

  Gfx::Get()->Rectangle(&target, 0, false);

  logger << "Finished making blank screen" << endl;

  assert(level >= 0);

  if (!NetInGame()) {
    FillAccomplishmentLines();
  }

  // load the file by getting a pointer to the resource

  // GET A POINTER TO RESOURCE DATA
  pair<const BYTE *, HGLOBAL> level_resource = LoadLevelPaletteOnly();
  const BYTE *data_ptr = level_resource.first;

  // load the palettes
  if (load_bone) {
    int level_width, level_height;

    AnalyzePalette();

    Fire::AnalyzePalette();
    WtrAnalyzePalette();
    Character::AnalyzePalette();

    ExtractWord(&data_ptr, &level_width);
    ExtractWord(&data_ptr, &level_height);
    LoadCmps(level_width, level_height, !load_flesh);
  } else {
    // skip level width and height data (four bytes)
    data_ptr += sizeof(WORD) * 2;
  }

  // skip the rest if we don't have to load item data and positions
  if (!load_flesh) {
    FreeResource(level_resource.second);
    return;
  }

  logger << "loading weather script index" << endl;
  int script_index;
  ExtractByte(&data_ptr, &script_index);

  if(!NetInGame()) {
     WtrBeginScript(script_index);
  }

  possible_starting_spots.clear();

  // get hero data

  // get turner's coordinates
  ExtractWord(&data_ptr, &i);
  ExtractWord(&data_ptr, &j);
  i = FixedCnvTo<long>(i);
  j = FixedCnvTo<long>(j);
	
  AddPossibleStartingSpot(i, j);

  ClearCharacters(false);
  
  contexts.Resize(1);
  current_context = 0;

  for (int i = 0; i < contexts.Size(); i++) {
    contexts[i].hero = AddCharacter();
    contexts[i].AmmoReset();
  }
  
  if(!NetProtocolInitialized()) {
     // single-player behaviour
    contexts[0].hero->Setup(i, j, CHAR_TURNER, false, true);
  } else {
    // multi-player behavior
    contexts[0].hero->Setup(-1, -1, model, true, true);
  }

  ul_cached_sector_x = ul_cached_sector_y = -1;

  assert(i >= 0);
  assert(j >= 0);

  walking_data = CBitMatrix::forDimensions(width_in_tiles, height_in_tiles);

  // loop through each rectangle which defines indoor regions

  ExtractByte(&data_ptr, &j);
  logger << "# of indoor regions: " << j << endl;

  for(i = 0; i < j; i++) {
    ExtractWord(&data_ptr, &m);
    ExtractWord(&data_ptr, &n);
    ExtractWord(&data_ptr, &o);
    ExtractWord(&data_ptr, &p);

    for(k = n; k < p; k+= TILE_HEIGHT) {
      assert(k/TILE_HEIGHT >= 0);
      assert(k >= 0);
      for(l = m; l < o; l+=TILE_WIDTH) {
        walking_data->set(l / TILE_WIDTH, k / TILE_HEIGHT);
      }
    }
  }

  for (i = 0; i < sectors.Size(); i++) {
     sectors[i].enemies.clear();
     sectors[i].powerups.clear();
     sectors[i].levelEnds.clear();
  }

  // so now let's do the level ends

  ExtractByte(&data_ptr, &j);

  lends.clear();

  for(i = 0; i < j; i++) {
     ExtractByte(&data_ptr, &k);
     ExtractWord(&data_ptr, &l);
     ExtractWord(&data_ptr, &m);

     if(!NetInGame()) {
        const CLevelEnd cle(FixedCnvTo<long>(l), FixedCnvTo<long>(m), k);
        lends.push_back(cle);
        sectors[(m/SECTOR_HEIGHT) * sector_width + (l/SECTOR_WIDTH)]
          .levelEnds.push_back(cle);
     }
  }

  max_score = 0; // reset the potential score counter

  // do the enemies
  assert(1 == Character::Count());

  for(p = 0; p < 3; p++) { // p is the current enemy type
    ExtractByte(&data_ptr, &j); // need to know how many there are

    for(i = 0; i < j; i++) {
      ExtractWord(&data_ptr, &k); // get x position
      ExtractWord(&data_ptr, &l); // get y position

      if(!NetInGame()) {
        Character::Ptr ch(AddCharacter());
        int sec_row, sec_col;

        ch->Setup(FixedCnvTo<long>(k), FixedCnvTo<long>(l), p, false, false);
        sectors[CalculateSector(ch)].enemies.insert(ch.Index());
      } else {
        AddPossibleStartingSpot(FixedCnvTo<long>(k),FixedCnvTo<long>(l));
      }

      // increment possible score
      if(CHAR_EVILTURNER == p) {
        max_score += GluScoreDiffKill(CHAR_SALLY);
      }
      max_score += GluScoreDiffKill(p);
    }
  }

  // do the ammo and health like we did the enemies
  powerups.clear();

  for(p = 0; p < 4; p++) {
     ExtractByte(&data_ptr, &j);
     powerups.resize(powerups.size()+j);

     for(i = 0; i < j; i++) {
        ExtractWord(&data_ptr, &k);
        ExtractWord(&data_ptr, &l);
        sectors[(l/SECTOR_HEIGHT) * sector_width + (k/SECTOR_WIDTH)]
          .powerups.insert(powerups.size()-i-1);

        k = FixedCnvTo<long>(k);
        l = FixedCnvTo<long>(l);

        powerups[powerups.size()-i-1].Setup(k,l,p);
			
        AddPossibleStartingSpot(k,l);

        max_score += GluScoreDiffPickup(p);
     }
  }

  std_powerups = powerups.size();

  for (int i = 0; i < contexts.Size(); i++) {
    contexts[i].ChangeScore(0);
  }

  // reset timer shown in bottom left of screen
  since_start = 0;

  // and now we've finished
  FreeResource(level_resource.second);
}

static void PrepareForMPGame() {
  DifSet(MPDIFFICULTY);

  // reset score and calculate its coordinates
  contexts[0].ChangeScore(0);
   
  HideMouseCursor(); 
  NetChangeWeather(WtrCurrentState());
  NetSetLevelIndex(level);
}

static void EndGame() {
  // TODO: ADD END-GAME CODE HERE
}

static void Flip() {
  while (true) {
    MSG msg;

    FlushKeyPresses();
    if (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    
    if (!Gfx::Get()->InFocus()) {
      Gfx::Get()->Flip();
      CTimer::Wait(0.5);
    } else {
      break;
    }
  }

  // display frame rate/Choppiness factor if the
  //  user is holding down the C key
  if(KeyPressed(DIK_C)) {
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
//   if(GLUESTATE_GAME == state) {
//      // show profiler results
//      vector<string> profiler_results;
//      GetProfileData(profiler_results);
//      HDC dc;
//      if(SUCCEEDED(Gfx::Get()->BackBuffer()->GetDC(&dc))) {
//         int old_bk_mode = SetBkMode(dc,TRANSPARENT);
//         HGDIOBJ old_font = SelectObject(dc,profiler_font);
//         COLORREF old_text_color = SetTextColor(dc,RGB(255,255,255));
//         int y = 0;
//         for(vector<string>::iterator i = profiler_results.begin();
//             i != profiler_results.end(); i++) {
//            TextOut(dc,0,y,i->c_str(),i->length());
//            y += PROFILER_FONT_SIZE;
//         }
//         SetBkMode(dc,old_bk_mode);
//         SetTextColor(dc,old_text_color);
//         SelectObject(dc,old_font);
//         Gfx::Get()->BackBuffer()->ReleaseDC(dc);
//      }
//   }
#endif

  Gfx::Get()->Flip();

  if(GLUESTATE_GAME == state) {
     static CTimer syncer;

     while (syncer.SecondsPassed32() < spf);
     syncer.Restart();

     // a complete frame has passed
     since_start += TIMER_INC_PER_FRAME;
     if(since_start > MAX_TIMERSECONDS) {
        since_start = MAX_TIMERSECONDS;
     }
  }
}

static void Levels(vector<string>& target) {
  // find the first available level from the end
  for (int i = NUM_LEVELS - 1; i >= 0; i--) {
    if (LEVELAVAIL_NONE != DeeLevelAvailability(i)) {
      target.resize(i+1);
      
      for (int j = 0; j <= i; j++) {
        target[j] = LEVELAVAIL_NONE != DeeLevelAvailability(j)
          ? LEVEL_NAMES[j] : UNAVAILABLE_LEVEL;
      }

      break;
    }
  }
}

static void ShowMouseCursor() {while (ShowCursor(TRUE) < 0);}

static void HideMouseCursor() {while (ShowCursor(FALSE) >= 0);}

static void LoadCmps(int level_width, int level_height, bool skip_wd_resize) {
  string level_name, level_name_u;
  int tw, th; // tile width and height
  auto_ptr<vector<CompactMap *> > cmp_set, cmp_set_u;

  // calculate these members so there is never
  //  any dead space at the edges of the screen when
  //  the hero is near the edge of the level
  max_center_screen_x = FixedCnvTo<long>(level_width  - GAME_MODEWIDTH/2);
  max_center_screen_y = FixedCnvTo<long>(level_height - GAME_MODEHEIGHT/2);

  tw = level_width / TILE_WIDTH + !!(level_width % TILE_WIDTH);
  th = level_height / TILE_HEIGHT + !!(level_height % TILE_HEIGHT);

  sector_width = level_width/SECTOR_WIDTH + !!(level_width%SECTOR_WIDTH);
  sector_height = level_height/SECTOR_HEIGHT + !!(level_height%SECTOR_HEIGHT);

  logger << "Loading level cmps -- size is " << level_width << "x" <<
      level_height << endl;

  if(!skip_wd_resize) {
    sectors.Resize(sector_width * sector_height);
    width_in_tiles = tw;
    height_in_tiles = th;
    walking_data = CBitMatrix::forDimensions(tw, th);
  }

  level_name = LEVEL_IDS[level] + string("_");
  level_name_u = LEVEL_IDS[level] + string("u");

  cmp_set = CompactMap::LoadMapSet(CMP_RESOURCE_TYPE, level_name.c_str());
  cmp_set_u = CompactMap::LoadMapSet(CMP_RESOURCE_TYPE, level_name_u.c_str());

  assert(sectors.Size() == sector_width * sector_height);

  for (int sect = 0; sect < sectors.Size(); sect++) {
    sectors[sect].lowerCell.reset((*cmp_set)[sect]);
    sectors[sect].upperCell.reset((*cmp_set_u)[sect]);
  }
}

static void FilterMovement(const POINT *start, POINT *end) {
  FIXEDNUM center_screen_x = contexts[current_context].center_screen_x;
  FIXEDNUM center_screen_y = contexts[current_context].center_screen_y;

  // first check to make sure they are on the screen, and that we have 
  //  the back buffer successfully locked

  if(abs(center_screen_x - start->x) >= Fixed(GAME_MODEWIDTH/2) ||
     abs(center_screen_y - start->y) >= Fixed(GAME_MODEHEIGHT/2)) {
    // this guy isn't on the screen
    *end = *start;
    return;
  }

  // clip the second part of the plans pair to the edge of the screen
  if(end->x - center_screen_x >= Fixed(GAME_MODEWIDTH/2)) {
    end->x = center_screen_x + Fixed(GAME_MODEWIDTH/2 - 1);
  } else if(center_screen_x - end->x >= Fixed(GAME_MODEWIDTH/2)) {
    end->x = center_screen_x - Fixed(GAME_MODEWIDTH/2 - 1);
  }

  if(end->y - center_screen_y >= Fixed(GAME_MODEHEIGHT/2)) {
    end->y = center_screen_y + Fixed(GAME_MODEHEIGHT/2 - 1);
  } else if(center_screen_y - end->y >= Fixed(GAME_MODEHEIGHT/2)) {
    end->y = center_screen_y - Fixed(GAME_MODEHEIGHT/2 - 1);
  }

  int x_change = FixedCnvFrom<long>(end->x - start->x);
  int y_change = FixedCnvFrom<long>(end->y - start->y);

  int gen_change, inc;
  LONG *axis, axis_inc;

  if(x_change) {
    if(y_change) {
      // moving diagonally (never use rerouting in this case)

      if(y_change < 0) {
        y_change = -y_change;
      }

      if(x_change < 0) {
        x_change = -x_change;
      }

      inc = Gfx::Get()->GetLockPitch() - x_change - 1;

      BYTE *surface_data
        = Gfx::Get()->GetLockPtr
        (FixedCnvFrom<long>(min(start->x, end->x)-center_screen_x
                            + Fixed(GAME_MODEWIDTH /2)),
         FixedCnvFrom<long>(min(start->y, end->y)-center_screen_y
                            + Fixed(GAME_MODEHEIGHT/2)));

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
      inc = -Gfx::Get()->GetLockPitch();
      gen_change = -y_change;
      axis_inc = -Fixed(1);
    } else {
      inc = Gfx::Get()->GetLockPitch();
      gen_change = y_change;
      axis_inc = Fixed(1);
    }

    axis = &end->y;
  } else {
    return;
  }

  BYTE *surface_data
    = Gfx::Get()->GetLockPtr(FixedCnvFrom<long>(start->x - center_screen_x
                                            + Fixed(GAME_MODEWIDTH / 2)),
                         FixedCnvFrom<long>(start->y - center_screen_y
                                            + Fixed(GAME_MODEHEIGHT / 2)));

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

static void SetGfx() {
  logger << "Leaving previous video mode if we were in one" << endl;

  Gfx::Release();

  switch (DeeVideoMode()) {
  case VIDEOMODE_FAST43:
    logger << " - Entering fast 43hz mode" << endl;
    GfxBasic::Initialize(hWnd, GAME_MODEWIDTH, GAME_MODEHEIGHT,
                         43, GAME_MODEWIDTH, GAME_MODEHEIGHT);
    break;
  case VIDEOMODE_FASTNORMAL:
    logger << " - Entering fast mode" << endl;
    GfxBasic::Initialize(hWnd, GAME_MODEWIDTH, GAME_MODEHEIGHT,
                         0, GAME_MODEWIDTH, GAME_MODEHEIGHT);
    break;
  case VIDEOMODE_PRETTY:
    logger << " - Entering pretty mode" << endl;
    GfxPretty::Initialize(hWnd);
  }

  assert(Gfx::Get());

  logger << "Creating surfaces for cached maps" << endl;
  for(int i = 0; i < CACHED_SECTORS; i++) {
    cached[i].first = Gfx::Get()->CreateSurface
      (SECTOR_WIDTH, SECTOR_HEIGHT, auto_ptr<Gfx::SurfaceFiller>
       (new Gfx::NilSurfaceFiller())).release();
    cached[i].second = Gfx::Get()->CreateSurface
      (SECTOR_WIDTH, SECTOR_HEIGHT, auto_ptr<Gfx::SurfaceFiller>
       (new Gfx::NilSurfaceFiller())).release();
  }
  upper_left_sector = 0;

  logger << "Initializing Pal with menu palette" << endl;
  GamInitializeWithMenuPalette();
	
  logger << "Loading single player and multiplayer bitmaps" << endl;
  LoadBitmaps(RESOURCELOAD_MP);

  logger << "Loading game font" << endl;
  font = Gfx::Get()->LoadFont(LoadResource(0, FindResource
                                       (0, MAKEINTRESOURCE(IDR_FONT),
                                        "DAT")),
                          FONTWIDTH, FONTHEIGHT, FIRST_FONTCHAR,
                          LAST_FONTCHAR);
}

static void ReleaseGfxElements() {
  font.reset();

  logger << "Unload all bitmaps" << endl;
  LoadBitmaps(RESOURCELOAD_NONE);

  sectors.Clear();
  
  logger << "Releasing cached map surfaces" << endl;
  for(int i = 0; i < CACHED_SECTORS; i++) {
    Delete(&cached[i].first);
    Delete(&cached[i].second);
  }

  logger << "Uncertifying CGraphics" << endl;
  Gfx::Release();
}

static int CalculateSector(Character::Ptr ch) {
  int row, col;

  assert(ch.Index() < character_sectors.size());

  row = FixedCnvFrom<long>(ch->Y()) / SECTOR_HEIGHT;
  col = FixedCnvFrom<long>(ch->X()) / SECTOR_WIDTH;

  character_sectors[ch.Index()] = (BYTE)(col + row * sector_width);
  return character_sectors[ch.Index()];
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


static void FlushKeyPresses() {
  while (!key_presses.empty()) {
    key_presses.pop();
  }
}

// returns true if the game should be started, false to quit
static bool Menu() {
  logger << "Now running Menu() function" << endl;

  while (true) {
    logger << "Right now we are at the main menu, " <<
      "where you can't press Escape" << endl;

    while (MENUACTION_ESCAPE == MenuLoop())
      ;
      
    logger << "The user pressed Enter, analyzing selection" << endl;

    switch(m->GetSelectionIndex()) {
    case MAINMENU_SP: {
        // do single-player game
      level_select:
        logger << "The user picked Single player" << endl;
        state = GLUESTATE_LEVELSELECT;
        logger << "PrepareMenu()'ing for level select" << endl;
        PrepareMenu();
        logger << "Done preparing menu." << endl;
        logger << "Waiting for user to press Enter or Escape" << endl;
        int menu_action;

        do {
          menu_action = MenuLoop();
          level = m->GetSelectionIndex();
        } while (MENUACTION_RETURN == menu_action
                && LEVELAVAIL_NONE == DeeLevelAvailability(level));

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
        DifSet(m->GetSelectionIndex());

        logger << "Menu() returning 'true'" << endl;
        return true;
      } case MAINMENU_MP:
      enter_name:
          state = GLUESTATE_ENTERNAME;
          if (!player_name.size()) {
            // the user has never entered a player_name before, so pick one
            player_name = m->GetString(CHAR_TURNER);
          }
          
          PrepareMenu();

          // now entering player_name:
          if(MENUACTION_ESCAPE == MenuLoop()) {
            // canceled the mp game plans
            state = GLUESTATE_MAINMENU;
            PrepareMenu();
            continue;
          }
      pick_character:
          state = GLUESTATE_PICKCHARACTER;
          PrepareMenu();

          // picking character:
          if(MENUACTION_ESCAPE == MenuLoop()) {
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

          while (true) {
            if(MENUACTION_ESCAPE == MenuLoop()) {
              NetReleaseProtocol();
              goto select_connection_method;
            }

            WtrBeginScript(m->GetSelectionIndex());
            Gfx::Get()->FlipToGDISurface();

            KeyRefreshState();

            if((KeyPressed(DIK_LSHIFT)) ||
               (KeyPressed(DIK_RSHIFT))) {
              logger << "try to host" << endl;
              try {
                NetCreateGame(m->GetSelectionIndex(), DeeSyncRate(),
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
                return true;
              } catch (NetJoinFailure& njf) { }
            }
          }

          // now we can select a level
          while (true) {
            if(MENUACTION_ESCAPE == MenuLoop()) {
              NetLeaveGame();
              goto pick_game;
            }

            if(LEVELAVAIL_NONE
               != DeeLevelAvailability(m->GetSelectionIndex())) {
              break;
            }
          }

          state = GLUESTATE_GAME;
          level = m->GetSelectionIndex();
          PrepareForMPGame();

          return true;
      case MAINMENU_VIDEOOPTIONS: {
        int new_mode;
        const int old_mode = DeeVideoMode();

      pick_video_mode:
        state = GLUESTATE_PICKVIDEOMODE;
        PrepareMenu();

        if (MENUACTION_ESCAPE == MenuLoop()) {
          state = GLUESTATE_MAINMENU;
          PrepareMenu();
          continue;
        }

        new_mode = m->GetSelectionIndex();

      press_escape_instructions:
        state = GLUESTATE_PRESSESCAPEINSTRUCTIONS;
        PrepareMenu();

        if (MENUACTION_ESCAPE == MenuLoop()
            || ESCAPEINSTRUCTIONS_CANCEL == m->GetSelectionIndex()) {
          goto pick_video_mode;
        }

        // change the video mode here
        DeeSetVideoMode(new_mode);
        ReleaseGfxElements();
        SetGfx();
        m->ChangeGraphicsConfig(font.get(), bitmaps[BMP_MENU].get());

      can_see_normally:
        state = GLUESTATE_CANSEENORMALLY;
        PrepareMenu();

        if (MENUACTION_ESCAPE == MenuLoop()
            || CONFIRMATION_NO == m->GetSelectionIndex()) {
          DeeSetVideoMode(old_mode);
          ReleaseGfxElements();
          SetGfx();
          m->ChangeGraphicsConfig(font.get(), bitmaps[BMP_MENU].get());

          goto pick_video_mode;
        }

        state = GLUESTATE_MAINMENU;
        PrepareMenu();
        continue;
      }
      case MAINMENU_QUIT:
        state =GLUESTATE_CONFIRMQUIT;
        PrepareMenu();

        while (MENUACTION_ESCAPE == MenuLoop())
          ;

        if(CONFIRMATION_YES == m->GetSelectionIndex()) {
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
  } // end of while (true) loop of Menu()
} // end of Menu()

static void Game() {
  logger << "Entering Game() loop" << endl;
  bool reload_flesh = true;
  bool reload_bone = true;
  
  InitializeProfiler(NUM_PROFILES);
  while (true) {
    multiset<Character::Ptr> drawing_order;

    StartProfileFrame();
    BeginProfile(Main_Game_Loop);

    // check for new input
    KeyRefreshState();
	
    // STEP 0: RELOAD LEVEL IF APPROPRIATE or RUN THE END GAME
    if(KeyPressed(DIK_RETURN) && !NetProtocolInitialized()) {
      logger << "User pressed Return, reloading level" << endl;
      reload_flesh = true;
    }
	
    if (reload_flesh || reload_bone) {
      LoadLevel(reload_flesh, reload_bone);
      reload_flesh = reload_bone = false;
      logger << "LoadLevel finished" << endl;
    } else if(NUM_LEVELS <= level) {
      logger << "NUM_LEVELS <= level, the ending sequence will be shown" <<
          endl;
      EndGame();
      break;
    }
	
    // STEP 2: CHECK FOR QUITTER
    if (KeyPressed(DIK_ESCAPE)) {
      logger << "User pressed escape, quitting game" << endl;
      break;
    }

    // check for pauser
    if (!key_presses.empty() && PAUSE_KEY == key_presses.front()) {
      if (!NetInGame()) {
        AutoComPtr<IDirectSoundBuffer> s = SndSound(WAV_PAUSE);
        logger << "User pressed pause" << endl;
	
        // play the sound for pausing the game, 
        if (s && !contexts[0].hero->Dead()) {
          s->SetFrequency(SOUNDRESOURCEFREQ);
          s->Play(0,0,0);
          CTimer::Wait(0.10);
          s->SetCurrentPosition(0);
          CTimer::Wait(0.20);
          s->SetCurrentPosition(0);
          CTimer::Wait(0.20);
          s->SetFrequency(SOUNDRESOURCEFREQ * 3 / 4);
          CTimer::Wait(0.20);
          s->SetFrequency(SOUNDRESOURCEFREQ * 2 / 3);
          CTimer::Wait(0.20);
          s->SetFrequency(SOUNDRESOURCEFREQ * 1 / 2);
        }

        // copy front buffer to back
        Gfx::Get()->CopyFrontBufferToBackBuffer();

        // show health
        BeginProfile(Draw_Meters);
        contexts[0].hero->DrawMeters(true);
        EndProfile();

        // put the "paused" text on the back buffer
        // load the string we'll need
        string paused;
        GluStrLoad(IDS_PAUSE,paused);

        WriteString((GAME_MODEWIDTH-(FONTWIDTH+1)*paused.length())/2,
                    (GAME_MODEHEIGHT-FONTHEIGHT)/2,
                    paused.c_str(), msg_and_score_color, 0);

        // now flip between the two surfaces
        FlushKeyPresses();
        while (true) {
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

        if (s) {
          s->SetFrequency(SOUNDRESOURCEFREQ);
        }
      }
    }

    // STEP 4: LOGIC POWERUPS
    CPowerUp::Rotate();

    // powerup logic is only used to see if regeneration is
    //  necessary.  There is not regeneration in a single-player game
    for (vector<CPowerUp>::iterator itr = powerups.begin();
        NetInGame() && itr != powerups.end(); (*itr++).Logic()) {}

    // STEP 5: DRAW EVERYTHING, INCLUDING POSTED MESSAGE, AND FLIP

    // draw lower-level compact map
    // figure center of screen coordinates

    for (int i = 0; i < contexts.Size(); i++) {
      contexts[i].hero->GetLocation(contexts[i].center_screen_x,
                                    contexts[i].center_screen_y);

      if(contexts[i].center_screen_x < Fixed(GAME_MODEWIDTH/2)) {
        contexts[i].center_screen_x = Fixed(GAME_MODEWIDTH/2);
      } else if(contexts[i].center_screen_x > max_center_screen_x) {
        contexts[i].center_screen_x = max_center_screen_x;
      }
      if(contexts[i].center_screen_y < Fixed(GAME_MODEHEIGHT/2)) {
        contexts[i].center_screen_y = Fixed(GAME_MODEHEIGHT/2);
      } else if(contexts[i].center_screen_y > max_center_screen_y) {
        contexts[i].center_screen_y = max_center_screen_y;
      }
    }

    int level_blit_x = GAME_MODEWIDTH/2
      - FixedCnvFrom<long>(contexts[0].center_screen_x);
    int level_blit_y = GAME_MODEHEIGHT/2
      - FixedCnvFrom<long>(contexts[0].center_screen_y);
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
    for (r = row1; r < row3; r++) {
      target_x = level_blit_x;
      for (c = column1; c < column3; c++) {
        cached[upper_left_sector++ % CACHED_SECTORS].first
          ->Draw(target_x, target_y, false);

        if ((!reload_flesh || !reload_bone) && !NetInGame()) {
          Sector *sect = sectors + (r * sector_width + c);

          // see if we collided with the end of a level in this sector
          // CHECK FOR WINNER
          for (list<CLevelEnd>::iterator itr = sect->levelEnds.begin();
               itr != sect->levelEnds.end(); itr++) {
            if (itr->Collides(contexts[0].hero->X(), contexts[0].hero->Y())) {
              int next_level = itr->Reference();
				
              GluSetMusic(false, IDR_YOUWINTUNE);

              if (!disable_music) {
                CTimer::Wait(YOUWINTUNE_LENGTH);
              } 

              // update level availability variable
              DeeLevelComplete(level, DifGet(), since_start,
                               atoi(contexts[0].score), next_level);
	
              level = next_level;

              if (NUM_LEVELS > next_level) {
                reload_flesh = true;
                reload_bone = true;
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
	
    Gfx::Get()->Lock();

    // allow the hero to try moving
    //  with respect to black lines
    // allow hero to do logic
    BeginProfile(Hero_Logic);
    for (int i = 0; i < contexts.Size(); i++) {
      contexts[i].hero->Logic();
      contexts[i].hero->PowerUpCollisions(&powerups);
    }    
    EndProfile();

    // drive the enemies based on AI or network messages
    BeginProfile(Enemy_Logic);
    
    if (NetInGame()) {
      NetLogic();

      for (int i = 0; i < Character::Count(); i++) {
        drawing_order.insert(Character::Get(i));
      }
    } else {
      drawing_order.insert(contexts[0].hero);

      for (r = row1; r < row3; r++) {
        for (c = column1; c < column3; c++) {
          set<int>::iterator iterate;
          set<int> *vctr = &sectors[r * sector_width + c].enemies;

          for (iterate = vctr->begin(); iterate != vctr->end(); iterate++) {
            Character::Ptr ch = Character::Get(*iterate);

            if (ch->EnemyLogic()) {
              drawing_order.insert(ch);
            }
          }
        }
      }
    }
    
    EndProfile();

    // now we can logic with the projectiles
    for (int i = 0; i < MAX_FIRES; i++) {
      fires[i].Logic();
    }

    Gfx::Get()->Unlock();
	
    // note that we picked a definite coordinate for the center of the
    //  screen before we allowed any movement.  Also note that the 
    //  coordinates decided on by the FilterMovement methods will not
    //  be applied until after the corresponding sprites have been
    //  drawn to the back buffer.  (look at the Character class)
    //  This will prevent shakiness and other strange distortions,
    //  such as making turner's head move like a pigeon's.
    //	However, the coordinates of the characters will always be
    //  one frame old, while the pistol projectiles will
    //  be right on track.

    // draw characters with respect to drawing order
    BeginProfile(Draw_Character);
    for (multiset<Character::Ptr>::iterator i = drawing_order.begin();
        i != drawing_order.end(); i++) {
      if ((**i).DrawCharacter() && !(**i).ControlledByHuman()) {
        // need to update sectors
        // the character may have left the current sector
        //  so remove the enemy from one sector and put it
        //  in another
        sectors[character_sectors[i->Index()]].enemies.erase(i->Index());
        sectors[CalculateSector(*i)].enemies.insert(i->Index());
      }
    }
    EndProfile();

    BeginProfile(Draw_Things);

    // draw power ups
    //  draw ones of sectors
    for (r = row1; r < row3; r++) {
      for (c = column1; c < column3; c++) {
        set<int> *sect_pups = &sectors[r * sector_width + c].powerups;
	  
        for (set<int>::iterator itr = sect_pups->begin();
            itr != sect_pups->end(); itr++) {
          powerups[*itr].Draw();
        }
      }
    }

    // draw extra ones
    for (vector<CPowerUp>::iterator itr = powerups.begin() + std_powerups;
        itr < powerups.end(); itr++) {
      itr->Draw();
    }

    // draw bullets
    for (int bullet_i = 0; bullet_i < MAX_FIRES; bullet_i++) {
      fires[bullet_i].Draw();
    }
    EndProfile(); // powerup and bullets drawing

    // draw upper-level bitmap if outdoors
    BeginProfile(U_Cmp_Drawing_N_Weather);
    
    bool locked = !GluWalkingData(contexts[0].hero->X(), contexts[0].hero->Y());
    
    if (locked) {
      Gfx::Get()->Lock();

      target_y = level_blit_y;
      for (r = row1; r < row3; r++) {
        target_x = level_blit_x;
        for (c = column1;c < column3; c++) {
          Sector *current_sector = &sectors[r * sector_width + c];

          if(current_sector->upperCell->NoStep2()) {
            //  just draw step one normally
            current_sector->upperCell->RenderStep1
              (Gfx::Get(), target_x, target_y, true);
          } else {
            // this cell is complex!  draw the uncompressed bitmap form
            if (locked) {
              Gfx::Get()->Unlock();
              locked = false;
            }
            
            cached[(upper_left_sector + (r - row1)
                    * CACHED_SECTORS_WIDE + (c - column1))
                   % CACHED_SECTORS].second->Draw(target_x, target_y, true);
          }
          target_x += SECTOR_WIDTH;
        }
        target_y += SECTOR_HEIGHT;
      }

      if (!locked) {
        Gfx::Get()->Lock();
        locked = true;
      }
    }

    if (1 == contexts.Size()) {
      WriteMessageTimerAndScore(contexts + 0);
    }

    BeginProfile(Weather_One_Frame);
    if (locked) {
      PlayMusicAccordingly(WtrOneFrameOutdoors());

      Gfx::Get()->Unlock();
    } else {
      PlayMusicAccordingly(WtrOneFrameIndoors());
    }
    EndProfile();

    if ((!NetInGame() || NetIsHost()) && WtrPermitStateChange()) {
      NetChangeWeather(WtrCurrentState());
    }

    EndProfile(); // upper cmp drawing and weather

    BeginProfile(Draw_Meters);

    for (int i = 0; i < contexts.Size(); i++) {
      contexts[i].hero->DrawMeters(KeyPressed(DIK_H));
    }

    EndProfile();

    EndProfile(); // main game loop

    Flip();
  }
}

static void GetLevelTimerMinSec(int *min, int *sec, int *hund) {
  // get how many second we have been playing from the CGlue timer that
  //  was restarted when the level was loaded
  *sec = FixedCnvFrom<long>(since_start);
  *min = *sec / SECONDSPERMINUTE;
  *sec %= SECONDSPERMINUTE;
  *hund = FixedCnvFrom<unsigned long>(since_start * 100) % 100;
}

static void WriteString(int x, int y, const char *str,
                        int color, int shadow_color) {
  Gfx::Get()->Lock();
  if (-1 != shadow_color) {
    font->WriteString(x+1, y+1, str, shadow_color);
  }
  
  font->WriteString(x, y, str, color);
  Gfx::Get()->Unlock();
}

static void WriteScorePretty(Score *score, BYTE color) {
  int score_width = strlen(*score) * font->GetCharWidth() * 2;
  RECT target;
  GfxPretty *p_gfx = GfxPretty::Get();
      
  p_gfx->WriteToFrontBuffer
    (font.get(), p_gfx->GetModeWidth() - score_width - 4, 4, *score, color, 0);

  score_width += strlen("  SCORE ") * font->GetCharWidth() * 2;
  p_gfx->WriteToFrontBuffer
    (font.get(), p_gfx->GetModeWidth() - score_width - 4, 4, "  SCORE ",
     led_color, 0);

  // DRAW THE BLACK REGION AROUND THE SCORE
  target.left = 321;
  target.right = 640;
  target.top = 0;
  target.bottom = 4;
  p_gfx->FrontBufferRectangle(&target, 0);

  target.right = p_gfx->GetModeWidth() - score_width - 4;
  target.top = 4;
  target.bottom = 38;
  p_gfx->FrontBufferRectangle(&target, 0);

  target.left = 636;
  target.right = 640;
  p_gfx->FrontBufferRectangle(&target, 0);
}

static void WriteMessageTimerAndScore(Context *cxt) {
  // this array will contain the text buffer which holds the time
  char time[MAX_TIMERCHAR];
  int minutes, seconds, hund;
  GfxPretty *p_gfx = GfxPretty::Get();
  RECT target;

  BeginProfile(Draw_Extra_Stats_N_Msgs);

  Gfx::Get()->Lock();

  // call a member function that will fill these two
  //  integers with the numbers to be displayed in the timer
  GetLevelTimerMinSec(&minutes, &seconds, &hund);

  sprintf(time, "%02d:%02d.%02d", minutes, seconds, hund);

  // PART 1 - WRITE TIMER
  if (!p_gfx) {
    WriteString(0, SCORE_AND_TIMER_Y, time, msg_and_score_color, 0);
  } else {
    char timer[MAX_TIMERCHAR + 5]; // 5: strlen("TIME ")

    sprintf(timer, "TIME %s", time);
    p_gfx->WriteToFrontBuffer(font.get(), 4, 4, timer, led_color, 0);

    // DRAW THE WHITE BORDER
    target.left = 0;
    target.top = 38;
    target.bottom = 40;
    target.right = 640;
    p_gfx->FrontBufferRectangle(&target, border_color);

    target.top = 0;
    target.left = 319;
    target.right = 321;
    target.bottom = 40;
    p_gfx->FrontBufferRectangle(&target, border_color);

    // DRAW THE BLACK REGION AROUND THE TIMER
    target.left = 0;
    target.right = 319;
    target.top = 0;
    target.bottom = 4;
    p_gfx->FrontBufferRectangle(&target, 0);

    target.top = 4;
    target.bottom = 38;
    target.right = 4;
    p_gfx->FrontBufferRectangle(&target, 0);

    target.right = 319;
    target.left = (sizeof(timer)-1) * font->GetCharWidth() * 2 + 4;
    p_gfx->FrontBufferRectangle(&target, 0);
  }

  // PART 2 - WRITE MESSAGE
  if(++frames_for_current_message < FRAMESTODISPLAYMSG) {
    // print message itself
    WriteString(msg_x, 0, message.c_str(), msg_and_score_color, 0);
  }

  // PART 3 - WRITE SCORE
  if(cxt->score_print_x > 0) {
    if (!p_gfx) {
      WriteString(cxt->score_print_x, SCORE_AND_TIMER_Y, cxt->score,
                  msg_and_score_color, 0);
    } else {
      WriteScorePretty(&cxt->score, led_color);
    }
  } else {
    // score_print_x is negative, which means we should be printing it
    //  in flashing colors also, the last character in score string
    //  contains countdown to stop flashing

    // print score in flashing red/yellow
    int color = frames_for_current_message & 1
      ? score_flash_color_1 : score_flash_color_2;

    if (!p_gfx) {
      // put the score on the back buffer
      WriteString(-cxt->score_print_x, SCORE_AND_TIMER_Y, cxt->score, color, 0);
    } else {
      // put the score on the front buffer in the upper black region
      WriteScorePretty(&cxt->score, color);
    }

    // countdown to stop flashing
    if(FRAMESTODISPLAYMSG == frames_for_current_message) {
      cxt->score_print_x = -cxt->score_print_x;
    }
  }

  Gfx::Get()->Unlock();

  EndProfile(); // extra stats and messages
}

static void Recache(int flags) {
  if(0 == flags) {
    return; // already cached everything
  }
  
  int bit = 1, cached_sector = upper_left_sector;
  
  for (int y = 0; y < CACHED_SECTORS_HIGH; y++) {
    // find the sector coordinates at this height
    int sector_y = ul_cached_sector_y + y;
    if(sector_y >= sector_height) {
      return; // all done, we are out of range
    }
    
    for (int x = 0; x < CACHED_SECTORS_WIDE; x++, bit <<= 1) {
      if(bit & flags) {
        // we have to recache the map at x,y
        int sector_x = ul_cached_sector_x + x;

	logger << "Reload sector " << sector_x << "x" << sector_y << endl;

	// now make sure this fits into the sector grid of the whole level
	if(sector_x < sector_width) {
          CompactMap *source_a, *source_b;
	  logger << "Sector coor is in range, loading" << endl;
	  
          cached_sector %= CACHED_SECTORS;
          source_a = sectors[sector_y*sector_width + sector_x].lowerCell.get();
          source_b = sectors[sector_y*sector_width + sector_x].upperCell.get();

          cached[cached_sector].first->ChangeFiller(source_a->Filler());
          cached[cached_sector].second->ChangeFiller(source_b->Filler());
        }

        // all done rendering to surface
	logger << "Reloaded sector " << sector_x << "x" << sector_y << endl;

        flags &= ~bit;
        if(!flags) {
	  logger << "Recache finished" << endl;
          return;
        }
      }
      cached_sector++;
    }
  }

  logger << "Recache finished" << endl;
}

static void PlayMusicAccordingly(int state_change_indicator) {
  switch (state_change_indicator) {
  case WTROF_TURNMUSICON:
    GluSetMusic(true, LEVEL_IDS[level]);
    break;
  case WTROF_TURNMUSICOFF:
    MusicStop();
    last_music = "";
  }
}

static void FillAccomplishmentLines() {
  if (DeeHasBeatenLevel(level, DifGet())) {
    char buffer[MAX_STRINGLEN];
    pair<FIXEDNUM, int> best_time_and_score
      = DeeGetBestTimeAndScore(level, DifGet());
    FIXEDNUM timeAtBestScore
      = DeeGetTimeAtBestScore(level, DifGet());

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
            DeeGetScoreAtBestTime(level, DifGet()));
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

static Character::Ptr AddCharacter() {
  assert(Character::Count() == character_sectors.size());

  character_sectors.push_back(0);

  return Character::Add();
}

static void ClearCharacters(bool keep_first) {
  Character::Clear(keep_first);

  character_sectors.resize(Character::Count());
}

static void AnalyzePalette() {
  score_flash_color_1 = Gfx::Get()->MatchingColor(RGB(0xff, 0, 0));
  msg_and_score_color = Gfx::Get()->MatchingColor(RGB(0xff, 0xff, 0xff));
  border_color = msg_and_score_color;
  led_color = Gfx::Get()->MatchingColor(RGB(0xff, 0xff, 0x00));

  if (GfxPretty::Get()) {
    score_flash_color_2 = msg_and_score_color;
  } else {
    score_flash_color_2 = led_color;
  }
}

static void CleanUpAfterGame() {
  logger << "Main game loop has terminated" << endl;

  // cleanup after the game
  if(NetProtocolInitialized()) {
    if(NetInGame()) {
      NetLeaveGame();
    }
    NetReleaseProtocol();
  }

  GamInitializeWithMenuPalette();
  state = GLUESTATE_MAINMENU;

  FlushKeyPresses();

  PrepareMenu();
			
  WtrEndScript();
  GluSetMusic(true, IDR_MENUMUSIC);
}

FIXEDNUM Context::AmmoAsPercentage(int weapon) {
  return Fixed(ammo[weapon]) / AMMOCAPACITY[weapon];
}

bool Context::AmmoFull(int weapon) {
  return ammo[weapon] == AMMOCAPACITY[weapon];
}

void Context::AmmoAdd(int weapon, int amount) {
  ammo[weapon]
    = (unsigned short)min(amount + ammo[weapon], (int)AMMOCAPACITY[weapon]);
}

void Context::AmmoReset() {
  for (int i = 0; i < WEAPON_COUNT; i++) {
    ammo[i] = AMMOSTARTING[i];
  }
}

void Context::Draw(unsigned int bmp, int x, int y) {
  bitmaps[bmp]->Draw(x, y, true);
}

void Context::DrawScale(unsigned int bmp, RECT *target) {
  Gfx::Get()->AttachScalingClipper();
  bitmaps[bmp]->DrawScale(target, 0, true);
  Gfx::Get()->DetachScalingClipper();
}

void Context::ChangeScore(int diff) {
  int new_score, dummy;

  sscanf(score, TWO_NUMBERS_FORMAT, &new_score, &dummy);

  new_score = diff ? new_score + diff : 0;

  if (new_score > MAXSCORE) {
    new_score = MAXSCORE;
  } else if (new_score < MINSCORE) {
    new_score = MINSCORE;
  }

  if (NetInGame()) {
    // don't care about maximum score
    sprintf(score, ONE_NUMBER_FORMAT, new_score);
  } else {
    sprintf(score, TWO_NUMBERS_FORMAT, new_score, max_score);
  }

  // now calculate printing coordinates
  score_print_x = GAME_MODEWIDTH
    - strlen(score) * (FONTWIDTH+1) + SCORE_X_OFFSET;

  if (!NetInGame() && max_score == new_score) {
    // colors should be flashing because we have the highest score,
    //  so flag it by making score_print_x negative
    score_print_x = -score_print_x;

    GluPostMessage(MAXSCORE_MESSAGE);
  }
}

Context *GluContext() {return contexts + current_context;}

bool GluInitialize(HINSTANCE hInstance_, HWND hWnd_) { 
  com.reset(new Com());
  hInstance = hInstance_;
  hWnd = hWnd_;

  if (TryAndReport(DeeInitialize())) {
    string msg;
    GluStrLoad(IDS_BUDGETCUTS, msg);
    MessageBox(hWnd, msg.c_str(), WINDOW_CAPTION, MB_ICONINFORMATION);
  }

  SndInitialize(hWnd);
  TryAndReport(MusicInit(hWnd, SndDirectSound().Get()));

  NetInitialize();

  // show welcome dialog (aka cfg dialog)
  if(IDQUIT == DialogBox(hInstance,MAKEINTRESOURCE(IDD_CFG),hWnd,CfgDlgProc)) {
    return true; // return true to quit
  }

  HideMouseCursor();

  // create direct input and setup the device state change event
  KeyInitialize(hInstance, hWnd);

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

  state = GLUESTATE_INTRODUCTION;

  sectors.Clear();

  level = LEVEL_NONE;

  assert(!bitmaps_loaded);

  return false;
}

void GluRelease() {
  logger << "GluReleease() called" << endl;

  if(GLUESTATE_UNINITIALIZED == state) {
    logger << "Glue is already released" << endl;
    return;
  }

  logger << "calling NetRelease()" << endl;
  NetRelease();
  logger << "deleting menu" << endl;
  m.reset();
  logger << "calling MusicUninit()" << endl;
  MusicUninit();
  SndRelease();

#ifdef _DEBUG
  TryAndReport(DeleteObject(profiler_font));
#endif

  KeyRelease();

  WtrRelease();

  logger << "Releasing Gfx elements" << endl;

  ReleaseGfxElements();

  logger << "Saving level completion data" << endl;
  DeeRelease();
  logger << "Closing COM" << endl;
  com.reset();
  logger << "Calling ShowMouseCursor to show the mouse" << endl;
  ShowMouseCursor();

  state = GLUESTATE_UNINITIALIZED;

  logger << "GluRelease returning" << endl;
}

bool GluCanQuit() {return bool(GLUESTATE_UNINITIALIZED == state);}

HWND GluMain() {
  logger << "Calling Introduction() to display intro screen" << endl;

  Introduction();

  logger << "Starting Menu() for the first time" << endl;
  
  while (TryAndReport(Menu())) {
    Game();
    logger << "Game() terminated" << endl;
    CleanUpAfterGame();
    logger << "Starting Menu() again" << endl;
  }

  logger << "Menu() returned false" << endl;

  return hWnd;
}

void GluPostForcePickupMessage() {
  static bool shown_message_already = false;

  // only display the "hold p" message if
  //  we have never showed it before,
  //  no other message is showing, the hero
  //  cares about the score, and we are not
  //  in multiplayer
  if (!shown_message_already
      && frames_for_current_message > FRAMESTODISPLAYMSG) {
    string msg;
    if (NetInGame()) {
	  GluStrLoad(IDS_FORCEPICKUPMP,msg);
	} else {
	  GluStrLoad(IDS_FORCEPICKUPSP,msg);
	}
    GluPostMessage(msg.c_str());
    shown_message_already = true;
  }
}

void GluPostMessage(const char *str) {
  // make sure we are done with the current message
  if (frames_for_current_message >= MINFRAMESTOKEEPMESSAGE) {
    message = str;

    if (message.length() > MAX_CHARS_PER_LINE) {
      message = message.substr(0, MAX_CHARS_PER_LINE);
    }
    
    msg_x = (GAME_MODEWIDTH - (message.length() * FONTWIDTH)) / 2;

    // reset the timer
    frames_for_current_message = 0;
  }
}

void GluPlaySound(int i, FIXEDNUM x_source, FIXEDNUM y_source) {
  FIXEDNUM x_dist, y_dist;

  if (contexts.Size() > 1) {
    x_dist = y_dist = 0;
  } else {
    x_dist = x_source - contexts[0].center_screen_x;
    y_dist = y_source - contexts[0].center_screen_y;
  }

  SndPlay(i, x_dist, y_dist);
}

void GluSetMusic(bool loop, WORD music_resource) {
  logger << "SetMusic type B called to use music resource " <<
      music_resource << endl;

  if (!disable_music) {
    TryAndReport(MusicPlay(loop, MIDI_RESOURCE_TYPE,
                           MAKEINTRESOURCE(music_resource)));
    last_music = "";
  }

  SetSpeed(1);

  logger << "SetMusic finished." << endl;
}

void GluSetMusic(bool loop, const char *music_resource) {
  // only play music if it was not disabled by
  //  the intro/welcome dialog, and make sure we
  //  don't play music that's already going
  logger << "SetMusic type A called to use music resource " <<
      music_resource << endl;
  
  if (!disable_music && last_music != music_resource) {
    TryAndReport(MusicPlay(loop,MIDI_RESOURCE_TYPE, music_resource));
    last_music = music_resource;
  }

  if (GLUESTATE_GAME == state) {
    SetSpeed(GetSpeed());
  } else {
    SetSpeed(1);
  }
  
  logger << "SetMusic finished" << endl;
}

//  the plans parameter is passed as a non-const reference because
//  we will change the second part of the pair to tell the mover where
//  they can go which is closest to where they wanted to go
void GluGetRandomStartingSpot(POINT& p) {
  p = possible_starting_spots[rand()%possible_starting_spots.size()];
  ul_cached_sector_x = ul_cached_sector_y = -1;
}

void GluFilterMovement(const POINT *start, POINT *end) {
  Gfx::Get()->Lock();
  FilterMovement(start, end);
  Gfx::Get()->Unlock();
}

void GluCharPress(char c) {
  if (GLUESTATE_ENTERNAME == state) {
    switch(c)	{
	case '\b':
	  // backspace was pressed
	  if (player_name.length() > 0) {
        player_name = player_name.substr(0, player_name.length()-1);
      }

	  SndPlay(WAV_BING, Fixed(1), false);
	  break;
      case '\r': case '\n': case '\t': case '\a':
      case '\f': case '\v': case 27:
        // a key we don't care about was pressed
        break;
      default:
        SndPlay(WAVSET_POINTLESS+(rand()%WAVSINASET), Fixed(1), rand() & 1);
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
  } else if (GLUESTATE_GAME == state && !NetInGame()) {
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

void GluKeyPress(BYTE scan_code) {key_presses.push(scan_code);}

bool GluWalkingData(FIXEDNUM x, FIXEDNUM y) {
  x /= TILE_WIDTH; 
  y /= TILE_HEIGHT;

  return walking_data->get(
      FixedCnvFrom<long>(x), FixedCnvFrom<long>(y));
}

void GluPostSPKilledMessage() {GluPostMessage(SPKILLED.c_str());}

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

void GluStrLoad(unsigned int id, string& target) {
  char buffer[LONG_STRINGLEN];

  LoadString(hInstance, id, buffer, LONG_STRINGLEN);
	
  target = buffer;
}

int GluScoreDiffPickup(int x) {return (2 == x) ? 2 : 1;}

int GluScoreDiffKill(int x) {return x * 2 + 2;}

