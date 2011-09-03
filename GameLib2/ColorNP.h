// ColorNP.h: interface for the CColorNP class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _5A3217A0_6FD0_11d4_B6FE_0050040B0541_INCLUDED_
#define _5A3217A0_6FD0_11d4_B6FE_0050040B0541_INCLUDED_

#include "StdAfx.h"

namespace NGameLib2
{

class CColorNP : public CColor // non-palettized color translation
{
	friend class CGraphics;

public:
	unsigned short Color16b() const;
	unsigned long Color32b() const;
	virtual int Certify();
	CColorNP();
	CColorNP(BYTE r_,BYTE g_,BYTE b_,BYTE a_ = 0);

private:
	unsigned long color;
};

} // end namespace of NGameLib2

#endif // !defined(AFX_COLOR_H__15C72422_5E64_11D4_B6FE_0050040B0541__INCLUDED_)
