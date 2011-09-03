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

/** \file Net.cpp
 * Contains variables and functions that are internal to the Net
 * module, which facilitates communication between Andradion 2
 * computers over a network. These are the messages that can be sent
 * over the network:
 * <p>&nbsp;</p>
 * <table border="0">
 * <tr valign="top">
 *  <td><h4>Sync message</h4></td>
 *  <td><p>Sent every couple of frames indicating to the other
 *  players:</p>
 *   <ul>
 *    <li>The x- and y-coordinates of the player. (X and Y)</li>
 *    <li>The direction the player is facing. (D)</li>
 *    <li>The weapon the player is holding. (W)</li>
 *    <li>Whether or not the player is moving. (M)</li>
 *    <li>Whether or not the player is firing the machine gun. (F)</li>
 *   </ul>
 *  <p>The message is <strong>4 bytes long</strong>, and its format
 *  is: <tt>0FMWWDDD XXXXXXXX XXXXYYYY YYYYYYYY</tt> and the byte
 *  order is little-endian</p></td> 
 * </tr>
 * <tr><td colspan="2"><hr></td></tr>
 * <tr valign="top">
 *  <td><h4>Start moving message</h4></td>
 *  <td><p>Indicates that the player has begun to move from a standing
 *  position or has changed direction. Included in the message is the
 *  direction in which the player is moving (D).</p>
 *  <p>The message is <strong>1 byte long</strong>, and its format is:
 *  <tt>00000DDD</tt></p></td>
 * </tr>
 * <tr><td colspan="2"><hr></td></tr>
 * <tr valign="top">
 *  <td><h4>Stop moving message</h4></td>
 *  <td><p>Indicates that the player has stopped moving and was
 *  previously walking.</p>
 *  <p>The message is <strong>1 byte long</strong>, and its format is:
 *  <tt>10000000</tt></p></td>
 * </tr>
 * <tr><td colspan="2"><hr></td></tr>
 * <tr valign="top">
 *  <td><h4>Start firing machine gun message</h4></td>
 *  <td><p>Indicates that the player has begun to fire the machine
 *  gun. The machine gun should be firing until a Stop firing machine
 *  gun message is sent. Included in the message is the direction in
 *  which the player is facing (D).</p>
 *  <p>The message is <strong>1 byte long</strong>, and its format is:
 *  <tt>01000DDD</tt></p></td>
 * </tr>
 * <tr><td colspan="2"><hr></td></tr>
 * <tr valign="top">
 *  <td><h4>Stop firing machine gun message</h4></td>
 *  <td><p>Indicates that the player has stopped firing the machine
 *  gun.</p>
 *  <p>The message is <strong>1 byte long</strong>, and its format is:
 *  <tt>11000000</tt></p></td>
 * </tr>
 * <tr><td colspan="2"><hr></td></tr>
 * <tr valign="top">
 *  <td><h4>Fire pistol message</h4></td>
 *  <td><p>Indicates that the player has just fired the
 *  pistol. Included in the message is the direction in which the
 *  player is facing (D).</p>
 *  <p>The message is <strong>1 byte long</strong>, and its format is:
 *  <tt>00100DDD</tt></p>
 * </tr>
 * <tr><td colspan="2"><hr></td></tr>
 * <tr valign="top">
 *  <td><h4>Fire bazooka message</h4></td>
 *  <td><p>Indicates that the player has just fired the
 *  bazooka. Included in the message is the point at which the bazooka
 *  bullet stopped and exploded (X and Y).</p>
 *  <p>The message is <strong>3 bytes long</strong>, and its format is:
 *  <tt>XXXXXXXX XXXXYYYY YYYYYYYY</tt></p></td>
 * </tr>
 * <tr><td colspan="2"><hr></td></tr>
 * <tr valign="top">
 *  <td><h4>Change weapon message</h4></td>
 *  <td><p>Indicates that the player just changed his weapon. Included
 *  in the message is the weapon type (W).</p>
 *  <p>The message is <strong>1 byte long</strong>, and its format is:
 *  <tt>101000WW</tt></p></td>
 * </tr>
 * <tr><td colspan="2"><hr></td></tr>
 * <tr valign="top">
 *  <td><h4>Died message</h4></td>
 *  <td><p>Indicates that the player has died and left some ammo in
 *  his place. Included in the message is the:</p>
 *   <ul>
 *    <li>amount of pistol ammo he was carrying (P)</li>
 *    <li>amount of machine gun ammo he was carrying (M)</li>
 *    <li>amount of bazooka ammo he was carrying (B)</li>
 *   </ul>
 *  <p>The message is <strong>6 bytes long</strong>, and its format
 *  is: <tt>PPPPPPPP PPPPPPPP MMMMMMMM MMMMMMMM BBBBBBBB
 *  BBBBBBBB</tt> and the byte order is little-endian.</p></td>
 * </tr>
 * <tr><td colspan="2"><hr></td></tr>
 * <tr valign="top">
 *  <td><h4>Admit hit message</h4></td>
 *  <td><p>Indicates that the player is in pain and has just "sworn." 
 *  Swearing is a sound that lets the player know some character is
 *  hurt.</p>
 *  <p>The message is <strong>1 byte long</strong>, and its format is:
 *  <tt>01100000</tt></p></td>
 * </tr>
 * <tr><td colspan="2"><hr></td></tr>
 * <tr valign="top">
 *  <td><h4>Pick up power up message</h4></td>
 *  <td><p>Indicates that the player has picked up a power
 *  up. Included in the message is the index of the powerup (P).</p>
 *  <p>The message is <strong>2 bytes long</strong>, and its format is:
 *  <tt>PPPPPPPP PPPPPPPP</tt> and the byte order is
 *  little-endian</p></td> 
 * </tr>
 * <tr><td colspan="2"><hr></td></tr>
 * <tr valign="top">
 *  <td><h4>Change weather message</h4></td>
 *  <td><p>Indicates that the host has issued a new weather
 *  state. Included in the message is the number indicating the
 *  weather state (W).</p>
 *  <p>The message is <strong>1 byte long</strong>, and its format is:
 * <tt>1110WWWW</tt></p></td>
 * </tr>
 * <tr><td colspan="2"><hr></td></tr>
 * <tr valign="top">
 *  <td><h4>Hit message</h4></td>
 *  <td><p>Indicates that this computer determined the player on it
 *  has hit another player with a machine gun or pistol bullet. This
 *  is the only message that is directed at a particular player,
 *  instead of everyone in the session. Included in the message is the
 *  type of bullet we hit with, given by the weapon type value
 *  (W).</p>
 *  <p>The message is <strong>1 byte long</strong>, and its format is:
 *  <tt>00010WWW</tt></p></td></tr>
 * 
 *  
 * 
 * </table>
 */
