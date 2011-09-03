#include "Stdafx.h"
#include "Certifiable.h"
#include "Color.h"

CColor::CColor(                               ) : CCertifiable()                          {}

CColor::CColor(BYTE r_,BYTE g_,BYTE b_,BYTE a_) : CCertifiable(), r(r_),g(g_),b(b_),a(a_) {}

void CColor::SetColor(BYTE r_, BYTE g_, BYTE b_, BYTE a_)
{
	assert(!this->Certified());
	this->r = r_;
	this->g = g_;
	this->b = b_;
	this->a = a_;
}

void CColor::GetColor(BYTE &r_, BYTE &g_, BYTE &b_, BYTE &a_) const
{
	r_ = this->r;
	g_ = this->g;
	b_ = this->b;
	a_ = this->a;
}

void CColor::GetColor(BYTE &r_, BYTE &g_, BYTE &b_) const
{
	r_ = this->r;
	g_ = this->g;
	b_ = this->b;
}

COLORREF CColor::GetColor() const
{
	return RGB(this->r,this->g,this->b);
}

void CColor::SetColor(COLORREF new_color)
{
	assert(!this->Certified());
	this->r = BYTE(new_color & 0xff);
	new_color >>= 8;
	this->g = BYTE(new_color & 0xff);
	new_color >>= 8;
	this->b = BYTE(new_color & 0xff);
}

