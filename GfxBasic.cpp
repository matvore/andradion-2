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
#include "GfxBasic.h"
#include "Logger.h"

using namespace std;

GfxBasic *GfxBasic::gfx_basic = 0;

GfxBasic::GfxBasic(HWND hWnd, int mode_width, int mode_height, int refresh_rate,
                   int virtual_buffer_width, int virtual_buffer_height)
  : Gfx(hWnd, mode_width, mode_height, refresh_rate, true,
        virtual_buffer_width, virtual_buffer_height,
        (mode_width / virtual_buffer_width)
        * (mode_height / virtual_buffer_height)),
    single_buffer_standard(mode_width == virtual_buffer_width
                           && mode_height == virtual_buffer_height) {
  int excess_width = mode_width % virtual_buffer_width;
  int excess_height = mode_height % virtual_buffer_height;
  int grid_width = mode_width / virtual_buffer_width;
  int grid_height = mode_height / virtual_buffer_height;

  int border_width = grid_width*2;
  int border_height = grid_height*2;

  bool border_padding_x = border_width <= excess_width;
  bool border_padding_y = border_height <= excess_height;

  int grid_interval_x = virtual_buffer_width;
  int grid_interval_y = virtual_buffer_height;

  int val;

  RECT non_blank;
  RECT area;

  if (border_padding_x) {
    excess_width -= border_width;
    grid_interval_x += 2;
  }

  if (border_padding_y) {
    excess_height -= border_height;
    grid_interval_y += 2;
  }

  non_blank = Rect
    (excess_width/2, excess_height/2,
     mode_width - excess_width/2 - (excess_width&1),
     mode_height - excess_height/2 - (excess_height&1));

  blank_areas.Resize(!!non_blank.left + (non_blank.right != mode_width)
                     + !!non_blank.top + (non_blank.bottom != mode_height));
  val = 0;

  if (non_blank.left) {
    // left blank area
    blank_areas[val++]
      = Rect(0, non_blank.top, non_blank.left, non_blank.bottom);
  }

  if (non_blank.right != mode_width) {
    // right blank area
    blank_areas[val++]
      = Rect(non_blank.right, non_blank.top, mode_width, non_blank.bottom);
  }

  if (non_blank.top) {
    // top blank area
    blank_areas[val++] = Rect(0, 0, mode_width, non_blank.top);
  }

  if (non_blank.bottom != mode_height) {
    // bottom blank area
    blank_areas[val++] = Rect(0, non_blank.bottom, mode_width, mode_height);
  }

  border_areas.Resize(grid_width + grid_height - 2);

  val = non_blank.left + grid_interval_x - (border_padding_x ? 2 : 1);
  area = Rect(val, non_blank.top, val + 2, non_blank.bottom);

  for (int i = 1; i < grid_width;
       i++, area.left+=grid_interval_x, area.right+=grid_interval_x) {
    border_areas[i - 1] = area;
  }

  val = non_blank.top + grid_interval_y - (border_padding_y ? 2 : 1);
  area = Rect(non_blank.left, val, non_blank.right, val + 2);

  for (int i = 1; i < grid_height;
       i++, area.top+=grid_interval_y, area.bottom+=grid_interval_y) {
    border_areas[i - 1 + grid_width] = area;
  }

  clip_areas.Resize(grid_width * grid_height);

  for (int y = 0, i = 0; y < grid_height; y++) {
    for (int x = 0; x < grid_width; x++, i++) {
      int left = grid_interval_x * x + non_blank.left;
      int top = grid_interval_y * y + non_blank.top;

      clip_areas[i] = Rect(left, top, left + virtual_buffer_width,
                           top + virtual_buffer_height);

      logger << "Clip area for virtual buffer " << x << "x" << y;
      logger << " created: [" << left << ", " << top << ", ";
      logger << (left + virtual_buffer_width) << ", ";
      logger << (top + virtual_buffer_height) << "]" << endl;
    }
  }

  clippers.Resize(clip_areas.Size());

  for (int i = 0; i < clip_areas.Size(); i++) {
    clippers[i] = CreateClipper(&clip_areas[i]);
  }
}

void GfxBasic::SurfaceBasic::Draw(int x, int y, bool transparency) {
  GfxBasic *owner = GetOwner<GfxBasic>();
  RECT source = Rect(0, 0, width, height);
  RECT *clip_area = &owner->clip_areas[owner->target_virtual_buffer];

  x += clip_area->left;
  y += clip_area->top;

  if (Clip(&x, &y, &source, clip_area)) {
    owner->back_buffer->BltFast
      (x, y, surface.Get(), &source,
       transparency
       ? DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT
       : DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);
  }
}

