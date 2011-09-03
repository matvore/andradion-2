// module for the CNet singleton

#include "StdAfx.h"
#include "Fixed.h"
#include "SharedConstants.h"
#include "Net.h"
#include "Character.h"
#include "Fire.h"
#include "Glue.h"
#include "Resource.h"
#include "PowerUp.h"
#include "Weather.h"

// if this next line is commented out, then no network protocols will
// be enumerated
#define ENABLEMULTIPLAYER

using NGameLib2::tstring;

enum {NETSTATE_JOINING,NETSTATE_HOSTING,NETSTATE_NOGAME};

// define what we will use for a direct play interface
//  based on if we are using UNICODE
#ifdef _UNICODE
typedef IDirectPlay4 *DPInt;
#define DPLAYREFIID IID_IDirectPlay4
#else
typedef IDirectPlay4A *DPInt;
#define DPLAYREFIID IID_IDirectPlay4A
#endif

struct REMOTEPLAYER
{
  DPID id;

  // location of remote player at end
  //  of sync period
  POINT loc_at_end_of_sync; // stored in fixed-point

  // location of remote player at the beginning
  //  of sync period
  POINT loc_at_start_of_sync; // stored in fixed-point

  // equals true if the guy is firing
  bool firing;

  // name of this guy
  tstring name;

  // timer for the other guy's sync
  DWORD sync;

  // everything else is stored in the
  //  actual CCharacter class
};

static HRESULT CreatePlayer();

// gets a message, returns zero if nothing was available, or returns
// the message size if successful.  May change the data_size parameter
// if the data_buffer had to be resized
static DWORD GetMessage(DPID &from,
			BYTE **data_buffer,
			DWORD &data_size);

// posts a "X has joined" message
static void AnnounceArrival(const TCHAR *name);

// all of our internal members
static DPInt dp; // DPInt is either IDirectPlay4 * or IDirectPlay4A *
static int state;
static VCTR_STRING protocol_names;
static vector<vector<BYTE> > protocol_data;
static DWORD sync_rate;

// timer to count to our sync
static DWORD sync;

static vector<REMOTEPLAYER> remotes; // data for remote players we
				     // need 

// our dp id
static DPID us;

static union
{
  int level; // used to communicate level value through enumeration
	     // function 
  int room_index; // used to communicate room index value to enum
		  // function 
};
	
static queue<DPMSG_GENERIC *> system_messages;

// message types are listed in Message Types.doc

const int MAX_PLAYERS = 16;
const int INITIAL_RECEIVE_BUFFER_SIZE = 6;
const int BAZOOKAFIREMESSAGE_SIZE = 3;
const int BAZOOKAFIREMESSAGE_BITSPERCOORCOMPONENT = 12;
const int SYNCMESSAGE_SIZE = 4;
const int SYNCMESSAGE_BITSPERCOORCOMPONENT = 12;
const int SYNCMESSAGE_BITMASKFORCOORCOMPONENT = 0xfff;
const int SYNCMESSAGE_BITSFORCOORS = 24;
const int SYNCMESSAGE_BITSFORCURRENTWEAPON = 2;
const int SYNCMESSAGE_BITMASKFORCURRENTWEAPON = 0x03;
const int POWERUPPICKUPMESSAGE_SIZE = 2;
const int POWERUPPICKUPMESSAGE_BITSFORPOWERUPINDEX = 8;
const BYTE TOPTWOBITS = 0xc0;
const BYTE TOPTWOBITSMASK_WEATHERSTATEMESSAGE = 0x40;
const BYTE TOPTWOBITSMASK_WEAPONCHANGEMESSAGE = 0x80;
const int WEATHERSTATEMESSAGE_SIZE = 1;
const int DEATHMESSAGE_SIZE = 3;
const BYTE TOPTWOBITSMASK_PISTOLFIREMESSAGE = 0xc0;
const BYTE ONEBYTESIGNAL_ADMITHIT = 0;
const BYTE ONEBYTESIGNAL_MACHINEGUNFIRESTART = 1;
const BYTE ONEBYTESIGNAL_MACHINEGUNFIRESTOP = 2;
const BYTE ONEBYTESIGNAL_CLAIMPISTOLHIT = 3;

// {CF58E020-9BD3-11d4-B6FE-0050040B0541}
static const GUID ANDRADION2GUID = 
  { 0xcf58e020,
    0x9bd3,
    0x11d4,
    { 0xb6, 0xfe, 0x0, 0x50, 0x4, 0xb, 0x5, 0x41 } };

const DWORD SEND_ALLPLAYERS = 0xffffffff;

#ifdef NDEBUG
inline // this function should only be inline in non-debug mode; the
       // debug version of this function is very long
#endif
static void Send(void *data,
		 DWORD size,
		 bool guaranteed = true,
		 DWORD target_player = SEND_ALLPLAYERS)
{
  WriteLog("Send wrapper called");

  TryAndReport
    (dp->SendEx
     (us,
      (target_player == SEND_ALLPLAYERS) ?
      DPID_ALLPLAYERS : remotes[target_player].id,
      (true == guaranteed) ?
      DPSEND_GUARANTEED|DPSEND_ASYNC|DPSEND_NOSENDCOMPLETEMSG:
      DPSEND_ASYNC|DPSEND_NOSENDCOMPLETEMSG,
      (void *)data,
      size,0,0,NULL,NULL)
     );

#ifdef _DEBUG
  // we have to log this sending action
  if(SEND_ALLPLAYERS != target_player)
    {
      WriteLog("   sent a message to player #%d" LogArg(target_player));
    } // end if sent to a single player

  if(true != guaranteed)
    {
      WriteLog("   not sent guaranteed");
    }

  WriteLog("   contents of message: ");

  BYTE *byte_ptr = (BYTE *)data;
  
  for(DWORD i = 0; i < size; i++)
    {
      WriteLog("      byte #%d contains data: %2x"
	       LogArg(i+1) LogArg((DWORD)byte_ptr[i]));
    } // end for each byte in message

  WriteLog("Send wrapper returning");
#endif
}