#include "StdAfx.h"
#include "Logger.h"
#include "Buffer.h"
#include "Net.h"

using std::pair;
using std::string;
using std::bad_alloc;
using std::queue;
using std::auto_ptr;
using std::vector;

class CreatePlayerFailure : public std::exception {
public:
  virtual const char *what() const throw() {
    return "Unable to create the player for this computer in the Net game";
  }
};

class PlayerNotFoundException : public std::exception {
public:
  virtual const char *what() const throw() {
    return "The player of the given ID is no longer in the game";
  }
};

enum NETSTATE {NOTINIT, JOINING, HOSTING, NOGAME};

struct REMOTEPLAYER {
  REMOTEPLAYER(DPID i, const char *n) throw()
    : id(i), name(n), firing(false), walking(false) {}
  DPID id;
  bool firing, walking;
  string name;
};

static IDirectPlay4A *dp = 0;
static NETSTATE state = NOTINIT;
static NetFeedback *feedback = 0;
static vector<pair<string, Buffer> > protocols;
static DWORD sync_rate, sync;
static vector<REMOTEPLAYER> remotes; 
static DPID us;
static unsigned int room_index;

// Information stored away after calls to the Net*** functions
static unsigned int weather, direction, weapon;
static unsigned short x_pos, y_pos;
static bool firing_machine_gun, walking;

// message types are listed in Message Types.doc

const int MAX_PLAYERS = 16;

const unsigned int MSGSIZE_SYNC = 4;
const unsigned int MSGSIZE_FIREBAZOOKA = 3;
const unsigned int MSGSIZE_DIED = 6;
const unsigned int MSGSIZE_PICKUPPOWERUP = 2;
const BYTE MSGTYPE_BITS = 0xf0;
const BYTE MSGTYPE_STARTMOVING = 0x00;
const BYTE MSGTYPE_STOPMOVING = 0x80;
const BYTE MSGTYPE_STARTFIRINGMACHINEGUN = 0x40;
const BYTE MSGTYPE_STOPFIRINGMACHINEGUN = 0xc0;
const BYTE MSGTYPE_FIREPISTOL = 0x20;
const BYTE MSGTYPE_CHANGEWEAPON = 0xa0;
const BYTE MSGTYPE_ADMITHIT = 0x60;
const BYTE MSGTYPE_CHANGEWEATHER = 0xe0;
const BYTE MSGTYPE_HIT = 0x10;

