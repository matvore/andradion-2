#include "stdafx.h"
#include "Certifiable.h"
#include "CompactMap.h"
#include "Graphics.h"
#include "Color.h"
#include "Color256.h"
#include "Bob.h"
#include "Menu.h"
#include "logger.h"

// Comment the next three lines to allow this module to log
#define WriteLog(x)
#undef LogArg
#define LogArg(x)
#undef TryAndReport
#define TryAndReport(x) x

namespace NGameLib2
{
  const int CMenu::SELECTION_NOSTRINGSYET = -1;

  CMenu::CMenu(const LOGFONT& font_,
	       COLORREF font_color_,COLORREF font_shadow,
	       COLORREF selected_color_,COLORREF selected_shadow,
	       COLORREF header_color_,COLORREF header_shadow,
	       int shadow_offset_
	       ) : 
    // create a font for our menu
    font((HFONT)CreateFontIndirect(&font_)),

    // copy various constructor parameters into member variables
    font_color(font_color_,font_shadow),
    selected_color(selected_color_,selected_shadow),
    header_color(header_color_,header_shadow),
    shadow_offset(shadow_offset_),

    // get the palette handle if we are in a palettized mode, or set
    // pal member to NULL otherwise
    pal((1 == CGraphics::BytesPerPixel()) ? 
	CColor256::GetGDIPalette() : NULL) 
  {} // end of constructor

  CMenu::CMenu(const LOGFONT& font_,
	       COLORREF font_color_,
	       COLORREF selected_color_,
	       COLORREF header_color_) :

    // create a font for our menu
    font((HFONT)CreateFontIndirect(&font_)), 
    
    // copy various constructor parameters into corresponding member
    //  variables
    font_color(font_color_,0),
    selected_color(selected_color_,0),
    header_color(header_color_,0),
    shadow_offset(0),

    // get the palette handle if we are in a palettized mode, or set
    // pal member to NULL otherwise
    pal((1 == CGraphics::BytesPerPixel()) ?
	CColor256::GetGDIPalette() : NULL) 
  {} // end of constructor

  CMenu::~CMenu()
  {
    DeleteObject((HGDIOBJ)this->font);
  }

  void CMenu::FillSurface(CGraphics& gr)
  {
    // make sure we've been passed strings
    assert(SELECTION_NOSTRINGSYET != this->current_selection);

    HDC dc;

    LPDIRECTDRAWSURFACE2 x = gr.GetTargetBufferInterface();

    if(FAILED(TryAndReport(x->GetDC(&dc))))
      {
	WriteLog("CMenu::FillSurface could not get DC of target "
		 "surface; returning early");
	return;
      }

    // select the right objects into the dc
    HFONT old_font = (HFONT)SelectObject(dc,HGDIOBJ(this->font));
    int old_bkmode = SetBkMode(dc,TRANSPARENT);

    HPALETTE old_pal;

    if(NULL != this->pal)
      {
	old_pal = SelectPalette(dc,this->pal,FALSE);
      }

    COLORREF old_color;
    if(0 != shadow_offset)
      {
	old_color = SetTextColor(dc,this->header_color.second);
	TextOut(dc,hc.first+this->shadow_offset,hc.second+this->shadow_offset,this->header.c_str(),this->header.length());
	SetTextColor(dc,this->header_color.first);
      }
    else
      {
	old_color = SetTextColor(dc,this->header_color.first);
      }
		
    TextOut(dc,hc.first,hc.second,this->header.c_str(),this->header.length());
    SetTextColor(dc,this->font_color.first);
    for(int i = 0; i < this->num_strings;i++)
      {
	if((const int)i == this->current_selection)
	  {
	    continue; // print the currently selected with a different color
	  }
	if(0 != this->shadow_offset)
	  {
	    SetTextColor(dc,this->font_color.second);
	    TextOut(dc,tc[i].first+this->shadow_offset,tc[i].second+this->shadow_offset,this->strings[i].c_str(),this->strings[i].length());
	    SetTextColor(dc,this->font_color.first);
	  }
	TextOut(dc,tc[i].first,tc[i].second,this->strings[i].c_str(),this->strings[i].length());
      }

    if(0 != this->shadow_offset)
      {
	SetTextColor(dc,this->selected_color.second);
	TextOut
	  (
	   dc,
	   tc[this->current_selection].first+this->shadow_offset,
	   tc[this->current_selection].second+this->shadow_offset,
	   this->strings[this->current_selection].c_str(),
	   this->strings[this->current_selection].length()
	   );
      }
    SetTextColor(dc,this->selected_color.first);
    TextOut(
	    dc,
	    tc[this->current_selection].first,
	    tc[this->current_selection].second,
	    this->strings[this->current_selection].c_str(),
	    this->strings[this->current_selection].length()
	    );

    if(NULL != this->pal)
      {
	SelectPalette(dc,old_pal,FALSE);
      }

    // unselect all those things we selected before
    SelectObject(dc,(HGDIOBJ)old_font);
    SetTextColor(dc,old_color);
    SetBkMode(dc,old_bkmode);

    TryAndReport(x->ReleaseDC(dc));
  } // end function CMenu::FillSurface

  int CMenu::GetSelectionIndex() const
  {
    // make sure we've been passed strings
    assert(SELECTION_NOSTRINGSYET != this->current_selection);

    return this->current_selection;
  }

  bool CMenu::MoveDown()
  {
    if((const int)(this->current_selection) == this->num_strings - 1)
      {
	return false; // can't move
      }
    else
      {
	this->current_selection++;
	return true;
      }
  }

  bool CMenu::MoveUp()
  {
    if(0 == this->current_selection)
      {
	return false; // can't move
      }
    else
      {
	this->current_selection--;
	return true;
      }
  }

  void CMenu::SetStrings(const tstring& header_,const vector<tstring>& strings_,int current_selection_)
  {
    this->header = header_;
    this->strings = strings_;
    this->num_strings = strings.size();
    this->current_selection = current_selection_;
		
    // figure out text and header coordinates
    int width = CGraphics::ModeWidth();
    int height = CGraphics::ModeHeight();

    HDC dc = CreateCompatibleDC(NULL);
			
    HGDIOBJ old_font = SelectObject(dc,(HGDIOBJ)this->font);
			
    // figure out the coordinates of everything
    SIZE size;
    TEXTMETRIC tm;

    GetTextMetrics(dc,&tm);
	
    int collective_height = tm.tmHeight * (this->num_strings+1);

    hc.second = (height - collective_height) / 2;
    GetTextExtentPoint32(dc,this->header.c_str(),this->header.length(),&size);
    hc.first = (width - size.cx) / 2;

    tc.resize(this->num_strings);

    for(DWORD i = 0; i < tc.size(); i++)
      {
	GetTextExtentPoint32(dc,this->strings[i].c_str(),this->strings[i].length(),&size);
	tc[i].first  = (width - size.cx) / 2;
	tc[i].second = hc.second + ((i+1) * tm.tmHeight);
      }

    SelectObject(dc,(HGDIOBJ)old_font);
    DeleteDC(dc);
  }
}

