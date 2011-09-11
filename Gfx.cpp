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
#include "Gfx.h"
#include "Buffer.h"
#include "Logger.h"

using namespace std;

auto_ptr<Gfx> Gfx::gfx;

static HRESULT WINAPI EnumSurfacesCallback(IDirectDrawSurface *lpDDSurface,
                                           DDSURFACEDESC *, void *bb);

Gfx::Exception::Exception(int line, HRESULT error_code) throw()
  : line(line), error_code(error_code) {}
    
Gfx::Exception::Exception(int line, const string& message) throw()
  : line(line), error_code(DDERR_GENERIC), message(message) {}

Gfx::SurfaceLock::SurfaceLock(IDirectDrawSurface *surface)
  : surface(surface) {
  DDSURFACEDESC desc;

  InitDXStruct(&desc);

  if (SUCCEEDED(surface->Lock(0, &desc, DDLOCK_WAIT, 0))) {
    pitch = desc.lPitch;
    surface_ptr = (BYTE *)desc.lpSurface;
  } else {
    pitch = 0;

    DoOrFail(__LINE__, surface->GetSurfaceDesc(&desc));

    surface_ptr = new BYTE[desc.dwWidth];
  }
}

Gfx::SurfaceLock::~SurfaceLock() {
  if (surface) {
    surface->Unlock((void *)surface_ptr);
  } else {
    logger << "Destroying fake surface pointer" << endl;
    delete [] surface_ptr;
  }
}

Gfx::Surface::~Surface() {
  logger << "(destroying surface) ";
  TryAndReport(this);

  if (owner) {
    owner->surfaces.erase(this);
  }
}

bool Gfx::Surface::Refill() {return SUCCEEDED(surface->Restore());}

Gfx::Font::Font(Gfx *owner, void *resource_data,
                int font_width, int font_height,
                int first_font_char, int last_font_char)
  : OwnedByGraphics(owner), font_width(font_width), font_height(font_height),
    first_font_char(first_font_char), last_font_char(last_font_char),
    font_data(last_font_char - first_font_char + 1) {
  BYTE *resource = (BYTE *)resource_data;
  
  for (CharacterSet::Iterator itr = font_data.Begin(); itr != font_data.End();
       itr++) {
    itr->Resize(font_height);

    for (int i = 0; i < font_height; i++) {
      (*itr)[i] = *resource++;
    }
  }
}

void Gfx::Font::WriteString(int x, int y, const char *str, BYTE color) {
  owner->Lock();

  while (*str) {
    WriteChar(x, y, *str++, color);

    x += font_width;
  }

  owner->Unlock();
}

void Gfx::Font::WriteChar(int x, int y, int ch, BYTE color) {
  const int cl = 0;
  const int cr = owner->virtual_buffer_width;
  const int ct = 0;
  const int cb = owner->virtual_buffer_height;

  BYTE *dest;

  if (ch < first_font_char || ch > last_font_char) {
    return;
  }
  ch -= first_font_char;

  owner->Lock();

  owner->lock_pitch;
  dest = owner->lock_ptr + owner->lock_pitch * y + x;

  for (Character::Iterator itr = font_data[ch].Begin();
       itr != font_data[ch].End() && y < cb;
       itr++, y++, dest += owner->lock_pitch) {
    int row_data = *itr;
    int x_off = x;
    BYTE *dest_off = dest;
    
    if (y < ct) {
      continue;
    }

    for (int col = 0; col < font_width && x_off < cr;
         col++, x_off++, dest_off++, row_data <<= 1) {
      if (x_off >= cl && (row_data & 0x80)) {
        *dest_off = color;
      }
    }
  }

  owner->Unlock();
}

Gfx::BitmapSurfaceFiller::BitmapSurfaceFiller
(HINSTANCE hInst, LPCTSTR res_name)
  : hInst(hInst), res_name(res_name), width(-1), height(-1) {
  logger << "(Created new BitmapSurfaceFiller) ";
  TryAndReport(this);
}