const unsigned int DEFAULT_INITIAL_BUFFER_SIZE = 20;
const DWORD SESSION_FLAGS = DPSESSION_DIRECTPLAYPROTOCOL
      | DPSESSION_KEEPALIVE | DPSESSION_MIGRATEHOST
      | DPSESSION_NOPRESERVEORDER | DPSESSION_OPTIMIZELATENCY;

static inline void SendSimpleMsg(BYTE data,
                                 bool guaranteed) throw() {
  assert(NetInGame());

  DWORD flags = guaranteed
    ? DPSEND_ASYNC | DPSEND_NOSENDCOMPLETEMSG | DPSEND_GUARANTEED
    : DPSEND_ASYNC | DPSEND_NOSENDCOMPLETEMSG;
  
  dp->SendEx(us, DPID_ALLPLAYERS, flags, &data, 1, 0, 0, 0, 0);

  WriteLog("Sent a simple message (1-byte of data to all players), "
	   "containing number: %x\n" LogArg((int)data));
}

static inline bool ValidCoordinate(const unsigned short c) {
  return !(c & ~0xfff);
}

static inline bool ValidDirection(const unsigned int d) {
  return !(d & ~7);
}

static inline bool ValidWeapon(const unsigned int w) {
  return !(w & ~3);
}

static inline bool ValidWeatherState(const unsigned int w) {
  return !(w & ~0x0f);
}

static inline void SendSyncMsg(const unsigned short x,
                               const unsigned short y,
                               const unsigned int direction,
                               const unsigned int weapon,
                               const bool walking,
                               const bool firing_machine_gun) throw() {
  DWORD data;

  assert(ValidCoordinate(x) && ValidCoordinate(y));
  assert(ValidDirection(direction));
  assert(ValidWeapon(weapon));
  assert(NetInGame());

  data = firing_machine_gun ? 1 : 0;
  data <<= 1;
  data |= walking ? 1 : 0;
  data <<= 2;
  data |= weapon;
  data <<= 3;
  data |= direction;
  data <<= 12;
  data |= x;
  data <<= 12;
  data |= y;

  dp->SendEx(us, DPID_ALLPLAYERS,
             DPSEND_ASYNC | DPSEND_NOSENDCOMPLETEMSG | DPSEND_GUARANTEED,
             &data, MSGSIZE_SYNC, 0, 0, 0, 0);
}

// {CF58E020-9BD3-11d4-B6FE-0050040B0541}
static const GUID ANDRADION2GUID = 
  { 0xcf58e020, 0x9bd3, 0x11d4, { 0xb6, 0xfe, 0x0, 0x50, 0x4, 0xb,
                                  0x5, 0x41 } };

static void CreatePlayer(unsigned int model, const char *name)
  throw(CreatePlayerFailure) {
  DPNAME our_name;
  BYTE player_data = (BYTE)model; 
  our_name.dwSize = sizeof(our_name);
  our_name.dwFlags = 0;
  our_name.lpszLongNameA = our_name.lpszShortNameA
    = const_cast<char *>(name);
	
  if(FAILED(TryAndReport(dp->CreatePlayer(&us, &our_name, 0,
                                          &player_data, 1, 0)))) {
    WriteLog("Call to DirectPlay.CreatePlayer failed\n");
    throw CreatePlayerFailure();
  }

  WriteLog("Successfully created our player\n");
}

static BOOL FAR PASCAL
FoundProtocol(LPCGUID, LPVOID lpConnection,
              DWORD dwConnectionSize, LPCDPNAME lpName,
              DWORD, LPVOID context) {
  BOOL *mem_error = (BOOL *)context;
  
//   WriteLog("Enumeration of protocols callback called for %s "
//            "with connection size of %d\n"
// 	   LogArg(lpName->lpszShortNameA)
//            LogArg(dwConnectionSize));

//   // make sure we can create this protocol
//   IDirectPlay4A *dp;
//   if(FAILED(CoCreateInstance(CLSID_DirectPlay, NULL, CLSCTX_ALL,
//                              IID_IDirectPlay4A, (void**)&dp))) {
//     return FALSE; // stop enumeration
//   }

//   WriteLog("Try to initiate it for ourselves...\n");
//   if(FAILED(dp->InitializeConnection(lpConnection, 0))) {
//     WriteLog("Didn't work, returning to get on with the enumeration\n");
//     TryAndReport(dp->Release());
//     return TRUE;
//   }

//   WriteLog("It worked, recording connection data and protocol "
//            "name, etc...\n");

//   TryAndReport(dp->Release());

  try {
    protocols.push_back(pair<string, Buffer>(string(lpName->lpszShortNameA),
                                             Buffer(lpConnection,
                                                    dwConnectionSize)));
  } catch (bad_alloc& ba) {
    *mem_error = TRUE;
    return FALSE;
  }
  
  WriteLog("Successfully enum'd %s\n" LogArg(lpName->lpszShortNameA));

  return TRUE;
}

