#include "StdAfx.h"
#include "Certifiable.h"
#include "Color.h"
#include "ColorNP.h"
#include "CompactMap.h"
#include "Graphics.h"

CColorNP::CColorNP() : CColor() {}

CColorNP::CColorNP(BYTE r_, BYTE g_, BYTE b_, BYTE a_) : CColor(r_,g_,b_,a_) {}

int CColorNP::Certify()
{
	switch(CGraphics::BytesPerPixel()) // this val can say a lot
	{ 
	case 4: // 32-bit color
		color = 
			(a << 24) +
			(r << 16) +
			(g << 8) +
			(b);

		break;
	case 3: // 24-bit color
		color =
			(r << 16) +
			(g << 8) +
			(b);

		break;
	case 2: // 16/15-bit
		if(CGraphics::_15bColor())  // 15-bit color + alpha
			color =
				((a>>7)<<15) +
				((r>>3)<<10)+
				((g>>3)<<5);					
		else // 16-bit color
			color =
				((r>>3)<<11)+
				((g>>2)<<5);
				
		color += b>>3;
	}

	// note:  colors can never fail certification
	return CCertifiable::Certify();
}

unsigned long CColorNP::Color32b() const
{
	assert(this->Certified());
	return this->color;
}

unsigned short CColorNP::Color16b() const
{
	assert(this->Certified());
	return (unsigned short)this->color;
}

