// Graphics.cpp: implementation of the CGraphics class.
//
//////////////////////////////////////////////////////////////////////

#include "Certifiable.h"
#include "SurfaceLock.h"
#include "Color.h"
#include "ColorNP.h"
#include "Color256.h"
#include "CompactMap.h"
#include "Graphics.h"
#include "LazyErrHandling.h"
#include "logger.h"
#include "Timer.h"
#include "StdAfx.h"
#include "Bob.h"

// Comment the next five lines to allow this module to log
/*#define WriteLog(x)
#undef LogArg
#define LogArg(x)
#undef TryAndReport
#define TryAndReport(x) x*/

using std::random_shuffle;
using std::vector;

#ifdef BORLAND
#define __min min
#define __max max
using std::min;
using std::max;
#endif

namespace NGameLib2
{
  bool          CGraphics::_15b_color      = false ;
  int           CGraphics::bytes_per_pixel = 0     ;
  int           CGraphics::mode_height     = 0     ;
  int           CGraphics::mode_width      = 0     ;
  IDirectDraw2 *CGraphics::dd              = NULL  ;
  IDirectDraw4 *CGraphics::dd4             = NULL  ;

  static HRESULT WINAPI EnumSurfacesCallback(
					     LPDIRECTDRAWSURFACE lpDDSurface,
					     DDSURFACEDESC *,
					     void *cgraphics)
  {
    CGraphics *g = (CGraphics *)cgraphics;

    int current_buffer_index = g->GetTargetBufferIndex();

    // get the  newer interface of the surface
    HRESULT res;
    MemoryAllocFunction
      (
       res = lpDDSurface->QueryInterface(IID_IDirectDrawSurface2,(void **)&g->buffers[current_buffer_index]),
       sizeof(IDirectDrawSurface2),
       FAILED(res)
       );

    g->current_buffer = g->buffers[current_buffer_index+1];

    return DDENUMRET_OK; // continue the enumeration
  }

  CGraphics::CGraphics(
		       HWND hWnd_,int allow_mode_x_,int mode_width_,int mode_height_,
		       int mode_bpp_,int num_buffers_,const GUID& directdraw_driver,
		       int mode_refresh_,
		       DWORD advanced_coop_flags_or_,DWORD advanced_coop_flags_and_) :
    CCertifiable(),
    hWnd(hWnd_),current_buffer(0),
    mode_bpp(mode_bpp_),mode_refresh(mode_refresh_),num_buffers(num_buffers_),
    allow_mode_x(allow_mode_x_),advanced_coop_flags_or(advanced_coop_flags_or_),
    advanced_coop_flags_and(advanced_coop_flags_and_)
  {
    assert(NULL == dd);

    this->mode_dim.cx = mode_width_;
    this->mode_dim.cy = mode_height_;

    // create the directdraw instance
    CoInitialize(NULL);
    CoCreateInstance
      (
       CLSID_DirectDraw,
       NULL,
       CLSCTX_ALL,
       IID_IDirectDraw2,
       (void **)&dd
       );

    HRESULT res;
    // make a copy of the direct draw driver guid we need that is not
    //  constant.  For some reason, the directdraw::Initialize function needs
    //  a non-constant copy.
    GUID non_const_copy = directdraw_driver;
    GUID *non_const_ptr = GUID_NULL == non_const_copy ? NULL : &non_const_copy;
    MemoryAllocFunction(
			res = dd->Initialize(non_const_ptr),
			sizeof(IDirectDraw2),
			DDERR_OUTOFMEMORY == res
			);

    // try to get an enhanced interface
    if(FAILED(dd->QueryInterface(IID_IDirectDraw4,(void **)&dd4)))
      {
	WriteLog("IDirectDraw4 not available- any Mode 13h request will turn into Mode X 320x200");
	dd4 = NULL;
      }
  }

  CGraphics::CGraphics() : CCertifiable()
  {
    assert(NULL == dd);

    // this version of the constructor will
    //  create a DirectDraw object using the
    //  default driver
    CoInitialize(NULL);
    CoCreateInstance
      (
       CLSID_DirectDraw,
       NULL,
       CLSCTX_ALL,
       IID_IDirectDraw2,
       (void **)&dd
       );

    HRESULT res;
		
    MemoryAllocFunction
      (
       res = dd->Initialize(NULL),
       sizeof(IDirectDraw2),
       DDERR_OUTOFMEMORY == res
       );

    // try to get an enhanced interface
    if(FAILED(dd->QueryInterface(IID_IDirectDraw4,(void **)&dd4)))
      {
	WriteLog("IDirectDraw4 not available- any Mode 13h request will turn into Mode X 320x200");
	dd4 = NULL;
      }
  }

  CGraphics::~CGraphics() 
  {
    if(this->Certified())
      {
	this->Uncertify(); // terminate our graphics mode
      }

    if(NULL != dd4)
      {
	dd4->Release();
	dd4 = NULL;
      }
		
    dd->Release();
    dd = NULL;

    CoUninitialize();
  }

