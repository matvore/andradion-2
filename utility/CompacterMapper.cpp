// a simple program that makes .cmp files
#include <cassert>
#include <iostream>
#include <cstring>
#include <vector>
#include <fstream>
#include <windows.h>

using namespace std;

static int MINIMUMPATTERNAREA, MINIMUMBLOCKAREA;
static vector<SIZE> PATTERNAREAS;
static BYTE TRANS;

class datafile {
public:
  void putRect(const RECT& r) {
    putByte(r.left);
    putByte(r.top);
    putByte((WORD)(r.right - r.left - 1));
    putByte((WORD)(r.bottom - r.top - 1));
  }
};

// functions for manipulating loaded data . . .

static bool AllOneColor(const BYTE *data,
                        int data_width,
                        const RECT& area,
                        BYTE color)
{
  // returns true if every pixel within area is equal to color

  const BYTE *pos;

  // set data to the first pixel we are observing
  data += area.top * data_width + area.left;

  for(int y = area.top; y < area.bottom; y++)
    {
      pos = data;

      for(int x = area.left; x < area.right; x++)
	{
	  if(color != *pos)
	    {
	      return false; // found a conflicting pixel
	    }
	  pos++;
	}

      data += data_width;
    }

  // if we got here, than all pixels were equal to color
  return true; // it was all one color
}

// finds rectangles of the color at start_coor and puts them in a vector, only returning rectangles whose upper-left
//  coordinates are after or at start_coor, and replacing them with black at the same time.  Ignores rectangles whose
//  areas are too small to be compressed into blocks
static void FindRectangles(BYTE *data,
                           const SIZE& data_size,
                           const POINT& start_coor,
                           vector<RECT>& rects)
{
  int curr_x = start_coor.x, curr_y = start_coor.y;

  rects.clear();

  BYTE *data_position = &data[data_size.cx*start_coor.y+start_coor.x];
  BYTE base = *data_position;

  // the address of the pixel which is right after the one we check last
  BYTE *after_last = &data[data_size.cx*data_size.cy];

  for(;data_position<after_last;data_position++)
    {
      if(base == *data_position)
	{
	  RECT x;

	  x.left = curr_x;
	  x.right = curr_x+1;
	  x.top = curr_y;
	  x.bottom = curr_y+1;

	  // expand to the right
	  do
	    {
	      x.right++;
	      x.left++;
	    }
	  while((int)x.right <= (int)data_size.cx && AllOneColor(data,data_size.cx,x,base));

	  // back up a bit
	  x.right--;
			
	  // recall upper-left coordinates
	  x.left = curr_x;

	  // expand downward
	  do
	    {
	      x.bottom++;
	      x.top++;
	    }	
	  while((int)x.bottom <= (int)data_size.cy && AllOneColor(data,data_size.cx,x,base));

	  // back up a bit
	  x.bottom--;

	  // recall upper-left coordinates
	  x.top = curr_y;

	  // check area
	  if((x.right - x.left) * (x.bottom - x.top) >= MINIMUMBLOCKAREA)
	    {
	      // we found a new block
	      rects.resize(rects.size()+1,x);

	      // clear out this new area
	      DrawRectangle(data, data_size.cx, TRANS, x);
	    }
	}

      // increment x and change y if appropriate
      if(++curr_x >= data_size.cx)
	{
	  curr_x = 0;
	  curr_y++;
	}
    }
}

// returns true if both rects have the same data
static bool SameData(const BYTE *data,
                     int data_width,
                     const SIZE& size,
                     const pair<POINT,POINT>& coors)
{
  // find what to check in memory by making shortcuts
  const BYTE *d1 = &data[coors.first.y * data_width + coors.first.x];
  const BYTE *d2 = &data[coors.second.y * data_width + coors.second.x];
  int changing_rows = data_width - size.cx;

  for(int y = 0; y < size.cy; y++)
    {
      for(int x = 0; x < size.cx; x++)
	{
	  if(*d1++ != *d2++)
	    {
	      return false;
	    }
	}

      d1+=changing_rows;
      d2+=changing_rows;
    }

  return true; // all pixels are the same, and there is no overlap
}