static BOOL FAR PASCAL
FoundSession(LPCDPSESSIONDESC2 lpThisSD,
             LPDWORD, DWORD dwFlags, LPVOID context) {
  int *const communicated = (int *)context;

  if(DPESC_TIMEDOUT == dwFlags) {
    WriteLog("Session enumeration timed out\n");
    return FALSE; 
  }

  assert (NULL != context && NULL != lpThisSD);

  // make sure we have right session name
  if('a' + char(*communicated) != lpThisSD->lpszSessionNameA[0]) {
    WriteLog("Enumerated session of name %s; were looking for %c\n"
	     LogArg(lpThisSD->lpszSessionNameA)
	     LogArg('a' + char(*communicated)));
    return TRUE;
  }

  // we found a session with the right everything, including name,
  //  so let's try and open it right here
  DPSESSIONDESC2 sd_to_join;
  memset((void *)&sd_to_join, 0, sizeof(sd_to_join));
  sd_to_join.dwSize = sizeof(sd_to_join);
  sd_to_join.guidInstance = lpThisSD->guidInstance;
  if(FAILED(TryAndReport(dp->Open(&sd_to_join, DPOPEN_JOIN)))) {
    return FALSE;
  }

  // get session description
  DWORD sd_size = 0;
  TryAndReport(dp->GetSessionDesc(NULL, &sd_size));
  Buffer sd_buffer(sd_size);
  assert(sd_size >= sizeof(DPSESSIONDESC2));
  DPSESSIONDESC2 *sd = (DPSESSIONDESC2 *)sd_buffer.Get();

  if(0 == sd_size ||
     FAILED(TryAndReport(dp->GetSessionDesc(sd, &sd_size)))) {
    TryAndReport(dp->Close());
    return FALSE;
  }

  // we got in!
  *communicated = (int)sd->dwUser1; // get level index
  sync_rate = sd->dwUser2;
  sync = 0;
  state = JOINING;

  WriteLog("Successfully enumerated the game we wanted to join\n");

  return FALSE;
}

static BOOL FAR PASCAL
FoundPlayer(DPID dpId, DWORD, LPCDPNAME lpName, DWORD, LPVOID) {
  BYTE m;
  DWORD size = 1;
  
  WriteLog("Enumeration of players callback called for player %s\n"
           LogArg(lpName));
	
  if(FAILED(dp->GetPlayerData(dpId, &m, &size, DPGET_REMOTE))) {
    WriteLog("Failed to get player data, skipping enumeration of this "
             "participant\n");
    return TRUE;
  }
  WriteLog("Player uses model %d\n" LogArg((int)m));
  
  feedback->CreateEnemy(m);
  remotes.push_back(REMOTEPLAYER(dpId, lpName->lpszShortNameA));
  
  WriteLog("Finished enumerating %s, callback is returning\n"
           LogArg(lpName->lpszShortName));

  return TRUE;
}

static void RecountPlayers() throw(NetSessionLost, bad_alloc) {
  // enumerate players getting the player data of them and therefore
  //  their model indices
  remotes.clear();
  feedback->ClearEnemyArray();

  WriteLog("Calling EnumPlayers to enumerate all "
           "players and get their models\n");
  if (FAILED(TryAndReport(dp->EnumPlayers(0, FoundPlayer,
                                          0, DPENUMPLAYERS_REMOTE)))) {
    NetLeaveGame();
    throw NetSessionLost();
  }
}