HBITMAP Gfx::BitmapSurfaceFiller::LoadImage(int wanted_width,
                                                 int wanted_height) {
  HBITMAP res = (HBITMAP)::LoadImage(hInst, res_name, IMAGE_BITMAP,
                                     wanted_width, wanted_height,
                                     LR_CREATEDIBSECTION);
  BITMAP bmp_info;

  logger << "BitmapSurfaceFiller::LoadImage called with arguments (";
  logger << wanted_width << ", " << wanted_height << ")" << endl;

  if (!TryAndReport(res)) {
    throw Exception(__LINE__, string("Could not load resource image"));
  }

  if (width < 0 && !wanted_width && !wanted_height) {
    if (!GetObject(res, sizeof(bmp_info), &bmp_info)) {
      logger << "Could not get width and height of bitmap" << endl;
      width = 1;
      height = 1;
    } else {
      width = bmp_info.bmWidth;
      height = bmp_info.bmHeight;
    }
  }

  logger << "Returning from BitmapSurfaceFiller::LoadImage()" << endl;

  return res;
}

void Gfx::BitmapSurfaceFiller::Fill(BYTE *surf_ptr, int surf_pitch,
                                         Surface *surface) {
  BITMAPINFOHEADER binfo;
  BYTE *row = new BYTE[surface->GetWidth() * 3];
  HBITMAP bmp = LoadImage(surface->GetWidth(), surface->GetHeight());
  HDC hdc = CreateCompatibleDC(0);
  HGDIOBJ old_bmp = SelectObject(hdc, (HGDIOBJ)bmp);

  logger << "Refilling surface " << hex << surface << dec <<
      " with bitmap" << endl;

  memset((void *)&binfo, 0, sizeof(binfo));

  binfo.biSize = sizeof(BITMAPINFOHEADER);
  binfo.biWidth = surface->GetWidth();
  binfo.biHeight = surface->GetHeight();
  binfo.biPlanes = 1;
  binfo.biBitCount = 24;
  binfo.biCompression = BI_RGB;

  for (int y = surface->GetHeight()-1; y >= 0; y--, surf_ptr+=surf_pitch) {
    BYTE *row_ptr = row;
    
    TryAndReport(GetDIBits(hdc, bmp, y, 1, (void *)row,
                           (BITMAPINFO *)&binfo, DIB_RGB_COLORS));

    for (int x = 0; x < surface->GetWidth(); x++, row_ptr += 3) {
      surf_ptr[x] = surface->GetOwner()
        ->MatchingColor(RGB(row_ptr[2], row_ptr[1], row_ptr[0]));
    }
  }

  Delete(&row);

  SelectObject(hdc, old_bmp);
  DeleteObject(bmp);
  DeleteDC(hdc);
}

AutoComPtr<IDirectDrawSurface> Gfx::CreateOffscreenSurface(int w, int h) {
  DDSURFACEDESC desc;
  AutoComPtr<IDirectDrawSurface> result;
  DDCOLORKEY color_key = {0, 0};

  InitDXStruct(&desc);

  desc.dwWidth = w;
  desc.dwHeight = h;
  desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
  desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;

  DoOrFail(__LINE__, direct_draw->CreateSurface
           (&desc, result.GetPtrToPtr(), 0));

  result->SetColorKey(DDCKEY_SRCBLT, &color_key);

  return result;
}

bool Gfx::Clip(int *dest_abs_x, int *dest_abs_y, RECT *source,
               const RECT *clip_area) {
  const int cl = clip_area->left;
  const int cr = clip_area->right;
  const int ct = clip_area->top;
  const int cb = clip_area->bottom;

  source->left = max(0, cl - *dest_abs_x);
  source->top = max(0, ct - *dest_abs_y);

  source->right = min(source->right, (long)(cr - *dest_abs_x));
  source->bottom = min(source->bottom, (long)(cb - *dest_abs_y));

  *dest_abs_x = max(*dest_abs_x, cl);
  *dest_abs_y = max(*dest_abs_y, ct);

  return source->left < source->right && source->top < source->bottom;
}

static HRESULT WINAPI EnumSurfacesCallback(IDirectDrawSurface *lpDDSurface,
                                           DDSURFACEDESC *, void *bb) {
  IDirectDrawSurface **back_buffer = (IDirectDrawSurface **)bb;
  *back_buffer = lpDDSurface;

  return DDENUMRET_OK; // continue the enumeration
}