template <class c>
inline static void Send(c data,
		 bool guaranteed = true,
		 int target_player = SEND_ALLPLAYERS)
{
  Send((void *)&data,sizeof(data),guaranteed,target_player);
}

static BOOL FAR PASCAL EnumConnectionMethodsCallback
(
 LPCGUID,
 LPVOID lpConnection,
 DWORD dwConnectionSize,
 LPCDPNAME lpName,
 DWORD,
 LPVOID
 )
{
  WriteLog("Enumeration of protocals callback called for %s"
	   LogArg(lpName->lpszShortNameA));  

  assert(protocol_names.size() == protocol_data.size());

  // make sure we can create this protocol
  DPInt dp;
  if(FAILED(CoCreateInstance(CLSID_DirectPlay,
			     NULL,
			     CLSCTX_ALL,
			     DPLAYREFIID,
			     (VOID**)&dp)))
    {
      return FALSE; // stop enumeration
    }

  WriteLog("Try to initiate it for ourselves . . .");
  if(FAILED(dp->InitializeConnection(lpConnection,0)))
    {
      WriteLog("Didn't work, returning to get on with the "
	       "enumeration");
      TryAndReport(dp->Release());
      return TRUE;
    }

  WriteLog("It worked, recording connection data and protocol name, "
	   "etc . . .");

  // done with dp
  TryAndReport(dp->Release());

  int magic_index = protocol_names.size();

  // copy protocol name
  protocol_names.resize(magic_index+1);
  protocol_names[magic_index] = lpName->lpszShortNameA;

  // make room for connection data
  protocol_data.resize(magic_index+1);
  protocol_data[magic_index].resize(dwConnectionSize);

  // copy connection data
  memcpy((void *)protocol_data[magic_index].begin(),
	 lpConnection,dwConnectionSize);

  WriteLog("Successfully enumerated %s"
	   LogArg(lpName->lpszShortNameA));

  return TRUE;
}

void NetInitialize()
{
  WriteLog("NetInitialize() called");
  state = NETSTATE_NOGAME;
#ifdef ENABLEMULTIPLAYER
  if(FAILED(CoInitialize(NULL)))
    {
      WriteLog("CoInitialize() Failed, no protocols will be "
	       "initialized");
      return;
    }

  // get a list of all the protocols
  DPInt dp;
  WriteLog("Attempting to create temporary DirectPlay interface to "
	   "enumerate protocols");
  if(SUCCEEDED(CoCreateInstance(CLSID_DirectPlay,
				NULL, CLSCTX_ALL,
				DPLAYREFIID, (VOID**)&dp)))
    {
      WriteLog("DirectPlay created!  Now calling enumeration method");
      TryAndReport(dp->EnumConnections(&ANDRADION2GUID,
				       EnumConnectionMethodsCallback,
				       NULL,DPCONNECTION_DIRECTPLAY));
      TryAndReport(dp->Release());
    }
	
  dp = NULL;

  WriteLog("Done enumerating");

  if(0 == protocol_names.size())
    {
      WriteLog("Didn't get any protocols");
      CoUninitialize();
    }

#endif
  WriteLog("Now leaving NetInitialize()");
}

void NetRelease()
{
  WriteLog("NetRelease called, say goodbye to everybody");
  if(true == NetProtocolInitialized())
    {
      WriteLog("Protocol is currently intialized, checking if still "
	       "in a game");
      if(true == NetInGame())
	{
	  WriteLog("Game is still open, closing now");
	  NetLeaveGame();
	}
      NetReleaseProtocol();
      WriteLog("Protocol and game closed");
    }

  if(0 != protocol_names.size())
    {
      WriteLog("Protocols had been enumerated, now we must called "
	       "CoUninitialize()");
      CoUninitialize();
      WriteLog("CoUnin called");
    }

  WriteLog("Now leaving NetRelease()");
}

static BOOL FAR PASCAL EnumPlayersCB
(
 DPID dpId,
 DWORD /*dwPlayerType*/,
 LPCDPNAME lpName,
 DWORD /*dwFlags*/,
 LPVOID new_player_id
 )
{
  WriteLog("Enumeration of players callback called for player %s"
	   LogArg(lpName));
	
  WriteLog("Trying to find player's model");
  BYTE m;
  DWORD size = 1;
  if(FAILED(dp->GetPlayerData(dpId,(void *)&m,&size,DPGET_REMOTE)))
    {
      WriteLog("Failed to get player data, skipping enumeration of "
	       "this participant");
      return TRUE;
    }
  WriteLog("Player uses model %d" LogArg((int)m));

  assert(GLUenemies.size() == remotes.size());

  int magic_index = remotes.size();

  remotes.resize(magic_index+1);

  WriteLog("Storing name and ID . . .");
  remotes[magic_index].id = dpId;
  remotes[magic_index].name = (TCHAR *)lpName->lpszShortName;

  if(dpId == (DPID)new_player_id)
    {
      WriteLog("Posting message announcing players arrival . . .");
      AnnounceArrival((const TCHAR *)lpName->lpszShortName);
    } // end if we just enumerated a new player

  WriteLog("Building CCharacter object in GLUenemies array . . .");
  GLUenemies.resize(magic_index+1);
  CCharacter *c = &GLUenemies[magic_index];
  c->model = (int)m;
  c->current_weapon = 0;
  c->state = 0;
  c->direction = DSOUTH;
  c->coor.first.x = 0;
  c->coor.first.y = 0;
  c->coor.second = c->coor.first;
	
  WriteLog("Preparing other variables and data");
  remotes[magic_index].loc_at_end_of_sync.x = 0;
  remotes[magic_index].loc_at_end_of_sync.y = 0;
  remotes[magic_index].loc_at_start_of_sync =
    remotes[magic_index].loc_at_end_of_sync;
  remotes[magic_index].firing = false;
  remotes[magic_index].sync = 0;
	
  WriteLog("Finished enumerating Mr. or Ms. %s, callback is returning"
	   LogArg(lpName->lpszShortNameA));
  
  return TRUE;
}

