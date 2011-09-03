#include "StdAfx.h"
#include "Fixed.h"
#include "SharedConstants.h"
#include "Certifiable.h"
#include "Graphics.h"
#include "Character.h"
#include "Fire.h"
#include "Glue.h"
#include "resource.h"
#include "Logger.h"

LRESULT WINAPI WindowProc(HWND hWnd, UINT Msg, WPARAM wParam,
			  LPARAM lParam) {
  switch(Msg) {
  case WM_CLOSE:
    if(GluCanQuit()) {
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
  WriteLog("Checking for DirectX 3.0 by loading DirectInput DLL\n");

  // DirectInput was added for DX3
  HINSTANCE DIHinst = LoadLibrary( "DINPUT.DLL" );
  if( DIHinst == 0 ) {
    // No DInput... must not be DX3
    WriteLog("Could not find DINPUT.DLL . . Quitting\n");
    string failure;
    string title;
    GluStrLoad(IDS_OLDDX,failure);
    GluStrLoad(IDS_WINDOWCAPTION,title);
    MessageBox(NULL,failure.c_str(),title.c_str(),MB_ICONSTOP);
    WriteLog("Terminate\n");
    return 0;
  }

  // Close open library
  WriteLog("Unloading experimental DirectInput DLL\n");
  FreeLibrary( DIHinst );

  WNDCLASSEX winclass;
  HWND hWnd;

  WriteLog("Filling out WNDCLASSEX structure\n");
  winclass.cbSize = sizeof(WNDCLASSEX);
  winclass.cbClsExtra = 0;
  winclass.cbWndExtra = 0;
  winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  winclass.hCursor = LoadCursor(NULL,IDC_ARROW);
  winclass.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_APPICON));
  winclass.hIconSm = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_APPICON));
  winclass.hInstance = hInstance;
  winclass.lpfnWndProc = WindowProc;
  string wnd_class;
  GluStrLoad(IDS_WINDOWCLASS,wnd_class);
  winclass.lpszClassName = wnd_class.c_str();
  winclass.lpszMenuName = NULL;
  winclass.style = 0;

  WriteLog("Registering window class\n");
	
  if(0 == RegisterClassEx(&winclass)) // register the window class
    {
      WriteLog("Failed to register window class\n");
      WriteLog("Andradion 2 terminate without crash\n");
      return 0; 
    }

  string wnd_cap;
  GluStrLoad(IDS_WINDOWCAPTION,wnd_cap);

  // precalc window coordinates
  WriteLog("Calling CreateWindow\n");
  int wx = GetSystemMetrics(SM_CXSCREEN)/2-GAME_MODEWIDTH/2;
  int wy = GetSystemMetrics(SM_CYSCREEN)/2-GAME_MODEHEIGHT/2;
  hWnd =
    CreateWindow(wnd_class.c_str(), wnd_cap.c_str(),WS_CAPTION|WS_POPUP|WS_VISIBLE,
		 wx, wy, GAME_MODEWIDTH, GAME_MODEHEIGHT,
		 NULL, NULL, hInstance, NULL
		 );	

  if(NULL == hWnd) {
    WriteLog("Failed to CreateWindow\n");
    WriteLog("Andradion 2 terminate without crash\n");
    return 0;
  }

  WriteLog("Calling GluInitialize\n");
  if(true == GluInitialize(hInstance,hWnd)) {
    WriteLog("GluInitialize Failed.  About to DestroyWindow.\n");
    DestroyWindow(hWnd);
    WriteLog("Terminate\n");
    return 0;
  }

  srand(TryAndReport((unsigned int)GetTickCount()));

  WriteLog("About to enter GluMain\n");
  GluMain();
  WriteLog("Making sure window is closed by PostMessage'ing with WM_CLOSE message\n");
  PostMessage(hWnd,WM_CLOSE,0,0);
  MSG msg;
  WriteLog("Entering stripped-down message loop until WM_QUIT is received\n");
  while(0 == TryAndReport(GetMessage(&msg,NULL,0,0))) {
    DispatchMessage(&msg);
    WriteLog("Dispatched.\n");
  }
	
  WriteLog("Terminate\n");
  return 0;
}