Gfx::Gfx
(HWND hWnd, int mode_width, int mode_height, int refresh_rate,
 bool create_back_buffer, int virtual_buffer_width,
 int virtual_buffer_height, int virtual_buffer_count) 
  : lock_ptr(0), lock_pitch(0),
    lock_count(0), virtual_buffer_width(virtual_buffer_width),
    virtual_buffer_height(virtual_buffer_height),
    mode_width(mode_width), mode_height(mode_height),
    virtual_buffer_count(virtual_buffer_count), target_virtual_buffer(0),
    clip_count(0) {
  DWORD flags = DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE
    | DDSCL_ALLOWREBOOT | DDSCL_ALLOWMODEX;

  DDSURFACEDESC desc;
  DDCAPS hw_caps, sw_caps;

  direct_draw = CreateDirectDraw();

  if (!direct_draw) {
    throw Exception
      (__LINE__, string("Could not create DirectDraw interface"));
  }

  logger << "Setting cooperative level and display mode" << endl;
  if (FAILED(direct_draw->SetCooperativeLevel(hWnd, flags))
      || FAILED(direct_draw->SetDisplayMode(mode_width, mode_height,
                                            8, refresh_rate, 0))) {
    DoOrFail(__LINE__, direct_draw->SetCooperativeLevel
             (hWnd, flags & ~DDSCL_ALLOWMODEX));
    DoOrFail(__LINE__, direct_draw->SetDisplayMode
             (mode_width, mode_height, 8, refresh_rate, 0));
  }

  InitDXStruct(&desc);

  desc.ddsCaps.dwCaps |= DDSCAPS_PRIMARYSURFACE;

  if (create_back_buffer) {
    desc.dwBackBufferCount = 1;
    desc.dwFlags |= DDSD_BACKBUFFERCOUNT;
    desc.ddsCaps.dwCaps |= DDSCAPS_COMPLEX;
    desc.ddsCaps.dwCaps |= DDSCAPS_FLIP;
  } 

  desc.dwFlags |= DDSD_CAPS;

  logger << "Creating primary surface(s)" << endl;
  DoOrFail(__LINE__, direct_draw->CreateSurface
           (&desc, front_buffer.GetPtrToPtr(), 0));

  if (create_back_buffer) {
    DoOrFail(__LINE__, front_buffer->EnumAttachedSurfaces
             (back_buffer.GetPtrToPtr(), EnumSurfacesCallback));
  }

  InitDXStruct(&hw_caps);
  InitDXStruct(&sw_caps);

  async_blit_flags = SUCCEEDED(direct_draw->GetCaps(&hw_caps, &sw_caps))
    && (hw_caps.dwCaps & DDCAPS_BLTQUEUE) ? DDBLT_ASYNC : DDBLT_WAIT;

  logger << "Gfx constructor finishing" << endl;
}

Gfx::~Gfx() {
  for (set<Surface *>::iterator itr = surfaces.begin();
       itr != surfaces.end(); itr++) {
    (**itr).owner = 0;
  }

  lock.release();
}

void Gfx::Lock() {
  if (!lock_count++) {
    lock.reset(new SurfaceLock(back_buffer.Get()));

    lock_pitch = lock->GetPitch();
    lock_ptr = lock->GetPtr();
  }
}

void Gfx::Unlock() {
  if (!--lock_count) {
    lock.reset();

    lock_pitch = 0;
    lock_ptr = 0;
  }
}

void Gfx::DetachScalingClipper() {
  if (!--clip_count) {
    back_buffer->SetClipper(0);
  }
}

void Gfx::Rectangle(const RECT *area, BYTE color, bool clip) {
  RECT target;
  BYTE *dest;
  int width;

  target = *area;

  if (clip) {
    const int cl = 0;
    const int ct = 0;
    const int cr = virtual_buffer_width;
    const int cb = virtual_buffer_height;
  
    if (target.left < cl) {
      target.left = cl;
    }
  
    if (target.right > cr) {
      target.right = cr;
    }

    if (target.bottom > cb) {
      target.bottom = cb;
    }

    if (target.top < ct) {
      target.top = ct;
    }
  } 

  dest = lock_ptr + target.left + target.top * lock_pitch;

  width = target.right - target.left;

  if (width > 0) {
    Lock();
    Rectangle(lock_ptr, lock_pitch, target.left, target.top,
              width, target.bottom - target.top, color);
    Unlock();
  }
}