void NetWelcome(DPID new_player)
{
  WriteLog("NetWelcome() called");
  // sets the level of the session desc
  // right before the game
	
  // get size of session description
  if(NETSTATE_HOSTING == state)
    {
      WriteLog("We are host, so we must set the SessionDesc to store "
	       "level (should already contain sync rate)");
      DWORD size = 0;
      dp->GetSessionDesc(NULL,&size);

      // allocate buffer for it
      DPSESSIONDESC2 *sd = (DPSESSIONDESC2 *)new BYTE[size];

      sd->dwSize = sizeof(DPSESSIONDESC2);

      // request desc
      dp->GetSessionDesc((void *)sd,&size);

      WriteLog("SessionDesc obtained, value of User2 (sync_rate) is "
	       "%d" LogArg((int)sd->dwUser2));

      // make necessary changes and apply
      sd->dwUser1 = (DWORD)GLUlevel;
      WriteLog("Value of User1 (level) is %d"
	       LogArg((int)sd->dwUser1));
      dp->SetSessionDesc(sd,0);

      WriteLog("SetSessionDesc completed");

      // delete unneeded buffer
      delete (BYTE *)sd;
    }

  // enumerate players getting the player data of them and therefore
  //  their model indices
  remotes.clear();
  GLUenemies.clear();
      /*WriteLog("Checking message count");
      DWORD message_count = 0;
      if(FAILED(dp->GetMessageCount(us,&message_count)) || 0 == message_count)
	{
	  WriteLog("No messages in queue");
	  return 0;
	} // end if no more messages

	WriteLog("There are %d messages in the queue" LogArg(message_count));*/


  WriteLog("Calling EnumPlayers to enumerate all players and get "
	   "their models");
  dp->EnumPlayers(NULL,
		  EnumPlayersCB,
		  (void *)new_player,
		  DPENUMPLAYERS_REMOTE);
}

const VCTR_STRING& NetProtocols()
{
  return protocol_names;
}

int NetInitializeProtocol(DWORD index)
{
  assert(false == NetProtocolInitialized());
  WriteLog("NetInitializeProtocol() called to start protocol #%d"
	   LogArg(index));

  if(index < 0 || index >= protocol_names.size())
    {
      WriteLog("Invalid protocol index was used, returning with "
	       "error");
      return INITIALIZEPROTOCOL_INVALIDINDEX;
    }

  WriteLog("Attempting to create DirectPlay interface and "
	   "InitializeConnection()");
  CoCreateInstance(CLSID_DirectPlay,
		   NULL,
		   CLSCTX_ALL,
		   DPLAYREFIID,
		   (void **)&dp);
  dp->InitializeConnection((void *)protocol_data[index].begin(),0);
	
  WriteLog("NetInitializeProtocol() success!  Return success code");
  return INITIALIZEPROTOCOL_SUCCESS;
}

void NetReleaseProtocol()
{
  WriteLog("NetReleaseProtocol() was called, releasing protocol");
  assert(true == NetProtocolInitialized());
  TryAndReport(dp->Release());
  dp = NULL;
  state = NETSTATE_NOGAME;
  WriteLog("NetReleaseProtocol() finished, now leaving");
}

bool NetProtocolInitialized()
{
  return bool(NULL != dp);
}

