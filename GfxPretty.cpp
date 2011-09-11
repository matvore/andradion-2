#include "StdAfx.h"
#include "Gfx.h"
#include "GfxPretty.h"
#include "Logger.h"

using namespace std;

GfxPretty *GfxPretty::gfx_pretty = 0;

void GfxPretty::SurfacePretty::Draw(int x, int y, bool transparency) {
  RECT source;
  GfxPretty *owner = GetOwner<GfxPretty>();

  source.left = source.top = 0;
  source.right = width;
  source.bottom = height << 1;

  y <<= 1;

  if (owner->Clip(&x, &y, &source, &owner->physical_clip_rect)) {
    owner->back_buffer->BltFast
      (x, y, surface.Get(), &source,
       transparency
       ? DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT
       : DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);
  }
}

void GfxPretty::SurfacePretty::DrawScale
(const RECT *dest_rect, const RECT *source_rect, bool transparency) {
  GfxPretty *owner = GetOwner<GfxPretty>();
  RECT source;
  RECT source_scan;
  RECT dest;
  DWORD flags = owner->async_blit_flags | (transparency ? DDBLT_KEYSRC : 0);

  int source_height;
  int dest_height = dest_rect->bottom - dest_rect->top;
  int error = 0;
  bool scan = false;

  if (source_rect) {
    source.left = source_rect->left;
    source.top = source_rect->top << 1;
    source.right = source_rect->right;
    source.bottom = source.top + 1;

    source_height = source_rect->bottom - source_rect->top;
  } else {
    source.left = 0;
    source.right = width;
    source.top = 0;
    source.bottom = 1;

    source_height = height;
  }

  source_scan.left = source.left;
  source_scan.top = source.top + 1;
  source_scan.right = source.right;
  source_scan.bottom = source_scan.top + 1;

  dest.left = dest_rect->left;
  dest.top = dest_rect->top << 1;
  dest.right = dest_rect->right;
  dest.bottom = dest.top + 1;

  dest_height *= 2;
  for (int y = 0; y < dest_height; y++, scan = !scan) {
    error += source_height;

    if (error >= dest_height) {
      int amount = (error / dest_height) << 1;

      error %= dest_height;

      source.top+=amount;
      source.bottom+=amount;
      source_scan.top+=amount;
      source_scan.bottom+=amount;
    }

    owner->back_buffer->Blt(&dest, surface.Get(),
                            scan ? &source : &source_scan, flags, 0);

    dest.top++;
    dest.bottom++;
  }
}

bool GfxPretty::SurfacePretty::Refill() {
  BYTE *dim_palette = GetOwner<GfxPretty>()->dim_palette;
  
  if (Surface::Refill()) {
    SurfaceLock lock(surface.Get());
    int virtual_pitch = lock.GetPitch() << 1;
    BYTE *surf = lock.GetPtr(); 
    BYTE *surf_scan = lock.GetPtr() + lock.GetPitch();

    filler->Fill(lock.GetPtr(), virtual_pitch, this);

    // fill in every-other line with the dim scan lines
    for (int y = 0; y < GetHeight();
         y++, surf += virtual_pitch, surf_scan += virtual_pitch) {
      for (int x = 0; x < GetWidth(); x++) {
        surf_scan[x] = dim_palette[surf[x]];
      }
    }
  } else {
    return false;
  }
}

void GfxPretty::FontPretty::WriteChar(int x, int y, int ch, BYTE color) {
  GfxPretty *gfx = GetOwner<GfxPretty>();
  gfx->Lock();

  Font::WriteChar(x, y, ch, color);

  // draw the portion of the character that is within the scanlines
  gfx->lock_ptr += gfx->lock_pitch >> 1;
  Font::WriteChar(x, y, ch, gfx->dim_palette[color]);
  gfx->lock_ptr -= gfx->lock_pitch >> 1;

  gfx->Unlock();
}

GfxPretty::GfxPretty(HWND hWnd)
  : Gfx(hWnd, 640, 480, 0, false, 320, 200, 1),
    physical_clip_rect(Rect(0, 0, 320, 400)) {
  assert(!back_buffer);
  
  back_buffer = CreateOffscreenSurface(320, 400);

  clipper = CreateClipper(&physical_clip_rect);
}

auto_ptr<Gfx::Font> GfxPretty::LoadFont
(HGLOBAL resource_handle, int font_width, int font_height,
 int first_font_char, int last_font_char) {
  void *resource_data = LockResource(resource_handle);

  if (!resource_data) {
    throw Exception(__LINE__, string("Could not lock font resource"));
  }

  WriteLog("Returning a FontPretty object");

  return auto_ptr<Font>(new FontPretty
                        (this, resource_data, font_width, font_height,
                         first_font_char, last_font_char));
}

void GfxPretty::Flip() {
  if (InFocus()) {
    RECT dest;

    dest.left = 0;
    dest.top = 40;
    dest.bottom = 440;
    dest.right = 640;

    direct_draw->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);

    front_buffer->Blt(&dest, back_buffer.Get(), 0, async_blit_flags, 0);
  } else {
    Gfx::Flip();
  }

}

