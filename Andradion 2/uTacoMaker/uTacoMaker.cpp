#include <windows.h>
#include <iostream>
#include <algorithm>

using std::cout;
using std::endl;
using std::swap;

const int BMPW = 1392;
const int BMPH = 1494;
const int BMPA = BMPW * BMPH;
const int TREEH = 90;
const int TREEW = 89;

struct BMPFILE24B
{
	BITMAPFILEHEADER bmp_header;
	BITMAPINFOHEADER info_header;
	RGBTRIPLE *data;
};

static void WriteBmp(const char *fn,const BMPFILE24B& d)
{
	HANDLE f = CreateFile(fn,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

	DWORD written;
	WriteFile(f,(const void *)&d.bmp_header,sizeof(BITMAPFILEHEADER),&written,NULL);
	WriteFile(f,(const void *)&d.info_header,sizeof(BITMAPINFOHEADER),&written,NULL);
	WriteFile(f,(const void *)d.data,BMPA*sizeof(RGBTRIPLE),&written,NULL);

	CloseHandle(f);
}

int TREELOC[] =
{
	612,638,
	660,643,
	714,646,
	764,642,
	814,646,
	865,639,
	914,647,
	960,644,
	1008,642,
	1056,647,
	1110,650,
	1160,646,
	1210,650,
	1261,643,
	1310,651,
	1362,651,
	463,538,
	461,645,
	107,1190,
	628,1287,
	569,549,
	569,611,
	459,746,
	445,837,
	425,905,
	375,938,
	325,957,
	274,961,
	221,954,
	164,960,
	64,1135,
	627,1226,
	629,1159,
	453,971,
	511,1022,
	545,1081,
	597,1102,
	166,1197,
	333,1079,
	454,1172,
	231,1201,
	23,1082,
	367,1366,
	279,1226,
	337,1234,
	359,1295,
	372,1439,
	682,1292
};

const int NUMLOC = 48;

// copy with transparency
void memcp(void *d_,const void *s_,unsigned long l_)
{
	RGBTRIPLE *d = (RGBTRIPLE *)d_;
	const RGBTRIPLE *s = (const RGBTRIPLE *)s_;
	unsigned long l = l_ / sizeof(RGBTRIPLE);

	while(l-- > 0)
	{
		if(0 != s->rgbtRed || 0 != s->rgbtGreen || 0 != s->rgbtBlue)
		{
			*d = *s;
		}
		s++;
		d++;
	}
}

void main()
{
	cout << "Getting pillar data . . ." << endl;

	// get pillar data
	HBITMAP pillar = (HBITMAP)LoadImage(NULL,"C:\\Mis Documentos\\tree.bmp",IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_LOADFROMFILE);
	HDC pillar_dc = CreateCompatibleDC(NULL);
	HGDIOBJ old_bitmap_pillar_dc = SelectObject(pillar_dc,(HGDIOBJ)pillar);

	RGBTRIPLE upillar[TREEH][TREEW];
	for(int y = 0; y < TREEH; y++)
	{
		for(int x = 0; x < TREEW; x++)
		{
			DWORD pix = GetPixel(pillar_dc,x,y);
			upillar[y][x].rgbtRed = GetRValue(pix);
			upillar[y][x].rgbtGreen = GetGValue(pix);
			upillar[y][x].rgbtBlue = GetBValue(pix);
		}
	}

	SelectObject(pillar_dc,old_bitmap_pillar_dc);
	DeleteDC(pillar_dc);
	DeleteObject((HGDIOBJ)pillar);
	
	cout << "Loading source (7b_) bitmap . . . " << endl;

	HBITMAP loaded = (HBITMAP)LoadImage(NULL,"C:\\Andradion 2\\CmpSrc\\7b_\\7b_.bmp",IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_LOADFROMFILE);
	HDC loaded_dc = CreateCompatibleDC(NULL);
	HGDIOBJ old_bitmap_loaded_dc = SelectObject(loaded_dc,(HGDIOBJ)loaded);

	BMPFILE24B u; // upper level bitmap
	u.data = new RGBTRIPLE[BMPA];
	u.bmp_header.bfType = 0x4d42;
	u.bmp_header.bfSize = sizeof(BMPFILE24B);
	u.bmp_header.bfReserved1 = 0;
	u.bmp_header.bfReserved2 = 0;
	u.bmp_header.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
	u.info_header.biSize = sizeof(BITMAPINFOHEADER);
	u.info_header.biWidth = BMPW;
	u.info_header.biHeight = BMPH;
	u.info_header.biPlanes = 1;
	u.info_header.biBitCount = 24;
	u.info_header.biCompression = BI_RGB;
	u.info_header.biSizeImage = BMPA* sizeof(RGBTRIPLE);
	u.info_header.biXPelsPerMeter = 100;
	u.info_header.biYPelsPerMeter = 100;
	u.info_header.biClrUsed = 0;
	u.info_header.biClrImportant = 0;

	// clear out new bitmap data
	memset((void *)u.data,0,sizeof(RGBTRIPLE)*BMPA);

	cout << "Applying pillars . . . " << endl;
	int num_pillars = 0;

	// sort TREELOC array
	int i;
	for(i = NUMLOC - 1; i >= 1; i--)
	{
		for(int bottom_element = 1; bottom_element <= i; bottom_element++)
		{
			if(TREELOC[2 * bottom_element + 1] < TREELOC[2 * bottom_element - 1])
			{
				swap(TREELOC[2 * bottom_element],TREELOC[2 * (bottom_element - 1)]);
				swap(TREELOC[2 * bottom_element + 1],TREELOC[2 * bottom_element - 1]);
			}
		}
	}

	// apply the pillars
	for(i = 0; i < NUMLOC; i++)
	{
		num_pillars++;
		for(int upillary = 0; upillary < TREEH; upillary++)
		{
			int destx = TREELOC[i * 2]-26;
			int desty = (TREELOC[i * 2 + 1]-89+upillary) * BMPW;
			memcp(
				(void *)&u.data[destx+desty],
				(const void *)upillar[upillary],
				sizeof(RGBTRIPLE)*TREEW
			);
		}
	}

	cout << "Applied " << num_pillars << " pillars.\n";

	cout << "Writing 6au file . . . " << endl;

	WriteBmp("C:\\Andradion 2\\CmpSrc\\7bu\\7bu.bmp",u);
	
	delete u.data;

	SelectObject(loaded_dc,old_bitmap_loaded_dc);
	DeleteDC(loaded_dc);
	DeleteObject((HGDIOBJ)loaded);

	cout << "Done" << endl;
}
