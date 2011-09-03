
#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#include <cassert>

// general constants
const unsigned char HIGHEST_BIT8 = 0x80;
const char WINDOW_CAPTION[] = "Andradion 2 Demo";
const char WINDOW_CLASS[] = "Andradion 2 Demo WndCls";
const char STARTUP_ERROR[] = "There was an error with initializing the keyboard or creating the window.";
const char ERROR_MSG_CAPTION[] = "Error";
const int GAME_MODE_WIDTH = 320;
const int GAME_MODE_HEIGHT = 240;
const int GAME_MODE_BPP = 16;
const int GAME_MODE_BUFFERS = 2;
const int GAME_MODE_REFRESH_RATE = 0;

const float SECS_PER_FRAME = 0.03f;
const int PISTOL = 0;
const int MACHINE_GUN = 1;
const int BAZOOKA = 2;

// resources
const char LOADING_BMP_PATH[] =    "IMAGES\\LOADING.BMP";


const char LOADING_MUSIC_PATH[] =  "MUSIC\\BETWIXT.MID";

// tokens
const int TURNER = 0;     // these are in the token array
const int WALL = 1;       //  and limited in quantity
const int LEVEL_END = 2;

const int ENEMY = 3;      // these are in the token array
const int AMMO = 4;       //  but unlimited
const int HEALTH_PACK = 5;

const int BAZOOKA_FIRE = 6; // these are not in the token
const int TRIGGER = 7;      //  array and are generally
const int BULLET = 8;       //  unlimited

const int NUM_TOKENS = 9;

// media
const int NUM_LEVELS = 1;
const int NUM_BITMAPS = 50; // does not include those not used by CMedia
const int NUM_SOUNDS = 18;

const int B1DON = 0;
const int B1HARRY = 4;
const int B1SALLY = 8;
const int B1TURNER = 12;
const int B2DON = 16;
const int B2HARRY = 20;
const int B2SALLY = 24;
const int B2TURNER = 28;
const int BPISTOL = 32;
const int BMACHINEGUN = 36;
const int BBAZOOKA = 40;
const int BDECAPITATE1 = 44;
const int BDECAPITATE2 = 45;
const int BEXPLOSION = 46;
const int BHEALTH = 47;
const int BBLOODSTAIN = 48;
const int BBULLET = 49;

const int RENDERED_DIRECTIONS = 4;
const int DEAST = 0;
const int DNORTH = 1;
const int DWEST = 2;
const int DSOUTH = 3;

// non-drawn directions

const int DNE = 4;
const int DNW = 5;
const int DSW = 6;
const int DSE = 7;

// this function interprets directions
inline void InterpretDirection(int direction,int& xf,int& yf)
{
	switch(direction)
	{
	case DNORTH: case DSOUTH: xf = 0; break;
	case DNW: case DWEST: case DSW: xf = -1; break;
	case DNE: case DEAST: case DSE:	xf = 1;	break;
	default: assert(false);
	}

	switch(direction)
	{
	case DNORTH: case DNE: case DNW: yf = -1; break;
	case DWEST: case DEAST: yf = 0; break;
	case DSOUTH: case DSE: case DSW: yf= 1; break;
	default: assert(false);
	}
}

const int NUM_ENEMIES = 3;
const int NUM_WEAPONS = 3;

// files to open

const int MAX_BITMAP_FILE_LEN = 16;

const char BITMAPS[NUM_BITMAPS][MAX_BITMAP_FILE_LEN] = {
	"1done.bmp",
	"1donn.bmp",
	"1donw.bmp",
	"1dons.bmp",
	"1harrye.bmp",
	"1harryn.bmp",
	"1harryw.bmp",
	"1harrys.bmp",
	"1sallye.bmp",
	"1sallyn.bmp",
	"1sallyw.bmp",
	"1sallys.bmp",
	"1turnere.bmp",
	"1turnern.bmp",
	"1turnerw.bmp",
	"1turners.bmp",
	"2done.bmp",
	"2donn.bmp",
	"2donw.bmp",
	"2dons.bmp",
	"2harrye.bmp",
	"2harryn.bmp",
	"2harryw.bmp",
	"2harrys.bmp",
	"2sallye.bmp",
	"2sallyn.bmp",
	"2sallyw.bmp",
	"2sallys.bmp",
	"2turnere.bmp",
	"2turnern.bmp",
	"2turnerw.bmp",
	"2turners.bmp",
	"pistole.bmp",
	"pistoln.bmp",
	"pistolw.bmp",
	"pistols.bmp",
	"mge.bmp",
	"mgn.bmp",
	"mgw.bmp",
	"mgs.bmp",
	"bazookae.bmp",
	"bazookan.bmp",
	"bazookaw.bmp",
	"bazookas.bmp",
	"decapit1.bmp",
	"decapit2.bmp",
	"explosio.bmp",
	"health.bmp",
	"bloodsta.bmp",
	"bullet.bmp"
};

const char LEVEL_PATH[] = "LEVELS\\";
const char SOUND_PATH[] = "SOUNDS\\";
const char BITMAP_PATH[] = "IMAGES\\";
const char MUSIC_PATH[] = "MUSIC\\";

const int MAX_LEVEL_FILE_LEN = 16;

const char LEVELS[NUM_LEVELS][MAX_LEVEL_FILE_LEN] = {
	"front.dat"
};

const int LEVEL_UNLOAD = -1;

