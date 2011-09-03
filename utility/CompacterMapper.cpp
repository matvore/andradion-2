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

int main(int argc, char **argv)
{
  {
      // find patterns
      int i;
      for(i = 0; i < PATTERNAREAS.size(); i++)
	{
	  cout << "Checking for patterns of size "
               << PATTERNAREAS[i].cx << "x" << PATTERNAREAS[i].cy
               << "..." << endl;
	  loc.x = 0;
	  loc.y = 0;
			
	  while(FindPixel(ColorThat.IS_NOT, TRANS, data,size,loc))
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

	      if(!AreaIs(ColorThat.IS_NOT, TRANS, data,size.cx,loc,pat))
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
				
	      while(FindPixel(ColorThat.IS, base, data,size,points[ci])) {
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
		      AreaIs(ColorThat.IS_NOT, TRANS, data,size.cx,points[0],pat));
		pat.cx--;

		// now spread downward
		while(++pat.cy + max.y <= size.cy &&
		      SameAndSeparate(data,size.cx,pat,points) &&
		      AreaIs(ColorThat.IS_NOT, TRANS, data,size.cx,points[0],pat));
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
