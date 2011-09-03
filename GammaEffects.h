// the gammaeffects module has functions which are all preceded by Gam
void GamOneFrame();
void GamGetShot(FIXEDNUM current_health);
void GamPickupHealth(FIXEDNUM current_health);
void GamPickupAmmo();
void GamRelease();
void GamInitialize(CGraphics& gr);
bool GamShowHealth();
FIXEDNUM GamVirtualHealth(FIXEDNUM current_health);
