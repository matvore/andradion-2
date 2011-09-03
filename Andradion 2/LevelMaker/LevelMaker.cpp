// LevelMaker.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"

using std::vector;
using std::string;
using std::ifstream;
using std::ofstream;
using std::endl;

const int CURSOR_WIDTH = 32;
const int CURSOR_HEIGHT = 30;
const int NUM_CURSORS = 8;
const int CHARACTERS[NUM_CURSORS] =
{
	IDB_TURNER,
	IDB_SALLY,
	IDB_MILTON,
	IDB_EVILTURNER,
	IDB_PISTOL,
	IDB_MACHINEGUN,
	IDB_BAZOOKA,
	IDB_HEALTH
};

const int CURSORS[NUM_CURSORS] =
{
	IDC_TURNER,
	IDC_SALLY,
	IDC_MILTON,
	IDC_EVILTURNER,
	IDC_PISTOL,
	IDC_MACHINEGUN,
	IDC_BAZOOKA,
	IDC_HEALTH
};

const int NUM_LEVELS = 11;
const char LEVEL_NAMES[][NUM_LEVELS] =
{
	"1_",
	"2a",
	"2b",
	"3a",
	"3b",
	"4_",
	"5_",
	"6a",
	"7a",
	"6b",
	"7b"
};

// bitmaps to each cursor
static HDC characters[NUM_CURSORS];
static HCURSOR cursors[NUM_CURSORS];

// current level
static int current_level;
// data that appears before the enemies and powerups in the .dat file
static vector<int> pre_data;
static vector<int> pre_data_b;
// points we have
static vector<POINT> locations[NUM_CURSORS];
static int current_cursor;
// coordinates of the level in the upper-left corner of the window
//  (may be out of range of the level itself)
static POINT level_orientation;
// stores a bitmap of what is behind the cursor
static HDC window;
static HWND hWnd;
static HINSTANCE hInstance;

static HGDIOBJ old_character_bmps[NUM_CURSORS];

static HDC level = NULL;
static HGDIOBJ old_level_bitmap;

static HGDIOBJ old_window_pen;
static HGDIOBJ old_window_brush;

static void GetLevelDataPath(string& lvl_name)
{
	lvl_name = "C:\\Andradion 2\\DatSrc\\";
	lvl_name += LEVEL_NAMES[current_level];
	lvl_name += ".dat";
}

static void RepaintAll()
{
	// blit blackness
	BitBlt(window,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN),NULL,0,0,BLACKNESS);
	// blit the current level bitmap
	POINT c ={level_orientation.x,level_orientation.y};
	BitBlt(window,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN),level,c.x,c.y,SRCCOPY);

	// do the perspective-correction rectangle so we know how big a screen is
	int rx = (GetSystemMetrics(SM_CXSCREEN)-320)/2;
	int ry = (GetSystemMetrics(SM_CYSCREEN)-240)/2;
	Rectangle(window,rx,ry,rx+320,ry+240);

	// blit the characters
	c.x += CURSOR_WIDTH/2;
	c.y += CURSOR_HEIGHT/2;
	for(int i = 0; i < NUM_CURSORS; i++)
	{
		for(int j = 0; j < locations[i].size(); j++)
		{
			BitBlt(window,locations[i][j].x-c.x,locations[i][j].y-c.y,CURSOR_WIDTH,CURSOR_HEIGHT,characters[i],0,0,SRCCOPY);
		}
	}
}

static void LoadCursors()
{
	window = GetDC(hWnd);
	old_window_pen = SelectObject(window,GetStockObject(WHITE_PEN));
	old_window_brush = SelectObject(window,GetStockObject(HOLLOW_BRUSH));
	for(int i = 0; i < NUM_CURSORS; i++)
	{
		characters[i] = CreateCompatibleDC(window);
		old_character_bmps[i] = SelectObject(characters[i],(HGDIOBJ)LoadBitmap(hInstance,MAKEINTRESOURCE(CHARACTERS[i])));
		cursors[i] = LoadCursor(hInstance,MAKEINTRESOURCE(CURSORS[i]));
	}
}

