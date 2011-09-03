#include "StdAfx.h"
#include "Fixed.h"
#include "SharedConstants.h"
#include "Net.h"
#include "Certifiable.h"
#include "SurfaceLock.h"
#include "SurfaceLock256.h"
#include "Graphics.h"
#include "Character.h"
#include "Fire.h"
#include "Glue.h"
#include "Resource.h"
#include "PowerUp.h"
#include "Weather.h"
#include "Logger.h"

class Connection {
public:
  Connection() : data(NULL), data_size(0) {}
  Connection(const Connection &rhs) {
    Connection();
    *this = rhs;    
  }
  ~Connection() {free(data);}

  void Set(const char *source_name, const void *source, DWORD size) {
    free(data);
    data = malloc(size);
    memcpy(data, source, size);
    data_size = size;
    name = source_name;
  }

  Connection& operator =(const Connection& rhs) {
    if (this != &rhs) {
      Set(rhs.name.c_str(), rhs.data, rhs.data_size);
    }

    return *this;
  }

  DWORD data_size;
  void *data;
  string name;
};

enum {NETSTATE_JOINING, NETSTATE_HOSTING, NETSTATE_NOGAME};

typedef IDirectPlay4A *DPInt;
#define DPLAYREFIID IID_IDirectPlay4A

struct REMOTEPLAYER {
  DPID id;

  POINT loc_at_end_of_sync; // stored in fixed-point
  POINT loc_at_start_of_sync; // stored in fixed-point

  bool firing;
  string name;

  // timer for the other guy's sync
  DWORD sync;

  // everything else is stored in the
  //  actual CCharacter class
};

// sends one byte of data to all players, guaranteed depending
//  on parameter
static void SimpleMessage(BYTE data,bool guaranteed);
static HRESULT CreatePlayer();

// all of our internal members
static DPInt dp; // DPInt is either IDirectPlay4 * or IDirectPlay4A *
static int state;
static vector<Connection> protocols;
static DWORD sync_rate, sync;
static vector<REMOTEPLAYER> remotes; // data for remote players
static DPID us;

static queue<DPMSG_GENERIC *> system_messages;

// message types are listed in Message Types.doc

const int MAX_PLAYERS = 16;

// {CF58E020-9BD3-11d4-B6FE-0050040B0541}
static const GUID ANDRADION2GUID = 
  { 0xcf58e020, 0x9bd3, 0x11d4, { 0xb6, 0xfe, 0x0, 0x50, 0x4, 0xb, 0x5, 0x41 } };

static BOOL FAR PASCAL EnumConnectionMethodsCallback(LPCGUID,
						     LPVOID lpConnection,
						     DWORD dwConnectionSize,
						     LPCDPNAME lpName,
						     DWORD, LPVOID) {
  WriteLog("Enumeration of protocals callback called for %s\n"
	   LogArg(lpName->lpszShortNameA));

  // make sure we can create this protocol
  DPInt dp;
  if(FAILED(CoCreateInstance(CLSID_DirectPlay, NULL, CLSCTX_ALL, DPLAYREFIID,
			     (void**)&dp))) {
    return FALSE; // stop enumeration
  }

  WriteLog("Try to initiate it for ourselves...\n");
  if(FAILED(dp->InitializeConnection(lpConnection,0))) {
    WriteLog("Didn't work, returning to get on with the enumeration\n");
    TryAndReport(dp->Release());
    return TRUE;
  }

  WriteLog("It worked, recording connection data and protocol name, etc...\n");

  // done with dp
  TryAndReport(dp->Release());

  int magic_index = protocols.size();

  protocols.resize(magic_index + 1);
  protocols[magic_index].Set(lpName->lpszShortNameA, lpConnection,
			     dwConnectionSize);
  
  WriteLog("Successfully enumerated %s\n" LogArg(lpName->lpszShortNameA));

  return TRUE;
}