bool NetCheckForSystemMessages()
{
  assert(NETSTATE_NOGAME != state);

  while(false == system_messages.empty())
    {
      WriteLog("NetCheckForSystemMessages() was called and the "
	       "system_messages queue is not empty yet...");
      DPMSG_GENERIC *msg = system_messages.front();
      system_messages.pop();

      WriteLog("switch(msg->dwType)");
      switch(msg->dwType)
	{
	case DPSYS_HOST:
	  {
	    WriteLog("case DPSYS_HOST:  we are now host!");
	    state = NETSTATE_HOSTING;
	    break;
	  }
	case DPSYS_DESTROYPLAYERORGROUP:
	  {
	    WriteLog("case DPSYS_DESTROYPLAYERORGROUP:  a player "
		     "left the game!");
	    // cast a pointer to the msg
	    DPMSG_DESTROYPLAYERORGROUP *p_msg =
	      (DPMSG_DESTROYPLAYERORGROUP *)msg;
	
	    // show the announcement of this player's joining
	    int name_strlen =
	      _tcslen((TCHAR *)p_msg->dpnName.lpszShortName);
	    tstring format;
	    // load the format of this message
	    GluStrLoad(IDS_OLDPLAYER,format);
	    // allocate memory for buffer
	    TCHAR *buffer = new TCHAR[name_strlen + format.length()];
	    // post the message
	    wsprintf(buffer,
		     format.c_str(),
		     (LPCTSTR)p_msg->dpnName.lpszShortName);
				
	    WriteLog("Posting message of player leaving to screen");
	    GluPostMessage(buffer);
				
	    // delete buffer
	    delete buffer; // don't need buffer anymore

	    // get rid of old people we were counting
	    WriteLog("Enumerating players again");
	    NetWelcome();

	    break;
	  }
	case DPSYS_SESSIONLOST:
	  {
	    WriteLog("case DPSYS_SESSIONLOST:  we lost the session!");
	    delete (BYTE *)msg;
	    WriteLog("session lost; NetCheckSystemMessages() returns "
		     "true");
	    return true;
	  }
	case DPSYS_SETPLAYERORGROUPDATA:
	  {
	    WriteLog("case DPSYS_SETPLAYERORGROUPDATA:  player has "
		     "selected model");

	    // cast a pointer to the msg
	    DPMSG_SETPLAYERORGROUPDATA *p_msg =
	      (DPMSG_SETPLAYERORGROUPDATA *)msg;
	    if((const)us == (const)p_msg->dpId)
	      {
		WriteLog("The player in question is this computer, "
			 "so ignoring the message...");
		break;
	      }

	    // send the player a weather state message
	    //  note we are actually sending it to everybody,
	    //  but that's not important
	    WriteLog("Sending new player (and all old ones) a "
		     "weather state message");
	    NetSendWeatherStateMessage(WtrCurrentState());

	    // welcome new player
	    NetWelcome(p_msg->dpId);
	  } // end case set player data
  	  // any other cases we don't really care about or aren't
  	  // expecting . . . 
	} // end switch message type

      WriteLog("switch() finished, if message didn't match, we "
	       "didn't care about it anyway!"); 

      WriteLog("Deleting buffer that contained system message data");
      delete (BYTE *)msg;
    }

  WriteLog("Returning from NetCheckForSystemMessages with value of "
	   "false.");
  return false;
}

int NetCreateGame(int index)
{
  WriteLog("NetCreateGame called to join room %d" LogArg(index));

  assert(NETSTATE_NOGAME == state);

  WriteLog("Filling out session description of the game we want to "
	   "start");
  DPSESSIONDESC2 sd;
  memset((void *)&sd,0,sizeof(sd));
  sd.dwSize = sizeof(sd);
  // set the flags we want
  sd.dwFlags |= DPSESSION_DIRECTPLAYPROTOCOL;
  sd.dwFlags |= DPSESSION_KEEPALIVE;
  sd.dwFlags |= DPSESSION_MIGRATEHOST;
  sd.dwFlags |= DPSESSION_NOPRESERVEORDER;
  sd.dwFlags |= DPSESSION_OPTIMIZELATENCY;
  sd.guidApplication = ANDRADION2GUID;
  sd.dwMaxPlayers = MAX_PLAYERS;
	
  // get session name (just one letter A-L in lowercase generated from
  // index) 
  TCHAR session_name[] = TEXT("a");
  session_name[0] += TCHAR(index);
  sd.lpszSessionName = (LPWSTR)session_name;
	
  sd.dwUser1 = 0xffffffff; // we don't know the level yet

  // get the sync rate and put it in User2
  sd.dwUser2 = sync_rate = GLUsync_rate;

  WriteLog("Calling DirectPlay::Open");
  if(FAILED(
	    dp->Open(&sd,DPOPEN_CREATE)
	    ))
    {
      WriteLog("Failed to create game session; leaving "
	       "NetCreateGame");
      state = NETSTATE_NOGAME;
      return CREATEGAME_FAILURE;
    }

  WriteLog("Calling Net module's CreatePlayer");
  if(FAILED(CreatePlayer()))
    {
      WriteLog("Failed to create player; leaving NetCreateGame");
      state = NETSTATE_NOGAME;
      dp->Close();
      return CREATEGAME_FAILURE;
    }

  WriteLog("Finished creating session and player, now leaving "
	   "NetCreateGame"); 
  state = NETSTATE_HOSTING;
  return CREATEGAME_SUCCESS;
}

bool NetInGame()
{
  return bool(NETSTATE_NOGAME != state);
}

// callback for enumerating sessions to join
static BOOL FAR PASCAL EnumSessionsCB(
				      LPCDPSESSIONDESC2 lpThisSD,
				      LPDWORD,
				      DWORD dwFlags,
				      LPVOID
				      )
{
  if(DPESC_TIMEDOUT == dwFlags)
    {
      // connection timed out, just quit
      return FALSE;
    }

  // make sure we have right session name
  if(TCHAR('a') + TCHAR(room_index) !=
     ((TCHAR *)lpThisSD->lpszSessionName)[0]) 
    {
      return TRUE;
    }

  // we found a session with the right everything, including name
  //  so let's try and open it right here
  DPSESSIONDESC2 sd_to_join;
  memset((void *)&sd_to_join,0,sizeof(sd_to_join));
  sd_to_join.dwSize = sizeof(sd_to_join);
  sd_to_join.guidInstance = lpThisSD->guidInstance;
  if(FAILED(dp->Open(&sd_to_join,DPOPEN_JOIN)))
    {
      return FALSE;
    }

  // get session description
  DWORD sd_size = 0;
  dp->GetSessionDesc(NULL,&sd_size);
  DPSESSIONDESC2 *sd = NULL;

  if
    (
     0 == sd_size ||
     NULL == (sd = (DPSESSIONDESC2 *)new BYTE[sd_size]) ||
     FAILED(dp->GetSessionDesc((void *)sd,&sd_size)) ||
     sd->dwUser1 >= NUM_LEVELS
     )
    {
      delete (BYTE *)sd;
      dp->Close();
      return FALSE;
    }

  // we got in!
  level = (int)sd->dwUser1; // get level index
  sync_rate = sd->dwUser2;
  delete (BYTE *)sd;
  state = NETSTATE_JOINING;
  assert(level < NUM_LEVELS);
  assert(level >= 0);
  return FALSE;
}

