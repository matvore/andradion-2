// this module handles palette setting and dark/bright effects

const BYTE *PalInitialize(const BYTE *ifs);
void PalInitializeWithMenuPalette();
void PalInitializeWithIntroPalette();
void PalSetBrightnessFactor(FIXEDNUM br, FIXEDNUM bg, FIXEDNUM bb);
