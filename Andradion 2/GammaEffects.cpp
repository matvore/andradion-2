// GammaEffects.cpp: implementation of the CGammaEffects class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Fixed.h"
#include "GammaEffects.h"

// Comment the next two lines to allow this module to log
#define WriteLog(x)
#undef LogArg
#define LogArg(x)
#undef TryAndReport
#define TryAndReport(x) x

enum {GESTATE_UNINITIALIZED,GESTATE_INITIALIZEDUSING,GESTATE_INITIALIZEDNOTUSING}; // gamma effects states
enum {GETYPE_NONE,GETYPE_BLOOD,GETYPE_HEALTH,GETYPE_AMMO}; // gamma effects types

// effects parameters
const int GE_FRAMESTOBLOOD =  18;
const int GE_FRAMESTOAMMO =   6;
const int GE_FRAMESTOHEALTH = 6;

static int state = GESTATE_UNINITIALIZED;
static DWORD frames_since_effect_started;
static int effect_type;
static LPDIRECTDRAWGAMMACONTROL gamma_con;

void GamOneFrame()
{
  assert(GESTATE_UNINITIALIZED != state);

  if(GESTATE_INITIALIZEDNOTUSING == state || GETYPE_NONE == effect_type)
    {
      return;
    }

  FIXEDNUM scale = FixedCnvTo<long>(frames_since_effect_started);
  DDGAMMARAMP ramp;
  int lim = 0;
  DWORD max_time;

  switch(effect_type)
    {
    case GETYPE_BLOOD:
      {
	memset((void *)&ramp.green[0],0,256*sizeof(WORD));
	scale /= GE_FRAMESTOBLOOD;

	WORD *r = &ramp.red[0];
	WORD *g = &ramp.green[0];

	for(int i = 0; i < 65536; i+=256)
	  {
	    *r++ = (WORD)i;
	    *g++ = (WORD)FixedCnvFrom<long>(scale * i);
	  }

	// copy green ramp to blue
	memcpy((void *)&ramp.blue[0],(const void *)&ramp.green[0],sizeof(WORD)*256);

	max_time = GE_FRAMESTOBLOOD;
	break;
      }
    case GETYPE_HEALTH:
      {
	scale <<= 8;
	scale /= GE_FRAMESTOHEALTH;		

	lim = FixedCnvFrom<long>(scale);
	if(lim < 0)
	  {
	    lim = 0;
	  }
	else if(lim > 256)
	  {
	    lim = 256;
	  }
		
	memset((void *)(ramp.red+lim),0xff,(256-lim)*sizeof(WORD));
	memset((void *)(ramp.green+lim),0xff,(256-lim)*sizeof(WORD));
	memset((void *)(ramp.blue+lim),0xff,(256-lim)*sizeof(WORD));

	max_time =	GE_FRAMESTOHEALTH;
	break;
      }
    case GETYPE_AMMO:
      {
	memset((void *)&ramp.red[0],0,256*sizeof(WORD));

	scale /= GE_FRAMESTOAMMO;
	scale += Fixed(15.0f);
	scale >>= 4;

	WORD *g = &ramp.green[0];
	WORD *b = &ramp.blue[0];

	for(int i = 0; i < 65536; i+=256)
	  {
	    *g++ = *b++ = (WORD)FixedCnvFrom<long>(scale * i);
	  }

	max_time = GE_FRAMESTOAMMO;
      }
    }
		
  if(++frames_since_effect_started >= max_time) 
    {
      effect_type = GETYPE_NONE;
      lim = 256;
    }

  // reset gamma ramp
  for(int i = 0,val = 0; i < lim; i++,val+=256)
    {
      ramp.red[i] = ramp.green[i] = ramp.blue[i] = (WORD)val;
    }

  // set the gamma ramp if it is possible
  if(GESTATE_INITIALIZEDUSING == state)
    {
      gamma_con->SetGammaRamp(0,&ramp);
    }
}

void GamInitialize(CGraphics &gr)
{
  assert(GESTATE_UNINITIALIZED == state);
  effect_type = GETYPE_NONE;
  state = GESTATE_INITIALIZEDNOTUSING; // assume we are not using it

  // get interface to DirectDrawSurface7
  LPDIRECTDRAWSURFACE7 dds7;
  if(FAILED(gr.Buffer(0)->QueryInterface(IID_IDirectDrawSurface7,(void **)&dds7)))
    {
      return;
    }

  // we got gamma control supported!
  if(SUCCEEDED(dds7->QueryInterface(IID_IDirectDrawGammaControl,(void **)&gamma_con)))
    {
      // gamma control is supported
      state = GESTATE_INITIALIZEDUSING;
    }

  TryAndReport(dds7->Release());
}

void GamRelease()
{
  if(GESTATE_INITIALIZEDUSING == state)
    {
      TryAndReport(gamma_con->Release());
      gamma_con = NULL;
    }

  state = GESTATE_UNINITIALIZED;
}

void GamPickupAmmo()
{
  assert(GESTATE_UNINITIALIZED != state);
  effect_type = GETYPE_AMMO;
  frames_since_effect_started = 0;
}

void GamPickupHealth()
{
  assert(GESTATE_UNINITIALIZED != state);
  effect_type = GETYPE_HEALTH;
  frames_since_effect_started = 0;
}

void GamGetShot()
{
  assert(GESTATE_UNINITIALIZED != state);
  effect_type = GETYPE_BLOOD;
  frames_since_effect_started = 0;
}

