#ifndef _29625A40_9AFA_11d4_B6FE_0050040B0541_INCLUDED_
#define _29625A40_9AFA_11d4_B6FE_0050040B0541_INCLUDED_

void NetInitialize();
void NetRelease();

// use this function when the weather state changes.
//  it will return true if you are the host and
//  have the right to change the weather, or it will
//  return false if you are not the host and need to
//  stay in the current state
bool NetSendWeatherStateMessage(int weather_state);

// if you are the host, this function will
//  set the description of the session.
// if you are joining, it will receive
//  host's welcome messages and everybody
//  else's welcome messages, and you send
//  out your welcome message
void NetWelcome(DPID new_player = 0);

// these functions define this class's interface with the
//  rest of the code
// returns an ordered list of the protocols that can be used to pass to InitiateProtocol member
const VCTR_STRING& NetProtocols(); 

// the only way this function can fail is if you give an out-of-range index
//  or you already have a protocol started.  If you do either, it fails
//  an assertion
int NetInitializeProtocol(DWORD index);

// releases any currently used protocol
//  fails assertion if no protocol is available
void NetReleaseProtocol();

// returns true if a protocol is initiated
bool NetProtocolInitialized();

// this function JOINS an already playing game
//  return codes:
// JOINGAME_FAILURE
// 0-based index of the level
//
// fails assertion if invalid index is given or no
//  protocol is used.  Gets player name from CGlue accessor
int NetJoinGame(int index);

// this function CREATES a game for an index
//  return codes:
// CREATEGAME_FAILURE
// CREATEGAME_SUCCESS
// fails assertion if invalid index is given,
//  non-positive sync rate is given, invalid
//  level is given, or no protocol is initiated
//  level,syncrate, and name are retrieved by
//  calling accessors of the CGlue class
int NetCreateGame(int index);

// leaves a currently open game
//  fails assertion if no game is loaded
void NetLeaveGame();

// returns true if we are playing a game
bool NetInGame();

// should be called every now and then
//  fails assertion if not in  a game
// returns true if the game was just left
//  and the protocol was just released due
//  to the fact that ya just had to leave
// false is the okay alarm!
bool NetCheckForSystemMessages();

// performs logic for remote players, returns false if you should be using AI
//  like in a single-player game
bool NetRemoteLogic(); 

// called by hero, don't have to be in game, pass coordinates of explosion
void NetSendBazookaFireMessage(FIXEDNUM x,FIXEDNUM y);

// stop firing (let go of spacebar), don't have to be in game
void NetSendMachineGunFireStopMessage();

// start firing (pressing spacebar), don't have to be in game
void NetSendMachineGunFireStartMessage();

// changing weapons, don't have to be in game to call
void NetSendWeaponChangeMessage(int type);

// just lost all health, must be in game to call
void NetSendDeathMessage(FIXEDNUM *ammo);

// call if being hurt by anything
//  not each frame, just when you swear
//  don't have to be in game to call
void NetSendAdmitHitMessage();

// call if hit somebody with machine gun or pistol (bazookas don't count)
//  pass amount of damage you should be dealing.  Player will respond with
//  AdmitHit message sent to all players.  Pass index of remote player
//  don't have to be in game to call.  Will call the subtract health
//  function of corresponding enemy if you are playing a single
//  player game
void NetSendHitMessage(int player,int weapon_type);

// call if you should be picking up a powerup
//  Can be "picked up" and marked as
//  used right after/before sending the message
//  do not have to be in game to call
void NetSendPowerUpPickUpMessage(int powerup_index);

// call if you fired a pistol.  As a visual cue
//  on other computers, a pistol bullet will be
//  rendered as closely as possible to the one 
//  fired on your screen.  Don't have to be in
//  game to call
void NetSendPistolFireMessage(int direction);

// called by the Glue module when the level is loaded.  By setting our
// data, we confirm that we are ready to start receiving messages and
// ready to play
void NetSetPlayerData();

#endif