void Gfx::SetPalette(PALETTEENTRY *pe, int entry_count,
                          bool visual_effect) {
  AutoComPtr<IDirectDrawPalette> pal;

  assert(entry_count >= 0 && entry_count <= 256);

  if (TryAndReport(pe != palette && !visual_effect)) {
    memcpy((void *)palette, (void *)pe, entry_count * sizeof(PALETTEENTRY));
  }

  logger << "Getting/Creating DirectDraw palette" << endl;

  if (FAILED(TryAndReport(front_buffer->GetPalette(pal.GetPtrToPtr())))) {
    if (FAILED(TryAndReport(direct_draw->CreatePalette
                            (DDPCAPS_8BIT | DDPCAPS_ALLOW256,
                             palette, pal.GetPtrToPtr(), 0)))
        || FAILED(TryAndReport(front_buffer->SetPalette(pal.Get())))) {
      return;
    }
  }

  TryAndReport(pal->SetEntries(0, 0, visual_effect ? entry_count : 256, pe));

  if (!visual_effect) {
    logger << "Refilling surfaces" << endl;
    RefillSurfaces();
  }

  logger << "SetPalette method returning" << endl;
}

BYTE Gfx::MatchingColor(COLORREF rgb) {
  int best_match_entry = 0;
  int best_match_score = 3 * 256 * 256 + 1;

  for (int i = 0; i < 256; i++) {
    int red_diff = GetRValue(rgb) - (int)palette[i].peRed;
    int green_diff = GetGValue(rgb) - (int)palette[i].peGreen;
    int blue_diff = GetBValue(rgb) - (int)palette[i].peBlue;
    int this_score = red_diff * red_diff + green_diff * green_diff
      + blue_diff * blue_diff;

    if (this_score < best_match_score) {
      best_match_score = this_score;
      best_match_entry = i;
    }
  }

  return (BYTE)best_match_entry;
}

bool Gfx::InFocus() {return SUCCEEDED(front_buffer->IsLost());}

bool Gfx::Screenshot(const char *filename) {
  HANDLE bmp_file = CreateFile(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL, 0);
  SurfaceLock front_lock(front_buffer.Get());
  BITMAPFILEHEADER header;
  BITMAPINFOHEADER info;
  DWORD written;
  PALETTEENTRY system_palette[256];
  PALETTEENTRY *to_use;
  AutoComPtr<IDirectDrawPalette> pal;

  if (!bmp_file) {
    return false;
  }

  header.bfType = 19778;
  header.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
    + sizeof(RGBQUAD) * 256 + mode_width * mode_height;
  header.bfReserved1 = 0;
  header.bfReserved2 = 0;
  header.bfOffBits = sizeof(BITMAPFILEHEADER)
    + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256;

  memset((void *)&info, 0, sizeof(info));

  info.biSize = sizeof(info);
  info.biWidth = mode_width;
  info.biHeight = mode_height;
  info.biPlanes = 1;
  info.biBitCount = 8;

  if (!WriteFile(bmp_file, (void *)&header, sizeof(header), &written, 0)
      || !WriteFile(bmp_file, (void *)&info, sizeof(info), &written, 0)) {
    CloseHandle(bmp_file);
    return false;
  }

  to_use = SUCCEEDED(front_buffer->GetPalette(pal.GetPtrToPtr()))
    && SUCCEEDED(pal->GetEntries(0, 0, 256, system_palette))
    ? system_palette : palette;

  pal.Reset();

  for (int i = 0; i < 256; i++) {
    RGBQUAD rgb;

    rgb.rgbRed = to_use[i].peRed;
    rgb.rgbGreen = to_use[i].peGreen;
    rgb.rgbBlue = to_use[i].peBlue;
    rgb.rgbReserved = 0;

    if (!WriteFile(bmp_file, (void *)&rgb, sizeof(RGBQUAD), &written, 0)) {
      CloseHandle(bmp_file);
      return false;
    }
  }

  for (int y = mode_height - 1; y >= 0; y--) {
    if (!WriteFile(bmp_file, (void *)front_lock.GetPtr(0, y), mode_width,
                   &written, 0)) {
      CloseHandle(bmp_file);
      return false;
    }
  }

  CloseHandle(bmp_file);

  return true;
}