  int CGraphics::Certify()
  {
    if(this->Certified())
      {
	return 0; // don't certify twice
      }

    // check values of various parameters
    assert(mode_refresh >= 0);
    WriteLog("Number of buffers: %d" LogArg(num_buffers));
    assert(num_buffers > 0);
    assert(num_buffers <= MAX_BUFFERS);
    assert(mode_dim.cx > 0);
    assert(mode_dim.cy > 0);

    // now set the flags for the coop level
    DWORD flags = 0;

    // allow these flags nomatter what
    if(NULL == this->hWnd)
      {
	// we are in windowed mode
	flags |= DDSCL_NORMAL;
      } // end if in windowed mode
    else
      {
	// we are in fullscreen mode
	flags |= DDSCL_FULLSCREEN;
	flags |= DDSCL_EXCLUSIVE;
	flags |= DDSCL_ALLOWREBOOT;
	if(MXS_NONE != this->allow_mode_x)
	  {
	    flags |= DDSCL_ALLOWMODEX;
	  } // end if wishes for modex
      } // end if in fullscreen mode


    flags |= this->advanced_coop_flags_or;
    flags &= this->advanced_coop_flags_and;

    // call the actual set coop level function until we have enough memory
    HRESULT res; // result of some ddraw calls
    MemoryAllocFunction(
			res = TryAndReport(dd->SetCooperativeLevel(this->hWnd,flags)),
			1,
			FAILED(res)
			);

    if(MXS_13 == allow_mode_x && NULL != dd4)
      {
	WriteLog("13h under IDirectDraw4 used");
	res = CGraphics::dd4->SetDisplayMode(mode_dim.cx,mode_dim.cy,this->mode_bpp,this->mode_refresh,DDSDM_STANDARDVGAMODE);
      } // end if using modex
    else if(NULL != hWnd)
      {
	// we aren't supposed to be in modex, but we aren't windowed
	// either.  So set the video mode normally
	res = TryAndReport(CGraphics::dd->SetDisplayMode(mode_dim.cx,mode_dim.cy,
							 this->mode_bpp,
							 this->mode_refresh,0));
      } // end if in fullscreen mode

    // check for failure
    if(FAILED(res))
      {
	return 1;
      }

    // create the primary surface
    DDSURFACEDESC surf;
    memset((void *)&surf,0,sizeof(surf));
    surf.dwSize = sizeof(surf);

    surf.ddsCaps.dwCaps |= DDSCAPS_PRIMARYSURFACE;
		
    if(num_buffers > 1 && NULL != this->hWnd)
      {
	surf.dwBackBufferCount = num_buffers - 1;
	surf.dwFlags |= DDSD_BACKBUFFERCOUNT;
	surf.ddsCaps.dwCaps |= DDSCAPS_COMPLEX;
	surf.ddsCaps.dwCaps |= DDSCAPS_FLIP;
      }
	
    surf.dwFlags |= DDSD_CAPS;

    LPDIRECTDRAWSURFACE version1_of_primary;

    MemoryAllocFunction(
			res = TryAndReport(dd->CreateSurface(&surf,&version1_of_primary,NULL)),
			sizeof(IDirectDrawSurface2),
			FAILED(res)
			);

    // get the newer version of the direct draw surface interface
    MemoryAllocFunction(
			res = TryAndReport(version1_of_primary->QueryInterface(IID_IDirectDrawSurface2,(void **)&this->buffers[0])),
			sizeof(IDirectDrawSurface2),
			FAILED(res)
			);

    // release the older version of the directdraw surface interface
    version1_of_primary->Release();

    if(num_buffers > 1 && NULL != this->hWnd)
      {
	current_buffer = this->buffers[1]; // use current buffer member to keep count of enumerated buffers

	// create the secondary surfaces
	this->buffers[0]->EnumAttachedSurfaces(this,EnumSurfacesCallback);

	current_buffer = this->buffers[1]; // reset count
      }
    else
      {
	current_buffer = this->buffers[0];

	if(NULL == this->hWnd)
	  {
	    // let's make all the buffers manually
	    for(int i = 1; i < this->num_buffers; i++)
	      {
		memset((void *)&surf,0,sizeof(surf));
		surf.dwSize = sizeof(surf);
		surf.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		surf.dwWidth = this->mode_dim.cx;
		surf.dwHeight = this->mode_dim.cy;
		surf.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		LPDIRECTDRAWSURFACE os; // old version surface
		MemoryAllocFunction
		  (
		   res = TryAndReport(dd->CreateSurface(&surf,&os,NULL)),
		   sizeof(IDirectDrawSurface),
		   FAILED(res)
		   );
		
		// get the newer version of the direct draw surface interface
		MemoryAllocFunction
		  (
		   res = TryAndReport(os->QueryInterface(IID_IDirectDrawSurface2,(void **)&this->buffers[i])),
		   sizeof(IDirectDrawSurface2),
		   FAILED(res)
		   );

		// release the older version of the directdraw surface interface
		os->Release();
	      } // end for each buffer we must make
	  } // end if in windowed mode
      }

    // if we are in windowed mode, we must get the bit depth of the
    // current video mode
    if(NULL == this->hWnd)
      {
	memset((void *)&surf,0,sizeof(surf));
	surf.dwSize = sizeof(surf);
	this->dd->GetDisplayMode(&surf);
	this->mode_bpp = surf.ddpfPixelFormat.dwRGBBitCount;
	if(15 == this->mode_bpp) {this->mode_bpp = 16;}
      } // end if in windowed mode

    if(this->mode_bpp > PALETTIZED_COLOR_8B)
      {
	DDPIXELFORMAT pf; // pixel format of the new surfaces
		
	// setup the pf data structure
	memset((void *)&pf,0,sizeof(pf));
	pf.dwSize = sizeof(pf);

	// get the pixel format data
	this->current_buffer->GetPixelFormat(&pf);

	_15b_color = bool(15 == pf.dwRGBBitCount);

	// find bytes per pixel
	bytes_per_pixel = pf.dwRGBBitCount / 8;
	if(0 != pf.dwRGBBitCount%8)
	  {
	    bytes_per_pixel++;
	  }
      }
    else
      {
	// 8-bit palettized mode used
	CGraphics::_15b_color = false;
	CGraphics::bytes_per_pixel = 1;
      }

    // setup the simple clipper rect to the default
    this->simple_clipper_rect.left = 0;
    this->simple_clipper_rect.top = 0;
    this->simple_clipper_rect.right = this->mode_dim.cx;
    this->simple_clipper_rect.bottom = this->mode_dim.cy;

    mode_width = this->mode_dim.cx;
    mode_height = this->mode_dim.cy;

    return CCertifiable::Certify();
  }