static void ProcessSystemMessage(DPMSG_GENERIC *msg)
  throw(NetSessionLost, bad_alloc) {
  switch(msg->dwType) {
  case DPSYS_HOST: {
    WriteLog("System mesage: we are new host\n");
    state = HOSTING;
    break;
  }
  case DPSYS_DESTROYPLAYERORGROUP: {
    WriteLog("System message: player has left\n");
    DPMSG_DESTROYPLAYERORGROUP *p_msg = (DPMSG_DESTROYPLAYERORGROUP *)msg;
	
    feedback->PlayerLeaving(p_msg->dpnName.lpszShortNameA);

    RecountPlayers();
    break;
  }
  case DPSYS_SESSIONLOST: {
    WriteLog("System message: session lost\n");
    throw NetSessionLost();
  }
  case DPSYS_SETPLAYERORGROUPDATA: {
    WriteLog("System message: set player data\n");

    DPMSG_SETPLAYERORGROUPDATA *p_msg = (DPMSG_SETPLAYERORGROUPDATA *)msg;
    
    if(us != p_msg->dpId) {
      WriteLog("Telling the new player what the weather is like\n");
      NetChangeWeather(weather);
      RecountPlayers();
    }
    break;
  }
  case DPSYS_CREATEPLAYERORGROUP: {
    WriteLog("System message: player has joined\n");

    DPMSG_CREATEPLAYERORGROUP *p_msg = (DPMSG_CREATEPLAYERORGROUP *)msg;

    WriteLog("Posting player join message to screen\n");
    feedback->PlayerJoining(p_msg->dpnName.lpszShortNameA);
  }
    // there are other system messages; we don't care about them
  }
}

static unsigned int FindPlayerIndex(DPID id)
  throw (PlayerNotFoundException) {
  for (int times_tried = 0; times_tried < 2; times_tried++) {
    unsigned int i;
    
    for (i = 0; i < remotes.size() && id != remotes[i].id; i++)
      ;

    if (remotes.size() == i) {
      WriteLog("Didn't find a matching player, so "
               "enuming the players again\n");
      RecountPlayers();
    } else {
      return i;
    }
  }

  WriteLog("The player with the given ID has left\n");
  throw PlayerNotFoundException();
}

void NetInitialize() throw(std::bad_alloc) {
  IDirectPlay4A *dp;
  
  WriteLog("NetInitialize() called\n");

  if (NOTINIT != state) {
    return;
  }

  if(FAILED(CoInitialize(NULL))) {
    WriteLog("CoInitialize() Failed, no protos will be init'd\n");
    throw bad_alloc();
  }

  // get a list of all the protocols
  WriteLog("Creating temporary DirectPlay interface "
           "to enum protos\n");
  if(SUCCEEDED(CoCreateInstance(CLSID_DirectPlay, NULL, CLSCTX_ALL,
                                IID_IDirectPlay4A, (VOID**)&dp))) {
    BOOL mem_error = FALSE;
    WriteLog("DirectPlay created!  Now calling enum method\n");
    TryAndReport(dp->EnumConnections(&ANDRADION2GUID, FoundProtocol,
                                     &mem_error, DPCONNECTION_DIRECTPLAY));
    TryAndReport(dp->Release());

    if (mem_error) {
      throw bad_alloc();
    }
  }
	
  state = NOGAME;
  
  WriteLog("Now leaving NetInitialize()\n");
}

void NetRelease() throw() {
  if (NOTINIT != state) {
    WriteLog("Releasing...\n");
    NetReleaseProtocol();

    CoUninitialize();
    protocols.clear();

    state = NOTINIT;
  }

  WriteLog("Now leaving NetRelease()\n");
}

const char *NetProtocolName(unsigned int protocol_index) throw() {
  assert(protocol_index < protocols.size());
  return protocols[protocol_index].first.c_str();
}

unsigned int NetProtocolCount() throw() {
  assert(NOTINIT != state);
  return protocols.size();
}

void NetInitializeProtocol(unsigned int index) throw() {
  assert(index < protocols.size());
  assert(NOTINIT != state);

  NetReleaseProtocol();
  
  WriteLog("NetInitializeProtocol() called to start protocol "
           "#%d\n" LogArg(index));

  TryAndReport(CoCreateInstance(CLSID_DirectPlay, 0, CLSCTX_ALL,
                                IID_IDirectPlay4A, (void **)&dp));
  TryAndReport(dp->InitializeConnection(protocols[index].second.Get(), 0));
	
  WriteLog("NetInitializeProtocol() finished\n");

  assert(NetProtocolInitialized());
}

void NetReleaseProtocol() throw() {
  WriteLog("NetReleaseProtocol() was called, releasing protocol\n");
  
  if (NetProtocolInitialized()) {
    NetLeaveGame();

    TryAndReport(dp->Release());
  
    dp = 0;
    state = NOGAME;
  }
  
  WriteLog("NetReleaseProtocol() finished, now leaving\n");

  assert(!NetInGame() && !NetProtocolInitialized());
}

bool NetProtocolInitialized() throw() {return bool(dp);}

