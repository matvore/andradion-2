// Color.h: interface for the CColor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COLOR_H__15C72422_5E64_11D4_B6FE_0050040B0541__INCLUDED_)
#define AFX_COLOR_H__15C72422_5E64_11D4_B6FE_0050040B0541__INCLUDED_

#include "StdAfx.h"

namespace NGameLib2
{

class CColor : public CCertifiable
{
public:
	// accessors which work with the entire structure
	void GetColor(BYTE& r_,BYTE& g_,BYTE& b_) const;
	void GetColor(BYTE& r_,BYTE& g_,BYTE& b_,BYTE& a_) const;
	COLORREF GetColor() const;
	void SetColor(BYTE r_,BYTE g_,BYTE b_,BYTE a_ = 0);
	void SetColor(COLORREF new_color);
		
	virtual int Certify() = 0;
	CColor();
	CColor(BYTE r_,BYTE g_,BYTE b_,BYTE a_ = 0);

protected: CertParamA(BYTE,r,Red);
protected: CertParamA(BYTE,g,Green);
protected: CertParamA(BYTE,b,Blue);
protected: CertParamA(BYTE,a,Alpha);
};

} // end namespace of NGameLib2

#endif // !defined(AFX_COLOR_H__15C72422_5E64_11D4_B6FE_0050040B0541__INCLUDED_)