// compares a vector of areas
static bool SameAndSeparate(const BYTE *data,
                            int data_width,
                            const SIZE& size,
                            const vector<POINT>& coors)
{
  // compare similar data
  pair<POINT,POINT> x;
  x.first = coors[0];
  for(int i = coors.size()-1; i >= 1; i--)
    {			
      x.second = coors[i];
      if(!SameData(data,data_width,size,x))
	{
	  return false;
	}
    }

  // look for overlapping (the first coor does not overlap at all)
  for(int c1 = 0; c1 < coors.size(); c1++)
    {
      RECT a;

      a.left = coors[c1].x;
      a.right = coors[c1].x + size.cx;
      a.top = coors[c1].y;
      a.bottom = coors[c1].y + size.cy;

      for(int c2 = c1+1; c2 < coors.size(); c2++)
	{
	  if(!((a.left >= coors[c2].x + size.cx) ||
	       (a.right <= coors[c2].x) ||
	       (a.top >= coors[c2].y + size.cy) ||
	       (a.bottom <= coors[c2].y)))
	    {
	      return false;
	    }
	}
    }

  return true;
}

static bool FindFirstPixel(const BYTE *data,
                           const SIZE& data_size,
                           POINT& coor, BYTE base)
{
  data += data_size.cx*coor.y+coor.x;

  while(true)
    {
      if(*data == base)
	{
	  return true; // we found something
	}

      // increment current position
      if(++coor.x >= data_size.cx)
	{
	  coor.x=0;
	  if(++coor.y >= data_size.cy)
	    {
	      break;
	    }
	}

      data++;
    }

  // move this coor struct to the lower right corner
  coor.x = data_size.cx-1;
  coor.y = data_size.cy-1;

  return false; // we got all the way here and couldn't find a pixel that wasn't transparent
}

// puts coordinates of first non-transparent pixel into coor (returns false if none found, works from a starting location)
static bool FindFirstNonTransparentPixel(const BYTE *data, const SIZE& data_size, POINT& coor)
{
  //cout << "FF";

  data += data_size.cx*coor.y+coor.x;
  for(;coor.y < data_size.cy;data++)
    {
      if(TRANS != *data)
	{
          return true; // we found something
	}

      // increment current position
      if(++coor.x >= data_size.cx)
	{
	  coor.x=0;
	  coor.y++;
	}
    }

  //cout << "FF'";

  // move this coor struct to the lower right corner
  coor.x = data_size.cx-1;
  coor.y = data_size.cy-1;

  //cout << "GG'";

  return false; // we got all the way here and couldn't find a pixel that wasn't transparent
}

// returns true if there are only non-transparent pixels
static bool IsOpaque(const BYTE *data, int data_width,
                     const POINT& point, const SIZE& size)
{
  // calculate number of bytes to increment data pointer
  //  to move to the next row
  int to_change_rows = (data_width - size.cx);

  // point data to first pixel to check
  data += data_width * point.y + point.x;

  for(int y = 0; y < size.cy; y++)
    {
      for(int x = 0; x < size.cx; x++)
	{
	  if(TRANS == *data)
	    {
	      return false;
	    }
	  data++;
	}
      data+=to_change_rows; // move to next row
    }

  return true;
}

