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
#include "Fixed.h"
#include "Gfx.h"
#include "Logger.h"
#include "Palette.h"

using std::endl;

const int MENUPALETTE_BASECOLORS = 249;
const int INTROPALETTE_BASECOLORS = 26;

static FIXEDNUM old_br = Fixed(1.0f);
static FIXEDNUM old_bg = Fixed(1.0f);
static FIXEDNUM old_bb = Fixed(1.0f);

/** Fills a given byte with one of three values. These values are
 * 0, 0x80, and 0xff.
 * @param target reference to the byte value to fill.
 * @return 1 if <tt>target</tt> was set to 0, 0 otherwise.
 */
static int RandomElementValue(BYTE& target) {
  int return_value = 0;
  switch(rand()%3) {
  case 0:
    return_value = 1;
    target = 0;
    break;
  case 1:
    target = 0x80;
    break;
  case 2:
    target = 0xff;
  }

  return return_value;
}

static inline BYTE ScaleColor(BYTE source, FIXEDNUM scale) {
  long x = FixedCnvFrom<long>(scale*FIXEDNUM(source));

  return x <= 0xff ? (BYTE)x : 0xff;
}

const BYTE *PalInitialize(const BYTE *ifs) {
  PALETTEENTRY pe[256];

  old_br = old_bg = old_bb = Fixed(1.0f);

  // get number of palette entries
  int entries = (int)(*ifs++);
  if(0 == entries) {
    entries = 256;
  }

  int i;
  for(i = 0; i < entries; i++) {
    pe[i].peRed = *ifs++;
    pe[i].peGreen = *ifs++;
    pe[i].peBlue = *ifs++;
    pe[i].peFlags = 0;
  }

  // fill with bright colors
  for(; i < 256; i++) {
    // count black color elements
    //  so we don't have more than one
    int num_black_colors = 0;

    pe[i].peFlags = 0;

    num_black_colors += RandomElementValue(pe[i].peRed);
    num_black_colors += RandomElementValue(pe[i].peGreen);
    num_black_colors += RandomElementValue(pe[i].peBlue);

    if(3 == num_black_colors) {
      // set all to pure white
      pe[i].peRed = pe[i].peGreen = pe[i].peBlue = 0xff;
    }
  }

  logger << "Setting new level palette" << endl;

  Gfx::Get()->SetPalette(pe, 256, false);

  return ifs;
}

