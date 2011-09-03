#include "stdafx.h"
#include "Graphics.h"
#include "Menu.h"
#include "Logger.h"

const int CMenu::SELECTION_NOSTRINGSYET = -1;

using std::string;
using std::vector;

CMenu::CMenu(const LOGFONT& font_,
	     COLORREF font_color_,COLORREF font_shadow,
	     COLORREF selected_color_,COLORREF selected_shadow,
	     COLORREF header_color_,COLORREF header_shadow,
	     int shadow_offset_, surf_t backdrop)
  : font((HFONT)CreateFontIndirect(&font_)), backdrop(backdrop),
    font_color(font_color_, font_shadow),
    selected_color(selected_color_, selected_shadow),
    header_color(header_color_, header_shadow),
    shadow_offset(shadow_offset_) {}

CMenu::CMenu(const LOGFONT& font_, COLORREF font_color_,
             COLORREF selected_color_, COLORREF header_color_,
             surf_t backdrop)
  : font((HFONT)CreateFontIndirect(&font_)),
    backdrop(backdrop), font_color(font_color_, 0),
    selected_color(selected_color_, 0),
    header_color(header_color_, 0), shadow_offset(0) {}

CMenu::~CMenu() {
  DeleteObject(font);
}

void CMenu::FillSurface()
{
  // make sure we've been passed strings
  assert(SELECTION_NOSTRINGSYET != current_selection);

  HDC dc;

  GfxPut(backdrop, 0, 0, false);

  if(FAILED(GfxBackBuffer()->GetDC(&dc))) {return;}

  // select the right objects into the dc
  HFONT old_font = (HFONT)SelectObject(dc,HGDIOBJ(this->font));
  int old_bkmode = SetBkMode(dc,TRANSPARENT);

  HPALETTE old_pal = SelectPalette(dc, GfxGDIPalette(), FALSE);

  COLORREF old_color;
  if(shadow_offset) {
    old_color = SetTextColor(dc, header_color.second);
    TextOut(dc,hc.first + shadow_offset, hc.second + shadow_offset,
            header.c_str(), header.length());
    SetTextColor(dc, header_color.first);
  } else {
    old_color = SetTextColor(dc, header_color.first);
  }
		
  TextOut(dc,hc.first,hc.second, header.c_str(), header.length());
  SetTextColor(dc, font_color.first);
  for(int i = 0; i < num_strings;i++)
    {
      if(i == current_selection) {
        continue; // print the currently selected with a different color
      }
      if(shadow_offset) {
        SetTextColor(dc,font_color.second);
        TextOut(dc,tc[i].first+shadow_offset, tc[i].second+shadow_offset,
                strings[i].c_str(), strings[i].length());
        SetTextColor(dc, font_color.first);
      }
      TextOut(dc,tc[i].first, tc[i].second,
              strings[i].c_str(), strings[i].length());
    }

  if(shadow_offset) {
    SetTextColor(dc,this->selected_color.second);
    TextOut(dc,
            tc[current_selection].first+shadow_offset,
            tc[current_selection].second+shadow_offset,
            strings[current_selection].c_str(),
            strings[current_selection].length());
  }
  
  SetTextColor(dc,this->selected_color.first);
  TextOut(dc,
	  tc[current_selection].first,
          tc[current_selection].second,
	  strings[current_selection].c_str(),
	  strings[current_selection].length());

  SelectPalette(dc, old_pal, FALSE);

  // unselect all those things we selected before
  SelectObject(dc,(HGDIOBJ)old_font);
  SetTextColor(dc,old_color);
  SetBkMode(dc,old_bkmode);
		
  GfxBackBuffer()->ReleaseDC(dc);
}

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

void CMenu::SetStrings(const string& header_,const vector<string>& strings_,int current_selection_)
{
  this->header = header_;
  this->strings = strings_;
  this->num_strings = strings.size();
  this->current_selection = current_selection_;
		
  // figure out text and header coordinates
  int width = GfxModeWidth(), height = GfxModeHeight();

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