  int CGraphics::Recertify(int allow_mode_x_,
			   int mode_width_,int mode_height_,int mode_bpp_,
			   int num_buffers_,
			   int mode_refresh_,
			   DWORD advanced_coop_flags_or_,
			   DWORD advanced_coop_flags_and_)
  {
    this->Uncertify();
		
    this->allow_mode_x = allow_mode_x_;
    this->mode_dim.cx = mode_width_;
    this->mode_dim.cy = mode_height_;
    this->mode_bpp = mode_bpp_;
    this->num_buffers = num_buffers_;
    this->mode_refresh = mode_refresh_;
    this->advanced_coop_flags_or = advanced_coop_flags_or_;
    this->advanced_coop_flags_and = advanced_coop_flags_and_;		

    return this->Certify();
  }

  void CGraphics::Uncertify()
  {
    assert(Certified());

    // leave graphics
    // release all buffers, starting with the last in the chain
    for(int i = this->num_buffers - 1; i >= 0; i--)
      {
	this->buffers[i]->Release();
      }

    dd->RestoreDisplayMode();
    dd->SetCooperativeLevel(hWnd,DDSCL_NORMAL);

    CCertifiable::Uncertify();
  }

  void CGraphics::Transition(int transition) 
  {
    // perform a screen transition
    //  (at the end of this function, the current target buffer
    //   will have been copied to the front buffer; no
    //   clippers allowed)
    // surface description and pointer shortcuts:
    LPDIRECTDRAWSURFACE2 front_buffer = this->buffers[0];
    LPDIRECTDRAWSURFACE2 back_buffer = this->current_buffer;
    CTimer timer;
				
    // sanity checks
    assert(num_buffers > 1);
    assert(this->Certified()); // need double buffering for these effects

    // seed random number generator
    srand((unsigned int)GetTickCount());

    switch(transition) {
    case TRANSITION_HORIZONTAL_SWIPE: {
      RECT rect;

      // set the constant rect properties:
      rect.top = 0;
      rect.bottom = CGraphics::mode_height;

      for(rect.right = 2;rect.right <= CGraphics::mode_width;rect.right+=2)
	{
	  timer.Restart();
	  rect.left = rect.right - 2;
			
	  // ready to blt to surface
	  while(
		DDERR_WASSTILLDRAWING ==
		front_buffer->BltFast(rect.left,rect.top,back_buffer,&rect,DDBLTFAST_NOCOLORKEY)
		);

	  while(timer.SecondsPassed32() < 1.0f/float(CGraphics::mode_width/2));
	}
      break; }
    case TRANSITION_BLINDS: {
      RECT rect;

      // set the constant rect values:
      rect.left = 0;
      rect.right = CGraphics::mode_width;

      // there are 6 blinds
      for(int i = 1; i <= CGraphics::mode_width / 6; i++) {
	timer.Restart();

	for(int j = 0; j < 6; j++)
	  { 
	    // must loop the blinds here:
	    rect.top = j * (CGraphics::mode_width / 6);
	    rect.bottom = rect.top+i;
				
	    // ready to blt to surface
	    while(
		  DDERR_WASSTILLDRAWING ==
		  front_buffer->BltFast(rect.left,rect.top,back_buffer,&rect,DDBLTFAST_NOCOLORKEY)
		  );
	  }
			
	while(timer.SecondsPassed32() < 12.0f/float(CGraphics::mode_height));;
      }
      break; }
    case TRANSITION_JEOPARDY_BLOCKS: {
      vector<RECT> jeopardy(48); // jeopardy rectangles
      int i;

      // puts a bunch of blocks on the screen
      //  (8 columns x 6 rows)

      // create an organize list of rectangles:
      for(i = 0; i < 48; i++)
	{
	  jeopardy[i].left = (i % 8) * (CGraphics::mode_width/8);
	  jeopardy[i].right = jeopardy[i].left + (CGraphics::mode_width/8);
	  jeopardy[i].top = (i / 8) * (CGraphics::mode_height/6);
	  jeopardy[i].bottom = jeopardy[i].top + (CGraphics::mode_height/6);
	}

      // now let's mess that list up by shuffling its
      //  contents
      random_shuffle(jeopardy.begin(),jeopardy.end());

      // and at last, now we blit the blocks to the screen:
      for(i = 0; i < 48; i++) 
	{
	  timer.Restart();

	  // ready to blt to surface
	  while(
		DDERR_WASSTILLDRAWING ==
		front_buffer->BltFast(
				      jeopardy[i].left,jeopardy[i].top,
				      back_buffer,
				      &jeopardy[i],
				      DDBLTFAST_NOCOLORKEY
				      )
		);
			
	  while(timer.SecondsPassed32() < 1.0f/24.0f);
	}
      break; }
    case TRANSITION_MELTING: {
      int i;
      int mw = CGraphics::mode_width;
      vector<int> worm_progress(mw,0);
      vector<int> worm_speed(mw);

      for(i = 0; i < mw; i++)
	{
	  worm_speed[i] = int(
			      float((rand()%4)+5) * ((float)CGraphics::mode_height/200.0)
			      );
	}

      // give them all loops to get to the bottom
      for(i = 0; i < 60; i++)
	{
	  timer.Restart();

	  // move the worms and draw:
	  for(int j = 0;j < mw;j++)
	    {
	      RECT rect;
	      rect.top = worm_progress[j];
	      rect.left = j;
	      rect.right = j+1;
	      worm_progress[j] = __min(worm_progress[j]+worm_speed[j],CGraphics::mode_height);
	      rect.bottom = worm_progress[j];
	      while(
		    DDERR_WASSTILLDRAWING ==
		    front_buffer->BltFast(rect.left,rect.top,back_buffer,&rect,DDBLTFAST_NOCOLORKEY)
		    );
	    }
	  // wait a while:
	  while(timer.SecondsPassed32() < 1.0f/30.0f);
	}
    }
    }
  }