int NetJoinGame(int index)
{
  DPSESSIONDESC2 session;
  // create an enumeration filter so we only get what we want
  memset((void *)&session,0,sizeof(session));
  session.dwSize = sizeof(session);
  session.guidApplication = ANDRADION2GUID;
  room_index = index;

  assert(NETSTATE_NOGAME == state);
  dp->EnumSessions(&session,
		   0,
		   EnumSessionsCB,
		   NULL,
		   DPENUMSESSIONS_AVAILABLE); 
	
  if(NETSTATE_JOINING != state)
    {
      return JOINGAME_FAILURE;
    }

  // try to create our player
  if(FAILED(CreatePlayer()))
    {
      dp->Close();
      state = NETSTATE_NOGAME;
      return JOINGAME_FAILURE;
    }

  // return the level we play on
  return level;
}

void NetLeaveGame()
{
  dp->Close();
  state = NETSTATE_NOGAME;
}

bool NetRemoteLogic()
{
  if(NETSTATE_NOGAME == state)
    {
      return false;
    }

  // first check for messages we may have
  // then set the coor.first coordinates of each character
  //  by interpreting sync rate and destination coor
  // then send a sync message if it's time

  // this loop checks for messages we may have
  DWORD data_size = INITIAL_RECEIVE_BUFFER_SIZE;
  BYTE *data = new BYTE[INITIAL_RECEIVE_BUFFER_SIZE];

  while(true)
    {
      DPID from;
      WriteLog("Getting a message");
      DWORD message_size = GetMessage(from,&data,data_size);

      if(0 == message_size)
	{
	  WriteLog("Finished getting messages");
	  break;
	} // end if data size is zero and therefor no msg's

      WriteLog("Message data obtained");

      // see if this was a system message
      if(DPID_SYSMSG == from)
	{
	  WriteLog("Message was system message, putting on "
		   "system_message queue, going to check for more "
		   "messages, already done with this one");
	  BYTE *extra_copy = new BYTE[message_size];
	  memcpy((void *)extra_copy,(const void *)data,message_size);
	  system_messages.push((DPMSG_GENERIC *)extra_copy);
	  WriteLog("done with this message, we can go on to the "
		   "next");
	  continue;
	}

      WriteLog("Message is not from system; from a player");

      // first check for messages which are
      //  not player-dependent, meaning it don't matter who
      //  sent them
      // the only ones like that are the powerup pickup message and
      // weather state message 
      if(POWERUPPICKUPMESSAGE_SIZE == message_size)
	{
	  WriteLog("Message is a powerup pickup, processing...");
	  vector<CPowerUp> *v = &GLUpowerups;
	  if((DWORD)data[0] < v->size())
	    {
	      // data is valid, because the index is valid
	      // get a pointer to the powerup in question
	      CPowerUp *p = &(*v)[data[0]];
	      if(true == p->is_ammo_set)
		{
		  // this is an ammo set, and will not regenerate
		  v->erase(p);
		}
	      else
		{
		  // make coor of powerups
		  //  negative so they are invisible,
		  //  and later on it will regenerate
		  if(p->x > 0)
		    {
		      assert(p->y > 0);
		      p->x *= -1;
		      p->y *= -1;
		    }
		  p->frames_since_picked_up = 0;
		}
	    }
	  WriteLog("Done processing message: powerup pickup");
	}
      else if(1 == WEATHERSTATEMESSAGE_SIZE &&
	      ((TOPTWOBITS & data[0]) ==
	       TOPTWOBITSMASK_WEATHERSTATEMESSAGE))
	{
	  WriteLog("weather state change, processing message...");
	  WtrNextState(data[0] & ~TOPTWOBITSMASK_WEATHERSTATEMESSAGE);
	  WriteLog("Done processing message: weather state change");
	}
      else
	{
	  WriteLog("Sending player of message is important, "
		   "checking for match with database of remote "
		   "players");
	  DWORD i;
	  for(i = 0; i < remotes.size(); i++)
	    {
	      if((const DPID)remotes[i].id == from)
		{
		  break; // get out of this for loop and save i value
		}
	    }

	  if(remotes.size() == i)
	    {
	      WriteLog("Didn't find a matching player, so "
		       "enumerating the players again with call to "
		       "NetWelcome()");
	      NetWelcome();
	      WriteLog("NetWelcome() returned");
	      // delete data and return for now
	      delete data;
	      WriteLog("Leaving RemoteLogic function prematurely");
	      return true;
	    } // end if didn't find a matching player
	  else
	    {
	      WriteLog("Found matching player: %d, named %s"
		       LogArg(i) LogArg(remotes[i].name.c_str()));

	      if(DEATHMESSAGE_SIZE == message_size)
		{
		  WriteLog("We got a death msg, someone died, "
			   "processing...");
		  CPowerUp new_p;

		  new_p.Setup
		    (
		     GLUenemies[i].coor.first.x,
		     GLUenemies[i].coor.first.y,
		     (FIXEDNUM *)data
		     );

		  // add new powerup to end of vector
		  GLUpowerups.resize(GLUpowerups.size()+1,new_p);

		  WriteLog("Finished processing message: death "
			   "message");
		}
	      else if(SYNCMESSAGE_SIZE == message_size)
		{
		  WriteLog("Got a sync message; processing...");
		  DWORD d; // contains the data of the message in a
			   // single DWORD 
		  memcpy((void *)&d,
			 (const void *)data,
			 SYNCMESSAGE_SIZE);
		  remotes[i].loc_at_start_of_sync =
		    remotes[i].loc_at_end_of_sync;
		  remotes[i].loc_at_end_of_sync.x = 
		    FixedCnvTo<long>
		    ((d >> SYNCMESSAGE_BITSPERCOORCOMPONENT)&
		     SYNCMESSAGE_BITMASKFORCOORCOMPONENT);
		  
		  remotes[i].loc_at_end_of_sync.y =
		    FixedCnvTo<long>
		    ((d) & SYNCMESSAGE_BITMASKFORCOORCOMPONENT);
		  
		  remotes[i].sync = 0;
		  // get current weapon: bit shift and mask
		  GLUenemies[i].current_weapon =
		    (d >> SYNCMESSAGE_BITSFORCOORS) &
		    SYNCMESSAGE_BITMASKFORCURRENTWEAPON;
		  // get direction: bit shift (mask not necessary)
		  GLUenemies[i].direction
		    = (d >> (SYNCMESSAGE_BITSFORCOORS +
		     SYNCMESSAGE_BITSFORCURRENTWEAPON));

		  WriteLog("Finished processing message: sync "
			   "message");
		}
	      else if(BAZOOKAFIREMESSAGE_SIZE == message_size)
		{
		  WriteLog("Got a bazooka fired message; processing");

		  // look for open projectile slot
		  CFire *fires = GLUfires;
		  for(int i = 0; i < MAX_FIRES; i++)
		    {
		      if(fires[i].OkayToDelete())
			{
			  break;
			}
		    }

		  if(MAX_FIRES != i)
		    {
		      fires += i;
		      DWORD x;
		      DWORD y;
		      DWORD d = 0; // contains all the data of this
				   // message 
		      memcpy((void *)&d,
			     (const void *)data,
			     BAZOOKAFIREMESSAGE_SIZE);
		      // extract 12 least significant bits for y, the
		      // 12 more sig bits for x 
		      x = d >> BAZOOKAFIREMESSAGE_SIZE;
		      y = d &
			(
			 (1 <<
			  BAZOOKAFIREMESSAGE_BITSPERCOORCOMPONENT)
			 -1
			 ); 
		      // call fire setup for remotely created bazookas
		      fires->Setup
			(
			 GLUenemies[i].coor.first.x,
			 GLUenemies[i].coor.first.y,
			 FixedCnvTo<long>(x),FixedCnvTo<long>(y)
			 );				
		    }

		  WriteLog("Finished processing message: bazooka "
			   "fire message");
		}
	      else if((TOPTWOBITS & data[0]) ==
		      TOPTWOBITSMASK_WEAPONCHANGEMESSAGE)
		{
		  WriteLog("Got a weapon change message; processing");
		  GLUenemies[i].current_weapon = data[0] &
		    ~TOPTWOBITSMASK_WEAPONCHANGEMESSAGE;
		  WriteLog("Finished processing message: "
			   "weapon change message");
		}
	      else if((TOPTWOBITS & data[0]) ==
		      TOPTWOBITSMASK_PISTOLFIREMESSAGE)
		{
		  WriteLog("Got a pistol fire message; processing");

		  // look for a good slot to use
		  int j;
		  for(j = 0; j < MAX_FIRES; i++)
		    {
		      if(GLUfires[j].OkayToDelete())
			{
			  // we found memory that the CFire object can
			  // occupy 
			  break;
			}
		    }
	
		  if(MAX_FIRES != j)
		    {
		      // a good slot was found
		      GLUfires[j].Setup
			(GLUenemies[i].X(),
			 GLUenemies[i].Y(),
			 data[0] & ~TOPTWOBITSMASK_PISTOLFIREMESSAGE,
			 WEAPON_PISTOL,true);
		    }
			
		  WriteLog("Finished processing message: pistol fire "
			   "message");
		}
	      else if(ONEBYTESIGNAL_ADMITHIT == data[0])
		{
		  WriteLog("Got an admit hit message; processing");
		  GLUenemies[i].state = CHARSTATE_HURT;
		  GLUenemies[i].frames_in_this_state = 0;
		  WriteLog("Finished processing message: admit hit "
			   "message");
		}
	      else if(ONEBYTESIGNAL_MACHINEGUNFIRESTART == data[0])
		{
		  WriteLog("Got a start fire message; processing");
		  remotes[i].firing = true;
		  WriteLog("Done processing message: start fire "
			   "message");
		}
	      else if(ONEBYTESIGNAL_MACHINEGUNFIRESTOP == data[0])
		{
		  WriteLog("Got a stop fire message; processing");
		  remotes[i].firing = false;
		  WriteLog("Done processing message: stop fire "
			   "message");
		}
	      else
		{
		  WriteLog("Got a hit w/pistol or machine gun "
			   "message; processing");
		  GLUhero.SubtractHealth
		    (data[0]-ONEBYTESIGNAL_CLAIMPISTOLHIT);
		  WriteLog("Done processing message: "
			   "hit w/pistol or machine gun message");
		} // end if clause of type of message search
	    } // end if found a matching sending player for the msg
	} // end if DPID from is important
      WriteLog("About to do another message-get");
    } // end while messages are available

  WriteLog("Deleting message receive buffer");
  delete data;

  // see if we should send a syncronization message
  if(++sync >= sync_rate)
    {
      WriteLog("Sending synchronization message");
      sync = 0;
      DWORD sync_data = 0;
		
      // calculate where we will be at the end of the sync
      CCharacter *hero = &GLUhero;
      int end_y;
      int end_x;
      if((const)hero->coor.first.x != hero->coor.second.x ||
	 (const)hero->coor.first.y != hero->coor.second.y)
	{
	  // we are going to move!
	  POINT *start = &hero->coor.first;
	  FIXEDNUM dir_x;
	  FIXEDNUM dir_y;
	  GluInterpretDirection(hero->direction,dir_x,dir_y);

	  // calculate speed
	  FIXEDNUM speed;
	  speed = FixedMul(HEROSPEED,HEROSPEED_MPFACTOR);

	  if(CHARSTATE_HURT == hero->state)
	    {
	      speed = FixedMul(speed,HEROSPEED_HURTFACTOR);
	    }

	  dir_x = FixedMul(dir_x,sync_rate * speed);
	  dir_y = FixedMul(dir_y,sync_rate * speed);

	  end_x = FixedCnvFrom<long>(start->x + dir_x);
	  end_y = FixedCnvFrom<long>(start->y + dir_y);

	  if     (end_x > 4095) {end_x = 4095;}
	  else if(end_x < 0)    {end_x = 0;   }
	  if     (end_y > 4095) {end_y = 4095;}
	  else if(end_y < 0)    {end_y = 0;   }
	}
      else
	{
	  end_x = FixedCnvFrom<long>(hero->coor.first.x);
	  end_y = FixedCnvFrom<long>(hero->coor.first.y);
	}

      // add the coordinates to the data
      sync_data |= end_y;
      sync_data |= (end_x << 12);

      // add the weapon type
      sync_data |= hero->current_weapon << 24;
	
      // add the direction
      sync_data |= hero->direction << 26;

      WriteLog("Finished processing synchronization data, about to "
	       "send");
      Send(sync_data,false);
      WriteLog("Finished sending sync message");
    }

  WriteLog("Updating enemy positions");

  DWORD enemy_count = remotes.size();
  assert(GLUenemies.size() == enemy_count);

  for(DWORD i = 0; i < enemy_count; i++)
    {
      POINT *where_ya_at = &GLUenemies[i].coor.second;

      // this variable will be used for calculations of both x and y
      int range =
	remotes[i].loc_at_end_of_sync.x -
	remotes[i].loc_at_start_of_sync.x;

      // how far we are through the sync of this enemy
      FIXEDNUM progress_factor =
	FixedCnvTo<long>(remotes[i].sync) / sync_rate;

      // multiply range by a progress factor
      range = FixedMul(progress_factor,range);
		
      where_ya_at->x = remotes[i].loc_at_start_of_sync.x + range;
		
      // now start calculating y coordinate like we did x
      range = remotes[i].loc_at_end_of_sync.y -
	remotes[i].loc_at_start_of_sync.y; 

      // multiply range once again by a progress factor
      range = FixedMul(progress_factor,range);

      where_ya_at->y = remotes[i].loc_at_start_of_sync.y + range;
    }

  return true;
} // end function NetRemoteLogic

