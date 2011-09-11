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

#include "StdAfx.h"
#include "Gfx.h"
#include "Menu.h"
#include "Logger.h"
#include "Fixed.h"

const int CMenu::SELECTION_NOSTRINGSYET = -1;

using namespace std;

CMenu::CMenu(Gfx::Font *font,
	     BYTE font_color, BYTE font_shadow,
	     BYTE selected_color, BYTE selected_shadow,
	     BYTE header_color, BYTE header_shadow,
	     int shadow_offset, Gfx::Surface *backdrop)
  : font(font), backdrop(backdrop),
    font_color(font_color, font_shadow),
    selected_color(selected_color, selected_shadow),
    header_color(header_color, header_shadow),
    shadow_offset(shadow_offset) {
  assert(font);
  assert(backdrop);
}

void CMenu::FillSurface() {
  assert(SELECTION_NOSTRINGSYET != current_selection);

  backdrop->Draw(0, 0, false);

  font->WriteString(hc.first+shadow_offset, hc.second+shadow_offset,
                    header.c_str(), header_color.second);
  font->WriteString(hc.first, hc.second, header.c_str(), header_color.first);

  for(int i = 0; i < num_strings;i++) {
    pair<BYTE, BYTE> color = i != current_selection
      ? font_color : selected_color;
    
    font->WriteString(tc[i].first+shadow_offset, tc[i].second+shadow_offset,
                      strings[i].c_str(), color.second);
    font->WriteString(tc[i].first, tc[i].second,
                      strings[i].c_str(), color.first);
  }
}

int CMenu::GetSelectionIndex() const {
  // make sure we've been passed strings
  assert(SELECTION_NOSTRINGSYET != this->current_selection);

  return this->current_selection;
}

bool CMenu::MoveDown() {
  if(++current_selection == num_strings) {
    current_selection--;
    return false; // can't move
  } else {
    return true;
  }
}

bool CMenu::MoveUp() {
  if(!current_selection) {
    return false; // can't move
  } else {
    current_selection--;
    return true;
  }
}

void CMenu::SetStrings(const string& header_,
                       const vector<string>& strings_,
                       int current_selection_)
{
  this->header = header_;
  this->strings = strings_;
  this->num_strings = strings.size();
  this->current_selection = current_selection_;

  CalculateTextCoordinates();
}

void CMenu::CalculateTextCoordinates() {
  // figure out text and header coordinates
  int width = Gfx::Get()->GetVirtualBufferWidth();
  int height = Gfx::Get()->GetVirtualBufferHeight();

  // figure out the coordinates of everything
  int collective_height = font->GetCharHeight() * (num_strings+1);

  hc.second = (height - collective_height) / 2;
  hc.first = (width - font->GetCharWidth() * header.length()) / 2;

  tc.resize(this->num_strings);

  for(DWORD i = 0; i < tc.size(); i++) {
    tc[i].first = (width - (font->GetCharWidth()) * strings[i].length()) / 2;
    tc[i].second = hc.second + ((i+1) * font->GetCharHeight());
  }
}

void CMenu::ChangeGraphicsConfig(Gfx::Font *f, Gfx::Surface *b) {
  font = f;
  backdrop = b;
  CalculateTextCoordinates();
}

void CMenu::SetStrings(const char *str, int current_selection_) {
  string current_str;

  bool has_header = false;

  string hdr;
  vector<string> strs;

  do {
    while ('\n' != *str) {
      current_str += *str++;
    }

    str++;

    if (has_header) {
      strs.push_back(current_str);
    } else {
      hdr = current_str;
      has_header = true;
    }

    current_str = "";
  } while (*str);

  SetStrings(hdr, strs, current_selection_);
}