  HRESULT CGraphics::Flip(DWORD flags)
  {
    return this->buffers[0]->Flip(NULL,flags);
  }

  HRESULT CGraphics::Put(CBob& bob,DWORD blt_behaviour_flags,DDBLTFX *blt_fx,bool wait_then_blit)
  {
    HRESULT ret;

    do
      {
	ret = this->current_buffer->Blt(&this->current_area,bob.data,NULL,blt_behaviour_flags,blt_fx);
      }
    while(true == wait_then_blit && DDERR_WASSTILLDRAWING == ret);

    return ret;
  }

  HRESULT CGraphics::RectangleInternal(DWORD c,bool wait_then_blit,DWORD behav)
  {
    assert(this->Certified());
    DDBLTFX fx;

    // init fx struct
    fx.dwSize = sizeof(fx);

    fx.dwFillColor = c;

    HRESULT ret;

    behav |= DDBLT_COLORFILL;

    do
      {
	ret = this->current_buffer->Blt(&this->current_area,NULL,NULL,behav,&fx);
      }
    while(true == wait_then_blit && DDERR_WASSTILLDRAWING == ret);

    return ret;
  }

  HRESULT CGraphics::PutFastClip(CBob& bob,bool use_transparency,bool wait_then_blit,DWORD trans_flags)
  {
    DWORD base_flag = (true == use_transparency ? DDBLTFAST_SRCCOLORKEY : DDBLTFAST_NOCOLORKEY);

    int x = this->current_area.left;
    int y = this->current_area.top;
    int bw = this->current_area.right - x; // bob width
    int bh = this->current_area.bottom - y; // and height

    // create abbreviated local variables
    int cl = this->simple_clipper_rect.left; // clipper left
    int cr = this->simple_clipper_rect.right; // clipper right
    int ct = this->simple_clipper_rect.top; // clipper top
    int cb = this->simple_clipper_rect.bottom; // clipper bottom

    RECT s =
      {
	__max(0,cl-x),
	__max(0,ct-y),
	__min(bw,cr-x),
	__min(bh,cb-y)
      };

    if(x < cl)
      {
	x = cl;
      }
    if(y < ct)
      {
	y = ct;
      }
		
    if(x+bw > 0 && y+bh > 0 && x < cr && y < cb)
      {
	HRESULT ret;
	do
	  {
	    ret = this->current_buffer->BltFast(x,y,bob.data,&s,trans_flags|base_flag);
	  }
	while(true == wait_then_blit && DDERR_WASSTILLDRAWING == ret);
	return ret;
      }
    else
      {
	return DD_OK;
      }
  }