static void UnloadCursors()
{
	SelectObject(window,old_window_pen);
	SelectObject(window,old_window_brush);
	ReleaseDC(hWnd,window);
	for(int i = 0 ; i < NUM_CURSORS; i++)
	{
		DeleteObject(SelectObject(characters[i],old_character_bmps[i]));
		DeleteDC(characters[i]);
		DestroyCursor(cursors[i]);
	}
}

static void SaveCurrentLevel()
{
	// save the current level, and then release the dc
	// store predata
	string lvl_name;
	GetLevelDataPath(lvl_name);
	ofstream target;
	target.open(lvl_name.c_str());
	int i;
	for(i = 0 ; i < pre_data.size(); i++)
	{
		target << pre_data[i] << ' ';
	}
	target << endl;
	target << locations[0][0].x << ' ' << locations[0][0].y;
	target << endl;
	for(i = 0 ; i <pre_data_b.size(); i++)
	{
		target << pre_data_b[i] << ' ';
	}
	target << endl;
	for(i = 1; i < 8; i++)
	{
		target << locations[i].size() << endl;
		for(int j = 0; j < locations[i].size(); j++)
		{
			target << locations[i][j].x << ' ' << locations[i][j].y << endl;
		}
		target << endl;
	}

	target.close();

	if(NULL != level)
	{
		DeleteObject(SelectObject(level,old_level_bitmap));
		DeleteDC(level);
	}
	// release stored locations and other stuff we don't need
	for(i = 0; i < NUM_CURSORS; i++)
	{
		locations[i].clear();
	}
	pre_data.clear();
	pre_data_b.clear();
}

static void LoadCurrentLevel()
{
	level = CreateCompatibleDC(window);
	string lvl_name;
	lvl_name += "C:\\Andradion 2\\CmpSrc\\";
	lvl_name += LEVEL_NAMES[current_level];
	lvl_name += "_\\";
	lvl_name += LEVEL_NAMES[current_level];
	lvl_name += "_.bmp"; 
	old_level_bitmap = SelectObject(level,(HGDIOBJ)LoadImage(NULL,lvl_name.c_str(),IMAGE_BITMAP,0,0,LR_LOADFROMFILE));
	level_orientation.x = 0;
	level_orientation.y = 0;

	// load the locations and stuff
	// find file name
	GetLevelDataPath(lvl_name);
	ifstream in;
	in.open(lvl_name.c_str());

	if(in.fail())
	{
		RepaintAll();
		return;
	}

	// get both sets of palettes
	int a;
	int i;
	for(i = 0; i < 2; i++)
	{
		in >> a;
		pre_data.resize(pre_data.size()+1,a);
		for(int j = 0; j < a*3; j++)
		{
			int input;
			in >> input;
			pre_data.resize(pre_data.size()+1,input);
		}
	}

	// get level width, height, and weather pattern index
	for(i = 0; i < 3; i++)
	{
		int input;
		in >> input;
		pre_data.resize(pre_data.size()+1,input);
	}

	// done with pre_data, now get turner's coor
	locations[0].resize(1);
	in >> locations[0][0].x;
	in >> locations[0][0].y;

	// now work on pre_data_b : indoor rects, then level ends
	in >> a;
	pre_data_b.resize(pre_data_b.size()+1,a);
	for(i = 0; i < a; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			int input;
			in >> input;
			pre_data_b.resize(pre_data_b.size()+1,input);
		}
	}
	in >> a;
	pre_data_b.resize(pre_data_b.size()+1,a);
	for(i = 0; i < a; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			int input;
			in >> input;
			pre_data_b.resize(pre_data_b.size()+1,input);
		}
	}

	// now get enemies/powerups
	for(i = 1; i < 8; i++)
	{
		in >> a;
		locations[i].resize(a);
		for(int j = 0; j < a; j++)
		{
			in >> locations[i][j].x;
			in >> locations[i][j].y;
		}
	}

	// close file
	in.close();

	// procede past the pre-data
	RepaintAll();
}

static void Put(int x,int y)
{
	POINT c = {x+level_orientation.x,y+level_orientation.y};

	if(0 == current_cursor)
	{
		// turner, only one allowed
		locations[0].clear();
	}

	locations[current_cursor].resize(locations[current_cursor].size()+1,c);
	RepaintAll();
}

