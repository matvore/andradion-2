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
#include "Gfx.h"
#include "CompactMap.h"
#include "Logger.h"
#include "Resource.h"

using std::pair;
using std::min;
using std::max;
using std::string;
using std::bad_alloc;
using std::vector;
using std::auto_ptr;
using std::endl;

class CmpFiller : public Gfx::SurfaceFiller {
  CompactMap *parent;

public:
  CmpFiller(CompactMap *parent) : parent(parent) {}

  virtual void Fill(BYTE *surf_ptr, int pitch,
                    Gfx::Surface *surface) throw() {
    Gfx::Rectangle(surf_ptr, pitch, 0, 0, surface->GetWidth(),
                        surface->GetHeight(), 0);

    parent->Render(surf_ptr, pitch);
  }
};

void CompactMap::RenderStep1(Gfx *gfx, int x, int y, bool clip) {
  Array<BYTE>::Iterator block_i;
  Array<Array<RCT> >::Iterator area_i;

  gfx->Lock();

  for (block_i = blocks.Begin(), area_i = block_areas.Begin();
       block_i != blocks.End(); block_i++, area_i++) {
    RECT dest;

    for (Array<RCT>::Iterator rct_i = area_i->Begin();
         rct_i != area_i->End(); rct_i++) {
      dest.left = rct_i->x + x;
      dest.top = rct_i->y + y;
      dest.right = dest.left + rct_i->w + 1;
      dest.bottom = dest.top + rct_i->h + 1;

      gfx->Rectangle(&dest, *block_i, clip);
    }
  }

  gfx->Unlock();
}

void CompactMap::Render(BYTE *surface, int pitch) {
  Array<BYTE>::Iterator block_i;
  Array<Array<RCT> >::Iterator area_i;

  for (block_i = blocks.Begin(), area_i = block_areas.Begin();
       block_i != blocks.End(); block_i++, area_i++) {
    for (Array<RCT>::Iterator rct_i = area_i->Begin();
         rct_i != area_i->End(); rct_i++) {
      Gfx::Rectangle(surface, pitch, rct_i->x, rct_i->y,
                          rct_i->w + 1, rct_i->h + 1, *block_i);
    }
  }

  // figure first and last row to blit in leftover
  // get a pointer into the surface, too
  BYTE *source = (BYTE *)left_over_pixels.Get();
  BYTE *surf = surface;
  
  for(VCTR_ROW::Iterator row = left_over.Begin();
      row != left_over.End(); row++) {
    for(ROW::Iterator run_i = row->Begin(); run_i != row->End(); run_i++) {
      const int amount = run_i->opaque_count + 1;
      
      memcpy(surf + run_i->x, source, amount);
      source += amount;
    }

    surf += pitch;
  }

  source = (BYTE *)pattern_pixels.Get();

  for(Array<PATTERN>::Iterator pitr = patterns.Begin();
      pitr != patterns.End(); pitr++) {
    int area = ((int)pitr->width + 1) * ((int)pitr->height + 1);
    int actual_width = (int)pitr->width + 1;

    for (Array<PNT>::Iterator point = pitr->spots.Begin();
         point != pitr->spots.End(); point++) {
      BYTE *pattern_source = source;
      BYTE *pattern_dest = surface
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

auto_ptr<std::vector<CompactMap *> >
CompactMap::LoadMapSet(const char *type, const char *res_name) {
  auto_ptr<vector<CompactMap *> > res(new vector<CompactMap *>());
  Resource resource(type, res_name);

  BYTE *res_ptr = const_cast<BYTE *>(resource.GetPtr());
  BYTE *res_end = res_ptr + resource.GetSize();

  while (res_ptr != res_end) {
    res->push_back(new CompactMap(&res_ptr));
  }

  return res;
}

CompactMap::CompactMap(BYTE **source) {
  logger << "Cons cmp" << endl;

  datafile f(*source);

  blocks.Resize(LogResult("# of block colors", f.getByte()));
  block_areas.Resize(blocks.Size());

  Array<BYTE>::Iterator block_i;
  Array<Array<RCT> >::Iterator area_i;
  for(block_i = blocks.Begin(), area_i = block_areas.Begin();
      block_i != blocks.End(); block_i++, area_i++) {
    // get this block's color
    *block_i = f.getByte();
      
    // get how many blocks of it we have
    area_i->Resize(f.getUsuallyByte());

    for(Array<RCT>::Iterator rect_i = area_i->Begin();
        rect_i != area_i->End(); rect_i++) {
      rect_i->x = f.getByte();
      rect_i->y = f.getByte();
      rect_i->w = f.getByte();
      rect_i->h = f.getByte();
    }
  }

  patterns.Resize(LogResult("# of patterns", f.getUsuallyByte()));

  Array<PATTERN>::Iterator pitr;

  for(pitr = patterns.Begin(); pitr != patterns.End(); pitr++) {
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

    pitr->spots.Resize(f.getUsuallyByte());

    for(Array<PNT>::Iterator sitr = pitr->spots.Begin();
        sitr != pitr->spots.End(); sitr++) {
      sitr->x = f.getByte();
      sitr->y = f.getByte();
    }
  }

  logger << "get number of left over rows" << endl;
  left_over.Resize(f.getUsuallyByte());

  VCTR_ROW::Iterator ritr;
  for(ritr = left_over.Begin(); ritr != left_over.End(); ritr++) {
    ritr->Resize(f.getByte());
    
    // now get each left over run in this row
    int current_offset = 0;
      
    for(ROW::Iterator run_i = ritr->Begin(); run_i != ritr->End(); run_i++) {
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

auto_ptr<Gfx::SurfaceFiller> CompactMap::Filler() {
  return auto_ptr<Gfx::SurfaceFiller>(new CmpFiller(this));
}
