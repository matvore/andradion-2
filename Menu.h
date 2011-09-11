/*
Copyright 2011 Matt DeVore

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// this class will process menu data and display it
class CMenu {
public:
  Gfx::Surface *Backdrop() {return backdrop;}
	
  // note that there are shadow colors for each font in this constructor
  //  to not use shadows, use the other constructor
  CMenu(Gfx::Font *font,
        BYTE font_color,BYTE font_shadow,
        BYTE selected_color,BYTE selected_shadow,
        BYTE header_color,BYTE header_shadow,
        int shadow_offset, Gfx::Surface *backdrop);

  void SetStrings(const std::string& header_,
                  const std::vector<std::string>& strings_,
                  int current_selection_);
  void SetStrings(const char *str, int current_selection_);

  std::string GetHeader() const {return header;}
  std::string GetString(int index) const {return strings[index];}

  int GetSelectionIndex() const;

  bool MoveUp();
  bool MoveDown();

  void FillSurface(); // prints out to the back buffer

  void ChangeGraphicsConfig(Gfx::Font *f, Gfx::Surface *b);

protected:
  void CalculateTextCoordinates();

  Gfx::Font *font;
  Gfx::Surface *backdrop;
  std::pair<BYTE, BYTE> font_color, selected_color, header_color;
  int shadow_offset; // zero if shadows are to be disabled
  std::string header;
  std::vector<std::string> strings;
  std::vector<std::pair<int, int> > tc; // text coordinates
  std::pair<int,int> hc; // header coordinates
  int num_strings, current_selection;
  static const int SELECTION_NOSTRINGSYET;
};

