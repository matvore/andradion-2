//  program for sectorizing levels in andradion 2

// this program will accept as input a string
//  with three characters in it as a level id
//  then, in the c:\andradion 2\cmpsrc\???
//  directory, the program will look for the
//  file named ???.bmp and sectorize it creating
//  a bunch of different bmp files of size 160x120
//  that make up that file, naming those bmps
//  *x*.bmp, where the stars are the coors of the
//  image in the big picture. then, it created a
//  ???.cmb script to compress all those files, and
//  then a ???.dat file which contains the resource
//  script code to paste into the resource editor
//  those cmps will be created by the will be created by the
//  .cmb file (the ???.cmb file and ???.dat file are
//  put into the same directory as the 160x120 bitmaps
//  and the big .bmp file).

#include <fstream>
#include <windows.h>
#include <iostream>
#include <string>

using std::cout;
using std::cin;
using std::string;
using std::endl;
using std::ofstream;

const int SECTOR_WIDTH = 160;
const int SECTOR_HEIGHT = 100;
const int SECTOR_AREA = 100 * 160;

struct BMPFILE24B
{
	BITMAPFILEHEADER bmp_header;
	BITMAPINFOHEADER info_header;
	RGBTRIPLE data[SECTOR_HEIGHT][SECTOR_WIDTH];
};

static void WriteBmp(const char *fn,const BMPFILE24B& d)
{
	HANDLE f = CreateFile(fn,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

	DWORD written;
	WriteFile(f,(const void *)&d.bmp_header,sizeof(BITMAPFILEHEADER),&written,NULL);
	WriteFile(f,(const void *)&d.info_header,sizeof(BITMAPINFOHEADER),&written,NULL);
	WriteFile(f,(const void *)&d.data[0][0],SECTOR_AREA*sizeof(RGBTRIPLE),&written,NULL);

	CloseHandle(f);
}

int main(int, char**)
{
	char input[4];

	cout << "Enter 3-byte input string (* to quit): ";
	cin.getline(input,4);

	if(0 == strcmp("*",input))
	{
		return 0;
	}

	string path = "C:\\Andradion 2\\CmpSrc\\";
	path += input;
	path += '\\';

	string source = path + input + ".bmp";
	string dat_file = path + input + ".dat";
	string cmb_file = path + input + ".cmb";

	cout << "Source .bmp: " << source << endl;
	cout << "Destin .dat: " << dat_file << endl;
	cout << "Destin .cmb: " << cmb_file << endl;

	HBITMAP loaded = (HBITMAP)LoadImage(NULL,source.c_str(),IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_LOADFROMFILE);
	HDC loaded_dc = CreateCompatibleDC(NULL);
	HGDIOBJ old_bitmap_loaded_dc = SelectObject(loaded_dc,(HGDIOBJ)loaded);

	BITMAP loaded_info;

	GetObject((HGDIOBJ)loaded,sizeof(loaded_info),(void *)&loaded_info);

	cout << "Bitmap info of source: " << endl;
	cout << "width bytes: " << loaded_info.bmWidthBytes << endl;
	cout << "width      : " << loaded_info.bmWidth << endl;
	cout << "height     : " << loaded_info.bmHeight << endl;
	cout << "bits pixel : " << loaded_info.bmBitsPixel << endl;
	cout << "type       : " << loaded_info.bmType << endl;
	cout << "planes     : " << loaded_info.bmPlanes << endl;

	int width = loaded_info.bmWidth / SECTOR_WIDTH;
	int height = loaded_info.bmHeight / SECTOR_HEIGHT;

	if(0 != loaded_info.bmWidth % SECTOR_WIDTH || loaded_info.bmWidth < SECTOR_WIDTH)
	{
		width++;
	}
	
	if(0 != abs(loaded_info.bmHeight) % SECTOR_HEIGHT || loaded_info.bmHeight < SECTOR_HEIGHT)
	{
		height++;
	}

	cout << "Using master width and height of " << width << "x" << height << endl;
	
	BMPFILE24B new_bmp;
	new_bmp.bmp_header.bfType = 0x4d42;
	new_bmp.bmp_header.bfSize = sizeof(BMPFILE24B);
	new_bmp.bmp_header.bfReserved1 = 0;
	new_bmp.bmp_header.bfReserved2 = 0;
	new_bmp.bmp_header.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
						
	new_bmp.info_header.biSize = sizeof(BITMAPINFOHEADER);
	new_bmp.info_header.biWidth = SECTOR_WIDTH;
	new_bmp.info_header.biHeight = SECTOR_HEIGHT;
	new_bmp.info_header.biPlanes = 1;
	new_bmp.info_header.biBitCount = 24;
	new_bmp.info_header.biCompression = BI_RGB;
	new_bmp.info_header.biSizeImage = SECTOR_AREA * sizeof(RGBTRIPLE);
	new_bmp.info_header.biXPelsPerMeter = 100;
	new_bmp.info_header.biYPelsPerMeter = 100;
	new_bmp.info_header.biClrUsed = 0;
	new_bmp.info_header.biClrImportant = 0;

	ofstream cmb;
	ofstream dat;

	cmb.open(cmb_file.c_str());
	dat.open(dat_file.c_str());
	
	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			cout << "Writing bmp: ";

			int base_x = x*SECTOR_WIDTH;
			int base_y = y*SECTOR_HEIGHT;
			for(int sub_y = 0; sub_y < SECTOR_HEIGHT; sub_y++)
			{
				for(int sub_x = 0; sub_x < SECTOR_WIDTH; sub_x++)
				{
					COLORREF esto = GetPixel(loaded_dc,sub_x+base_x,sub_y+base_y);
					if(CLR_INVALID == esto)
					{
						esto = 0;
					}
					RGBTRIPLE *t = &new_bmp.data[SECTOR_HEIGHT-1-sub_y][sub_x];
					t->rgbtRed = GetRValue(esto);
					t->rgbtGreen = GetGValue(esto);
					t->rgbtBlue = GetBValue(esto);
					
					// if anything is real dim, just convert it to solid black
					if(t->rgbtRed <= 4 && t->rgbtGreen <= 4 && t->rgbtBlue <= 4)
					{
						memset((void *)t,0,sizeof(RGBTRIPLE));
					}
				}
			}
			string fn = path;
			string partial;
			char numero[10];
			partial += itoa(x,numero,10);
			partial += 'x';
			partial += itoa(y,numero,10);
			fn += partial;
			string bmp_name = fn + ".bmp";
			cout << bmp_name << endl;
			WriteBmp(bmp_name.c_str(),new_bmp);

			cmb << "c:\\Andradion 2\\CmpSrc\\" << input << "\\" << partial << ".bmp" << endl;
			cmb << "c:\\Andradion 2\\Resource\\" << input << partial << ".cmp" << endl;
			cmb << 16 << endl;
			cmb << "4x4" << endl;
			cmb << "*" << endl;
			cmb << 16 << endl;
			cmb << "y" << endl;
			dat << input << partial << " CMP DISCARDABLE \"Resource\\\\" << input << partial << ".cmp\"" << endl;
		}
	}

	cmb << "*q" << endl;

	cmb.close();
	dat.close();

	// delete everything we were just doing stuff with
	SelectObject(loaded_dc,old_bitmap_loaded_dc);
	DeleteObject((HGDIOBJ)loaded);
	DeleteDC(loaded_dc);
	return main(0,NULL);
}
