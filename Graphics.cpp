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
#include "GfxLock.h"
#include "Graphics.h"
#include "logger.h"

using std::vector;
using std::min;
using std::max;
using std::endl;
using std::exit;
using std::auto_ptr;
using std::set;
using std::pair;

static IDirectDraw2 *dd = 0;
static IDirectDrawSurface *front_buffer = 0, *back_buffer = 0;
static IDirectDrawClipper *clipper = 0;
static RECT simple_clipper_rect;
static int mode_width, mode_height;
static DWORD async_blit_flags;
static bool flatscreen;
static HWND hWnd;

static HPALETTE gdi_palette;

static HRESULT CreateSurfaces();
static void DestroySurfaces();

template <class DDInt> inline void Release(DDInt **x) {
  if (*x) {
    (*x)->Release();
    *x = 0;
  }
}

struct surface_info {
private:
  IDirectDrawSurface *surface;
  SurfaceFiller *filler;
  WORD width, height;

public:  
  surface_info(int w, int h, auto_ptr<SurfaceFiller> fill)
    : surface(0), filler(0) {
    Use(w, h, fill);
  }

  bool CreateSurface() {
    assert(InUse());

    if (dd && !surface) {
      DDSURFACEDESC desc;
      memset(&desc, 0, sizeof(desc));

      desc.dwSize = sizeof(desc);
      desc.dwWidth = width;
      desc.dwHeight = height;
      desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
      desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;

      dd->CreateSurface(&desc, &surface, 0);

      if (surface) {
        DDCOLORKEY k = {0, 0};
        surface->SetColorKey(DDCKEY_SRCBLT, &k);
      }
    }

    return bool(surface);
  }

  void DestroySurface() {assert(InUse()); Release(&surface);}

  IDirectDrawSurface *Surf() {assert(InUse()); return surface;}
  IDirectDrawSurface *operator ->() {assert(surface); return surface;}

  bool InUse() {return bool(filler);}

  void RefillSurface() {
    assert(InUse());

    if (surface) {
      filler->FillSurface(surface);
    }
  }

  auto_ptr<SurfaceFiller> DoNotUse() {
    auto_ptr<SurfaceFiller> old_filler(filler);

    Release(&surface);
    filler = 0;

    return old_filler;
  }

  auto_ptr<SurfaceFiller>
  Use(int w, int h, auto_ptr<SurfaceFiller> fill) {
    auto_ptr<SurfaceFiller> old_filler(DoNotUse());

    assert(w > 0);
    assert(h > 0);

    Release(&surface);

    width = w;
    height = h;
    filler = fill.release();
    assert(filler);

    CreateSurface();
    RefillSurface();

    return old_filler;
  }

  auto_ptr<SurfaceFiller> ChangeFiller(auto_ptr<SurfaceFiller> fill) {
    assert(InUse());

    auto_ptr<SurfaceFiller> old_filler(filler);

    if (surface) {
      fill->FillSurface(surface);
    }

    filler = fill.release();

    return old_filler;
  }

  int Width() {assert(InUse()); return width;}
  int Height() {assert(InUse()); return height;}
};

typedef vector<surface_info> surface_db;
static surface_db refs;

static HRESULT WINAPI EnumSurfacesCallback(IDirectDrawSurface *lpDDSurface,
                                           DDSURFACEDESC *, void *bb) {
  IDirectDrawSurface **back_buffer = (IDirectDrawSurface **)bb;
  *back_buffer = lpDDSurface;

  return DDENUMRET_OK; // continue the enumeration
}

static IDirectDrawClipper *CreateClipper() {
  IDirectDrawClipper *c; 

  assert(dd);

  if (FAILED(dd->CreateClipper(0, &c, 0))) {
    return 0;
  }

  // set clip list
  Buffer region_buffer(sizeof(RGNDATAHEADER) + sizeof(RECT));

  // allocate memory for clip list
  LPRGNDATA cr = LPRGNDATA(region_buffer.Get());

  cr->rdh.dwSize = sizeof(RGNDATAHEADER);
  cr->rdh.iType = RDH_RECTANGLES;
  cr->rdh.nCount = 1;
  cr->rdh.nRgnSize = sizeof(RECT);

  cr->rdh.rcBound.left = 0;
  cr->rdh.rcBound.top = 0;
  cr->rdh.rcBound.bottom = mode_height;
  cr->rdh.rcBound.right = mode_width;

  *((RECT *)cr->Buffer) = cr->rdh.rcBound;
    
  // set clipper clip list
  if (FAILED(c->SetClipList(cr, 0))) {
    Release(&c);    
  }

  return c;
}