void NetCreateGame(unsigned int index, unsigned int sr,
                   unsigned int initial_weather_state,
                   unsigned int player_model, const char *player_name,
                   auto_ptr<NetFeedback> fb)
  throw(NetCreateFailure) {
  try {
    assert(NetProtocolInitialized());
    NetLeaveGame();

    WriteLog("NetCreateGame called to join room %d\n" LogArg(index));

    DPSESSIONDESC2 sd;
    memset((void *)&sd, 0, sizeof(sd));
    sd.dwSize = sizeof(sd);
    sd.dwFlags = SESSION_FLAGS | DPSESSION_JOINDISABLED;
    sd.guidApplication = ANDRADION2GUID;
    sd.dwMaxPlayers = MAX_PLAYERS;
	
    // calculate session name 
    char session_name[2] = {'a', 0};
    session_name[0] += (char)index;
    room_index = index;
    sd.lpszSessionNameA = session_name;
	
    sd.dwUser1 = 0xffffffff; // we don't know the level yet
    sd.dwUser2 = sync_rate = sr;
    sync = 0;

    WriteLog("Calling DirectPlay::Open\n");
    if(FAILED(TryAndReport(dp->Open(&sd, DPOPEN_CREATE)))) {
      WriteLog("Failed to create game session; leaving NetCreateGame\n");
      throw NetCreateFailure();
    }

    WriteLog("Calling Net module's CreatePlayer\n");
    CreatePlayer(player_model, player_name);
    WriteLog("Finished creating session and player\n");
    state = HOSTING;
    feedback = fb.release();
    feedback->ClearEnemyArray();

    assert(NetInGame() && NetIsHost());
  } catch (CreatePlayerFailure& cpf) {
    WriteLog("Failed to create player; leaving NetCreateGame\n");
    dp->Close();
    throw NetCreateFailure();
  }

}

void NetSetLevelIndex(unsigned int index) throw(NetSessionLost) {
  if (NetIsHost()) {
    DPSESSIONDESC2 sd;
    char session_name[2] = {'a', 0};
    
    memset(&sd, 0, sizeof(sd));
    sd.dwSize = sizeof(sd);
    sd.dwFlags = SESSION_FLAGS;
    session_name[0] += room_index;
    sd.dwMaxPlayers = MAX_PLAYERS;
    sd.lpszSessionNameA = session_name;
    sd.dwUser1 = index;
    sd.dwUser2 = sync_rate;
    sd.guidApplication = ANDRADION2GUID;

    if (FAILED(dp->SetSessionDesc(&sd, 0))) {
      throw NetSessionLost();
    }
  }
}

bool NetInGame() throw() {return NOGAME != state && NOTINIT != state;}

bool NetIsHost() throw() {return HOSTING == state;}

unsigned int NetJoinGame(unsigned int index, unsigned int player_model,
                         const char *player_name, auto_ptr<NetFeedback> fb)
  throw(NetJoinFailure) {
  try {
    DPSESSIONDESC2 session;

    assert(NetProtocolInitialized());
    NetLeaveGame();
  
    // create an enumeration filter so we only get what we want
    memset((void *)&session, 0, sizeof(session));
    session.dwSize = sizeof(session);
    session.guidApplication = ANDRADION2GUID;

    room_index = index;

    TryAndReport(dp->EnumSessions(&session, 0,
                                  FoundSession, (LPVOID)&index,
                                  DPENUMSESSIONS_AVAILABLE));

    if(JOINING != state) {
      WriteLog("Enumeration did not change state\n");
      throw NetJoinFailure();
    }

    WriteLog("Trying to create our player\n");
    CreatePlayer(player_model, player_name);

    WriteLog("Clearing enemy data on local machine\n");
    feedback = fb.release();
    feedback->ClearEnemyArray();

    WriteLog("Successfully Joined game\n");
    assert(NetInGame() && !NetIsHost());
    
    return index;
  } catch (CreatePlayerFailure& cpf) {
    dp->Close();
    state = NOGAME;
    throw NetJoinFailure();
  }
}

void NetLeaveGame() throw() {
  if (NetInGame()) {
    dp->Close();
    state = NOGAME;
    delete feedback;
    feedback = 0;
  }
}

