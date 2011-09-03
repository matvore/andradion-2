#pragma warning (disable : 4786)

// a simple program that makes .cmp files
#include <cassert>
#include <windows.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <fstream>

using namespace std;

static int          MINIMUMPATTERNAREA;
static int          MINIMUMBLOCKAREA;
static vector<SIZE> PATTERNAREAS;
static DWORD        TRANS; // transparent color; TRANSPARENT is a constant defined by windows.h, so we don't use it!

class datafile
{
public:
	datafile()
	{
		this->file = NULL;
	}

	~datafile()
	{
		if(NULL != this->file)
		{
			CloseHandle(this->file);
		}
	}

	void open(const char *fn)
	{
		this->file = CreateFile(fn,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	}

	DWORD put(DWORD x,bool as_color = false)
	{
		WORD data; // everything is written word's

		if(true == as_color)
		{
			// color is stored in 5/6/5 format
			WORD b = (WORD)GetBValue(x);
			b >>= 3;
			WORD g = (WORD)GetGValue(x);
			g >>= 2;
			g <<= 5;
			WORD r = (WORD)GetRValue(x);
			r >>= 3;
			r <<= 11;
			data = r | g | b;
		}
		else
		{
			data = (WORD)x;
		}

		DWORD written;

		WriteFile(this->file,(const void *)&data,sizeof(data),&written,NULL);

		return written;
	}

	void close()
	{
		CloseHandle(this->file);
		this->file = NULL;
	}

private:
	HANDLE file;
};

// functions for manipulating loaded data . . .

// makes a redundant vector of rectangles
static void MakeRectangleArray(const vector<POINT>& p,const SIZE& s,vector<RECT>& t)
{
	t.resize(p.size());

	for(int i = 0; i < t.size(); i++)
	{
		t[i].left= p[i].x;
		t[i].right = p[i].x + s.cx;
		t[i].top = p[i].y;
		t[i].bottom = p[i].y + s.cy;
	}
}


// puts a black rectangle somewhere
static void BlackRectangle(DWORD *data,int data_width,const RECT& loc)
{
	// point data to the first pixel to set to black
	data += (loc.top * data_width + loc.left);

	// loop through each row
	for(int y = loc.top; y < loc.bottom; y++)
	{
		int width = loc.right - loc.left;
		for(int x = 0; x < width; x++)
		{
			data[x] = TRANS;
		}

		// go further into data buffer
		data += data_width;
	}
}

// transfers data from *data to pattern, replacing the area in *data with the transparent color
static void PatternRectangle(DWORD *data,int data_width,const vector<RECT>& area,vector< vector<COLORREF> >& pattern)
{
	DWORD *data_backup = data; // store a backup pointer to *data

	// point data to the first pixel to copy
	data += (area[0].top * data_width + area[0].left);

	int to_change_rows = (data_width - (area[0].right - area[0].left));

	int w = area[0].right - area[0].left;
	int h = area[0].bottom - area[0].top;

	pattern.resize(h);

	for(int y =0 ; y < h; y++)
	{
		pattern[y].resize(w);
		for(int x = 0; x < w; x++)
		{
			pattern[y][x] = (COLORREF)(*data);
			data++;
		}
		data+=to_change_rows;
	}

	// clear out these areas
	for(int i = 0; i < area.size(); i++)
	{
		BlackRectangle(data_backup,data_width,area[i]);
	}
}

static bool AllOneColor(const DWORD *data,int data_width,const RECT& area,DWORD color)
{
	// returns true if every pixel within area is equal to color

	const DWORD *pos;

	// set data to the first pixel we are observing
	data += area.top * data_width + area.left;

	for(int y = area.top; y < area.bottom; y++)
	{
		pos = data;

		for(int x = area.left; x < area.right; x++)
		{
			if((const DWORD)color != (const DWORD)(*pos))
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
static void FindRectangles(DWORD *data,const SIZE& data_size,const POINT& start_coor,vector<RECT>& rects)
{
	int curr_x = start_coor.x; // current coordinates
	int curr_y = start_coor.y;

	// clear out rects
	rects.resize(0);

	DWORD *data_position = &data[data_size.cx*start_coor.y+start_coor.x];
	DWORD base = *data_position;
	DWORD *after_last = &data[data_size.cx*data_size.cy]; // the address of the pixel which is right after the one we check last

	for(;data_position<after_last;data_position++)
	{
		if((const DWORD)base == (const DWORD)*data_position)
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
			while(((const int)x.right <= (int)data_size.cx) && true == AllOneColor(data,data_size.cx,x,base));

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
			while(((const int)x.bottom <= (int)data_size.cy) && true == AllOneColor(data,data_size.cx,x,base));

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
				BlackRectangle(data,data_size.cx,x);
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
	static bool SameData(const DWORD *data,int data_width,const SIZE& size,const pair<POINT,POINT>& coors)
	{
		// find what to check in memory by making shortcuts
		const DWORD *d1 = &data[coors.first.y * data_width + coors.first.x];
		const DWORD *d2 = &data[coors.second.y * data_width + coors.second.x];
		int changing_rows = // bytes to move to change rows
			(data_width-size.cx);
		for(int y = 0; y < size.cy; y++)
		{
			for(int x = 0; x < size.cx; x++)
			{
				if(*d1 != *d2)
				{
					return false; // conflicting pixels
				}

				d1++;
				d2++;
			}

			d1+=changing_rows;
			d2+=changing_rows;
		}

		return true; // all pixels are the same, and there is no overlap
	}

	// compares a vector of areas
	static bool SameAndSeparate(const DWORD *data,int data_width,const SIZE& size,const vector<POINT>& coors)
	{
		// compare similar data
		pair<POINT,POINT> x;
		x.first = coors[0];
		for(int i = coors.size()-1; i >= 1; i--)
		{			
			x.second = coors[i];
			if(false == SameData(data,data_width,size,x))
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
				if(!(
					(a.left >= coors[c2].x + size.cx) ||
					(a.right <= coors[c2].x) ||
					(a.top >= coors[c2].y + size.cy) ||
					(a.bottom <= coors[c2].y)
				))
				{
					return false;
				}
			}
		}

		return true;
	}

	static bool FindFirstPixel(const DWORD *data,const SIZE& data_size,POINT& coor,DWORD base)
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
	static bool FindFirstNonTransparentPixel(const DWORD *data,const SIZE& data_size,POINT& coor,const vector<DWORD>& ignore = vector<DWORD>())
	{
		data += data_size.cx*coor.y+coor.x;
		for(;coor.y < data_size.cy;data++)
		{
			if(TRANS != *data)
			{
				int i;

				for(i = 0; i < ignore.size(); i++)
				{
					if(ignore[i] == *data)
					{
						break;
					}
				}

				if(ignore.size() == i)
				{
					return true; // we found something
				}
			}

			// increment current position
			if(++coor.x >= data_size.cx)
			{
				coor.x=0;
				coor.y++;
			}
		}

		// move this coor struct to the lower right corner
		coor.x = data_size.cx-1;
		coor.y = data_size.cy-1;

		return false; // we got all the way here and couldn't find a pixel that wasn't transparent
	}

// returns true if there are only non-transparent pixels
static bool SolidWallOfMeat(const DWORD *data,int data_width,const POINT& point,const SIZE& size)
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

	// returns how many non-transparent pixels there are in a given area
	static int NumNonTransparentPixels(const DWORD *data,int data_width,const RECT& loc)
	{
		// calculate number of bytes to increment data pointer
		//  to move to the next row
		int to_change_rows = (data_width - (loc.right-loc.left));

		// point data to first pixel to check
		data += data_width * loc.top + loc.left;

		int found = 0;

		for(int y = loc.top; y < loc.bottom; y++)
		{
			for(int x = loc.left; x < loc.right; x++)
			{
				if(TRANS != *data)
				{
					found++;
				}
				data++;
			}
			data+=to_change_rows; // move to next row
		}

		return found;
	}

void main(int argc,char **argv)
{
	cout << "Official CCompactMap maker for GameLib2\n\n";

	istream *source;
	ifstream batch;

	if(argc > 1)
	{
		batch.open(argv[1]);
		if(0 == batch.fail())
		{
			source = (istream *)&batch;
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
		char in[256];

		cout << "Enter name of .bmp file to read (type *q to quit): ";
		source->getline(sn,256);

		if(0 == strcmp(sn,"*q") || 0 == strcmp(sn,"*Q"))
		{
			break;
		}

		cout << "Enter name of compressed file to write results to: ";
		source->getline(tn,256);

		do
		{
			cout << "Enter minimum color block area: ";
			source->getline(in,256);
			MINIMUMBLOCKAREA = atoi(in);
		}
		while (MINIMUMBLOCKAREA<1);

		cout << 
"\
Now you will be requested to enter the suggested pattern area.  The compacter  \n\
will search the bitmap for patterns of those sizes in the order entered.  When \n\
a pattern of a suggested size is found,the compacter will see if the pattern   \n\
area can be spread for increased optimization and efficiency.                  \n\
\n";

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

		do
		{
			cout << "Use black for transparent color (Y/N)? ";
			source->getline(in,256);

			yesslashno = in[0];
			
			if(yesslashno >= 'a' && yesslashno <= 'z')
			{
				yesslashno += ('A' - 'a');
			}
		}
		while (yesslashno != 'N' && yesslashno != 'Y');

		if('Y' == yesslashno)
		{
			TRANS = 0; // black is transparent
		}
		else
		{
			unsigned char rgb[3];
			const char RGBNAME[3] = {'R','G','B'};

			for(int i = 0; i < 3; i++)
			{
				int x = -1;

				do
				{
					cout << "Enter " << RGBNAME[i] << " element of transparent color (0-255): ";
					source->getline(in,256);
					x = atoi(in);

					if(0 == x && '0' != in[0])
					{
						x = -1;
					}
				}
				while(x < 0 || x > 255);

				rgb[i] = (unsigned char)x;
			}
			
			TRANS = (DWORD)RGB(rgb[0],rgb[1],rgb[2]);
		}

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

		area = size.cx * size.cy;

		cout << "Successfully loaded bitmap of dimensions:\n";
		cout << size.cx << " pixels wide by " << size.cy << " pixels high with an area of " << area << " pixels.\n";

		COLORREF *data = new COLORREF[area];

		HDC source = CreateCompatibleDC(NULL);
		bmp = (HBITMAP)SelectObject(source,(HGDIOBJ)bmp);

		cout << "Translating bitmap data into 32-bit COLORREF's . . . \n";

		int last_printed_percent = 0;

		cout << "0%->";

		COLORREF *pos = data;
		COLORREF *after_last = &data[area];

		int curr_x = 0;
		int curr_y = 0;

		for(; pos < after_last; pos++)
		{
			*pos = GetPixel(source,curr_x,curr_y);

			if(++curr_x >= size.cx)
			{
				curr_x = 0;
				curr_y++;
				
				int this_percent = int((((float)curr_y+1.0f)/(float)size.cy) * 100.0f);

				if(this_percent - 10 >= last_printed_percent)
				{
					cout << this_percent << "%->";
					last_printed_percent = this_percent;
				}
			}
		}

		cout << "\n";

		// now that we have stored the bitmap data in the data ptr, we can get rid of the bmp object
		DeleteObject(SelectObject(source,(HGDIOBJ)bmp));
		DeleteDC(source);

		vector<COLORREF> blocks;
		vector<vector<RECT> > block_areas;
		vector<vector<vector<COLORREF> > > patterns;
		vector<vector<RECT > > pattern_coors;

		blocks.resize(0);
		block_areas.resize(0);
		patterns.resize(0);
		pattern_coors.resize(0);

		// look for color blocks
		cout << "Searching for color blocks . . . \n";

		POINT loc;

		loc.x = 0;
		loc.y = 0;

		vector<COLORREF> checked; // an array of all checked pixel colors
		checked.resize(0);

		while(true == FindFirstNonTransparentPixel((DWORD *)data,size,loc))
		{
			COLORREF here = data[size.cx*loc.y+loc.x];

			// look for a block entry for this color in case we have already checked for it
			int i;
			for(i = 0; i < checked.size(); i++)
			{
				if((const DWORD)here == (const DWORD)checked[i])
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

				FindRectangles((DWORD *)data,size,loc,block_areas[bindex]);

				int percent = (int)((float(loc.y+1)/float(size.cy)) * 100.0f);

				// let's see if we actually found anything!
				if(0 == block_areas[bindex].size())
				{
					// we found nothing! so resize back to how we were a second ago
					blocks.resize(bindex);
					block_areas.resize(bindex);
				}
				else
				{
					cout << percent << "%->Found " << block_areas[bindex].size() << " block(s) for color " << here << "\n";
				}

				// add the current color to the end of the list
				checked.resize(checked.size()+1,here);
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
			cout << "Checking for patterns of size " << PATTERNAREAS[i].cx << "x" << PATTERNAREAS[i].cy << " . . . \n";
			loc.x = 0;
			loc.y = 0;
			
			while(true == FindFirstNonTransparentPixel(data,size,loc))
			{
				DWORD base = data[size.cx * loc.y + loc.x];

				SIZE pat = PATTERNAREAS[i];

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

				if(false == SolidWallOfMeat(data,size.cx,loc,pat))
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

				vector<POINT> points;
				points.resize(0);
				points.resize(2,loc);
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
				
				while(true == FindFirstPixel(data,size,points[ci],base))
				{
					if(points[ci].y + pat.cy > size.cy)
					{
						break;
					}

					if(
						points[ci].x + pat.cx <= size.cx &&
						true == SameAndSeparate(data,size.cx,pat,points))
					{
						// account for the pixel incase it has high coor values
						max.y = points[ci].y;
						if(points[ci].x > max.x)
						{
							max.x = points[ci].x;
						}
						// add an extra element to points vector for scratch space
						ci = points.size();
						points.resize(ci+1);
						points[ci] = points[ci-1];
					}

					if(++points[ci].x + pat.cx > size.cx)
					{
						points[ci].x = 0;
						if(++points[ci].y + pat.cy > size.cy)
						{
							// we're done finding extra instances
							break;
						}
					}
				}

				points.resize(ci);

				ci--;

				if(points.size() >= 2)
				{
					// spread this pattern outwards (down and to the right)

					// first spread to the right
					while
					(
						++pat.cx + max.x <= size.cx &&
						true == SameAndSeparate(data,size.cx,pat,points) &&
						true == SolidWallOfMeat(data,size.cx,points[0],pat)
					);
					pat.cx--;

					// now spread downward
					while
					(
						++pat.cy + max.y <= size.cy &&
						true == SameAndSeparate(data,size.cx,pat,points) &&
						true == SolidWallOfMeat(data,size.cx,points[0],pat)
					);
					pat.cy--;

					if(pat.cx * pat.cy >= MINIMUMPATTERNAREA)
					{
						// print percentage complete
						int percent = (int)((float(loc.y+1)/float(size.cy)) * 100.0f);
						cout << percent << "%->Found a pattern " << pat.cx << "x" << pat.cy << "x" << points.size() << "!\n";

						int pi = pattern_coors.size();

						pattern_coors.resize(pi+1);
						patterns.resize(pi+1);

						MakeRectangleArray(points,pat,pattern_coors[pi]);
						PatternRectangle(data,size.cx,pattern_coors[pi],patterns[pi]);
					}
				}

				if(++loc.x + PATTERNAREAS[i].cx > size.cx)
				{
					loc.x = 0;
					if(++loc.y + PATTERNAREAS[i].cy > size.cy)
					{
						break;
					}
				}
			}
		}

		// compile left over data piece
		vector<vector<WORD> > left_over;

		// each element in that vector is a row
		//  each row is a vector of WORDS with a size of at least 1
		//  if the first WORD is 0, then no pixels are on the line
		//  if the first WORD is x, where x > 0, then there are x trans/meat pairs
		//  a trans/meat pair has a run of transparent pixels, and a run of
		//  non-transparent pixels.  For each transmeat pair, there is a number
		//  which specifies the number of transparent pixels, then a number which
		//  specifies the number of non-transparent pixels, then the non-transparent
		//  "meat" part of it.

		// size left_over vector to the right size so it will never be
		//  resize again
		left_over.resize(size.cy);

		DWORD *data_cursor = data;
		for(i = 0; i < left_over.size(); i++)
		{
			left_over[i].resize(1);

			// at this point, data_cursor should be pointing at
			//  the beginning of the current row

			//  must make a compiled line
			left_over[i][0] = 0; // start at zero pairs
			for(int j = 0; j < size.cx; j++)
			{
				// count number of transparent pixels in this pair
				int transparent_count = 0;
				while(TRANS == data_cursor[j] && j < size.cx)
				{
					j++;
					transparent_count++;
				}

				if((const DWORD)j == size.cx)
				{
					// we got to the end, no more of these pairs for this row!
					break;
				}

				// another pair
				left_over[i][0]++;
				// add transparent count
				left_over[i].resize(left_over[i].size()+1,(WORD)transparent_count);

				// now find number of non-transparent pixels
				int non_transparent_count = 0;
				while(TRANS != data_cursor[j] && j < size.cx)
				{
					j++;
					non_transparent_count++;
				}
				
				// add non-transparent count
				left_over[i].resize(left_over[i].size()+1,(WORD)non_transparent_count);

				// catch what j should be when at the end of the non-transparent
				//  pixel run
				int j_limit = j;

				// move j back to normal
				j -= non_transparent_count;

				// get non-transparent pixels
				for(; j < j_limit; j++)
				{
					DWORD _4b_color = data_cursor[j];
					WORD _2b_color = 0;
					_2b_color|= GetBValue(_4b_color) >> 3;
					_2b_color|= (GetGValue(_4b_color) >> 2) << 5;
					_2b_color|= (GetRValue(_4b_color) >> 3) << 11;
					left_over[i].resize(left_over[i].size()+1,_2b_color);
				}
			}

			// go to next row
			data_cursor += size.cx;
		}

		// shrink down the left_over vector if
		//  some of the last elements are not needed
		for(i = left_over.size()-1; i >= 0; i--)
		{
			if(0 != left_over[i][0])
			{
				break;
			}
		}
		left_over.resize(i+1);

		// now that we have translated everything, we can delete data from memory
		delete data;

		// write to file
		cout << "Writing to '" << tn << "' . . . ";

		datafile f;
		f.open(tn);
		
		f.put((DWORD)blocks.size());

		int j;
		int k;
		for(i = 0; i < blocks.size(); i++)
		{
			// print the color
			f.put((DWORD)blocks[i],true);

			// how many blocks of it we have
			f.put((DWORD)block_areas[i].size());

			// now print the blocks bottom,left,right,top
			for(j = 0; j < block_areas[i].size(); j++)
			{
				f.put((DWORD)block_areas[i][j].bottom);
				f.put((DWORD)block_areas[i][j].left);
				f.put((DWORD)block_areas[i][j].right);
				f.put((DWORD)block_areas[i][j].top);
			}
		}

		// now print out stuff about our patterns to the file
		f.put((DWORD)patterns.size());
		for(i = 0; i < patterns.size(); i++)
		{
			// print the pattern definition
			f.put((DWORD)patterns[i][0].size()); // print width
			f.put((DWORD)patterns[i].size());    // print height
			
			for(j = 0; j < patterns[i].size(); j++)
			{
				for(k = 0; k < patterns[i][j].size(); k++)
				{
					f.put((DWORD)patterns[i][j][k],true);
				}
			}

			// print how many rects we have of it
			f.put((DWORD)pattern_coors[i].size());

			// print the rects bottom,left,right,top
			for(j = 0; j < pattern_coors[i].size(); j++)
			{
				f.put((DWORD)pattern_coors[i][j].bottom);
				f.put((DWORD)pattern_coors[i][j].left);
				f.put((DWORD)pattern_coors[i][j].right);
				f.put((DWORD)pattern_coors[i][j].top);
			}
		}

		// now print out stuff about the left over
		// number of rows
		f.put(left_over.size());
		for(i = 0; i < left_over.size(); i++)
		{
			for(j = 0; j < left_over[i].size(); j++)
			{
				f.put(left_over[i][j]);
			}
		}

		f.close();

		cout << "done\n";
	}
	

	cout << "Goodbye\n";

	batch.close();
}