void GfxBasic::SurfaceBasic::DrawScale(const RECT *dest_rect, const RECT
                                       *source_rect, bool transparency) {
  GfxBasic *owner = GetOwner<GfxBasic>();
  RECT dest;

  if (!owner->single_buffer_standard) {
    RECT& clip(owner->clip_areas[owner->target_virtual_buffer]);

    dest.left = clip.left + dest_rect->left;
    dest.top = clip.top + dest_rect->top;
    dest.right = clip.left + dest_rect->right;
    dest.bottom = clip.top + dest_rect->bottom;

    dest_rect = &dest;
  }

  owner->back_buffer->Blt
    (const_cast<RECT *>(dest_rect),
     surface.Get(), const_cast<RECT *>(source_rect),
     owner->async_blit_flags
     | (transparency ? DDBLT_KEYSRC : 0), 0);
}

bool GfxBasic::SurfaceBasic::Refill() {
  if (Surface::Refill()) {
    SurfaceLock lock(surface.Get());

    filler->Fill(lock.GetPtr(), lock.GetPitch(), this);

    return true;
  } else {
    return false;
  }
}

auto_ptr<Gfx::Font>
GfxBasic::LoadFont(HGLOBAL resource_handle, int font_width, int font_height,
                   int first_font_char, int last_font_char) {
  void *resource_data = LockResource(resource_handle);

  if (!resource_data) {
    throw Exception(__LINE__, string("Could not lock font resource"));
  }

  return auto_ptr<Font>(new Font(this, resource_data, font_width,
                                 font_height, first_font_char,
                                 last_font_char));
}

void GfxBasic::Flip() {
  Gfx::Flip();

  if (!InFocus()) {
    return;
  }

  if (!single_buffer_standard) {
    DDBLTFX fx;
    fx.dwSize = sizeof(fx);
    fx.dwFillColor = blank_color;

    for (int i = 0; i < blank_areas.Size(); i++) {
      back_buffer->Blt(blank_areas + i, 0, 0,
                       DDBLT_COLORFILL | async_blit_flags, &fx);
    }

    fx.dwFillColor = border_color;

    for (int i = 0; i < border_areas.Size(); i++) {
      back_buffer->Blt(border_areas + i, 0, 0,
                       DDBLT_COLORFILL | async_blit_flags, &fx);
    }
  }

  front_buffer->Flip(0, DDFLIP_WAIT);
}

void GfxBasic::Lock() {
  Gfx::Lock();

  if (1 == lock_count) {
    RECT& clip_area(clip_areas[target_virtual_buffer]);
    lock_ptr += clip_area.left + clip_area.top * lock_pitch;
  }
}

void GfxBasic::SetPalette(PALETTEENTRY *pe, int entry_count,
                          bool visual_effect) {
  Gfx::SetPalette(pe, entry_count, visual_effect);

  if (!visual_effect) {
    border_color = MatchingColor(RGB(0xff, 0xff, 0xff));
    blank_color = MatchingColor(RGB(0x00, 0x00, 0x00));
  }
}

void GfxBasic::CopyFrontBufferToBackBuffer() {
  back_buffer->BltFast(0, 0, front_buffer.Get(), 0, DDBLTFAST_WAIT);
}

auto_ptr<Gfx::Surface> GfxBasic::CreateSurface
(int w, int h, auto_ptr<Gfx::SurfaceFiller> filler) {
  auto_ptr<Surface> result(new SurfaceBasic(this, w, h));

  logger << "Changing filler of new surface" << endl;
  result->ChangeFiller(filler);

  logger << "Filling new surface for first time" << endl;
  result->Refill();

  logger << "GfxBasic::CreateSurface() returning" << endl;
  return result;
}

void GfxBasic::AttachScalingClipper() {
  if (!clip_count++) {
    back_buffer->SetClipper(clippers[target_virtual_buffer].Get());
  }
}

void GfxBasic::SetTargetVirtualBuffer(int index) {
  Gfx::SetTargetVirtualBuffer(index);

  if (clip_count > 0) {
    back_buffer->SetClipper(clippers[target_virtual_buffer].Get());
  }

  if (lock_count > 0) {
    lock_ptr = lock->GetPtr(clip_areas[target_virtual_buffer].left,
                            clip_areas[target_virtual_buffer].top);
  }
}

HDC GfxBasic::GetDC() {HDC dc = 0; back_buffer->GetDC(&dc); return dc;}

void GfxBasic::ReleaseDC(HDC dc) {back_buffer->ReleaseDC(dc);}