int main(int argc, char **argv)
{
  cout << "Official CompactMap maker\n\n";

  istream *source;
  ifstream batch;

  if(argc > 1)
    {
      batch.open(argv[1]);
      if(0 == batch.fail())
	{
	  source = &batch;
	  cout << "Using batch file.\n";
	}
      else
	{
	  // batch file failed
	  cout << "Couldn't load batch file.\n";
	  source = &cin;
	}
    }
  else
    {
      source = &cin;
    }

  while(true)
    {
      char sn[256];
      char tn[256];
      char pn[256];
      char in[256];
      ifstream pal;
      vector<RGBTRIPLE> palette;

      cout << "Enter name of .bmp file to read (type *q to quit): ";
      source->getline(sn,256);

      if(0 == strcmp(sn,"*q") || 0 == strcmp(sn,"*Q"))
	{
	  break;
	}

      cout << "Enter the file name of the palette to convert to: ";
      source->getline(pn, 256);

      pal.open(pn);
      if (pal.fail())
        {
          cout << "Could not load the palette file " << pn << "\n";
          continue;
        }
      else
        {
          int size;

          pal >> size;
          palette.resize(size);

          for (vector<RGBTRIPLE>::iterator itr = palette.begin();
               itr != palette.end(); itr++)
            {
              int r, g, b;
              pal >> r >> g >> b;

              itr->rgbtRed = r;
              itr->rgbtGreen = g;
              itr->rgbtBlue = b;
            }          
        }

      cout << "Enter name of compressed file to write results to: ";
      source->getline(tn, 256);

      do
	{
	  cout << "Enter minimum color block area: ";
	  source->getline(in,256);
	  MINIMUMBLOCKAREA = atoi(in);
	}
      while (MINIMUMBLOCKAREA<1);

      PATTERNAREAS.resize(0);
		
      do
	{
	  cout<< "Enter pattern area; WxH (as in 32x30); (* to finish or ? to redo): ";
	  source->getline(in,256);

	  if(0 == strlen(in))
	    {
	      continue;
	    }

	  if(0 == strcmp("*",in))
	    {
	      if(0 == PATTERNAREAS.size())
		{
		  cout << "No patterns will be generated.\n";
		}
	      break;
	    }
	  else if(0 == strcmp("?",in))
	    {
	      PATTERNAREAS.resize(0);
	      cout << "Cleared suggested pattern area data.\n";
	      continue;
	    }

	  char in2[256];

	  for(int i = 0; i < 256; i++)
	    {
	      if('x' == in[i] || 'X' == in[i])
		{
		  in[i] = 0; // terminate null
		  strcpy(in2,&in[i+1]);
		  break;
		}
	    }

	  SIZE curr;

	  curr.cx = atoi(in);
	  curr.cy = atoi(in2);

	  if(curr.cx > 0 && curr.cy > 0)
	    {
	      // valid pattern size
	      PATTERNAREAS.resize(PATTERNAREAS.size()+1,curr);
	      cout << "Okay: " << curr.cx << "x" << curr.cy << "\n";
	    }
	  else
	    {
	      cout << "Invalid input, pal.\n";
	    }
	}
      while (true);

      if(PATTERNAREAS.size() > 0)
	{
	  do
	    {
	      cout << "Enter minimum pattern area: ";
	      source->getline(in,256);
	      MINIMUMPATTERNAREA = atoi(in);
	    }
	  while(MINIMUMPATTERNAREA<1);
	}

      char yesslashno;

      TRANS = 0; // black is transparent
      HBITMAP bmp = (HBITMAP)LoadImage(NULL,sn,IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_CREATEDIBSECTION);

      if(NULL == bmp)
	{
	  // we have failed
	  cout << "LoadImage failed for file '" << sn << "':\n";

	  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,GetLastError(),0,sn,256,NULL);
	  cout << sn << "\n";

	  continue;
	}

      BITMAP bmp_info;
      GetObject((HGDIOBJ)bmp,sizeof(BITMAP),(void *)&bmp_info);

      SIZE size;
      DWORD area;

      size.cx = bmp_info.bmWidth;
      size.cy = bmp_info.bmHeight;

      if (size.cx > 256 || size.cy > 256)
        {
          cout << "Bitmap is too large; dimensions must be <= 256." << endl;

          // now that we have stored the bitmap data in the data ptr,
          //  we can get rid of the bmp object
          DeleteObject(bmp);
          
          continue;
        }

      area = size.cx * size.cy;

      cout << "Successfully loaded bitmap of dimensions:\n";
      cout << size.cx << " pixels wide by " << size.cy << " pixels high with an area of " << area << " pixels.\n";

      BYTE *data = new BYTE[area];

      HDC source = CreateCompatibleDC(NULL);
      bmp = (HBITMAP)SelectObject(source,(HGDIOBJ)bmp);

      cout << "Translating bitmap data into 8-bit palette entries...\n";

      BYTE *pos = data, *after_last = data + area;

      int curr_x = 0, curr_y = 0;

      for(; pos < after_last; pos++)
	{
	  COLORREF color = GetPixel(source,curr_x,curr_y);
          int r = GetRValue(color);
          int g = GetGValue(color);
          int b = GetBValue(color);
          int bestMatch = -1, closeness = 5000;

          for (int i = 0; i < palette.size() && closeness; i++)
            {
              RGBTRIPLE& t(palette[i]);
              int thisCloseness
                = abs(r - (int)t.rgbtRed)
                + abs(g - (int)t.rgbtGreen)
                + abs(b - (int)t.rgbtBlue);

              if (thisCloseness < closeness)
                {
                  bestMatch = i;
                  closeness = thisCloseness;
                }
            }

          *pos = (BYTE)bestMatch;

	  if(++curr_x >= size.cx)
	    {
	      curr_x = 0;
	      curr_y++;
	    }
	}

      cout << "\n";

      // now that we have stored the bitmap data in the data ptr,
      //  we can get rid of the bmp object
      DeleteObject(SelectObject(source,(HGDIOBJ)bmp));
      DeleteDC(source);

      vector<BYTE> blocks;
      vector<vector<RECT> > block_areas;
      vector<vector<vector<BYTE> > > patterns;
      vector<vector<RECT > > pattern_coors;

      // look for color blocks
      cout << "Searching for color blocks . . . \n";

      POINT loc;

      loc.x = 0;
      loc.y = 0;

      vector<BYTE> checked; // an array of all checked pixel colors
      checked.resize(0);

      while(FindFirstNonTransparentPixel(data,size,loc))
	{
	  BYTE here = data[size.cx*loc.y+loc.x];

	  // look for a block entry for this color in case we have already checked for it
	  int i;
	  for(i = 0; i < checked.size(); i++)
	    {
	      if(here == checked[i])
		{
		  break;
		}
	    }

	  if(i >= checked.size())
	    {
	      int bindex = blocks.size(); // index into blocks to use

	      blocks.resize(bindex+1); // resize both arrays by one element larger
	      block_areas.resize(bindex+1);

	      blocks[bindex] = here;

	      FindRectangles(data,size,loc,block_areas[bindex]);

	      // let's see if we actually found anything!
	      if(0 == block_areas[bindex].size())
		{
		  // we found nothing! so resize back to how we were a second ago
		  blocks.resize(bindex);
		  block_areas.resize(bindex);
		}
	      else
		{
		  cout << "Found " << block_areas[bindex].size()
                       << " block(s) for color " << (int)here << endl;
		}

	      // add the current color to the end of the list
	      checked.push_back(here);
	    }

	  // move to the right by one pixel
	  //  in case the current one is still transparent
	  if(++loc.x >= size.cx)
	    {
	      loc.x=0;
	      if(++loc.y >= size.cy)
		{
		  break; // we've finished
		}
	    }
	}

      // find patterns
      int i;
      for(i = 0; i < PATTERNAREAS.size(); i++)
	{
	  cout << "Checking for patterns of size "
               << PATTERNAREAS[i].cx << "x" << PATTERNAREAS[i].cy
               << "..." << endl;
	  loc.x = 0;
	  loc.y = 0;
			
	  while(FindFirstNonTransparentPixel(data,size,loc))
	    {
              //cout << "A";
	      BYTE base = data[size.cx * loc.y + loc.x];

	      SIZE& pat(PATTERNAREAS[i]);

	      if(loc.x + pat.cx > size.cx)
		{
		  // we can't use this location
		  loc.x = 0;
		  loc.y++;
		  continue;
		}

	      if(loc.y + pat.cy > size.cy)
		{
		  // we can't use this location
		  loc.x = size.cx-1;
		  loc.y = size.cy-1;
		  break;
		}

	      if(!IsOpaque(data,size.cx,loc,pat))
		{
		  if(++loc.x >= size.cx)
		    {
		      loc.x = 0;
		      if(++loc.y >= size.cy)
			{
			  loc.x = size.cx -1;
			  loc.y = size.cy -1;
			  break;
			}
		    }
		  continue;
		}

              //cout << "B";

	      vector<POINT> points;
	      points.resize(2, loc);
	      int ci = points.size()-1; // current index into points

	      // increment x to avoid overlapping with first instance
	      points[ci].x+=pat.cx;
	      if(points[ci].x + pat.cx > size.cx)
		{
		  points[ci].x = 0;
		  points[ci].y++;
		}

	      POINT max;

	      max.x = points[0].x;
	      max.y = points[0].y;
				
	      while(FindFirstPixel(data,size,points[ci],base)) {
                //cout << "C";
		if(points[ci].y + pat.cy > size.cy) {
		  break;
		}

		if(points[ci].x + pat.cx <= size.cx &&
		   SameAndSeparate(data,size.cx,pat,points)) {
		  // account for the pixel incase it has high coor values
		  max.y = points[ci].y;
		  if(points[ci].x > max.x) {
		    max.x = points[ci].x;
		  }
		  // add an extra element to points vector for scratch space
		  ci = points.size();
		  points.resize(ci+1);
		  points[ci] = points[ci-1];
		}

		if(++points[ci].x + pat.cx > size.cx) {
		  points[ci].x = 0;
		  if(++points[ci].y + pat.cy > size.cy) {
		    // we're done finding extra instances
		    break;
		  }
		}
	      }

	      points.resize(ci);

	      ci--;

	      if(points.size() >= 2) {
		// spread this pattern outwards (down and to the right)

		// first spread to the right
		while(++pat.cx + max.x <= size.cx &&
		      SameAndSeparate(data,size.cx,pat,points) &&
		      IsOpaque(data,size.cx,points[0],pat));
		pat.cx--;

		// now spread downward
		while(++pat.cy + max.y <= size.cy &&
		      SameAndSeparate(data,size.cx,pat,points) &&
		      IsOpaque(data,size.cx,points[0],pat));
		pat.cy--;

		if(pat.cx * pat.cy >= MINIMUMPATTERNAREA) {
		  // print percentage complete
		  int percent = (int)((float(loc.y+1)/float(size.cy)) * 100.0f);
		  cout << percent << "%->Found a pattern " << pat.cx << "x" << pat.cy << "x" << points.size() << "!\n";

		  int pi = pattern_coors.size();

		  pattern_coors.resize(pi+1);
		  patterns.resize(pi+1);

		  UniformDimensionRectangleList(points,pat,pattern_coors[pi]);
                  patterns[pi] = MakeCopy(data, size.cx, pattern_coors[pi][0]);

                  // clear out these areas
                  for(int i = 0; i < pattern_coors[pi].size(); i++)
                    {
                      DrawRectangle(data, size.cx, TRANS, pattern_coors[pi][i]);
                    }

		}
	      }

	      if(++loc.x + PATTERNAREAS[i].cx > size.cx) {
		loc.x = 0;
		if(++loc.y + PATTERNAREAS[i].cy > size.cy) {
		  break;
		}
	      }
            }
	}

      //cout << "H";

      // compile left over data piece
      vector<vector<BYTE> > left_over;

      // each element in that vector is a row
      //  each row is a vector of BYTES with a size of at least 1
      //  if the first BYTE is 0, then no pixels are on the line
      //  if the first BYTE is x, where x > 0, then there are x
      //  trans/opaque pairs
      // a trans/opaque pair has a run of transparent
      // pixels, and a run of non-transparent pixels.  For each
      // trans/opaque pair, there is a number which specifies the number
      // of transparent pixels, then a number which specifies the
      // number of opaque pixels - 1, then the opaque pixel data.

      left_over.resize(size.cy);

      BYTE *data_cursor = data;
      for(i = 0; i < left_over.size(); i++) {
	left_over[i].resize(1);

	// at this point, data_cursor should be pointing at
	//  the beginning of the current row

	//  must make a compiled line
	left_over[i][0] = 0; // start at zero pairs
	for(int j = 0; j < size.cx; j++) {
	  // count number of transparent pixels in this pair
	  int transparent_count = 0;
	  while(TRANS == data_cursor[j] && j < size.cx) {
	    j++;
	    transparent_count++;
	  }

	  if(j == size.cx) {
	    // no more pairs for this row
	    break;
	  }

	  // another pair
	  left_over[i][0]++;
	  // add transparent count
	  left_over[i].push_back((BYTE)transparent_count);

	  // now find number of non-transparent pixels
	  int opaque_count = 0;
	  while(TRANS != data_cursor[j] && j < size.cx) {
	    j++;
	    opaque_count++;
	  }
				
	  // add non-transparent count
	  left_over[i].push_back((BYTE)(opaque_count - 1));

	  // get non-transparent pixels
	  for(int k = j - opaque_count; k < j; k++) {
            left_over[i].push_back(data_cursor[k]);
	  }
	}


	// go to next row
	data_cursor += size.cx;
      }

      // shrink down the left_over vector if
      //  some of the last elements are not needed
      while (!left_over.empty() && !left_over[left_over.size() - 1][0]) {
        left_over.pop_back();
      }

      // we have finished translating bitmap data
      delete data;

      // write to file
      cout << "Writing to '" << tn << "' . . . ";

      datafile f(tn);
		
      f.putByte((BYTE)blocks.size());

      int j, k;
      for(i = 0; i < blocks.size(); i++) {
	// print the color
	f.putByte(blocks[i]);

	// how many blocks of it we have
	f.putUsuallyByte((WORD)block_areas[i].size());

	// now print the blocks bottom,left,right,top
	for(j = 0; j < block_areas[i].size(); j++) {
          f.putRect(block_areas[i][j]);
	}
      }

      // now print out stuff about our patterns to the file
      f.putUsuallyByte((WORD)patterns.size());
      for(i = 0; i < patterns.size(); i++) {
	// print the pattern definition
	f.putUsuallyByte((WORD)patterns[i][0].size()); // print width
	f.putUsuallyByte((WORD)patterns[i].size());    // print height
			
	for(j = 0; j < patterns[i].size(); j++) {
	  for(k = 0; k < patterns[i][j].size(); k++) {
	    f.putByte(patterns[i][j][k]);
	  }
	}

	// print how many rects we have of it
	f.putUsuallyByte((WORD)pattern_coors[i].size());

	// print the rects locations
	for(j = 0; j < pattern_coors[i].size(); j++) {
          f.putByte(pattern_coors[i][j].left);
          f.putByte(pattern_coors[i][j].top);
	}
      }

      // now print out stuff about the left over
      // number of rows
      f.putUsuallyByte(left_over.size());
      for(i = 0; i < left_over.size(); i++) {
	for(j = 0; j < left_over[i].size(); j++) {
	  f.putByte(left_over[i][j]);
	}
      }

      cout << "done\n";
    }
}