void NetInitialize() {
  WriteLog("NetInitialize() called");
  state = NETSTATE_NOGAME;

  if(FAILED(CoInitialize(NULL))) {
    WriteLog("CoInitialize() Failed, no protocols will be initialized");
    return;
  }

  // get a list of all the protocols
  DPInt dp;
  WriteLog("Attempting to create temporary DirectPlay interface to enumerate protocols");
  if(SUCCEEDED(CoCreateInstance(CLSID_DirectPlay, NULL, CLSCTX_ALL, DPLAYREFIID, (VOID**)&dp))) {
    WriteLog("DirectPlay created!  Now calling enumeration method");
    TryAndReport(dp->EnumConnections(&ANDRADION2GUID,EnumConnectionMethodsCallback,NULL,DPCONNECTION_DIRECTPLAY));
    TryAndReport(dp->Release());
  }
	
  WriteLog("Done enumerating\n");

  if(0 == protocols.size()) {
    WriteLog("Didn't get any protocols\n");
    CoUninitialize();
  }

  WriteLog("Directplay return codes:\n");
#define CODE(x) WriteLog(" %s: %x\n" LogArg(#x) LogArg(x))
  CODE(CLASS_E_NOAGGREGATION);
  CODE(DP_OK);
  CODE(DPERR_ACCESSDENIED);
  CODE(DPERR_ACTIVEPLAYERS);
  CODE(DPERR_ALREADYINITIALIZED);
  CODE(DPERR_APPNOTSTARTED);
  CODE(DPERR_AUTHENTICATIONFAILED);
  CODE(DPERR_BUFFERTOOLARGE);
  CODE(DPERR_BUFFERTOOSMALL);
  CODE(DPERR_BUSY);
  CODE(DPERR_CANCELFAILED);
  CODE(DPERR_CANNOTCREATESERVER);
  CODE(DPERR_CANTADDPLAYER);
  CODE(DPERR_CANTCREATEGROUP);
  CODE(DPERR_CANTCREATEPLAYER);
  CODE(DPERR_CANTCREATEPROCESS);
  CODE(DPERR_CANTCREATESESSION);
  CODE(DPERR_CANTLOADCAPI);
  CODE(DPERR_CANTLOADSECURITYPACKAGE);
  CODE(DPERR_CANTLOADSSPI);
  CODE(DPERR_CAPSNOTAVAILABLEYET);
  CODE(DPERR_CONNECTING);
  CODE(DPERR_CONNECTIONLOST);
  CODE(DPERR_ENCRYPTIONFAILED);
  CODE(DPERR_ENCRYPTIONNOTSUPPORTED);
  CODE(DPERR_EXCEPTION);
  CODE(DPERR_GENERIC);
  CODE(DPERR_INVALIDFLAGS);
  CODE(DPERR_INVALIDGROUP);
  CODE(DPERR_INVALIDINTERFACE);
  CODE(DPERR_INVALIDOBJECT);
  CODE(DPERR_INVALIDPARAMS);
  CODE(DPERR_INVALIDPASSWORD);
  CODE(DPERR_INVALIDPLAYER);
  CODE(DPERR_INVALIDPRIORITY);
  CODE(DPERR_LOGONDENIED);
  CODE(DPERR_NOCAPS);
  CODE(DPERR_NOCONNECTION);
  CODE(DPERR_NOINTERFACE);
  CODE(DPERR_NOMESSAGES);
  CODE(DPERR_NONAMESERVERFOUND);
  CODE(DPERR_NONEWPLAYERS);
  CODE(DPERR_NOPLAYERS);
  CODE(DPERR_NOSESSIONS);
  CODE(DPERR_NOTLOBBIED);
  CODE(DPERR_NOTLOGGEDIN);
  CODE(DPERR_OUTOFMEMORY);
  CODE(DPERR_PENDING);
  CODE(DPERR_PLAYERLOST);
  CODE(DPERR_SENDTOOBIG);
  CODE(DPERR_SESSIONLOST);
  CODE(DPERR_SIGNFAILED);
  CODE(DPERR_TIMEOUT);
  CODE(DPERR_UNAVAILABLE);
  CODE(DPERR_UNINITIALIZED);
  CODE(DPERR_UNKNOWNAPPLICATION);
  CODE(DPERR_UNKNOWNMESSAGE);
  CODE(DPERR_UNSUPPORTED);
  CODE(DPERR_USERCANCEL);
  // CODE(E_UNKNOWN);
#undef CODE

  WriteLog("Now leaving NetInitialize()\n");
}

void NetRelease()
{
  WriteLog("NetRelease called, say goodbye to everybody\n");
  if(NetProtocolInitialized()) {
      WriteLog("Protocol is currently intialized, checking if still in a game\n");
      if(NetInGame()) {
	  WriteLog("Game is still open, closing now\n");
	  NetLeaveGame();
	}
      NetReleaseProtocol();
      WriteLog("Protocol and game closed\n");
    }

  if(0 != protocols.size()) {
      WriteLog("Protocols had been enumerated, now we must called CoUninitialize()\n");
      CoUninitialize();
      WriteLog("CoUnin called\n");
    }

  WriteLog("Releasing connection data\n");

  protocols.clear();

  WriteLog("Now leaving NetRelease()\n");
}