static HRESULT CreateSurfaces() {
  HRESULT hr;
  DDSURFACEDESC surf;

  DestroySurfaces();

  memset((void *)&surf,0,sizeof(surf));
  surf.dwSize = sizeof(surf);

  surf.ddsCaps.dwCaps |= DDSCAPS_PRIMARYSURFACE;
		
  surf.dwBackBufferCount = 1;
  surf.dwFlags |= DDSD_BACKBUFFERCOUNT;
  surf.ddsCaps.dwCaps |= DDSCAPS_COMPLEX;
  surf.ddsCaps.dwCaps |= DDSCAPS_FLIP;

  surf.dwFlags |= DDSD_CAPS;

  if (FAILED(hr = dd->CreateSurface(&surf, &front_buffer, 0))
      || FAILED(hr = front_buffer->EnumAttachedSurfaces
                (&back_buffer, EnumSurfacesCallback))) {
    Release(&front_buffer);
  } else {
    for (surface_db::iterator itr = refs.begin();
	 itr != refs.end(); itr++) {
      if (itr->InUse() && !itr->CreateSurface()) {
	Release(&back_buffer);
	Release(&front_buffer);
	return E_FAIL;
      }
    }
  }
  
  return hr;
}

static void DestroySurfaces() {
  for (surface_db::iterator itr = refs.begin();
       itr != refs.end(); itr++) {
    if (itr->InUse()) {
      itr->DestroySurface();
    }
  }

  Release(&back_buffer);
  Release(&front_buffer);
}

bool GfxInFocus(bool tryToRestart) {
  if (!front_buffer || FAILED(front_buffer->IsLost())) {
    if (tryToRestart) {
      CreateSurfaces();
    }
    
    return bool(front_buffer);
  } else {
    return true;
  }
}

void GfxInit(HWND hWnd_, int mode_width_, int mode_height_,
             bool flatscreen_) {
  GfxUninit();

  DWORD refresh = (320 == mode_width_ && flatscreen_) ? 43 : 0;
  DWORD flags = DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT
    | DDSCL_ALLOWMODEX;

  hWnd = hWnd_;
  mode_width = mode_width_;
  mode_height = mode_height_;
  flatscreen = flatscreen_;

  // create the directdraw instance
  if (FAILED(CoInitialize(0))) {
    return;
  }

  if (FAILED(CoCreateInstance(CLSID_DirectDraw, 0,
                              CLSCTX_ALL, IID_IDirectDraw2,
                              (void **)&dd))
      || FAILED(TryAndReport(dd->Initialize(0)))
      || ((FAILED(dd->SetCooperativeLevel(hWnd, flags))
           || FAILED(dd->SetDisplayMode(mode_width, mode_height,
                                        8, refresh, 0)))
          && (FAILED(dd->SetCooperativeLevel
                     (hWnd, flags & ~DDSCL_ALLOWMODEX))
              || FAILED(dd->SetDisplayMode(mode_width, mode_height,
                                           8, refresh, 0))))
      || FAILED(CreateSurfaces()) || !(clipper = CreateClipper())) {
    DestroySurfaces();
    Release(&dd);
    CoUninitialize();
    return;
  }

  // setup the simple clipper rect to the default
  simple_clipper_rect.left = 0;
  simple_clipper_rect.top = 0;
  simple_clipper_rect.right = mode_width;
  simple_clipper_rect.bottom = mode_height;

  async_blit_flags = 0;
  DDCAPS hw_caps, sw_caps;
  if (SUCCEEDED(dd->GetCaps(&hw_caps, &sw_caps))) {
    async_blit_flags = (hw_caps.dwCaps & DDCAPS_BLTQUEUE)
      ? DDBLT_ASYNC : 0;
  }

}

