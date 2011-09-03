const FIXEDNUM HEROSPEED            = Fixed(2.80f);
const FIXEDNUM HEROSPEED_MPFACTOR   = Fixed(1.20f);
const FIXEDNUM HEROSPEED_HURTFACTOR = Fixed(0.75f);
// a small secret, if all the arrow keys are
//  pressed except one direction, the hero
//  will run in the direction opposite of the unpressed button
const FIXEDNUM HEROSPEED_RUNNINGFACTOR = Fixed(1.414f);
const FIXEDNUM FATNESS = Fixed(12); // used for collision detection with bullets/characters

const int SECONDSPERMINUTE = 60;
const int NUM_DIFFICULTYLEVELS = 3;
const int NUM_CHARACTERS = 9;
const int NUM_WEAPONS = 3;
const int NUM_SPSOUNDS = 16;
const int NUM_SOUNDS = 40;
const int WAV_OKGOTIT = 0;
const int WAV_STEP = 1;
const int WAV_GUNNOISE = 2;
const int WAVSET_WEAPONS = 3;
const int WAVSET_ALIENHIT = 6;
const int WAVSET_ALIENDEATH = 9;
const int WAVSET_FIRSTNONALIEN = 12;
const int WAVSET_POINTLESS = 36;
const int WAV_BING = 39;
const int WAVSINASET = 3;
const int BMP_BLOODSTAIN = 0;
const int BMP_BULLET = 1;
const int BMP_EXPLOSION = 2;
const int BMP_HEALTH = 3;
const int BMP_MENU = 4;
const int BMPSET_DECAPITATE = 5;
const int BMPSET_WEAPONS = 7;
const int BMPSET_PISTOL = 7;
const int BMPSET_MACHINEGUN = 11;
const int BMPSET_BAZOOKA = 15;
const int BMPSET_CHARACTERS = 19;
const int ANIMATIONFRAMESPERCHARACTER = 8;
const int RENDERED_DIRECTIONS = 4;
const int GAME_MODEWIDTH = 320;
const int GAME_MODEHEIGHT= 200;
const int GAME_PORTHEIGHT = 200;
const int GAME_MODEBUFFERS = 2; // number of buffers for the video mode (2 means double-buffering)
const int GAME_MODEREFRESH = 0; // indicate default refresh rate
const int GAME_MODEBPP = 8;
const BYTE EIGHTHBIT = 0x80;
const int KBBUFFERSIZE = 256;
const int TILE_WIDTH = 32;
const int TILE_HEIGHT = 30;
const int NUM_LEVELS = 11;
const int MAX_FIRES = 10; // maximum of ten bullets at a time
const int CREATEGAME_SUCCESS = 0;
const int CREATEGAME_FAILURE = 1;
const int JOINGAME_FAILURE = -1;
const int INITIALIZEPROTOCOL_SUCCESS = 0;
const int INITIALIZEPROTOCOL_INVALIDINDEX = 1;
const int INTROPALETTE_BASECOLORS = 26;
const DWORD SOUNDRESOURCEFREQ = 11025;
const DWORD SOUNDRESOURCEBPS = 8; // bits per sample of sounds
const int SECTOR_WIDTH = 160;
const int SECTOR_HEIGHT = 100;

enum {DEAST,DNORTH,DWEST,DSOUTH,DNE,DNW,DSW,DSE};
enum {LEVELAVAIL_NONE,LEVELAVAIL_DANGNABIT,LEVELAVAIL_MYDEARCHILD,LEVELAVAIL_DANGERDANGER};
enum {WEAPON_PISTOL,WEAPON_MACHINEGUN,WEAPON_BAZOOKA};
enum {FIRESTATE_GOING,FIRESTATE_FIRSTFRAMETODRAW,FIRESTATE_INACTIVE = -1};
enum {CHAR_SALLY,CHAR_MILTON,CHAR_EVILTURNER,CHAR_TURNER,CHAR_SWITZ,CHAR_CHARMIN,CHAR_PEPSIONE,CHAR_COCACOLACLASSIC,CHAR_KOOLAIDGUY,CHAR_HERO = -1,CHAR_UNKNOWN = -2};
enum {CHARSTATE_ALIVE,CHARSTATE_WALKING,CHARSTATE_HURT,CHARSTATE_DYING,CHARSTATE_DEAD,CHARSTATE_UNKNOWING,CHARSTATE_FIRING,CHARSTATE_UNINIT = -1};
enum {POWERUP_PISTOLAMMO,POWERUP_MACHINEGUNAMMO,POWERUP_BAZOOKAAMMO,POWERUP_HEALTHPACK};
enum {POWERUPCOLLIDES_NOTHINGHAPPENED,POWERUPCOLLIDES_PICKEDUPANDWILLNOTREGENERATE,POWERUPCOLLIDES_PICKEDUPANDWILLREGENERATE};
enum {SHOWHEALTH_NO,SHOWHEALTH_YES,SHOWHEALTH_IFHURT};

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
      Next_Sound_Slot,
      NUM_PROFILES};
