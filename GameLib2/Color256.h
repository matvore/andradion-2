// Color256.h: interface for the CColor256 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COLOR256_H__8F12B880_6FCB_11D4_B6FE_0050040B0541__INCLUDED_)
#define AFX_COLOR256_H__8F12B880_6FCB_11D4_B6FE_0050040B0541__INCLUDED_

#include "StdAfx.h"

namespace NGameLib2
{
	class CColor256 : public CColor
	{
	public:
		CColor256();
		CColor256(BYTE r_,BYTE g_,BYTE b_);

		virtual int Certify();
		BYTE Color() const; // must be certified to call

		static void ChangePalette(LPDIRECTDRAWPALETTE pal);
		static void ChangePalette(const PALETTEENTRY *pal,int entries);
		static void ClearPalette(); // clears palette data to free memory
		static HPALETTE GetGDIPalette(); // do not destroy the returned handle!

	private:
		// the color in 8-bit format (only valid if true == this->is_certified)
		BYTE col;
		
		// pointer to palette entries
		static HPALETTE gdi_palette;
	};
}

#endif // !defined(AFX_COLOR256_H__8F12B880_6FCB_11D4_B6FE_0050040B0541__INCLUDED_)
