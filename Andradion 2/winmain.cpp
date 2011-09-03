#include "StdAfx.h"
#include "Fixed.h"
#include "SharedConstants.h"
#include "Character.h"
#include "Fire.h"
#include "Glue.h"
#include "resource.h"

using NGameLib2::tstring;

LRESULT WINAPI WindowProc(
			  HWND hWnd,      // handle to window
			  UINT Msg,       // message identifier
			  WPARAM wParam,  // first message parameter
			  LPARAM lParam   // second message parameter
			  )
{
  switch(Msg)
    {
    case WM_CLOSE:
      if(true == GluCanQuit())
	{
	  DestroyWindow(hWnd);
	}
      return 0;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    case WM_KEYDOWN:
      GluKeyPress(wParam);
      return 0;
    case WM_CHAR:
      GluCharPress((TCHAR)wParam);
      return 0;
    default:
      return DefWindowProc(hWnd,Msg,wParam,lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE,LPSTR,int)
{
  // more first and foremost, enable the debug logger
  BeginLog("log.txt");
  WriteLog("Running Andradion 2 . . .");
  WriteLog("Checking for DirectX 3.0 by loading DirectInput DLL");

  // first and foremost, check for DirectX 7.0 or better
  // Check for DirectX 7 by creating a DDraw7 object
  // First see if DDRAW.DLL even exists.
  // DirectInput was added for DX3
  HINSTANCE DIHinst = LoadLibrary( "DINPUT.DLL" );
  if( DIHinst == 0 )
    {
      // No DInput... must not be DX3
      WriteLog("Could not find DINPUT.DLL . . Quitting");
      tstring failure;
      tstring title;
      GluStrLoad(IDS_OLDDX,failure);
      GluStrLoad(IDS_WINDOWCAPTION,title);
      MessageBox(NULL,failure.c_str(),title.c_str(),MB_ICONSTOP);
      WriteLog("Andradion 2 terminated without crash");
      return 0;
    }

  // Close open library
  WriteLog("Unloading experimental DirectInput DLL");
  FreeLibrary( DIHinst );

  WNDCLASSEX winclass;
  HWND hWnd;

  WriteLog("Filling out WNDCLASSEX structure");
  winclass.cbSize = sizeof(WNDCLASSEX);
  winclass.cbClsExtra = NULL;
  winclass.cbWndExtra = NULL;
  winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  winclass.hCursor = LoadCursor(NULL,IDC_ARROW);
  winclass.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_APPICON));
  winclass.hIconSm = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_APPICON));
  winclass.hInstance = hInstance;
  winclass.lpfnWndProc = WindowProc;
  tstring wnd_class;
  GluStrLoad(IDS_WINDOWCLASS,wnd_class);
  winclass.lpszClassName = wnd_class.c_str();
  winclass.lpszMenuName = NULL;
  winclass.style = 0;

  WriteLog("Registering window class");
	
  if(0 == RegisterClassEx(&winclass)) // register the window class
    {
      WriteLog("Failed to register window class");
      WriteLog("Andradion 2 terminate without crash");
      return 0; 
    }

  tstring wnd_cap;
  GluStrLoad(IDS_WINDOWCAPTION,wnd_cap);

  // precalc window coordinates
  WriteLog("Calling CreateWindow");
  int wx = GetSystemMetrics(SM_CXSCREEN)/2-GAME_MODEWIDTH/2;
  int wy = GetSystemMetrics(SM_CYSCREEN)/2-GAME_MODEHEIGHT/2;
  hWnd =
    CreateWindow(
		 wnd_class.c_str(), wnd_cap.c_str(),WS_CAPTION|WS_POPUP|WS_VISIBLE,
		 wx, wy, GAME_MODEWIDTH, GAME_MODEHEIGHT,
		 NULL, NULL, hInstance, NULL
		 );	

  if(NULL == hWnd)
    {
      WriteLog("Failed to CreateWindow");
      WriteLog("Andradion 2 terminate without crash");
      return 0;
    }

  WriteLog("Calling GluInitialize");
  if(true == GluInitialize(hInstance,hWnd))
    {
      WriteLog("GluInitialize Failed.  About to DestroyWindow.");
      DestroyWindow(hWnd);
      WriteLog("Andradion 2 terminate without crash");
      return 0;
    }

  WriteLog("Seeding random number generator with call to GetTickCount() and srand()");
  srand((unsigned int)GetTickCount());

  WriteLog("About to enter GluMain . . . prepare to play");
  GluMain();
  WriteLog("GluMain terminated");
  WriteLog("Making sure window is closed by PostMessage'ing with WM_CLOSE message");
  PostMessage(hWnd,WM_CLOSE,0,0);
  MSG msg;
  WriteLog("Entering stripped-down message loop until WM_QUIT is received");
  while(0 == GetMessage(&msg,NULL,0,0))
    {
      WriteLog("Got a message.  Dispatching . . .");
      DispatchMessage(&msg);
      WriteLog("Dispatched.");
    }
  WriteLog("GetMessage() returned non-zero, must have received WM_QUIT");
	
  WriteLog("Andradion 2 terminate without crash");
  return 0;
}