void NetSendWeaponChangeMessage(int type)
{
  assert(type >= 0);

  if(NETSTATE_NOGAME != state)
    {
      // no need to send it guaranteed; if it doesn't
      //  get there, the others will know within the next
      //  sync message
      Send((BYTE)TOPTWOBITSMASK_WEAPONCHANGEMESSAGE | (BYTE)type,false);
    }
  WriteLog("Sent message: weapon change message, change to weapon %d" LogArg(type));
}

void NetSendPowerUpPickUpMessage(int powerup_index)
{
  if(NETSTATE_NOGAME == state ||
     powerup_index >= (1 << POWERUPPICKUPMESSAGE_BITSFORPOWERUPINDEX))
    {
      // index too high or not playing a game anyway, so just quit
      return;
    }

  WriteLog("Sending powerup pickup message for powerup %d"
	   LogArg(powerup_index));
  Send((WORD)powerup_index);  // send the message
  WriteLog("Sent message: powerup pickup message, picked up powerup "
	   "w/index of %d" LogArg(powerup_index));
}

void NetSendHitMessage(int player,int weapon_type)
{
  if(NETSTATE_NOGAME == state)
    {
      GLUenemies[player].SubtractHealth(weapon_type);
    }
  else
    {
      // we hit you! mwa ha ha!
      Send((BYTE)(ONEBYTESIGNAL_CLAIMPISTOLHIT+(BYTE)weapon_type),
	   true,player);
    }

  WriteLog("Sent message: hit message, we hit player w/index of %d, w/weapon w/index of %d" LogArg(player) LogArg(weapon_type));
}