bool Gfx::ClipLine(int *x0, int *y0, int *x1, int *y1) {
  const int cl = 0;
  const int ct = 0;
  const int cr = virtual_buffer_width;
  const int cb = virtual_buffer_height;

  // record whether or not each point is visible by testing them
  // within the clipping region 
  bool visible0
    = *x0 >= cl && *x0 < cr && *y0 >= ct && *y0 < cb;
  bool visible1
    = *x1 >= cl && *x1 < cr && *y1 >= ct && *y1 < cb;
				
  // if both points are int the viewport, we have already finished
  if(visible0 && visible1) {
    return true;
  }

  // see if lines are completely invisible by running two tests:
  //  1 both points must be outside the clipping viewport
  //  2 both points must be on the same side of the clipping viewport

  if(visible0 == visible1) {
    // neither point is visible
    //  now do test 2.  If it doesn't pass test 2, then we continue normally
    if((*x0 < cl && *x1 < cl) ||
       (*x0 >= cr && *x1 >= cr) ||
       (*y0 < ct && *y1 < ct) ||
       (*y0 >= cb && *y1 >= cb)) {
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

  if(!visible1 || visible0) {
    // compute deltas
    int dx = *x1 - *x0;
    int dy = *y1 - *y0;

    // compute which boundary lines need to be clipped against
    if(*x1 >= cr) {
      // flag right edge
      right_edge = true;

      // compute intersection with right edge
      assert(0 != dx);
      yi = (int)(0.5f + ((float)dy/(float)dx) * float(cr-1 - *x0) + (float)*y0);
    } else if(*x1 < cl) {
      // flags left edge
      left_edge = true;

      // compute intersection with left edge
      assert(0 != dx);
      yi = (int)(0.5f + ((float)dy/(float)dx) * float(cl - *x0) + (float)*y0);
    }
		
    // that's it for left-right side intersections.  now for top-bottom intersections
    if(*y1 >= cb) {
      // flag edge
      bottom_edge = true;

      // compute intersection with bottom edge
      assert(0 != dy);
      xi = (int)(0.5f + ((float)dx/(float)dy) * float(cb-1 - *y0) + (float)*x0);
    } else if(*y1 < ct) {
      top_edge =true;
      
      assert(0 != dy);
      xi = (int)(0.5f + ((float)dx/(float)dy) * float(ct - *y0) + (float)*x0);
    }

    // now we know which line we passed through
    //  figure which edge is the right intersection
    if(yi >= ct && yi < cb) {
      if(true == right_edge) {
        *x1 = cr - 1;
        *y1 = yi;

        success = true;
      } else if(true == left_edge) {
        *x1 = cl;
        *y1 = yi;

        success = true;
      }
    }

    if(xi >= cl && xi < cr) {
      if(true == bottom_edge) {
        *x1 = xi;
        *y1 = cb-1;
	
        success = true;
      } else if(true == top_edge) {
        *x1 = xi;
        *y1 = ct;

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
	

  if(visible1 || !visible0) {
    // compute deltas
    int dx = *x0 - *x1;
    int dy = *y0 - *y1;

    // compute what boundary line needs to be clipped against
    if(*x0 >= cr) {
      // flag right edge
      right_edge = true;

      // computer intersection with right edge
      assert(0 != dx);
      yi = (int)(0.5f + ((float)dy/(float)dx) * float(cr - 1 - *x1) + (float)*y1);
    } else if(*x0 < cl) {
      // flag left edge
      left_edge = true;

      // compute intersection with left edge
      assert(0 != dx);
      yi = (int)(0.5f + ((float)dy/(float)dx) * float(cl - *x1) +
                 (float)*y1);
    }

    // bottom-top edge intersections
    if(*y0 >= cb) {
      bottom_edge = true;

      // compute intersection with bottom edge
      assert(0 != dy);
      xi = (int)(0.5f + ((float)dx/(float)dy) * float(cb - 1 - *y1) + (float)*x1);
    } else if(*y0 < ct) {
      top_edge = true;

      // computer intersection with bottom edge
      assert(0 != dy);
      xi = (int)(0.5f + ((float)dx/(float)dy) * float(ct - *y1) + (float)*x1);
    }

    // compute which edge is the proper intersection
    if(yi >= ct && yi < cb) {
      if(true == right_edge) {
        *x0 = cr - 1;
        *y0 = yi;
        success= true;
      } else if(true == left_edge) {
        *x0 = cl;
        *y0 = yi;
        success= true;
      }
    }	

    if(xi >= cl && xi < cr) {
      if(true == bottom_edge) {
        *x0 = xi;
        *y0 = cb -1;
        success = true;
      } else if(true == top_edge) {	
        *x0 = xi;
        *y0 = ct;
        success = true;
      }
    }
  }
  return success;
}

void Gfx::DrawLine
(int x0, int y0, int x1, int y1, Gfx::LineDrawer *drawer) {
  int xinc, error = 0;
  BYTE *surf = lock_ptr + x0 + y0 * lock_pitch;
  int pitch = lock_pitch;
  int dx = x1 - x0;
  int dy = y1 - y0;

  assert(lock_ptr);

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
    swap(dx, dy);
    swap(xinc, pitch);
  }

  drawer->BeginLine();

  for (int i = 0; i <= dx; i++) {
    int color = drawer->GetNextPixel();

    if (color >= 0) {
      *surf = (BYTE)color;
    }

    error += dy;

    if (error > dx) {
      error -= dx;
      surf += pitch;
    }

    surf += xinc;
  }
}

void Gfx::Flip() {
  if (!InFocus() && SUCCEEDED(front_buffer->Restore())) {
    back_buffer->Restore();

    SetPalette(palette, 256, false);

    RefillSurfaces();
  }
}

auto_ptr<Gfx::Surface>
Gfx::BitmapSurfaceFiller::CreateSurfaceFromBitmap
(Gfx *owner, HINSTANCE hInstance, LPCTSTR resource_name) {
  BitmapSurfaceFiller *filler
    = new BitmapSurfaceFiller(hInstance, resource_name);

  logger << "Creating new surface for bitmap filler" << endl;
  
  auto_ptr<Surface> new_surface(owner->CreateSurface
                                (filler->GetWidth(), filler->GetHeight(),
                                 auto_ptr<SurfaceFiller>(filler)));

  logger << "Returning from CreateSurfaceFromBitmap() method" << endl;
  return new_surface;
}

void Gfx::RefillSurfaces() {
  for (set<Surface *>::iterator itr = surfaces.begin();
       itr != surfaces.end(); itr++) {
    (**itr).Refill();
  }
}

void Gfx::FlipToGDISurface() {direct_draw->FlipToGDISurface();}

void Gfx::DoOrFail(int line, HRESULT hr) throw(Exception) {
  if (FAILED(hr)) {
    logger << "Operation on line " << line << " failed with code " << hex <<
        hr << dec << endl;
    throw Exception(line, hr);
  }
}

AutoComPtr<IDirectDrawClipper> Gfx::CreateClipper(const RECT *area) {
  AutoComPtr<IDirectDrawClipper> clipper;

  if (SUCCEEDED(direct_draw->CreateClipper(0, clipper.GetPtrToPtr(), 0))) {
    Buffer region_buffer(sizeof(RGNDATAHEADER) + sizeof(RECT));
    LPRGNDATA cr = (LPRGNDATA)region_buffer.Get();

    cr->rdh.dwSize = sizeof(RGNDATAHEADER);
    cr->rdh.iType = RDH_RECTANGLES;
    cr->rdh.nCount = 1;
    cr->rdh.nRgnSize = sizeof(RECT);

    cr->rdh.rcBound = *area;

    *((RECT *)cr->Buffer) = cr->rdh.rcBound;

    clipper->SetClipList(cr, 0);
  }

  return clipper;
}

AutoComPtr<IDirectDraw2> Gfx::CreateDirectDraw() {
  AutoComPtr<IDirectDraw2> direct_draw(CLSID_DirectDraw, IID_IDirectDraw2);

  logger << "Creating DirectDraw interface" << endl;

  if (FAILED(direct_draw->Initialize(0))) {
    direct_draw.Reset();
  } 
    
  return direct_draw;
}

HRESULT WINAPI EnumModesCallback(LPDDSURFACEDESC lpDDSurfaceDesc,
                                 LPVOID lpContext) {
  bool *found = (bool *)lpContext;

  *found = true;

  return DDENUMRET_CANCEL;
}

bool Gfx::VideoModeAvailable(int mode_width, int mode_height) {
  DDSURFACEDESC desc;
  bool result = false;
  Com com;
  AutoComPtr<IDirectDraw2> direct_draw = CreateDirectDraw();

  if (!direct_draw) {
    return false;
  }

  InitDXStruct(&desc);

  desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;

  desc.dwWidth = mode_width;
  desc.dwHeight = mode_height;
  
  desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);
  desc.ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED8;

  direct_draw->EnumDisplayModes(0, &desc, (void *)&result, &EnumModesCallback);

  return result;
}
