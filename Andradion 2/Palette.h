// Palette.h: interface for the CPalette class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PALETTE_H__7B574D60_A158_11D4_B6FE_0050040B0541__INCLUDED_)
#define AFX_PALETTE_H__7B574D60_A158_11D4_B6FE_0050040B0541__INCLUDED_

using NGameLib2::CGraphics;

// this module handles palette setting and dark/bright effects

extern FIXEDNUM current_brightness_factor;
const BYTE *PalInitialize(CGraphics& gr,const BYTE *ifs,bool ignore_data);
void PalInitializeWithMenuPalette(CGraphics& gr);
void PalInitializeWithIntroPalette(CGraphics& gr);
void PalRelease();
void PalApplyBrightnessFactor();
void PalSetBrightnessFactor(FIXEDNUM new_brightness_factor);

#endif // !defined(AFX_PALETTE_H__7B574D60_A158_11D4_B6FE_0050040B0541__INCLUDED_)
