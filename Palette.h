// this module handles palette setting and dark/bright effects

extern FIXEDNUM current_brightness_factor;
const BYTE *PalInitialize(CGraphics& gr,const BYTE *ifs,bool ignore_data);
void PalInitializeWithMenuPalette(CGraphics& gr);
void PalInitializeWithIntroPalette(CGraphics& gr);
void PalRelease();
void PalApplyBrightnessFactor();
void PalSetBrightnessFactor(FIXEDNUM bf_red, FIXEDNUM bf_green, FIXEDNUM bf_blue);