void GfxPretty::Lock() {
  Gfx::Lock();

  if (1 == lock_count) {
    lock_pitch <<= 1;
  }
}

void GfxPretty::Rectangle(const RECT *area, BYTE color, bool clip) {
  Lock();

  Gfx::Rectangle(area, color, clip);
  
  lock_ptr += lock_pitch >> 1;
  Gfx::Rectangle(area, dim_palette[color], clip);
  lock_ptr -= lock_pitch >> 1;

  Unlock();
}

void GfxPretty::SetPalette(PALETTEENTRY *pe, int entry_count,
                           bool visual_effect) {
  Gfx::SetPalette(pe, entry_count, visual_effect);

  if (!visual_effect) {
    pe += entry_count;
    
    while (entry_count > 0) {
      pe--;

      dim_palette[--entry_count]
        = MatchingColor(RGB(pe->peRed>>1, pe->peGreen>>1, pe->peBlue>>1));
    }
  }
}

auto_ptr<Gfx::Surface> GfxPretty::CreateSurface
(int w, int h, auto_ptr<Gfx::SurfaceFiller> filler) {
  auto_ptr<Surface> result(new SurfacePretty(this, w, h));

  result->ChangeFiller(filler);
  
  result->Refill();
  
  return result;
}

class DimmerLine : public Gfx::LineDrawer {
protected:
  LineDrawer *drawer;
  const BYTE *dim_palette;

public:
  DimmerLine(const BYTE *dim_palette, Gfx::LineDrawer *drawer)
    : drawer(drawer), dim_palette(dim_palette) {}

  virtual void BeginLine() {drawer->BeginLine();}
  virtual int GetNextPixel() {
    int color = drawer->GetNextPixel();

    return color >= 0 ? dim_palette[color] : -1;
  }
};

void GfxPretty::DrawLine(int x0, int y0, int x1, int y1,
                         Gfx::LineDrawer *drawer) {
  DimmerLine dimmer_line(dim_palette, drawer);

  Lock();

  Gfx::DrawLine(x0, y0, x1, y1, drawer);

  lock_ptr += lock_pitch >> 1;
  Gfx::DrawLine(x0, y0, x1, y1, &dimmer_line);
  lock_ptr -= lock_pitch >> 1;

  Unlock();
}

void GfxPretty::CopyFrontBufferToBackBuffer() {
  RECT source;

  source.left = 0;
  source.top = 40;
  source.right = 640;
  source.bottom = 440;

  back_buffer->Blt(0, front_buffer.Get(), &source, async_blit_flags, 0);
}

void GfxPretty::FrontBufferRectangle(RECT *area, BYTE color) {
  DDBLTFX fx;

  fx.dwSize = sizeof(fx);
  fx.dwFillColor = color;
  
  front_buffer->Blt(area, 0, 0, async_blit_flags | DDBLT_COLORFILL, &fx);
}

void GfxPretty::ClearBorderArea() {
  RECT dest;

  dest.left = dest.top = 0;
  dest.bottom = 40;
  dest.right = mode_width;

  FrontBufferRectangle(&dest, 0);

  dest.top = 440;
  dest.bottom = 480;

  FrontBufferRectangle(&dest, 0);
}

void GfxPretty::WriteToFrontBuffer(FontPretty *font, BYTE *surf, int pitch,
                                   int ch, BYTE color, BYTE back_color) {
  BYTE *doubled_line = surf + pitch;

  if (ch < font->first_font_char || ch > font->last_font_char) {
    int memset_width = font->GetCharWidth() * 2;

    for (int y = font->GetCharHeight() * 2; y > 0; y--, surf += pitch) {
      memset(surf, back_color, memset_width);
    }

    return;
  }
  
  pitch *= 2;

  ch -= font->first_font_char;

  for (FontPretty::Character::Iterator itr = font->font_data[ch].Begin();
       itr != font->font_data[ch].End();
       itr++, surf += pitch, doubled_line += pitch) {
    int row_data = *itr;
    int x_off = 0;

    for (int col = 0; col < font->GetCharWidth(); col++, row_data <<= 1) {
      BYTE c = (row_data & 0x80) ? color : back_color;

      surf[x_off] = c;
      doubled_line[x_off++] = c;
      surf[x_off] = c;
      doubled_line[x_off++] = c;
    }
  }
}

void GfxPretty::WriteToFrontBuffer(Font *font, int x, int y, const char *str,
                                   BYTE color, BYTE back_color) {
  Gfx::SurfaceLock f_lock(front_buffer.Get());
  FontPretty *pretty = dynamic_cast<FontPretty *>(font);
  BYTE *ptr = f_lock.GetPtr(x, y);

  while (*str) {
    WriteToFrontBuffer(pretty, ptr, f_lock.GetPitch(), *str++,
                       color, back_color);
    ptr += pretty->GetCharWidth() * 2;
  }
}

void GfxPretty::AttachScalingClipper() {
  if (!clip_count++) {
    back_buffer->SetClipper(clipper.Get());
  }
}
