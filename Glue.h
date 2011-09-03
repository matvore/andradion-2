using std::vector;
using std::pair;
using std::queue;
using std::multiset;

class CPowerUp;
class CLevelEnd;

// this function reloads the bitmap set (mp or sp) that has
//  already been specified, so it can be called whenever something
//  has made the data in the surfaces invalid (a palette change
//  or a surface loss)
void GluReloadBitmaps();
	
// this function returns true if CGlue has been
//  released and the app can quit without any
//  memory leak or anything bad like that
bool GluCanQuit();

// this function plays the game (including the intro, endgame, menu
//  screen), and returns the handle of the main window
HWND GluMain();

// this function tells the user
//  about the feature which allows
//  the player to pickup a powerup
//  even when it is not needed
void GluPostForcePickupMessage();

// this function stops the music temporarily
//  until SetMusic is called again
void GluStopMusic();

// puts a text message at the top of the screen
//  if the previous message has not been up there for very long,
//  this function does nothing
void GluPostMessage(const TCHAR *str);

// call once to disable music throughout the entire game
//  incase running on an older system
void GluDisableMusic();

// changes the score by the difference specified.
//  if a difference below zero is specified,
//  the score is reduced.  If a difference
//  above zero is specified, the score is increased
//  if the difference specified is zero, the score
//  is reset to zero
void GluChangeScore(int diff);

// plays the okay got it sound at the specified frequency
//  and reverses if necessary
void GluPlaySound(int i,FIXEDNUM freq_factor,bool reverse);

// finds the text colors from the palette that are used when printing
//  score, message, and timer
void GluFindTextColors();

// sets the music that should be played.
//  if music was disabled, this function does nothing
//  if the music specified is already playing, this fucntion
//   does nothing.
// one overided SetMusic function allows the caller to specify
//  the music in the form of an integer
// the other overided SetMusic function allows the caller to
//  specify the msuci the form of a string
void GluSetMusic(bool loop,WORD music_resource);
void GluSetMusic(bool loop,const TCHAR *music_resource);

// this will return a random spot in the level (for mp games)
//  that a player can start in without being trapped from everyone else
void GluGetRandomStartingSpot(POINT&);

// this function takes as parameter a reference to the movement
//  plans of a character.  If the character does not plan to walk through
//  any walls, this function will detect it and leaves the plans unaltered
// otherwise, the function will make it so the character stops walking right
//  before the wall by "filtering" plans.second to a valid destination
void GluFilterMovement(pair<POINT,POINT>& plans);

void GluRestoreSurfaces();
void GluCharPress(TCHAR c);
void GluKeyPress(BYTE scan_code);
void GluPlaySound(int i,FIXEDNUM x_dist=0,FIXEDNUM y_dist=0);
void GluRelease();
bool GluWalkingData(FIXEDNUM x,FIXEDNUM y);
void GluPostSPKilledMessage();
void GluStrLoad(unsigned int id,string& target);
void GluInterpretDirection(BYTE d,FIXEDNUM& xf,FIXEDNUM& yf);
void GluStrVctrLoad(unsigned int id,VCTR_STRING& target);

// this function returns the amount a points a player gets when he picks up weapon type x
int GluScoreDiffPickup(int x);

// this function returns the amount of points a player gets when he kills enemy of type x
int GluScoreDiffKill(int x);

// this is the function that initializes the glue class, and allocates
//  a lot of memory and junk for game data.  It does a lot more work
//  than the actual CGlue constructor.
bool GluInitialize(HINSTANCE hInstance_,HWND hWnd_);
	
// simple accessors
//  these functions give other classes and functions access to variables
//  of this class with a small amount of overhead due to their inline nature
//  the static __inline function is the main accessor which gives all
//  classes access to the CGlue singleton itself

extern int                    GLUdifficulty;
extern int                    GLUchar_demo_direction;
extern CBob                  *GLUbitmaps[];
extern LPDIRECTDRAWCLIPPER    GLUclipper;
extern int                    GLUlevel;
extern VCTR_CHARACTER         GLUenemies; // remote players or local aliens
extern CFire                  GLUfires[MAX_FIRES];
extern CCharacter             GLUhero; // us
extern int                    GLUhero_is_in_way;
extern BYTE                   GLUkeyb[KBBUFFERSIZE];
extern string                 GLUname;
extern vector<CPowerUp>       GLUpowerups;
extern DWORD                  GLUsync_rate;
extern FIXEDNUM               GLUcenter_screen_y;
extern FIXEDNUM               GLUcenter_screen_x;

typedef std::multiset<TCharacterPointer> MSET_PCHAR;
extern MSET_PCHAR GLUdrawing_order;