void GfxUninit() {
  Release(&clipper);
  DestroySurfaces();

  if (dd) {
    logger << "Closing gfx" << endl;

    dd->RestoreDisplayMode();
    dd->Release(), dd = 0;

    CoUninitialize();
  }

  if (gdi_palette) {
    DeleteObject(gdi_palette);
    gdi_palette = 0;
  }

  logger << "Gfx closed" << endl;
}

HRESULT GfxFlip() {return front_buffer->Flip(0, DDFLIP_WAIT);}

HRESULT GfxPutScale(surf_t surf, const RECT *target,
                    bool transparent, const RECT *src_rect)
{
  return back_buffer->Blt(const_cast<RECT *>(target),
                          refs[surf].Surf(), const_cast<RECT *>(src_rect),
                          DDBLT_WAIT | async_blit_flags
                          | (transparent ? DDBLT_KEYSRC : 0), 0);
}

void GfxPut(surf_t surf, int x, int y, bool use_transparency)
{
  DWORD trans_flag = use_transparency
    ? DDBLTFAST_SRCCOLORKEY : DDBLTFAST_NOCOLORKEY;

  int w = refs[surf].Width(), h = refs[surf].Height();

  // create abbreviated local variables
  int cl = simple_clipper_rect.left; // clipper left
  int cr = simple_clipper_rect.right; // clipper right
  int ct = simple_clipper_rect.top; // clipper top
  int cb = simple_clipper_rect.bottom; // clipper bottom

  RECT s;
  s.left = max(0, cl-x);
  s.top = max(0, ct-y);
  s.right = min(w, cr-x);
  s.bottom = min(h, cb-y);

  if(x < cl) {
    x = cl;
  }
  
  if(y < ct) {
    y = ct;
  }
		
  if(x + w > 0 && y + h > 0 && x < cr && y < cb) {
    back_buffer->BltFast(x, y, refs[surf].Surf(), &s,
                         trans_flag | DDBLTFAST_WAIT);
  }
}


static IDirectDrawPalette *
CreatePalette(PALETTEENTRY *init, int count) {
  PALETTEENTRY full_init[256];

  memcpy(full_init, init, count);

  for (; count < 256; count++) {
    full_init[count].peRed = full_init[count].peBlue = 0xff;
    full_init[count].peGreen = full_init[count].peFlags = 0x00;
  }

  IDirectDrawPalette *p;

  return SUCCEEDED(TryAndReport
                   (dd->CreatePalette(DDPCAPS_ALLOW256 | DDPCAPS_8BIT,
                                      full_init, &p, 0)))
    ? p : 0;
}

HRESULT GfxRectangle(BYTE c, const RECT *target) {
  DDBLTFX fx;

  fx.dwSize = sizeof(fx);
  fx.dwFillColor = c;

  return back_buffer->Blt(const_cast<RECT *>(target), 0, 0,
                          DDBLT_COLORFILL | DDBLT_WAIT
                          | async_blit_flags, &fx);
}