void PalInitializeWithMenuPalette() {
  old_br = old_bg = old_bb = Fixed(1.0f);

  const BYTE mpe[MENUPALETTE_BASECOLORS * 3] =
    {
  4,   2,   4,  8,   8,   8,  16,  16,  16,  1,   0,  57,
  15,  25,  53,  32,  32,  32,  40,  40,  40,  57,  44,  60,
  48,  48,  48,  52,  50,  52,  56,  56,  56,  60,  58,  60,
  0,   0,  64,  0,  24, 112,  8,  32,  88,  8,  56, 112,
  0, 104,   0,  4, 106,   4,  70,  48,  54,  81,  41,  54,
  82,  41,  58,  84,  58,  60,  93,  55,  53,  95,  51,  59,
  108,  26,  28,  122,   9,  10,  118,  26,  27,  96,  32,  32,
  97,  50,  55,  104,  50,  50,  110,  54,  61,  75,  61,  73,
  83,  61,  71,  106,  53,  69,  86,  67,  63,  96,  72,  16,
  106,  82,  61,  66,  65,  66,  83,  75,  70,  88,  72,  72,
  80,  80,  80,  84,  86,  84,  99,  69,  70,  98,  67,  74,
  97,  78,  80,  104,  82,  85,  110,  83,  88,  106,  90,  84,
  115,  76,  68,  116,  88,  70,  126,  84,  79,  112,  83,  86,
  112,  86,  95,  112,  89,  88,  119,  93,  96,  96,  96,  88,
  123,  97,  94,  104,  96, 104,  108, 102, 108,  104, 104, 104,
  108, 106, 108,  122,  99, 104,  124, 118, 124,  120, 120, 120,
  124, 122, 124,  0,   0, 128,  4,   2, 132,  0,   0, 248,
  4,   2, 252,  40,  72, 136,  44,  76, 140,  56,  96, 144,
  0, 248,   0,  104, 248,   0,  129,   0,   3,  140,   6,  12,
  145,  10,  13,  172,  13,  18,  188,  14,  20,  176,   0,  40,
  180,   2,  44,  160,  32,  32,  176,  48,  48,  128,  64,   0,
  128,  73,  63,  146,  78,  61,  160,  64,  48,  133,  92,  88,
  139,  82,  83,  146,  86,  79,  147,  85,  84,  146, 115,  82,
  133, 103, 101,  140, 100, 106,  152,  99,  96,  157, 110,  97,
  144, 112, 110,  164,  81,  69,  169,  84,  76,  166,  92,  92,
  168,  91,  90,  176,  90,  77,  181,  94,  88,  165,  96,  87,
  169, 118,  95,  178, 111,  95,  185, 102,  91,  188, 110,  86,
  181, 116,  85,  161, 122, 108,  161, 125, 119,  189, 107, 104,
  189, 120, 105,  180, 122, 115,  188, 125, 114,  195,   8,  16,
  198,  20,  24,  237,  24,  28,  247,   8,   9,  248,   0,   0,
  252,   2,   4,  232,  25,  32,  203,  77,  76,  192,  87,  78,
  203, 100,  72,  196, 105,  90,  206, 113,  83,  203, 126,  93,
  210, 107,  87,  210, 117,  87,  213, 122,  92,  217, 126,  90,
  200,  96,  96,  205, 100, 104,  204, 123, 102,  214, 126, 106,
  224,  82,  83,  243,  85,  84,  248,  83,  83,  248,   0, 128,
  158, 139, 121,  189, 136, 127,  220, 132,  93,  206, 128, 100,
  202, 133, 107,  212, 129, 105,  211, 137, 101,  214, 139, 107,
  217, 129,  99,  216, 140, 101,  210, 131, 113,  212, 140, 115,
  214, 140, 121,  218, 129, 113,  218, 137, 118,  221, 148, 110,
  212, 144, 118,  215, 151, 122,  215, 154, 127,  222, 148, 116,
  219, 151, 124,  220, 155, 118,  220, 165, 120,  228, 148, 111,
  228, 153, 116,  228, 154, 124,  225, 164, 116,  225, 162, 124,
  225, 170, 119,  240, 248,   0,  244, 250,   4,  248, 248,   0,
  128, 128, 128,  132, 130, 132,  139, 132, 139,  136, 136, 136,
  140, 138, 140,  152, 136, 128,  152, 152, 152,  156, 154, 156,
  165, 136, 129,  176, 152, 152,  172, 162, 162,  168, 168, 168,
  172, 170, 172,  176, 168, 176,  176, 176, 176,  180, 178, 180,
  180, 182, 180,  128, 128, 248,  132, 130, 252,  194, 146, 130,
  223, 139, 133,  215, 159, 130,  221, 156, 128,  218, 151, 152,
  215, 161, 134,  216, 162, 135,  222, 178, 146,  232, 142, 142,
  225, 146, 133,  231, 155, 134,  232, 157, 129,  232, 158, 140,
  228, 158, 156,  228, 165, 131,  228, 160, 137,  226, 172, 130,
  230, 174, 137,  234, 174, 133,  233, 172, 140,  225, 164, 145,
  229, 169, 145,  233, 167, 151,  235, 175, 147,  233, 177, 136,
  232, 184, 143,  229, 188, 146,  236, 180, 147,  233, 180, 154,
  235, 185, 151,  232, 189, 159,  230, 177, 160,  237, 181, 163,
  236, 185, 160,  237, 189, 169,  233, 189, 178,  225, 197, 158,
  233, 193, 150,  244, 192, 159,  231, 192, 163,  238, 192, 168,
  238, 200, 169,  235, 197, 177,  192, 192, 192,  196, 194, 196,
  202, 196, 202,  200, 200, 200,  204, 202, 204,  216, 208, 216,
  220, 214, 220,  227, 218, 212,  224, 224, 224,  228, 226, 228,
  235, 228, 235,  242, 234, 228,  240, 240, 240,  244, 242, 244,
  248, 248, 248
    };

  // get this data onto the video card
  PALETTEENTRY pe[256];

  const BYTE *mpe_ptr = mpe;

  int i;
  for(i = 0; i < MENUPALETTE_BASECOLORS; i++) {
    pe[i].peFlags = 0;
    pe[i].peRed = *mpe_ptr++;
    pe[i].peGreen = *mpe_ptr++;
    pe[i].peBlue = *mpe_ptr++;
  }

  // fill the rest with magenta
  for(; i < 256; i++) {
    pe[i].peFlags = 0;
    pe[i].peRed = pe[i].peBlue = 0xff;
    pe[i].peGreen = 0x00;
  }

  Gfx::Get()->SetPalette(pe, 256, false);
}

void PalInitializeWithIntroPalette() {
  const BYTE ipe[INTROPALETTE_BASECOLORS*3] =
    {
      0,0,0,
      36,34,4,
      64,64,32,
      0,0,248,
      80,96,144,
      0,248,0,
      0,128,128,
      112,128,248,
      128,24,0,
      128,64,0,
      224,0,0,
      248,0,0,
      252,2,4,
      252,34,4,
      252,66,68,
      252,98,68,
      128,128,64,
      248,248,0,
      252,254,4,
      128,128,128,
      252,130,132,
      252,162,132,
      192,192,192,
      252,194,252,
      248,248,248,
      252,250,252
    };

  PALETTEENTRY pe[256];

  int i;
  const BYTE *ipe_data = ipe;
  for(i = 0; i < INTROPALETTE_BASECOLORS; i++)
    {
      pe[i].peFlags = 0;
      pe[i].peRed = *ipe_data++;
      pe[i].peGreen = *ipe_data++;
      pe[i].peBlue = *ipe_data++;
    }

  // fill the rest with shades of gray
  // Fixed(256/(256-INTROPALETTE_BASECOLORS)) = 72944
  for(FIXEDNUM val = 1; i < 256; i++,val+=72944)
    {
      BYTE x = (BYTE)(val > Fixed(255) ? 255 : FixedCnvFrom<long>(val));

      pe[i].peRed = x;
      pe[i].peGreen = x;
      pe[i].peBlue = x;
      pe[i].peFlags = 0;
    }

  Gfx::Get()->SetPalette(pe, 256, false);
}

void PalSetBrightnessFactor(FIXEDNUM br, FIXEDNUM bg, FIXEDNUM bb) {
  if (br != old_br || bg != old_bg || bb != old_bb) {
    const PALETTEENTRY *pe = Gfx::Get()->GetPalette();
    PALETTEENTRY new_pe[256];

    // scale the colors
    for(int i = 0; i < 256; i++) {
      new_pe[i].peRed = ScaleColor(pe[i].peRed, br);
      new_pe[i].peGreen = ScaleColor(pe[i].peGreen, bg);
      new_pe[i].peBlue = ScaleColor(pe[i].peBlue, bb);
    }

    Gfx::Get()->SetPalette(new_pe, 256, true);

    old_br = br;
    old_bg = bg;
    old_bb = bb;
  }
}
