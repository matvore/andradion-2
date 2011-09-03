// Color256.cpp: implementation of the CColor256 class.
//
//////////////////////////////////////////////////////////////////////

#include "Certifiable.h"
#include "Color.h"
#include "Color256.h"

using std::vector;

// Comment the next five lines to allow this module to log
#define WriteLog(x)
#undef LogArg
#define LogArg(x)
#undef TryAndReport
#define TryAndReport(x) x

namespace NGameLib2
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

	CColor256::CColor256() : CColor()
	{
		// nothing to do here
	}

	CColor256::CColor256(BYTE r_,BYTE g_,BYTE b_) : CColor(r_,g_,b_)
	{
		// nothing to do here
	}

	void CColor256::ClearPalette()
	{
		if(NULL == CColor256::gdi_palette)
		{
			DeleteObject((HGDIOBJ)CColor256::gdi_palette);
			CColor256::gdi_palette = NULL;
		}
	}

	int CColor256::Certify()
	{
		assert(!this->Certified());
		assert(NULL != CColor256::gdi_palette);
		this->col = (BYTE)GetNearestPaletteIndex(CColor256::gdi_palette,RGB(this->r,this->g,this->b));
		return CCertifiable::Certify();
	}

	BYTE CColor256::Color() const
	{
		assert(this->Certified());

		return this->col;
	}

	void CColor256::ChangePalette(const PALETTEENTRY *pal,int entries)
	{
		// setup the log palette struct first
		int log_palette_size;
		
		log_palette_size = sizeof(PALETTEENTRY);
		log_palette_size *= entries;
		log_palette_size += sizeof(LOGPALETTE);

		LOGPALETTE *log = (LOGPALETTE *)new BYTE[log_palette_size];

		log->palVersion = 0x300; // ?
		log->palNumEntries = (WORD)entries;

		for(int i = 0; i < entries; i++)
		{
			log->palPalEntry[i].peFlags = 0;
			log->palPalEntry[i].peRed = pal[i].peRed;
			log->palPalEntry[i].peGreen = pal[i].peGreen;
			log->palPalEntry[i].peBlue = pal[i].peBlue;
		}

		// clear previous palette
		CColor256::ClearPalette();

		// all done, let's make it:
		CColor256::gdi_palette = CreatePalette(log);

		delete (BYTE *)log;

		// i would uncertify all CColor256's that are being used right
		//  now, but I can't, so let's just leave this->col and is_cert
		//  as they are
	}

	void CColor256::ChangePalette(LPDIRECTDRAWPALETTE pal)
	{
		// first job, figure out how many entries are in pal_ . . .
		DWORD pc;
		pal->GetCaps(&pc);

		int entries;

		if(pc & DDPCAPS_4BIT)
		{
			entries = 16;
		}
		else if(pc & DDPCAPS_2BIT)
		{
			entries  = 4;
		}
		else if(pc & DDPCAPS_1BIT)
		{
			entries = 2;
		}
		else
		{
			entries = 256;
		}

		vector<PALETTEENTRY> pe(entries);

		pal->GetEntries(0,0,entries,pe.begin());

		CColor256::ChangePalette(pe.begin(),entries);
	}

	HPALETTE CColor256::gdi_palette = NULL;
	
	HPALETTE CColor256::GetGDIPalette()
	{
		return CColor256::gdi_palette;
	}
}