BOOL FAR PASCAL EnumPlayersCB(DPID dpId, DWORD, LPCDPNAME lpName, DWORD, LPVOID)
{
  WriteLog("Enumeration of players callback called for player %s\n" LogArg(lpName));
	
  WriteLog("Trying to find player's model\n");
  BYTE m;
  DWORD size = 1;
  if(FAILED(dp->GetPlayerData(dpId,(void *)&m,&size,DPGET_REMOTE)))
    {
      WriteLog("Failed to get player data, skipping enumeration of this participant\n");
      return TRUE;
    }
  WriteLog("Player uses model %d\n" LogArg((int)m));

  assert(GLUenemies.size() == remotes.size());

  int magic_index = remotes.size();

  remotes.resize(magic_index+1);

  WriteLog("Storing name and ID...\n");
  remotes[magic_index].id = dpId;
  remotes[magic_index].name = (TCHAR *)lpName->lpszShortName;

  WriteLog("Building CCharacter object in GLUenemies array...\n");
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
  remotes[magic_index].loc_at_start_of_sync = remotes[magic_index].loc_at_end_of_sync;
  remotes[magic_index].firing = false;
  remotes[magic_index].sync = 0;
	
  WriteLog("Finished enumerating Mr. or Ms. %s, callback is returning" LogArg(lpName));
  return TRUE;
}

void NetWelcome()
{
  WriteLog("NetWelcome() called");
  // sets the level of the session desc
  // right before the game
	
  // get size of session description
  if(NETSTATE_HOSTING == state)
    {
      WriteLog("We are host, so we must set the SessionDesc to store level (should already contain sync rate)");
      DWORD size = 0;
      const HRESULT res = dp->GetSessionDesc(NULL,&size);

      assert (DPERR_BUFFERTOOSMALL == res); 

      // allocate buffer for it
      DPSESSIONDESC2 *sd = (DPSESSIONDESC2 *)new BYTE[size];

      sd->dwSize = sizeof(DPSESSIONDESC2);

      // request desc
      dp->GetSessionDesc((void *)sd,&size);

      WriteLog("SessionDesc obtained, value of User2 (sync_rate) is %d" LogArg((int)sd->dwUser2));

      // make necessary changes and apply
      sd->dwUser1 = (DWORD)GLUlevel;
      WriteLog("Value of User1 (level) is %d" LogArg((int)sd->dwUser1));
      dp->SetSessionDesc(sd,0);

      WriteLog("SetSessionDesc completed");

      // delete unneeded buffer
      delete (BYTE *)sd;
    }

  // enumerate players getting the player data of them and therefore
  //  their model indices
  remotes.clear();
  GLUenemies.clear();

  WriteLog("Calling EnumPlayers to enumerate all players and get their models");
  dp->EnumPlayers(NULL,EnumPlayersCB,NULL,DPENUMPLAYERS_REMOTE);
}

const char *NetProtocolName(int protocol_index) {
  assert(protocol_index >= 0 && protocol_index < protocols.size());
  return protocols[protocol_index].name.c_str();
}

int NetProtocolCount() {return protocols.size();}

int NetInitializeProtocol(DWORD index) {
  assert(!NetProtocolInitialized());
  WriteLog("NetInitializeProtocol() called to start protocol #%d" LogArg(index));

  if(index < 0 || index >= protocols.size()) {
    WriteLog("Invalid protocol index was used, returning with error");
    return INITIALIZEPROTOCOL_INVALIDINDEX;
  }

  WriteLog("Attempting to create DirectPlay interface and InitializeConnection()");
  TryAndReport(CoCreateInstance(CLSID_DirectPlay, NULL, CLSCTX_ALL, DPLAYREFIID, (void **)&dp));
  assert (NULL != protocols[index].data);
  assert (0 != protocols[index].data_size);
  TryAndReport(dp->InitializeConnection(protocols[index].data, 0));
	
  WriteLog("NetInitializeProtocol() success!  Return success code");
  return INITIALIZEPROTOCOL_SUCCESS;
}

void NetReleaseProtocol() {
  WriteLog("NetReleaseProtocol() was called, releasing protocol");
  assert(true == NetProtocolInitialized());
  TryAndReport(dp->Release());
  dp = NULL;
  state = NETSTATE_NOGAME;
  WriteLog("NetReleaseProtocol() finished, now leaving");
}

bool NetProtocolInitialized() {return NULL != dp;}