void NetSendAdmitHitMessage()
{
  if(NETSTATE_NOGAME != state)
    {
      Send(ONEBYTESIGNAL_ADMITHIT,false);
    }
  WriteLog("Sent message: admit hit message");
}

void NetSendBazookaFireMessage(FIXEDNUM x,FIXEDNUM y)
{
  if(NETSTATE_NOGAME == state ||
     x >= Fixed(1 << BAZOOKAFIREMESSAGE_BITSPERCOORCOMPONENT) ||
     y >= Fixed(1 << BAZOOKAFIREMESSAGE_BITSPERCOORCOMPONENT) )
    {
      // coordinates too high, don't care, or not in game . . .
      return;
    }

  // check validity of x and y (cannot be negative)
  if(x < 0)
    {
      x = 0;
    }
  if(y < 0)
    {
      y = 0;
    }

  DWORD msg; // only three bytes will be transferred
  x = FixedCnvFrom<long>(x);
  y = FixedCnvFrom<long>(y);

  msg = (DWORD)y;
  msg |= ((DWORD)x << BAZOOKAFIREMESSAGE_BITSPERCOORCOMPONENT);

  Send((void *)&msg,BAZOOKAFIREMESSAGE_SIZE);

  WriteLog("Sent message: bazooka fire message, explosion at %d,%d"
	   LogArg(FixedCnvFrom<long>(x))
	   LogArg(FixedCnvFrom<long>(y)));
}

