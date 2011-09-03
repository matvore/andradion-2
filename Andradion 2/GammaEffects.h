// GammaEffects.h: interface for the CGammaEffects class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMMAEFFECTS_H__4E492580_A088_11D4_B6FE_0050040B0541__INCLUDED_)
#define AFX_GAMMAEFFECTS_H__4E492580_A088_11D4_B6FE_0050040B0541__INCLUDED_

using NGameLib2::CGraphics;

// the gammaeffects module has functions which are all preceded by Gam
void GamOneFrame();
void GamGetShot();
void GamPickupHealth();
void GamPickupAmmo();
void GamRelease();
void GamInitialize(CGraphics& gr);

#endif // !defined(AFX_GAMMAEFFECTS_H__4E492580_A088_11D4_B6FE_0050040B0541__INCLUDED_)
