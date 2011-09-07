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
#include "CompactMap.h"
#include "Graphics.h"
#include "Logger.h"

using std::pair;
using std::min;
using std::max;
using std::string;
using std::bad_alloc;
using std::vector;
using std::auto_ptr;
using std::endl;

class CmpFiller : public SurfaceFiller {
  CCompactMap *parent;

public:
  CmpFiller(CCompactMap *parent) : parent(parent) {}

  virtual void FillSurface(IDirectDrawSurface *surf) throw() {
    DDSURFACEDESC desc;

    SurfaceFiller::FillSurface(surf);

    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);

    surf->GetSurfaceDesc(&desc);
    RECT clipper;
    clipper.left = 0;
    clipper.top = 0;
    clipper.right = desc.dwWidth;
    clipper.bottom = desc.dwHeight;

    parent->RenderStep1(surf, 0, 0, clipper);
  }

  virtual void FillSurface(BYTE *dest, int pitch,
                           int width, int height) throw() {
    BYTE *clear_point = dest;

    for (int y = 0; y < height; y++) {
      memset(clear_point, 0, width);
      clear_point += pitch;
    }
    
    parent->RenderStep2(dest, pitch);
  }
};

void CCompactMap::RenderStep1(IDirectDrawSurface *target_buffer,
                              int current_area_left,
                              int current_area_top,
                              const RECT& simple_clipper_rect) {
  DDBLTFX fx;
  fx.dwSize = sizeof(fx);
		
  // abbreviated variables
  const long& cl = simple_clipper_rect.left;
  const long& cr = simple_clipper_rect.right;
  const long& ct = simple_clipper_rect.top;
  const long& cb = simple_clipper_rect.bottom;

  // blit blocks
  vector<BYTE>::iterator block_i;
  vector<vector<RCT> >::iterator area_i;
  for(block_i = blocks.begin(), area_i = block_areas.begin();
      block_i != blocks.end(); block_i++, area_i++) {
    fx.dwFillColor = *block_i;

    vector<RCT>::iterator rect_i;
    for(rect_i = area_i->begin(); rect_i != area_i->end(); rect_i++) {
      RECT t;
      t.left = (int)rect_i->x + current_area_left;
      t.top = (int)rect_i->y + current_area_top;
      t.right = t.left + (int)rect_i->w + 1;
      t.bottom = t.top + (int)rect_i->h + 1;

      if(t.left < cl) {
        t.left = cl;
      }

      if(t.right > cr) {
        t.right = cr;
      }

      if(t.top < ct) {
        t.top = ct;
      }

      if(t.bottom > cb) {
        t.bottom = cb;
      }

      if(t.bottom > t.top && t.right > t.left 
         && FAILED(target_buffer->Blt(&t, 0, 0, DDBLT_COLORFILL
                                      | DDBLT_ASYNC, &fx))) {
        while(DDERR_WASSTILLDRAWING ==
              target_buffer->Blt(&t, 0, 0, DDBLT_COLORFILL, &fx))
          ;
      }
    }
  }
}

void CCompactMap::RenderStep2(void *surface, int pitch) {
  // figure first and last row to blit in leftover
  // get a pointer into the surface, too
  BYTE *surf_8bit = (BYTE *)surface;
  BYTE *source = (BYTE *)left_over_pixels.Get();
  
  for(VCTR_ROW::iterator row = left_over.begin();
      row != left_over.end(); row++) {
    for(ROW::iterator run_i = row->begin(); run_i != row->end(); run_i++) {
      const int amount = run_i->opaque_count + 1;
      
      memcpy(surf_8bit + run_i->x, source, amount);
      source += amount;
    }

    surf_8bit += pitch;
  }

  source = (BYTE *)pattern_pixels.Get();
  surf_8bit = (BYTE *)surface;

  for(vector<PATTERN>::iterator pitr = patterns.begin();
      pitr != patterns.end(); pitr++) {
    int area = ((int)pitr->width + 1) * ((int)pitr->height + 1);
    int actual_width = (int)pitr->width + 1;

    for (vector<PNT>::iterator point = pitr->spots.begin();
         point != pitr->spots.end(); point++) {
      BYTE *pattern_source = source;
      BYTE *pattern_dest = surf_8bit
        + (int)point->x + (int)point->y * pitch;

      for (int y = 0; y <= pitr->height; y++) {
        memcpy(pattern_dest, pattern_source, actual_width);
        pattern_source += actual_width;
        pattern_dest += pitch;
      }
    }

    source += area;
  }
}