const int SALIENDEATH1 = 0;
const int SALIENDEATH2 = 1;
const int SALIENDEATH3 = 2;
const int STURNERDEATH = 3;
const int SPISTOL = 4;
const int SMACHINEGUN = 5;
const int SBAZOOKA = 6;
const int SEXPLOSION = 7;
const int SPICKUP = 8;
const int SSWITCHWEAPONS = 9;
const int SALIENHIT1 = 10;
const int SALIENHIT2 = 11;
const int SALIENHIT3 = 12;
const int STURNERHIT1 = 13;
const int STURNERHIT2 = 14;
const int STURNERHIT3 = 15;
const int STURNERSTEP1 = 16;
const int STURNERSTEP2 = 17;


// how many redundant sounds there are for some of them
const int NUM_REDUNDANT_SOUNDS = 3; 

const int MAX_SOUND_FILE_LEN = 18;

const char SOUNDS[NUM_SOUNDS][MAX_SOUND_FILE_LEN] = {
	"aliendeath1.wav",
	"aliendeath2.wav",
	"aliendeath3.wav",
	
	"turnerdeath.wav",
	
	"pistol.wav",
	"machinegun.wav",
	"bazooka.wav",
	"explosion.wav",
	
	"pickup.wav",
	"switchweapons.wav",
	
	"alienhit1.wav",
	"alienhit2.wav",
	"alienhit3.wav",
	
	"turnerhit1.wav",
	"turnerhit2.wav",
	"turnerhit3.wav",

	"step1.wav",
	"step2.wav"
};

const int STATE_ALIVE = 0;
const int STATE_WALKING = 1; // use second walking frame when this state is on
const int STATE_UNKNOWING = 2; // only for aliens when they have no idea turner's here
const int STATE_FIRING = 3; // used only with aliens
const int STATE_HURT = 4;
const int STATE_DYING = 5;
const int STATE_DEAD = 6;
const int STATE_WON = 7;

const int STATE_UNGOTTEN = 0; // items, powerups
const int STATE_GOTTEN = 1;   // items, powerups

const int STATE_INACTIVE = -1; // for projectiles
const int STATE_PROJECTILE_READY = -2;

const int FRAMES_PER_STEP = 8;

const int FRAMES_TO_RECOVER_TURNER = 16;
const int FRAMES_TO_RECOVER_ENEMIES = 8;

const int TILE_WIDTH = 32;
const int TILE_HEIGHT = 30;

// about three seconds for an explosion to dissipate
const int FRAMES_FOR_EXPLOSION = 100; 

// walking data
const int WALKABLE_OUTSIDE = 0;
const int WALKABLE_INSIDE = 1;
const int WALKABLE_WALL = 2;

const char LEVEL_LOAD_ERROR[] = "Problem with loading a level.";
const char RESOURCE_LOAD_ERROR[] = "Problem with loading a resource (not enough memory).";

const int TURNER_SPEED_1 = 2; // pixels turner moves in one frame
const int TURNER_SPEED_2 = 3; // pixels turner moves when legs are apart
const int ENEMY_SPEED_1 = 1; // and for the enemies
const int ENEMY_SPEED_2 = 2;

// how much time it takes them to hurt and recover
const int TURNER_FRAMES_TO_HURT = 16;
const int ENEMY_FRAMES_TO_HURT = 32;

const int WEAPON_CAPACITY[NUM_WEAPONS] =
{
	50,
	100,
	8
};

const float START_AMMO[NUM_WEAPONS] =
{
	0.5f,
	0.5f,
	0.5f
};

const int WEAPON_POWERUP[NUM_WEAPONS] = // amount of ammo for each weapon picked up in one ammo powerup
{
	10,
	20,
	2
};

const int FRAMES_TO_RELOAD[NUM_WEAPONS] =
{
	16,
	1,
	64
};

const float HEALTH_INCREMENT = 0.4f; // how much health an hp gives

const char OUT_OF_MEMORY[] = "Out of memory.";

const int FATNESS = 12; // effective radius for collision detection
const int OBJECT_FATNESS = 20; // " " " for picking up item detection

const int FRAMES_TO_DIE = 64; // how long the blood and guts are animated

const int FRAMES_TO_FIRE = 64; // how long an alien will try shooting in one direction

const int PROJECTILE_SPEED = 8; // lower for more logical accuracy
const int NUM_PROJECTILE_LOGIC_LOOPS = 40; // how many until we give up

// enemy models
const int MODEL_DONPLAIN = 0;
const int MODEL_HARRY = 1;
const int MODEL_SALLYBLUETURNER = 2;

// meters
const int HEALTH_METER_X = 5;
const int AMMO_METER_X = 165;

const int METER_Y = 235;
const int METER_W = 150;
const int METER_H = 15;

const int HEALTH_METER_R = 255;
const int HEALTH_METER_G = 0;
const int HEALTH_METER_B = 0;

const int AMMO_METER_R = 0;
const int AMMO_METER_G = 255;
const int AMMO_METER_B = 255;

// distance squared before aliens notice turner
const int MAX_SAFE_PROX = 150;

const float BAZOOKA_DAMAGE_TO_TURNER = 0.3f;
const float BULLET_DAMAGE_TO_TURNER = 0.1f;
const float BULLET_DAMAGE_TO_ENEMY = 0.3f;

const int MAX_EXPLOSION_BULGE = 5;

const int  INITIAL_PROJ_X_MOVEMENT = 15;
const int  INITIAL_PROJ_Y_MOVEMENT = 15;

// projectile states
const int STATE_FIRED = 1;
const int STATE_NOTFIRED = 0;

const int MACHINE_GUN_SWAY = 10;

#endif