bool GfxClipLine(int& x0, int& y0, int& x1, int& y1) {
  assert(dd);
		
  const RECT *c = &simple_clipper_rect; // make a pointer shortcut so we are faster

  /// record whether or not each point is visible by testing them
  /// within the clipping region 
  bool visible0
    = x0 >= c->left && x0 < c->right && y0 >= c->top && y0 < c->bottom;
  bool visible1
    = x1 >= c->left && x1 < c->right && y1 >= c->top && y1 < c->bottom;
				
  // if both points are int the viewport, we have already finished
  if(visible0 && visible1)
    {
      return true;
    }

  // see if lines are completely invisible by running two tests:
  //  1 both points must be outside the clipping viewport
  //  2 both points must be on the same side of the clipping viewport

  if(visible0 == visible1) {
    // neither point is visible
    //  now do test 2.  If it doesn't pass test 2, then we continue normally
    if((x0 < c->left && x1 < c->left) ||
       (x0 >= c->right && x1 >= c->right) ||
       (y0 < c->top && y1 < c->top) ||
       (y0 >= c->bottom && y1 >= c->bottom)) {
      return false;
    }
    // and we're done for completely visible or completely invisible lines
  }

  bool right_edge = false;
  bool left_edge = false;
  bool top_edge = false;
  bool bottom_edge = false;

  int xi, yi; // points of intersection

  bool success;

  if(!visible1 || visible0)
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
	

  if(visible1 || !visible0)
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

int GfxModeWidth() {assert(dd); return mode_width;}
int GfxModeHeight() {assert(dd); return mode_height;}
IDirectDrawSurface *GfxBackBuffer() {return back_buffer;}
IDirectDrawSurface *GfxFrontBuffer() {return front_buffer;}
RECT& GfxSimpleClipperRect() {assert(dd); return simple_clipper_rect;}

void SurfaceFiller::FillSurface(IDirectDrawSurface *surf) throw() {
  DDSURFACEDESC desc;
  memset(&desc, 0, sizeof(desc));

  desc.dwSize = sizeof(desc);

  if (SUCCEEDED(surf->Lock(0, &desc, DDLOCK_WAIT | DDLOCK_WRITEONLY, 0))) {
    FillSurface((BYTE *)desc.lpSurface, desc.lPitch,
                desc.dwWidth, desc.dwHeight);
    surf->Unlock(desc.lpSurface);
  }
}

void SurfaceFiller::FillSurface(BYTE *desc, int pitch,
                                int width, int height) throw() {
  logger << "Call to unimplemented FillSurface()" << endl;
  exit(1);
}

void BitmapLoadingSurfaceFiller::FillSurface(IDirectDrawSurface *surf)
  throw() {
  DDSURFACEDESC desc;
  HDC surfDC;
  memset(&desc, 0, sizeof(desc));
  desc.dwSize = sizeof(desc);

  surf->GetSurfaceDesc(&desc);

  HBITMAP bmp = (HBITMAP)LoadImage(hInst, res_name, IMAGE_BITMAP,
                                   desc.dwWidth, desc.dwHeight,
                                   LR_CREATEDIBSECTION);

  if (!bmp || FAILED(surf->GetDC(&surfDC))) {
    DeleteObject(bmp);
    return;
  }

  HDC bit_dc = CreateCompatibleDC(surfDC);
  HGDIOBJ old_bmp = SelectObject(bit_dc, bmp);
  HPALETTE prev_pal = SelectPalette(surfDC, gdi_palette, FALSE);

  BitBlt(surfDC, 0, 0, desc.dwWidth, desc.dwHeight,
         bit_dc, 0, 0, SRCCOPY);

  SelectPalette(surfDC, prev_pal, FALSE);

  surf->ReleaseDC(surfDC);

  SelectObject(bit_dc, old_bmp);
  DeleteObject(bmp);
  DeleteDC(bit_dc);
}

surf_t GfxCreateSurface(int width, int height,
                        std::auto_ptr<SurfaceFiller> filler) {
  logger << "GfxCreateSurface of size " << width << "x" << height << endl;

  for (surface_db::iterator sitr = refs.begin();
       sitr != refs.end(); sitr++) {
    if (!sitr->InUse()) {
      sitr->Use(width, height, filler);
      logger << "Reusing old handle for surface" << endl;
      return sitr - refs.begin();
    }
  }

  refs.push_back(surface_info(width, height, filler));

  logger << "New handle for surface" << endl;

  return refs.size() - 1;
}

auto_ptr<SurfaceFiller> GfxDestroySurface(surf_t surf) {
  logger << "Destroy surf " << surf << endl;

  assert(refs[surf].InUse());

  auto_ptr<SurfaceFiller> old_filler(refs[surf].DoNotUse());

  while (!refs.empty() && !refs.back().InUse()) {
    refs.pop_back();
  }

  return old_filler;
}

std::auto_ptr<SurfaceFiller>
GfxChangeSurfaceFiller(surf_t surf,
                       std::auto_ptr<SurfaceFiller> filler) {
  return refs[surf].ChangeFiller(filler);
}

void GfxRefillSurfaces() {
  for (surface_db::iterator itr = refs.begin(); itr != refs.end(); itr++) {
    if (itr->InUse()) {
      itr->RefillSurface();
    } 
  }
}

surf_t BitmapLoadingSurfaceFiller
::CreateSurfaceFromBitmap(HINSTANCE hInst, const char *res_name) {
  HBITMAP bmp = (HBITMAP)LoadImage(hInst, res_name, IMAGE_BITMAP, 0, 0,
                                   LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
  BITMAP bmp_info;

  GetObject(bmp, sizeof(bmp_info), &bmp_info);
  DeleteObject(bmp);

  return GfxCreateSurface
    (bmp_info.bmWidth, bmp_info.bmHeight,
     auto_ptr<SurfaceFiller>(new BitmapLoadingSurfaceFiller(hInst,
                                                            res_name)));
}

BYTE GfxGetPaletteEntry(COLORREF clr) {
  return gdi_palette
    ? GetNearestPaletteIndex(gdi_palette, clr)
    : rand() & 0xff;
}

void GfxSetPalette(PALETTEENTRY *pe, int entry_count,
                   bool set_gdi_palette) {
  IDirectDrawPalette *pal = 0;

  assert(front_buffer);

  if (FAILED(front_buffer->GetPalette(&pal))) {
    pal = TryAndReport(CreatePalette(pe, entry_count));

    if (!pal) {
      return;
    }

    TryAndReport(front_buffer->SetPalette(pal));
  } 

  pal->SetEntries(0, 0, entry_count, pe);

  pal->Release();

  if (set_gdi_palette) {
    if (gdi_palette) {
      DeleteObject(gdi_palette);
    }

    Buffer log_palette_buffer(sizeof(PALETTEENTRY) * entry_count
                              + sizeof(LOGPALETTE));
    LOGPALETTE *log = (LOGPALETTE *)log_palette_buffer.Get();

    log->palVersion = 0x300;
    log->palNumEntries = entry_count;
    memcpy(log->palPalEntry, pe, sizeof(PALETTEENTRY) * entry_count);
    gdi_palette = CreatePalette(log);
  }
}

HPALETTE GfxGDIPalette() {return gdi_palette;}

void GfxFlipToGDISurface() {
  assert(dd);
  dd->FlipToGDISurface();
}

void GfxAttachScalingClipper() {
  assert(back_buffer);

  back_buffer->SetClipper(clipper);
}

void GfxDetachScalingClipper() {
  assert(back_buffer);

  back_buffer->SetClipper(0);
}

class GfxLockImpl : public GfxLock {
  IDirectDrawSurface *surface;

public:
  GfxLockImpl(BYTE *surf, int pitch, IDirectDrawSurface *surface)
      : GfxLock(surf, pitch), surface(surface) {
    // Do nothing.
  }

  virtual ~GfxLockImpl() {
    surface->Unlock(surf);
  }
};

class DummyGfxLock : public GfxLock {
public:
  DummyGfxLock(BYTE *surf, int pitch) : GfxLock(surf, pitch) {
    // Do nothing.
  }

  virtual ~DummyGfxLock() {
    delete [] surf;
  }
};

static auto_ptr<GfxLock> NewLock(IDirectDrawSurface *surface) {
  DDSURFACEDESC desc;
  memset(&desc, 0, sizeof(desc));
  desc.dwSize = sizeof(desc);
  if (SUCCEEDED(surface->Lock(0, &desc, DDLOCK_WAIT, 0))) {
    return auto_ptr<GfxLock>(new GfxLockImpl(
        (BYTE *)desc.lpSurface, desc.lPitch, surface));
  } else {
    logger << "failed to lock surface" << endl;
    return auto_ptr<GfxLock>(new DummyGfxLock(new BYTE[mode_width], 0));
  }
}

auto_ptr<GfxLock> GfxFrontBufferLock() {
  assert(front_buffer);
  return NewLock(front_buffer);
}

auto_ptr<GfxLock> GfxBackBufferLock() {
  assert(back_buffer);
  return NewLock(back_buffer);
}