static void Delete(int x,int y)
{
	POINT c = {x+level_orientation.x,y+level_orientation.y};
	// find something to delete
	for(int j = 1; j < NUM_CURSORS; j++)
	{
		for(int i = 0; i < locations[j].size(); i++)
		{
			if(abs(c.x-locations[j][i].x) < CURSOR_WIDTH/2 && abs(c.y - locations[j][i].y) < CURSOR_HEIGHT/2)
			{
				locations[j].erase(locations[j].begin()+i);
				RepaintAll();
				return;
			}
		}
	}	
}

static int mouse_x;
static int mouse_y;

LRESULT WINAPI WindowProc(
  HWND hWnd,      // handle to window
  UINT Msg,       // message identifier
  WPARAM wParam,  // first message parameter
  LPARAM lParam   // second message parameter
)
{
	switch(Msg)
	{
	case WM_KEYDOWN:
		// noooo! a nested switch.
		switch(wParam) // switch to the keycode
		{
		case VK_UP:
			level_orientation.y -= 20;
			RepaintAll();
			break;
		case VK_DOWN:
			level_orientation.y += 20;
			RepaintAll();
			break;
		case VK_LEFT:
			level_orientation.x -= 20;
			RepaintAll();
			break;
		case VK_RIGHT:
			level_orientation.x += 20;
			RepaintAll();
			break;
		case VK_DELETE:
			Delete(mouse_x,mouse_y);
			break;
		case 33:
			SaveCurrentLevel();
			if(--current_level < 0)
			{
				current_level = NUM_LEVELS - 1;
			}
			LoadCurrentLevel();
			break;
		case 34:
			SaveCurrentLevel();
			if(++current_level >= NUM_LEVELS)
			{
				current_level = 0;
			}
			LoadCurrentLevel();
			break;
		}
		return 0;
	case WM_LBUTTONUP:
		Put(mouse_x,mouse_y);
		return 0;
	case WM_RBUTTONUP:
		if(++current_cursor >= NUM_CURSORS)
		{
			current_cursor = 0;
		}
		SetCursor(cursors[current_cursor]);
		return 0;
	case WM_MOUSEMOVE:
		mouse_x = LOWORD(lParam);
		mouse_y = HIWORD(lParam);
		SetCursor(cursors[current_cursor]);
		return 0;
	case WM_PAINT:
		RepaintAll();
		DefWindowProc(hWnd,Msg,wParam,lParam);
		return 0;
	case WM_CREATE:
		LoadCursors();
		current_level = 0;
		level_orientation.x = 0;
		level_orientation.y = 0;
		LoadCurrentLevel();
		return 0;
	case WM_DESTROY:
		UnloadCursors();
		SaveCurrentLevel();
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hWnd,Msg,wParam,lParam);
	}
}

int APIENTRY WinMain(HINSTANCE ourInstance,HINSTANCE,LPSTR,int)
{
	MessageBox
	(
		NULL,
		"Arrow keys: change orientation in bitmap\n"
		"PgUp/PgDn: change current level\n"
		"Right click: change what to put\n"
		"Left click: put something\n"
		"Del: remove something that the cursor is over\n"
		"Alt+F4: leave\n\n"
		"Levels are saved automatically whenever the curent one is changed or the application ends.",
		"Instructions",
		MB_ICONINFORMATION
	);

	WNDCLASSEX winclass;
	MSG msg;
	hInstance = ourInstance;
	
	winclass.cbSize = sizeof(WNDCLASSEX);
	winclass.cbClsExtra = NULL;
	winclass.cbWndExtra = NULL;
	winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.hCursor = NULL;
	winclass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	winclass.hIconSm = LoadIcon(NULL,IDI_APPLICATION);
	winclass.hInstance = hInstance;
	winclass.lpfnWndProc = WindowProc;
	winclass.lpszClassName = "Andradion 2 Level Maker";
	winclass.lpszMenuName = NULL;
	winclass.style = 0;
	
	if(0 == RegisterClassEx(&winclass)) // register the window class
	{
		return 0; 
	}

	hWnd =
		CreateWindow(
			"Andradion 2 Level Maker","Andradion 2 Level Maker",WS_POPUP|WS_VISIBLE,
			0,0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
			NULL, NULL, hInstance, NULL
		);	

	if(NULL == hWnd)
	{
		return 0;
	}

	while(true)
	{
		if(!GetMessage(&msg,NULL,0,0))
		{
			break;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;

	return 0;
}
