#ifndef _77ADF345_46E9_11d4_B6FE_0050040B0541_INCLUDED_
#define _77ADF345_46E9_11d4_B6FE_0050040B0541_INCLUDED_

#include "StdAfx.h"

using std::vector;
using std::pair;

namespace NGameLib2
{
  class CColor;
  class CBob;
  class CGraphics;

  // this class will process menu data and display it
  class CMenu
    {
    public:
      // note that there are shadow colors for each font in this constructor
      //  to not use shadows, use the other constructor
      CMenu(const LOGFONT& font_,
	    COLORREF font_color_,COLORREF font_shadow,
	    COLORREF selected_color_,COLORREF selected_shadow,
	    COLORREF header_color_,COLORREF header_shadow,
	    int shadow_offset_);

      CMenu(const LOGFONT& font_,
	    COLORREF font_color_,
	    COLORREF selected_color_,
	    COLORREF header_color_);

      ~CMenu();

      void SetStrings(const tstring& header_,const vector<tstring>& strings_,int current_selection_);
      int GetSelectionIndex() const;

      bool MoveUp();
      bool MoveDown();

      void FillSurface(CGraphics& gr); // prints out to the current target buffer

    private:
      // CBob backdrop;
      HFONT font;
      pair<COLORREF,COLORREF> font_color;
      pair<COLORREF,COLORREF> selected_color;
      pair<COLORREF,COLORREF> header_color;
      int shadow_offset; // shadow offset will be zero if shadows are to be disabled
      tstring header;
      vector<tstring> strings;
      vector<pair<int,int> > tc; // text coordinates
      pair<int,int> hc; // header coordinates
      int num_strings;
      int current_selection;
      HPALETTE pal;
      static const int SELECTION_NOSTRINGSYET;
    };
}

#endif