void NetLogic() throw(NetSessionLost, std::bad_alloc) {
  if(!NetInGame()) {
    return;
  }

  // this loop checks for messages we may have
  Buffer msg_buffer(DEFAULT_INITIAL_BUFFER_SIZE);

  while(true) {
    DPID from, to;
    DWORD data_size = msg_buffer.Size();
    bool has_message;

    WriteLog("Checking for Net message\n");
      
    switch(dp->Receive(&from, &to, DPRECEIVE_ALL,
                       msg_buffer.Get(), &data_size)) {
    case DPERR_BUFFERTOOSMALL:
      WriteLog("Reallocating %d bytes\n" LogArg(data_size));
      msg_buffer.Reallocate(data_size);
      continue;
    case DP_OK:
      WriteLog("Message found\n");
      has_message = true;
      break;
    case DPERR_NOMESSAGES:
      WriteLog("No more messages\n");
      has_message = false;
      break;
    default:
      WriteLog("Severe error\n");
      NetLeaveGame();
      throw NetSessionLost();
    }

    if (!has_message) {
      break;
    }

    if (DPID_SYSMSG == from) {
      ProcessSystemMessage((DPMSG_GENERIC *)msg_buffer.Get());
      continue;
    }

    try {
      switch (data_size) {
      case MSGSIZE_PICKUPPOWERUP:
        WriteLog("Power up was picked up...\n");
        feedback->PickUpPowerUp(*(WORD *)msg_buffer.Get());
        break;
      case MSGSIZE_SYNC: {
        unsigned int player = FindPlayerIndex(from);
        DWORD d = *(DWORD *)msg_buffer.Get();
        WORD x = WORD((d >> 12) & 0x0fff);
        WORD y = WORD(d & 0x0fff);
        unsigned int weapon = (d >> 27) & 3;
        unsigned int direction = (d >> 24) & 7;

        WriteLog("Got a sync message; processing...\n");

        feedback->SetEnemyPosition(player, x, y);
        feedback->SetEnemyWeapon(player, weapon);
        feedback->SetEnemyDirection(player, direction);

        remotes[player].firing = bool(d & 0x40000000);
        remotes[player].walking = bool(d & 0x20000000);

        WriteLog("Finished processing message: sync message\n");
        break;
      }
      case MSGSIZE_DIED: {
        unsigned int player = FindPlayerIndex(from);
        WriteLog("We got a death msg, someone died, processing...\n");
        feedback->KillEnemy(player, (WORD *)msg_buffer.Get());
        break;
      }
      case MSGSIZE_FIREBAZOOKA: {
        unsigned int player = FindPlayerIndex(from);
        BYTE *as_bytes = (BYTE *)msg_buffer.Get();
        WriteLog("Got a bazooka fired message; processing\n");
        DWORD x = ((as_bytes[1] >> 4) & 0x0f) | ((DWORD)as_bytes[2] << 4);
        DWORD y = (DWORD)as_bytes[0] | ((DWORD)(as_bytes[1] & 0x0f) << 8);
        feedback->EnemyFiresBazooka(player, x, y);
        WriteLog("Finished processing message: bazooka fire message\n");
        break;
      }
      case 1: {
        BYTE type = *(BYTE *)msg_buffer.Get();
        BYTE data = type & ~MSGTYPE_BITS;
        unsigned int player = FindPlayerIndex(from);
        type &= MSGTYPE_BITS;
      
        switch (type) {
        case MSGTYPE_CHANGEWEATHER:
          WriteLog("Weather state was changed...\n");
          feedback->SetWeatherState(data);
          break;
        case MSGTYPE_CHANGEWEAPON: 
          WriteLog("Weapon change message; processing\n");
          feedback->SetEnemyWeapon(player, data);
          break;
        case MSGTYPE_FIREPISTOL: 
          WriteLog("Pistol fire message\n");
          feedback->SetEnemyDirection(player, data);
          feedback->EnemyFiresPistol(player);
          break;
        case MSGTYPE_ADMITHIT: 
          WriteLog("Admit hit message\n");
          feedback->HurtEnemy(player);
          break;
        case MSGTYPE_STARTFIRINGMACHINEGUN: 
          WriteLog("Start firing machine gun message\n");
          remotes[player].firing = true;
          feedback->SetEnemyDirection(player, data);
          break;
        case MSGTYPE_STOPFIRINGMACHINEGUN:
          WriteLog("Stop firing machine gun message\n");
          remotes[player].firing = false;
          break;
        case MSGTYPE_HIT:
          WriteLog("[I] Hit [you] message\n");
          feedback->HurtHero(data);
          break;
        case MSGTYPE_STARTMOVING:
          remotes[player].walking = true;
          feedback->SetEnemyDirection(player, data);
          break;
        case MSGTYPE_STOPMOVING:
          remotes[player].walking = false;
          break;
        default:
          WriteLog("UNKNOWN 1-BYTE MESSAGE OF TYPE %d\n" LogArg(type));
        }
      
        break;
      }
      default:
        WriteLog("UNKNOWN MESSAGE OF SIZE %d\n" LogArg(data_size));
      }
    } catch (PlayerNotFoundException& pnfe) {
      WriteLog("A message was sent by an absent player\n");
      // ignoring this message
    }
  }

  for(unsigned int i = 0; i < remotes.size(); i++) {
    if (remotes[i].walking) {
      feedback->WalkEnemy(i);
    }

    if (remotes[i].firing) {
      feedback->EnemyFiresMachineGun(i);
    }
  }

  if (++sync >= sync_rate) {
    sync = 0;

    SendSyncMsg(x_pos, y_pos, direction, weapon,
                walking, firing_machine_gun);
  }
}

