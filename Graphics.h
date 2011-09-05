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

typedef unsigned int surf_t;

class SurfaceFiller {
public:
  virtual ~SurfaceFiller() throw() {}

  virtual void FillSurface(IDirectDrawSurface *surf) throw();

  virtual void FillSurface(BYTE *dest, int pitch,
                           int width, int height) throw();
};

class BitmapLoadingSurfaceFiller : public SurfaceFiller {
  HINSTANCE hInst;
  const char *res_name;

public:
  static surf_t CreateSurfaceFromBitmap(HINSTANCE hInst,
                                        const char *res_name);

  BitmapLoadingSurfaceFiller(HINSTANCE hInst, const char *res_name)
    : hInst(hInst), res_name(res_name) {}

  virtual ~BitmapLoadingSurfaceFiller() throw() {}

  virtual void FillSurface(IDirectDrawSurface *surf) throw();
};

class NilSurfaceFiller : public SurfaceFiller {
public:
  virtual void FillSurface(IDirectDrawSurface *surf) throw() {}
};

void GfxInit(HWND hWnd_, int mode_width_,
             int mode_height_, bool flatscreen_);

bool GfxInFocus(bool tryToRestart);

void GfxUninit();

// fill an area of the current target buffer
HRESULT GfxRectangle(BYTE c, const RECT *target);

// put the bob using FastBlt (no DirectDrawClipper allowed on target
// buffer, no scaling) uses no clipping
void GfxPut(surf_t surf, int x, int y, bool use_transparency = true);

HRESULT GfxPutScale(surf_t surf, const RECT *target,
                    bool transparent = true, const RECT *src_rect = 0);

HRESULT GfxFlip();

IDirectDrawSurface *GfxBackBuffer(); 
IDirectDrawSurface *GfxFrontBuffer();

void GfxAttachScalingClipper();
void GfxDetachScalingClipper();

// change or read the rectangle where all
//  non-directdraw clipping happens.  This
//  can be faster; the CGraphics class will
//  do the clipping for you, which should
//  usually reduce some overhead of using
//  DirectDrawClipper.  If you want to use
//  more than one clipper rect, you'll have
//  to use a DirectDrawClipper 
RECT& GfxSimpleClipperRect();

// clips a set of line coordinates, and returns true if at least
// part of it is visible
// uses the simple clipping region
bool GfxClipLine(int& x0,int& y0,int& x1,int& y1);

int GfxModeWidth();
int GfxModeHeight();

surf_t GfxCreateSurface
(int width, int height,
 std::auto_ptr<SurfaceFiller> filler
 = std::auto_ptr<SurfaceFiller>(new NilSurfaceFiller()));

std::auto_ptr<SurfaceFiller> GfxDestroySurface(surf_t surf);

std::auto_ptr<SurfaceFiller>
GfxChangeSurfaceFiller(surf_t surf,
                       std::auto_ptr<SurfaceFiller> filler);

void GfxRefillSurfaces();

BYTE GfxGetPaletteEntry(COLORREF clr);
void GfxSetPalette(PALETTEENTRY *pal, int entry_count,
                   bool set_gdi_pal = true);
HPALETTE GfxGDIPalette();
void GfxFlipToGDISurface();

class GfxLock;
std::auto_ptr<GfxLock> GfxFrontBufferLock();
std::auto_ptr<GfxLock> GfxBackBufferLock();

template<class PI, class COLOR, class PATTERN>
void GfxLine(BYTE *surf, int pitch, int dx, int dy, 
             COLOR color, PATTERN pat) throw() {
  int xinc, error = 0;

  if (dx < 0) {
    dx = -dx;
    xinc = -1;
  } else {
    xinc = 1;
  }

  if (dy < 0) {
    dy = -dy;
    pitch = -pitch;
  }

  if (dx < dy) {
    // slope > 1
    std::swap(dx, dy);
    std::swap(xinc, pitch);
  }

  for (int i = 0; i <= dx; i++) {
    if (pat()) {
      *surf = color();
    }

    error += dy;

    if (error > dx) {
      error -= dx;
      surf += pitch;
    }

    surf += xinc;
  }
}