class datafile {
public:
  inline BYTE getByte() {return *res_ptr++;}

  WORD getWord() {
    WORD lo = getByte(), hi = getByte();
    return (hi << 8) | lo;
  }

  WORD getUsuallyByte() {
    BYTE bval = getByte();

    return bval ? bval : getWord();
  }

  datafile(BYTE *res_ptr) : res_ptr(res_ptr) {}

  BYTE *res_ptr;
};

auto_ptr<vector<CCompactMap *> >
CCompactMap::LoadMapSet(const char *res_name, const char *type,
                        HMODULE module, WORD language) {
  auto_ptr<vector<CCompactMap *> > res(new vector<CCompactMap *>());
  HRSRC res_handle = FindResourceEx(module, type, res_name, language);
  HGLOBAL res_data = LoadResource(module, res_handle);
  BYTE *res_ptr = (BYTE *)LockResource(res_data);
  BYTE *res_end = res_ptr + SizeofResource(module, res_handle);

  while (res_ptr != res_end) {
    res->push_back(new CCompactMap(&res_ptr));
  }

  TryAndReport(FreeResource(res_data));

  return res;
}

CCompactMap::CCompactMap(BYTE **source) {
  logger << "Cons cmp" << endl;

  datafile f(*source);

  logger << "(# of block colors) ";
  blocks.resize(TryAndReport(f.getByte()));
  block_areas.resize(blocks.size());

  vector<BYTE>::iterator block_i;
  vector<vector<RCT> >::iterator area_i;
  for(block_i = blocks.begin(), area_i = block_areas.begin();
      block_i != blocks.end(); block_i++, area_i++) {
    // get this block's color
    *block_i = f.getByte();
      
    // get how many blocks of it we have
    area_i->resize(f.getUsuallyByte()); 

    for(vector<RCT>::iterator rect_i = area_i->begin();
        rect_i != area_i->end(); rect_i++) {
      rect_i->x = f.getByte();
      rect_i->y = f.getByte();
      rect_i->w = f.getByte();
      rect_i->h = f.getByte();
    }
  }

  logger << "(# of patterns) ";
  patterns.resize(TryAndReport(f.getUsuallyByte()));
		
  vector<PATTERN>::iterator pitr;

  for(pitr = patterns.begin(); pitr != patterns.end(); pitr++) {
    const int width = f.getUsuallyByte();
    const int height = f.getUsuallyByte();

    pitr->width = width - 1;
    pitr->height = height - 1;

    unsigned int old_pattern_size = pattern_pixels.Size();
    pattern_pixels.Reallocate(old_pattern_size + width * height);
    
    BYTE *buffer = (BYTE *)pattern_pixels.Get() + old_pattern_size;

    for(DWORD y = 0; y < height; y++) {
      for(DWORD x = 0; x < width; x++) {
        buffer[x] = f.getByte();
      }
      buffer += width;
    }

    pitr->spots.resize(f.getUsuallyByte());

    for(vector<PNT>::iterator sitr = pitr->spots.begin();
        sitr != pitr->spots.end(); sitr++) {
      sitr->x = f.getByte();
      sitr->y = f.getByte();
    }
  }

  logger << "get number of left over rows" << endl;
  left_over.resize(f.getUsuallyByte());

  VCTR_ROW::iterator ritr;
  for(ritr = left_over.begin(); ritr != left_over.end(); ritr++) {
    ritr->resize(f.getByte());
    
    // now get each left over run in this row
    int current_offset = 0;
      
    for(ROW::iterator run_i = ritr->begin(); run_i != ritr->end(); run_i++) {
      const int trans = f.getByte(), opaque = f.getByte() + 1;
      const unsigned int old_size = left_over_pixels.Size();
      BYTE *data;

      current_offset += trans;
        
      // make room for the next non-transparent run
      left_over_pixels.Reallocate(old_size + opaque);
      
      data = (BYTE *)left_over_pixels.Get() + old_size;

      run_i->x = current_offset;
      run_i->opaque_count = opaque - 1;

      for(int i = 0; i < opaque; i++) {
        *data++ = f.getByte();
      }

      current_offset += opaque;
    }
  }

  logger << "Loaded cmp" << endl;

  *source = f.res_ptr;
}

auto_ptr<SurfaceFiller> CCompactMap::Filler() {
  return auto_ptr<SurfaceFiller>(new CmpFiller(this));
}