void NetFireBazooka(unsigned short x, unsigned short y) throw() {
  assert(ValidCoordinate(x) && ValidCoordinate(y));
  
  if (NetInGame()) {
    BYTE data[MSGSIZE_FIREBAZOOKA];

    data[0] = BYTE(x >> 4);
  
    data[1] = BYTE(x & 0x0f);
    data[1] <<= 4;
    data[1] |= BYTE(y >> 8);
  
    data[2] = BYTE(y & 0xff);

    dp->SendEx(us, DPID_ALLPLAYERS,
               DPSEND_ASYNC | DPSEND_NOSENDCOMPLETEMSG | DPSEND_GUARANTEED,
               data, MSGSIZE_FIREBAZOOKA,
               0, 0, 0, 0);
  }
}

void NetFireMachineGun(bool firing) throw() {
  if (NetInGame()) {
    if (firing_machine_gun) {
      if (!firing) {
        SendSimpleMsg(MSGTYPE_STOPFIRINGMACHINEGUN, false);
      }
    } else if (firing) {
      SendSimpleMsg(MSGTYPE_STARTFIRINGMACHINEGUN | direction, false);
    }
    firing_machine_gun = firing;
  }
}

void NetSetWeapon(unsigned int type) throw() {
  assert(ValidWeapon(weapon));
  
  if (NetInGame() && weapon != type) {
    weapon = type;
    SendSimpleMsg(MSGTYPE_CHANGEWEAPON | weapon, false);
  }
}

void NetDied(WORD *ammo) throw() {
  if (NetInGame()) {
    dp->SendEx(us, DPID_ALLPLAYERS,
               DPSEND_ASYNC | DPSEND_NOSENDCOMPLETEMSG | DPSEND_GUARANTEED,
               ammo, MSGSIZE_DIED, 0, 0, 0, 0);
  }
}

void NetSetPosition(unsigned short new_x,
                    unsigned short new_y,
                    unsigned int new_dir) throw() {
  assert(ValidCoordinate(new_x) && ValidCoordinate(new_y));
  assert(ValidDirection(new_dir));

  if (NetInGame()) {
    if (walking) {
      if (new_dir != direction) {
        SendSimpleMsg(MSGTYPE_STARTMOVING | new_dir, false);
      } else if (new_x == x_pos && new_y == y_pos) {
        walking = false;
        SendSimpleMsg(MSGTYPE_STOPMOVING, false);
      }
    } else if (new_x != x_pos || new_y != y_pos) {
      walking = true;
      SendSimpleMsg(MSGTYPE_STARTMOVING | new_dir, false);
    }

    x_pos = new_x;
    y_pos = new_y;
    direction = new_dir;
  }
}

void NetAdmitHit() throw() {
  if (NetInGame()) {
    SendSimpleMsg(MSGTYPE_ADMITHIT, true);
  }
}

void NetHit(unsigned int player,
            unsigned int weapon_type) throw() {
  assert(ValidWeapon(weapon_type));

  if (NetInGame()) {
    DPID to = remotes[player].id;
    BYTE data = weapon_type | MSGTYPE_HIT;

    dp->SendEx
      (us, to, DPSEND_ASYNC | DPSEND_NOSENDCOMPLETEMSG | DPSEND_GUARANTEED,
       &data, 1, 0, 0, 0, 0);
  }    
}

void NetPickUpPowerUp(unsigned short powerup_index) throw() {
  if (NetInGame()) {
    dp->SendEx(us, DPID_ALLPLAYERS, DPSEND_ASYNC
               | DPSEND_NOSENDCOMPLETEMSG | DPSEND_GUARANTEED,
               &powerup_index, MSGSIZE_PICKUPPOWERUP, 0, 0, 0, 0);
  }
}

void NetFirePistol(unsigned int dir) throw() {
  assert(ValidDirection(dir));
  if (NetInGame()) {
    SendSimpleMsg(MSGTYPE_FIREPISTOL | dir, false);
  }
}

void NetChangeWeather(unsigned int new_weather) throw() {
  assert(ValidWeatherState(new_weather));
  if (NetInGame()) {
    weather = new_weather;
    SendSimpleMsg(MSGTYPE_CHANGEWEATHER | weather, true);
  }
}