bool NetCheckForSystemMessages() {
  assert(NETSTATE_NOGAME != state);

  while(!system_messages.empty()) {
    WriteLog("NetCheckForSystemMessages() was called and the system_messages queue is not empty yet...");
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
	  WriteLog("case DPSYS_DESTROYPLAYERORGROUP:  a player left the game!");
	  // cast a pointer to the msg
	  DPMSG_DESTROYPLAYERORGROUP *p_msg = (DPMSG_DESTROYPLAYERORGROUP *)msg;
	
	  // show the announcement of this player's joining
	  int name_strlen = strlen(p_msg->dpnName.lpszShortNameA);
	  string format;
	  // load the format of this message
	  GluStrLoad(IDS_OLDPLAYER,format);
	  // allocate memory for buffer
	  TCHAR *buffer = new TCHAR[name_strlen + format.length()];
	  // post the message
	  wsprintf(buffer,format.c_str(),(LPCTSTR)p_msg->dpnName.lpszShortName);
				
	  WriteLog("Posting message of player leaving to screen");
	  GluPostMessage(buffer);
				
	  delete buffer;

	  // get rid of old people we were counting
	  WriteLog("Enumerating players again");
	  NetWelcome();

	  break;
	}
      case DPSYS_SESSIONLOST:
	{
	  WriteLog("case DPSYS_SESSIONLOST:  we lost the session!");
	  delete (BYTE *)msg;
	  WriteLog("session lost; NetCheckSystemMessages() returns true");
	  return true;
	}
      case DPSYS_SETPLAYERORGROUPDATA:
	{
	  WriteLog("case DPSYS_SETPLAYERORGROUPDATA:  player has selected model");

	  // cast a pointer to the msg
	  DPMSG_SETPLAYERORGROUPDATA *p_msg = (DPMSG_SETPLAYERORGROUPDATA *)msg;
	  if(us == p_msg->dpId)
	    {
	      WriteLog("The player in question is this computer, so ignoring the message...");
	      break;
	    }

	  // send the player a weather state message
	  //  note we are actually sending it to everybody,
	  //  but oh well!
	  WriteLog("Sending new player (and all old ones) a weather state message");
	  NetSendWeatherStateMessage(WtrCurrentState());

	  NetWelcome();
	}
      case DPSYS_CREATEPLAYERORGROUP:
	{
	  WriteLog("case DPSYS_CREATEPLAYERORGROUP:  player has joined, just post a message");

	  // cast a pointer to the msg
	  DPMSG_CREATEPLAYERORGROUP *p_msg = (DPMSG_CREATEPLAYERORGROUP *)msg;

	  // show the announcement of this player's joining
	  int name_strlen = strlen(p_msg->dpnName.lpszShortNameA);
	  string format;
	  // load the format of this message
	  GluStrLoad(IDS_NEWPLAYER,format);
	  // allocate memory for buffer
	  TCHAR *buffer = new TCHAR[name_strlen + format.length()];
	  // post the message
	  wsprintf(buffer,format.c_str(),(LPCTSTR)p_msg->dpnName.lpszShortName);
				
	  WriteLog("Posting player join message to screen");
	  GluPostMessage(buffer);

	  delete buffer;
	}


	// any other cases we don't really care about or aren't expecting...
      }

    WriteLog("Deleting buffer that contained system message data");
    delete (BYTE *)msg;
  }

  return false;
}

int NetCreateGame(int index) {
  WriteLog("NetCreateGame called to join room %d\n" LogArg(index));

  assert(NETSTATE_NOGAME == state);

  WriteLog("Filling out session description of the game we want to start\n");
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
	
  // calculate session name 
  char session_name[2] = {'a', 0};
  session_name[0] += (char)index;
  sd.lpszSessionNameA = (LPSTR)session_name;
	
  sd.dwUser1 = 0xffffffff; // we don't know the level yet
  sd.dwUser2 = sync_rate = GLUsync_rate;

  WriteLog("Calling DirectPlay::Open\n");
  if(FAILED(TryAndReport(dp->Open(&sd, DPOPEN_CREATE)))) {
    WriteLog("Failed to create game session; leaving NetCreateGame\n");
    state = NETSTATE_NOGAME;
    return CREATEGAME_FAILURE;
  }

  WriteLog("Calling Net module's CreatePlayer\n");
  if(FAILED(CreatePlayer())) {
    WriteLog("Failed to create player; leaving NetCreateGame\n");
    state = NETSTATE_NOGAME;
    dp->Close();
    return CREATEGAME_FAILURE;
  }

  WriteLog("Finished creating session and player\n");
  state = NETSTATE_HOSTING;
  return CREATEGAME_SUCCESS;
}

bool NetInGame() {
  return NETSTATE_NOGAME != state;
}