  HRESULT CGraphics::PutFast(CBob& bob,bool use_transparency,bool wait_then_blit,DWORD trans_flags)
  {
    DWORD base_flag = (true == use_transparency ? DDBLTFAST_SRCCOLORKEY : DDBLTFAST_NOCOLORKEY);

    HRESULT ret;

    do
      {
	ret= this->current_buffer->BltFast(
					   this->current_area.left,
					   this->current_area.top,
					   bob.data,
					   NULL,
					   trans_flags|base_flag
					   );
      }
    while(wait_then_blit == true && DDERR_WASSTILLDRAWING == ret);

    return ret;
  }

  // clipper creation function
  LPDIRECTDRAWCLIPPER CGraphics::CreateClipper(const vector<RECT>& list,bool set_hwnd) const
  {
    assert(this->Certified());

    LPDIRECTDRAWCLIPPER c; // the clipper we will return
    HRESULT res;
		
    MemoryAllocFunction(
			res=CGraphics::dd->CreateClipper(0,&c,NULL),
			sizeof(IDirectDrawClipper),
			DDERR_OUTOFMEMORY == res
			);

    assert(DDERR_INVALIDCLIPLIST != res);
    assert(DDERR_INVALIDOBJECT != res);
    assert(DDERR_INVALIDPARAMS != res);

    // set clip list:
    if(list.size() > 0)
      {
	// allocate memory for clip list
	LPRGNDATA cr =
	  LPRGNDATA(new BYTE[sizeof(RGNDATAHEADER)+sizeof(RECT)*list.size()]);

	// figure out rcBound and other members of the rect data header while filling in the rect buffer
	cr->rdh.dwSize = sizeof(RGNDATAHEADER);
	cr->rdh.iType = RDH_RECTANGLES;
	cr->rdh.nCount = list.size();
	cr->rdh.nRgnSize = sizeof(RECT)*list.size();
	// initiate rcBound with ridiculous values
	cr->rdh.rcBound.left = CGraphics::mode_width;
	cr->rdh.rcBound.top = CGraphics::mode_height;
	cr->rdh.rcBound.bottom = 0;
	cr->rdh.rcBound.right = 0;
	VCTR_RECTANGLE::const_iterator rect_i;
	RECT *buffer = (RECT *)cr->Buffer;
	for(rect_i = list.begin(); rect_i != list.end(); rect_i++)
	  {
	    if(cr->rdh.rcBound.left > rect_i->left)
	      {
		cr->rdh.rcBound.left = rect_i->left;
	      }
	    if(cr->rdh.rcBound.top > rect_i->top)
	      {
		cr->rdh.rcBound.top = rect_i->top;
	      }
	    if(cr->rdh.rcBound.bottom < rect_i->bottom)
	      {
		cr->rdh.rcBound.bottom = rect_i->bottom;
	      }
	    if(cr->rdh.rcBound.right < rect_i->right)
	      {
		cr->rdh.rcBound.right = rect_i->right;
	      }
	    memcpy((void *)buffer++,(const void *)rect_i,sizeof(RECT));
	  }			
	
	// set clipper clip list
	MemoryAllocFunction(
			    res = c->SetClipList(cr,0),
			    sizeof(RECT)*list.size() + sizeof(RGNDATAHEADER),
			    FAILED(res)
			    );

	// done with the region data
	delete (BYTE *)cr;
      }

    if(true == set_hwnd)
      {
	MemoryAllocFunction(
			    res = c->SetHWnd(0,this->hWnd),
			    sizeof(HWND),
			    DDERR_OUTOFMEMORY == res
			    );
      }

    return c;
  }

  LPDIRECTDRAWPALETTE CGraphics::CreatePalette(
					       bool for_primary,
					       PALETTEENTRY *init,
					       bool can_change_256,
					       bool vb_sync,
					       DWORD advanced_flags_or,
					       DWORD advanced_flags_and
					       ) const
  {
    assert(this->Certified());

    DWORD create = 0; // creation flags

    if(true == for_primary)
      {
	create |= DDPCAPS_PRIMARYSURFACE;
      }

    if(true == vb_sync)
      {
	create |= DDPCAPS_VSYNC;
      }

    if(true == can_change_256)
      {
	create |= DDPCAPS_ALLOW256;
      }

    create |= DDPCAPS_8BIT;

    // add customization to creation flags
    create |= advanced_flags_or;
    create &= advanced_flags_and;

    // the creation flags have been setup, so now
    //  let's do the initiation array
    vector<PALETTEENTRY> pev(0);
    if(NULL == init)
      {
	PALETTEENTRY default_color = {0,0,0,0};
	pev.resize(256,default_color);
	init = &pev[0];
      }

    LPDIRECTDRAWPALETTE p;
    HRESULT res;

    MemoryAllocFunction(
			res = CGraphics::dd->CreatePalette(create,init,&p,NULL),
			sizeof(PALETTEENTRY)*256,
			DDERR_OUTOFMEMORY == res
			);

    return p;
  }