void NetSendDeathMessage(FIXEDNUM *ammo)
{
  assert(NETSTATE_NOGAME != state);

  Send((void *)ammo,DEATHMESSAGE_SIZE);

  WriteLog("Sent message: death message");
}

void NetSendMachineGunFireStartMessage()
{
  if(NETSTATE_NOGAME != state)
    {
      Send(ONEBYTESIGNAL_MACHINEGUNFIRESTART,false);
    }

  WriteLog("Sent message: machine gun fire start message");
}

void NetSendMachineGunFireStopMessage()
{
  if(NETSTATE_NOGAME != state)
    {
      Send(ONEBYTESIGNAL_MACHINEGUNFIRESTOP,false);
    }

  WriteLog("Sent message: machine gun fire stop message");
}

static HRESULT CreatePlayer()
{
  // fill out our name structure
  DPNAME our_name;
  our_name.dwSize = sizeof(our_name);
  our_name.dwFlags = 0;
  our_name.lpszShortName =
    (LPWSTR)GLUname.c_str();
  our_name.lpszLongName = NULL;
	
  HRESULT hr = dp->CreatePlayer(&us,&our_name,NULL,NULL,0,0);
  if(FAILED(hr))
    {
      return hr;
    }

  return hr;
}

static DWORD GetMessage(DPID &from,BYTE **data_buffer,DWORD &data_size)
{
  WriteLog("GetMessage called");
  DPID to;
  DWORD message_size = data_size;

  while(true)
    {
      from = 0;
      to   = 0;

      WriteLog("data_buffer: %8x; message_size: %d"
	       LogArg((DWORD)(*data_buffer))
	       LogArg(message_size));

      HRESULT hr = TryAndReport(dp->Receive(&from,
					    &to,
					    DPRECEIVE_ALL,
					    *data_buffer,
					    &message_size));

      WriteLog("Done getting message");

      if(DPERR_BUFFERTOOSMALL == hr)
	{ 
	  WriteLog("Deleting old message buffer");
	  delete *data_buffer;
	  WriteLog("Allocating new one of size %d" LogArg(message_size));
	  *data_buffer = new BYTE[data_size = message_size];
	  continue;
	} // end if buffer was too small
      else if(FAILED(hr))
	{
	  WriteLog("Failed to get message somehow");
	  message_size = 0;
	} // end if there is no message
      WriteLog("Leaving GetMessage loop");
      break;
    } // end while couldn't get message yet

  WriteLog("GetMessage returning with message_size of %d" LogArg(message_size));

  return message_size;
} // end function GetMessage

bool NetSendWeatherStateMessage(int weather_state)
{
  WriteLog("NetSendWeatherStateMessage function called with state "
	   "of %d" LogArg(weather_state));

  if(NETSTATE_JOINING == state)
    {
      WriteLog("Did not send message: weather state message, but "
	       "were going to, because we are not host"); 
      return false;
    }
  else if(NETSTATE_HOSTING == state)
    {
      Send(TOPTWOBITSMASK_WEATHERSTATEMESSAGE | (BYTE)weather_state);
      WriteLog("Sent message: weather state message");
    }

  WriteLog("NetSendWeatherStateMessage function returning");
  return true;
}

void NetSendPistolFireMessage(int direction)
{
  if(NETSTATE_NOGAME != state)
    {
      Send((BYTE)(TOPTWOBITSMASK_PISTOLFIREMESSAGE | direction),false);
    }

  WriteLog("Sent message: pistol fire message");
}

void NetSetPlayerData()
{
  WriteLog("NetSetPlayerData called");
  WriteLog("dp has value of %8x" LogArg((DWORD)dp));

  // set player's data
  BYTE player_data = (BYTE)GLUhero.Model(); // set the player's data to its model to use
  dp->SetPlayerData(us,(void *)&player_data, 1,
		    DPSET_GUARANTEED | DPSET_REMOTE);
  
  WriteLog("NetSetPlayerData returning");
}

static void AnnounceArrival(const TCHAR *name)
{
  WriteLog("Calculating length of new guy's name");
  int name_strlen = _tcslen(name);

  WriteLog("Loading the format of this message");
  tstring format;
  GluStrLoad(IDS_NEWPLAYER,format);

  WriteLog("Allocating memory for buffer");
  TCHAR *buffer = new TCHAR[name_strlen + format.length() + 1];

  WriteLog("Post the new guy's joining message");
  wsprintf(buffer,format.c_str(),name);

  GluPostMessage(buffer);

  WriteLog("Deleting name buffer");
  delete buffer;
} // end function AnnounceArrival
