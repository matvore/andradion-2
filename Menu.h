class CColor;
class SurfaceFiller;

// this class will process menu data and display it
class CMenu {
 public:
  surf_t Backdrop() {return backdrop;}
		
  // note that there are shadow colors for each font in this constructor
  //  to not use shadows, use the other constructor
  CMenu(const LOGFONT& font_,
	COLORREF font_color_,COLORREF font_shadow,
	COLORREF selected_color_,COLORREF selected_shadow,
	COLORREF header_color_,COLORREF header_shadow,
	int shadow_offset_, surf_t backdrop);
  CMenu(const LOGFONT& font_, COLORREF font_color_,
        COLORREF selected_color_, COLORREF header_color_,
	surf_t backdrop);
  ~CMenu();

  void SetStrings(const std::string& header_,
                  const std::vector<std::string>& strings_,
                  int current_selection_);
  int GetSelectionIndex() const;

  bool MoveUp();
  bool MoveDown();

  void FillSurface(); // prints out to the back buffer

 private:
  HFONT font;
  surf_t backdrop;
  std::pair<COLORREF, COLORREF> font_color, selected_color, header_color;
  int shadow_offset; // zero if shadows are to be disabled
  std::string header;
  std::vector<std::string> strings;
  std::vector<std::pair<int, int> > tc; // text coordinates
  std::pair<int,int> hc; // header coordinates
  int num_strings, current_selection;
  static const int SELECTION_NOSTRINGSYET;
};