  HRESULT CGraphics::Rectangle(const CColorNP& c,bool wait_then_blit,DWORD behav) 
  {
    return this->RectangleInternal(c.Color32b(),wait_then_blit,behav);
  }

  HRESULT CGraphics::Rectangle(BYTE c,bool wait_then_blit,DWORD behav)
  {
    return this->RectangleInternal((DWORD)c,wait_then_blit,behav);
  }

  LPDIRECTDRAWSURFACE2 CGraphics::Buffer(int i)
  {
    assert(i >= 0);
    assert(i < this->num_buffers);
    assert(this->Certified());
    return this->buffers[i];
  }

  RECT& CGraphics::SimpleClipperRect()
  {
    assert(this->Certified());
    return this->simple_clipper_rect;
  }

  const RECT& CGraphics::cSimpleClipperRect() const
  {
    assert(this->Certified());
    return this->simple_clipper_rect;
  }

  bool CGraphics::ClipLine(int& x0,int& y0,int& x1,int& y1) const
  {
    assert(this->Certified());
		
    const RECT *c = &this->simple_clipper_rect; // make a pointer shortcut so we are faster

    /// record whether or not each point is visible by testing them within the clipping region
    bool visible0 = bool(x0 >= c->left && x0 < c->right && y0 >= c->top && y0 < c->bottom);
    bool visible1 = bool(x1 >= c->left && x1 < c->right && y1 >= c->top && y1 < c->bottom);
				
    // if both points are int the viewport, we have already finished
    if(true == visible0 && true == visible1)
      {
	return true;
      }

    // see if lines are completely invisible by running two tests:
    //  1 both points must be outside the clipping viewport
    //  2 both points must be on the same side of the clipping viewport

    if((const bool)visible0 == visible1)
      {
	// neither point is visible
	//  now do test 2.  If it doesn't pass test 2, then we continue normally
	if
	  (
	   (x0 < c->left && x1 < c->left) ||
	   (x0 >= c->right && x1 >= c->right) ||
	   (y0 < c->top && y1 < c->top) ||
	   (y0 >= c->bottom && y1 >= c->bottom)
	   )
	  {
	    return false;
	  }
	// and we're done for completely visible or completely invisible lines
      }

    bool right_edge = false;
    bool left_edge = false;
    bool top_edge = false;
    bool bottom_edge = false;

    int xi; // points of intersection
    int yi;

    bool success;

    if(visible1 == false || visible0 == true)
      {
	// compute deltas
	int dx = x1 - x0;
	int dy = y1 - y0;

	// compute which boundary lines need to be clipped against
	if(x1 >= c->right)
	  {
	    // flag right edge
	    right_edge = true;

	    // compute intersection with right edge
	    assert(0 != dx);
	    yi = (int)(0.5f + ((float)dy/(float)dx) * float(c->right-1 - x0) + (float)y0);

	  } // end if right edge
	else if(x1 < c->left)
	  {
	    // flags left edge
	    left_edge = true;

	    // compute intersection with left edge
	    assert(0 != dx);
	    yi = (int)(0.5f + ((float)dy/(float)dx) * float(c->left - x0) + (float)y0);
	  }
		
	// that's it for left-right side intersections.  now for top-bottom intersections
	if(y1 >= c->bottom)
	  {
	    // flag edge
	    bottom_edge = true;

	    // compute intersection with bottom edge
	    assert(0 != dy);
	    xi = (int)(0.5f + ((float)dx/(float)dy) * float(c->bottom-1 - y0) + (float)x0);
	  }
	else if(y1 < c->top)
	  {
	    top_edge =true;

	    assert(0 != dy);
	    xi = (int)(0.5f + ((float)dx/(float)dy) * float(c->top - y0) + (float)x0);
	  }

	// now we know which line we passed through
	//  figure which edge is the right intersection
	if(yi >= c->top && yi < c->bottom)
	  {
	    if(true == right_edge)
	      {
		x1 = c->right - 1;
		y1 = yi;

		success = true;
	      } // end if intersected right edge
	    else if(true == left_edge)
	      {
		x1 = c->left;
		y1 = yi;

		success = true;
	      }
	  }

	if(xi >= c->left && xi < c->right)
	  {
	    if(true == bottom_edge)
	      {
		x1 = xi;
		y1 = c->bottom-1;
	
		success = true;
	      }
	    else if(true == top_edge)
	      {
		x1 = xi;
		y1 = c->top;

		success = true;
	      }
	  }
      }

    // reset edge flags
    top_edge = false;
    bottom_edge = false;
    left_edge = false;
    // right_edge = true . . . no wait! I meant . . .
    right_edge = false;

    // now we have to do the same friggin thing for the second endpoint
	

    if(true == visible1 || false == visible0)
      {
	// compute deltas
	int dx = x0 - x1;
	int dy = y0 - y1;

	// compute what boundary line needs to be clipped against
	if(x0 >= c->right)
	  {
	    // flag right edge
	    right_edge = true;

	    // computer intersection with right edge
	    assert(0 != dx);
	    yi = (int)(0.5f + ((float)dy/(float)dx) * float(c->right - 1 - x1) + (float)y1);
	  }	
	else if(x0 < c->left)
	  {
	    // flag left edge
	    left_edge = true;

	    // compute intersection with left edge
	    assert(0 != dx);
	    yi = (int)(0.5f + ((float)dy/(float)dx) * float(c->left - x1) + (float)y1);
	  }

	// bottom-top edge intersections
	if(y0 >= c->bottom)
	  {
	    bottom_edge = true;

	    // compute intersection with bottom edge
	    assert(0 != dy);
	    xi = (int)(0.5f + ((float)dx/(float)dy) * float(c->bottom - 1 - y1) + (float)x1);
	  }
	else if(y0 < c->top)
	  {
	    top_edge = true;

	    // computer intersection with bottom edge
	    assert(0 != dy);
	    xi = (int)(0.5f + ((float)dx/(float)dy) * float(c->top - y1) + (float)x1);
	  }

	// compute which edge is the proper intersection
	if(yi >= c->top && yi < c->bottom)
	  {
	    if(true == right_edge)
	      {
		x0 = c->right - 1;
		y0 = yi;
		success= true;
	      }
	    else if(true == left_edge)
	      {
		x0 = c->left;
		y0 = yi;
		success= true;
	      }
	  }	

	if(xi >= c->left && xi < c->right)
	  {
	    if(true == bottom_edge)
	      {
		x0 = xi;
		y0 = c->bottom -1;
		success = true;
	      }
	    else if(true == top_edge)
	      {	
		x0 = xi;
		y0 = c->top;
		success = true;
	      }
	  }
      }
    return success;
  }