// callback for enumerating sessions to join
static BOOL FAR PASCAL EnumSessionsCB(LPCDPSESSIONDESC2 lpThisSD,
				      LPDWORD,
				      DWORD dwFlags,
				      LPVOID context) {
  int *const communicated = (int *)context;

  if(DPESC_TIMEDOUT == dwFlags) {
    WriteLog("Session enumeration timed out\n");
    return FALSE; 
  }

  assert (NULL != context && NULL != lpThisSD);

  // make sure we have right session name
  if('a' + char(*communicated) != lpThisSD->lpszSessionNameA[0]) {
    WriteLog("Enumerated session of name %s; were looking for %c"
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
  DPSESSIONDESC2 *sd = NULL;

  if(0 == sd_size ||
     NULL == (sd = (DPSESSIONDESC2 *)new BYTE[sd_size]) ||
     FAILED(dp->GetSessionDesc((void *)sd,&sd_size)) ||
     sd->dwUser1 >= NUM_LEVELS) {
    delete (BYTE *)sd;
    dp->Close();
    return FALSE;
  }

  // we got in!
  *communicated = (int)sd->dwUser1; // get level index
  sync_rate = sd->dwUser2;
  delete (BYTE *)sd;
  state = NETSTATE_JOINING;
  assert(*communicated < NUM_LEVELS);
  assert(*communicated >= 0);
  return FALSE;
}

int NetJoinGame(int index) {
  DPSESSIONDESC2 session;
  
  // create an enumeration filter so we only get what we want
  memset((void *)&session, 0, sizeof(session));
  session.dwSize = sizeof(session);
  session.guidApplication = ANDRADION2GUID;

  assert(NETSTATE_NOGAME == state);
  
  dp->EnumSessions(&session, 0,
		   EnumSessionsCB, (LPVOID)&index,
		   DPENUMSESSIONS_AVAILABLE);
	
  if(NETSTATE_JOINING != state) {
    return JOINGAME_FAILURE;
  }

  // try to create our player
  if(FAILED(CreatePlayer())) {
    dp->Close();
    state = NETSTATE_NOGAME;
    return JOINGAME_FAILURE;
  }

  // return the level we play on, given by the enumeration
  // callback function 
  return index;
}

void NetLeaveGame() {
  dp->Close();
  state = NETSTATE_NOGAME;
}

bool NetRemoteLogic() {
  const int DEFAULT_INITIAL_BUFFER_SIZE = 10;
  
  if(NETSTATE_NOGAME == state) {
    return false;
  }

  // first check for messages we may have
  // then set the coor.first coordinates of each character
  //  by interpreting sync rate and destination coor
  // then send a sync message if it's time

  // this loop checks for messages we may have
  BYTE *dataB = new BYTE[DEFAULT_INITIAL_BUFFER_SIZE];
  DWORD buffer_size = DEFAULT_INITIAL_BUFFER_SIZE;
  while(true) {
    DPID from, to;
    DWORD data_size = buffer_size;
      
    if(FAILED(dp->Receive(&from, &to, DPRECEIVE_ALL, dataB, &data_size))) {
      if (data_size <= buffer_size) {
	break;
      }
	
      delete dataB;
      dataB = new BYTE[buffer_size = data_size];
      continue;
    }

    if(DPID_SYSMSG == from) {
      WriteLog("Message was system message, putting on system_message queue, going to check for more messages, already done with this one\n");
      system_messages.push((DPMSG_GENERIC *)dataB);
      dataB = new BYTE[DEFAULT_INITIAL_BUFFER_SIZE];
      buffer_size = DEFAULT_INITIAL_BUFFER_SIZE;
      continue; 
    }

    const DWORD *const dataD = (const DWORD *)dataB;
    const WORD *const dataW = (const WORD *)dataB;

    WriteLog("Message data obtained");

    // first check for messages which are
    //  not player-dependent, meaning it don't matter who
    //  sent them
    // the only ones like that are the powerup pickup message and weather state message
    if(2 == data_size) {
      WriteLog("Message is a powerup pickup, processing...");
      vector<CPowerUp> *v = &GLUpowerups;
      if(dataW[0] < v->size()) {
	// data is valid, because the index is valid
	// get a pointer to the powerup in question
	CPowerUp *p = &(*v)[dataW[0]];
	if(p->type < 0) {
	  (*v)[dataW[0]] = (*v)[v->size()-1];
	  v->resize(v->size()-1);
	} else {
	  // make coor of powerups
	  //  negative so they are invisible,
	  //  and later on it will regenerate
	  if(p->x > 0) {
	    assert(p->y > 0);
	    p->x *= -1;
	    p->y *= -1;
	  }
	  p->frames_since_picked_up = 0;
	}
      }
      WriteLog("Done processing message: powerup pickup\n");

      continue; 
    }

    if(1 == data_size && (0x40 & dataB[0])) {
      WriteLog("weather state change, processing message...\n");
      WtrNextState(dataB[0] & ~0x40);
      WriteLog("Done processing message: weather state change\n");
      continue;
    }
		
    WriteLog("Sending player of message is important, checking for match with database of remote players");
    DWORD i;
    for(i = 0; i < remotes.size() && remotes[i].id != from; i++) {}

    if(remotes.size() == i)
      {
	WriteLog("Didn't find a matching player, so enumerating the players again with call to NetWelcome()");
	NetWelcome();
	WriteLog("NetWelcome() returned, leaving RemoteLogic function for now");
	delete dataB;
	return true;
      }

    WriteLog("Found matching player: %d, named %s" LogArg(i) LogArg(remotes[i].name.c_str()));

    if(6 == data_size)
      {
	WriteLog("We got a death msg, someone died, processing...");
	// get a pointer to the powerup vector
	vector<CPowerUp> *p;
	p = &GLUpowerups;
	CPowerUp new_p;
	FIXEDNUM got_data[NUM_WEAPONS]; // must convert data into fixed point form

	for(int j = 0; j < NUM_WEAPONS; j++)
	  {
	    got_data[j] = (FIXEDNUM)dataW[j];
	  }

	new_p.Setup
	  (
	   GLUenemies[i].coor.first.x,
	   GLUenemies[i].coor.first.y,
	   got_data
	   );

	// add new powerup to end of vector
	p->resize(p->size()+1,new_p);

	WriteLog("Finished processing message: death message");
	continue;
      }
		
    if(4 == data_size)
      {
	WriteLog("Got a sync message; processing...");
	remotes[i].loc_at_start_of_sync = remotes[i].loc_at_end_of_sync;
	remotes[i].loc_at_end_of_sync.x = FixedCnvTo<long>((dataD[0] >> 12)&0xfff);
	remotes[i].loc_at_end_of_sync.y = FixedCnvTo<long>((dataD[0]) & 0xfff);
	remotes[i].sync = 0;
	// get current weapon: bit shift and mask
	GLUenemies[i].current_weapon = (dataD[0] >> 24) & 3;
	// get direction: bit shift and mask
	GLUenemies[i].direction = (dataD[0] >> 26);

	WriteLog("Finished processing message: sync message");
	continue;
      }
		
    if(3 == data_size)
      {
	WriteLog("Got a bazooka fired message; processing");

	// look for open projectile slot
	CFire *fires = GLUfires;
	for(i = 0; i < MAX_FIRES; i++)
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
	    // extract 12 least significant bits for y, the 12 more sig bits for x
	    x = (DWORD)(dataB[1] >> 4) | ((DWORD)dataB[2] << (DWORD)4);
	    y = dataW[0] & 0xfff;
	    // call fire setup for remotely created bazookas
	    fires->Setup
	      (
	       GLUenemies[i].coor.first.x,GLUenemies[i].coor.first.y,FixedCnvTo<long>(x),FixedCnvTo<long>(y)
	       );				
	  }

	WriteLog("Finished processing message: bazooka fire message");

	continue;
      }

    if(0x80 & dataB[0])
      {
	WriteLog("Got a weapon change message; processing");
	GLUenemies[i].current_weapon = dataB[0] & ~0x80;
	WriteLog("Finished processing message: weapon change message");
	continue;
      }

    if(0xc0 & dataB[0])
      {
	WriteLog("Got a pistol fire message; processing");

	// look for a good slot to use
	int j;
	for(j = 0; j < MAX_FIRES; i++)
	  {
	    if(GLUfires[j].OkayToDelete())
	      {
		// we found memory that the CFire object can occupy
		break;
	      }
	  }
	
	if(MAX_FIRES != j)
	  {
	    // a good slot was found
	    GLUfires[j].Setup(GLUenemies[i].X(),GLUenemies[i].Y(),dataB[0] & 0xc0,WEAPON_PISTOL,true);
	  }
			
	WriteLog("Finished processing message: pistol fire message");
	continue;
      }

    if(0 == dataB[0])
      {
	WriteLog("Got an admit hit message; processing");
	GLUenemies[i].state = CHARSTATE_HURT;
	GLUenemies[i].frames_in_this_state = 0;
	WriteLog("Finished processing message: admit hit message");
	continue;
      }

    if(1 == dataB[0])
      {
	WriteLog("Got a start fire message; processing");
	remotes[i].firing = true;
	WriteLog("Done processing message: start fire message");
	continue;
      }

    if(2 == dataB[0])
      {
	WriteLog("Got a stop fire message; processing");
	remotes[i].firing = false;
	WriteLog("Done processing message: stop fire message");
	continue;
      }

    WriteLog("Got a hit w/pistol or machine gun message; processing");
    GLUhero.SubtractHealth(dataB[0]-3);
    WriteLog("Done processing message: hit w/pistol or machine gun message");
  }

  delete dataB;

  // see if we should send a synchronization message
  if(++sync >= sync_rate)
    {
      WriteLog("Sending synchronization message");
      sync = 0;
      DWORD sync_data = 0;
		
      // calculate where we will be at the end of the sync
      CCharacter *hero = &GLUhero;
      int end_x, end_y;
      if(hero->coor.first.x != hero->coor.second.x || hero->coor.first.y != hero->coor.second.y) {
	// we are going to move!
	POINT *start = &hero->coor.first;
	FIXEDNUM dir_x, dir_y;
	FIXEDNUM speed = FixedMul(HEROSPEED, HEROSPEED_MPFACTOR);
	GluInterpretDirection(hero->direction,dir_x,dir_y);

	if(CHARSTATE_HURT == hero->state) {
	  speed = FixedMul(speed, HEROSPEED_HURTFACTOR);
	}

	dir_x = FixedMul(dir_x, sync_rate * speed);
	dir_y = FixedMul(dir_y, sync_rate * speed);

	end_x = FixedCnvFrom<long>(start->x + dir_x) & 4095;
	end_y = FixedCnvFrom<long>(start->y + dir_y) & 4095;
      } else {
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

      WriteLog("Finished processing synchronization data, about to send");
      dp->SendEx(
		 us,
		 DPID_ALLPLAYERS,
		 DPSEND_ASYNC | DPSEND_NOSENDCOMPLETEMSG,
		 (void *)&sync_data,
		 4,
		 0,
		 0,
		 NULL,
		 NULL
		 );
      WriteLog("Finished sending sync message");
    }

  int enemy_count = remotes.size();
  assert(GLUenemies.size() == enemy_count);

  for(int i = 0; i < enemy_count; i++)
    {
      POINT *where_ya_at = &GLUenemies[i].coor.second;

      // this variable will be used for calculations of both x and y
      int range = remotes[i].loc_at_end_of_sync.x - remotes[i].loc_at_start_of_sync.x;

      // how far we are through the sync of this enemy
      FIXEDNUM progress_factor =	FixedCnvTo<long>(remotes[i].sync) / sync_rate;

      // multiply range by a progress factor
      range = FixedCnvFrom<long>(progress_factor * range);
		
      where_ya_at->x = remotes[i].loc_at_start_of_sync.x + range;
		
      // now start calculating y coordinate like we did x
      range = remotes[i].loc_at_end_of_sync.y - remotes[i].loc_at_start_of_sync.y;

      // multiply range once again by a progress factor
      range = FixedCnvFrom<long>(progress_factor * range);

      where_ya_at->y = remotes[i].loc_at_start_of_sync.y + range;
    }

  return true;
}

void NetSendWeaponChangeMessage(int type)
{
  assert(type >= 0);
  assert(type < NUM_WEAPONS);

  if(NETSTATE_NOGAME != state)
    {
      // no need to send it guaranteed; if it doesn't
      //  get there, the others will know within the next
      //  sync message
      SimpleMessage(0x80 | (BYTE)type,false);
    }
  WriteLog("Sent message: weapon change message, change to weapon %d" LogArg(type));
}

void NetSendPowerUpPickUpMessage(int  powerup_index)
{
  if(NETSTATE_NOGAME == state || powerup_index > 0xff)
    {
      // index too high or not playing a game anyway, so just quit
      return;
    }

  WORD msg = WORD(BYTE(powerup_index));

  // send the message
  dp->SendEx
    (
     us,DPID_ALLPLAYERS,DPSEND_NOSENDCOMPLETEMSG | DPSEND_ASYNC | DPSEND_GUARANTEED,(void *)&msg,2,0,0,NULL,NULL
     );
	
  WriteLog("Sent message: powerup pickup message, picked up powerup w/index of %d" LogArg(powerup_index));
}

void NetSendHitMessage(int player,int weapon_type)
{
  if(NETSTATE_NOGAME == state) {
      GLUenemies[player].SubtractHealth(weapon_type);
  } else {
    // do something
    BYTE send = 4+(BYTE)weapon_type;

    // we hit you! mwa ha ha!
    dp->SendEx(us, remotes[player].id,
	       DPSEND_NOSENDCOMPLETEMSG | DPSEND_ASYNC | DPSEND_GUARANTEED,
	       (void *)&send, 1, 0, 0, NULL, NULL);
  }

  WriteLog("Sent message: hit message, we hit player w/index of %d, w/weapon w/index of %d" LogArg(player) LogArg(weapon_type));
}

void NetSendAdmitHitMessage() {
  if(NETSTATE_NOGAME != state) {
    SimpleMessage(0,false);
  }
  WriteLog("Sent message: admit hit message");
}

void NetSendBazookaFireMessage(FIXEDNUM x,FIXEDNUM y)
{
  if(NETSTATE_NOGAME == state || x >= Fixed(4096) || y >= Fixed(4096)) {
    // coordinates too high, don't care, or not in game...
    return;
  }

  // check validity of x and y (cannot be negative)
  if(x < 0) {x = 0;}
  if(y < 0) {y = 0;}

  DWORD msg; // only three bytes will be transferred
  x = FixedCnvFrom<long>(x);
  y = FixedCnvFrom<long>(y);

  msg = (DWORD)y;
  msg |= ((DWORD)x << 12);

  dp->SendEx(us,DPID_ALLPLAYERS,DPSEND_NOSENDCOMPLETEMSG | DPSEND_ASYNC | DPSEND_GUARANTEED,(void *)&msg,3,0,0,NULL,NULL);

  WriteLog("Sent message: bazooka fire message, explosion at %d,%d" LogArg(FixedCnvFrom<long>(x)) LogArg(FixedCnvFrom<long>(y)));
}

void NetSendDeathMessage(FIXEDNUM *ammo) {
  assert(NETSTATE_NOGAME != state);
  WORD ammo_send[NUM_WEAPONS];
	
  for(int i = 0; i < NUM_WEAPONS; i++) {
    ammo_send[i] = Fixed(1) <= ammo[i] ? 0-1 : (WORD)ammo[i];
  }

  dp->SendEx(us, DPID_ALLPLAYERS,
	     DPSEND_ASYNC | DPSEND_GUARANTEED | DPSEND_NOSENDCOMPLETEMSG,
	     (void *)ammo_send, NUM_WEAPONS * sizeof(WORD),
	     0,0,NULL,NULL);

  WriteLog("Sent message: death message");
}

void NetSendMachineGunFireStartMessage() {
  if(NETSTATE_NOGAME != state) {
    SimpleMessage(2,false);
  }

  WriteLog("Sent message: machine gun fire start message");
}

void NetSendMachineGunFireStopMessage()
{
  if(NETSTATE_NOGAME != state) {
    SimpleMessage(3,false);
  }

  WriteLog("Sent message: machine gun fire stop message");
}

HRESULT CreatePlayer() {
  // now create the player
  DPNAME our_name;
  our_name.dwSize = sizeof(our_name);
  our_name.dwFlags = 0;
  our_name.lpszLongName = our_name.lpszShortName =
    (LPWSTR)GLUname.c_str();
	
  HRESULT hr = dp->CreatePlayer(&us,&our_name,NULL,NULL,0,0);
  if(FAILED(hr)) {return hr;}

  // set player's data
  BYTE player_data = (BYTE)GLUhero.Model(); // set the player's data to its model to use

  hr = dp->SetPlayerData(us,(void *)&player_data,1,DPSET_GUARANTEED | DPSET_REMOTE);
  if(FAILED(hr)) {
    // we failed, destroy the player...
    dp->DestroyPlayer(us);
  }

  return hr;
}

void SimpleMessage(BYTE data, bool guaranteed) {
  // the caller should have checked if we were in the game
  assert(NETSTATE_NOGAME != state);

  DWORD flags = guaranteed
    ? DPSEND_ASYNC | DPSEND_NOSENDCOMPLETEMSG | DPSEND_GUARANTEED
    : DPSEND_ASYNC | DPSEND_NOSENDCOMPLETEMSG;
  
  dp->SendEx(us, DPID_ALLPLAYERS, flags, &data, 1, 0, 0, NULL, NULL);

  WriteLog("Sent a simple message (1-byte of data to all players), "
	   "containing number: %d\n" LogArg((int)data));
}

bool NetSendWeatherStateMessage(int weather_state) {
  if(NETSTATE_JOINING == state) {
    WriteLog("Did not send message: weather state message, but were "
	     "going to, because we are not host\n");
    return false;
  } else if(NETSTATE_HOSTING == state) {
    SimpleMessage(0x40 | weather_state,true);
    WriteLog("Sent message: weather state message\n");
  }	

  return true;
}

void NetSendPistolFireMessage(int direction) {
  if(NETSTATE_NOGAME != state) {
    SimpleMessage(0xc0 | direction,false);
  }

  WriteLog("Sent message: pistol fire message\n");
}