  DWORD CGraphics::Screenshot(HANDLE file)
  {
    assert(this->current_area.left < this->current_area.right);
    assert(this->current_area.top < this->current_area.bottom);

    // calculate bytes per pixel
    int bypp = CGraphics::bytes_per_pixel;

    // let's create an offscreen surface to contain the
    //  image data

    DDSURFACEDESC surf_desc;
    memset((void *)&surf_desc,0,sizeof(surf_desc));

    // set the appropriate data members so we will create a surface
    //  equal to the width and height of the current area
    surf_desc.dwSize = sizeof(surf_desc);
    surf_desc.dwFlags = (DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS);
    surf_desc.dwWidth = this->current_area.right - this->current_area.left;
    surf_desc.dwHeight = this->current_area.bottom - this->current_area.top;
    surf_desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

    LPDIRECTDRAWSURFACE version1_of_surf;
    if(FAILED(CGraphics::dd->CreateSurface(&surf_desc,&version1_of_surf,NULL)))
      {
	return 0;
      }

    // get the newer version of the surface
    LPDIRECTDRAWSURFACE2 surf;
    HRESULT res;
    MemoryAllocFunction(
			res = version1_of_surf->QueryInterface(IID_IDirectDrawSurface2,(void **)&surf),
			sizeof(IDirectDrawSurface2),
			FAILED(res)
			);

    // release the older version of the surface
    version1_of_surf->Release();

    // now copy the front buffer to this offscreen buffer
    while(DDERR_WASSTILLDRAWING == surf->Blt(NULL,this->buffers[0],&this->current_area,0,NULL));

    // lock the offscreen buffer
    LONG pitch;
    LONG width = this->current_area.right - this->current_area.left;
    LONG height = this->current_area.bottom - this->current_area.top;
    BYTE *buffer8;
    bool must_convert_555_to_565;
    if(FAILED(surf->Lock(NULL,&surf_desc,DDLOCK_WAIT|DDLOCK_READONLY,NULL)))
      {
	// the lock failed... synthesize the variables so the rest of the
	//  function doesn't know that!
	buffer8 = new BYTE[width*bypp];
	pitch = 0;
	surf->Release();
	surf = NULL; // make surface NULL so we know we didn't lock it
	must_convert_555_to_565 = false;
			
      }
    else
      {
	// the lock succeeded, so stuff some precalculated values and typing shortcuts
	//  into the following variables for convenience
	buffer8 = (BYTE *)surf_desc.lpSurface;
	pitch = surf_desc.lPitch;
	must_convert_555_to_565 = bool(15 == surf_desc.ddpfPixelFormat.dwRGBBitCount);
      }
		
    DWORD return_value = 0;

    // make an extra copy of the buffer
    //  buffer16 will be used during the copy process
    //  buffer8 will mark the beginning of the buffer
    //   and will be used to delete it at the end
    //   in the case where the lock failed
    WORD *buffer16 = (WORD *)(buffer8 + (height-1) * pitch);

    // now we will write the results to a bmp file
    DWORD size_of_not_last_sections = // calculate a head of time the size of the non-color data of the bmp
      sizeof(BITMAPFILEHEADER)+ // size of bitmap, involves some complicated calculations:
      sizeof(BITMAPINFOHEADER)+
      (1 == bypp ? 1024 : 0);

    // some data we need to write is defined here:
    BITMAPFILEHEADER bmp_file_header =
      {
	0x4d42, // file type; bmp's are file type 4d42h
	size_of_not_last_sections + width * height * bypp,
	0, // reserved; must be zero
	0, // also reserved, debe ser zero tambien
	size_of_not_last_sections
      };

    // now we must write the bitmap info structure to the file
    BITMAPINFOHEADER bmp_info_header =
      {
	sizeof(bmp_info_header), // size of this structure
	width,  // dimensions of bitmap
	height, //
	1, // number of planes, must be 1 i think
	(WORD)(bypp * 8), // bits per pixel
	BI_RGB, // "compression" type
	width * height * bypp, // size of image in bytes (technically can be omitted for our format, but just to be safe...)
	// stupid numbers no one uses:
	100,100,0,0
      };

    // setup the palette data we need to get in 8-bit color mode
    RGBQUAD palette_data[256];
    if(1 == bypp)
      {
	HPALETTE pal = CColor256::GetGDIPalette();
	if(NULL != pal)
	  {
	    // we can get the palette entries and put them in the file
	    GetPaletteEntries(pal,0,256,(PALETTEENTRY *)palette_data);
	  }
	for(int i = 0; i < 256; i++)
	  {
	    // make sure each reserved element is zero, or it may cause
	    //  problems when the bmp file is opened by Paint or whatever
	    palette_data[i].rgbReserved = 0;

	    // now we also have to reverse the red and blue order
	    //  because of how the palette data is stored in bitmap
	    //  format compared to stored by the GDI
	    BYTE *a = &palette_data[i].rgbRed, *b = &palette_data[i].rgbBlue, t;
	    t = *a;
	    *a = *b;
	    *b = t;
	  }
      }
				
    DWORD written;
    if
      (
       FALSE != WriteFile(file,(void *)&bmp_file_header,sizeof(bmp_file_header),&written,NULL) &&
       FALSE != WriteFile(file,(void *)&bmp_info_header,sizeof(bmp_info_header),&written,NULL) &&
       (1 != bypp || FALSE != WriteFile(file,(void *)palette_data,1024,&written,NULL))
       )
      {
	// now we can copy the surface data from "buffer16" to the handle "file"
	if(false == must_convert_555_to_565)
	  {
	    DWORD write_at_a_time = width * bypp;
	    for(int y = 0; y < height; y++,buffer16 = (WORD *)((BYTE *)buffer16 - pitch))
	      {
		if(FALSE == WriteFile(file,(void *)buffer16,write_at_a_time,&written,NULL))
		  {
		    return_value = GetLastError();
		  }
	      }
	  }
	else
	  {
	    for(int y = 0; y < height; y++,buffer16 -= pitch/2)
	      {
		for(int x = 0; x < width; x++)
		  {
		    // convert 555 buffer16[x] to 565 in file
		    int color_data; // results of conversion will be placed here

		    // transfer blue data:
		    color_data =  ((int)buffer16[x] & 0x001f)     ;
						
		    // transfer green data:
		    color_data |= ((int)buffer16[x] & 0x03e0) << 1;

		    // transfer red data:
		    color_data |= ((int)buffer16[x] & 0x7c00) << 1;

		    if(FALSE == WriteFile(file,(void *)&color_data,sizeof(WORD),&written,NULL))
		      {
			return_value = GetLastError();
		      } // end if failed write file
		  } // end for x
	      } // end for y
	  } // end if don't have to convert 555 to 565
      }
    else
      {
	return_value = GetLastError();
      }
		
    if(NULL != surf)
      {
	// unlock the offscreen surface we needed
	surf->Unlock(NULL);

	// release the offscreen surface, because we are done with it
	surf->Release();
      }
    else
      {
	delete buffer8;
      }

    // at this point, the file handle is still open;
    //  it should be closed by the caller process afterwards
    return return_value;
  }

  int CGraphics::GetTargetBufferIndex() const
  {
    //assert(Certified());
    if(this->current_buffer == this->buffers[0])
      {
	return 0;
      }
    else if(this->current_buffer == this->buffers[1])
      {
	return 1;
      }
    else
      {
	return 2;
      }
  }
} // end of namespace
