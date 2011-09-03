#include "StdAfx.h"
#include "Fixed.h"
#include "Logger.h"
#include "Glue.h"

using std::string;

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
  // DirectInput was added for DX3
  HINSTANCE DIHinst = TryAndReport(LoadLibrary("DINPUT.DLL"));
  WNDCLASSEX winclass;
  HWND hWnd;
  string wnd_cap, wnd_class;
  int wx, wy;
  MSG msg;

  if(!DIHinst) {
    // No DInput... must not be DX3
    string failure, title;
    GluStrLoad(IDS_OLDDX, failure);
    GluStrLoad(IDS_WINDOWCAPTION, title);
    MessageBox(0, failure.c_str(), title.c_str(), MB_ICONSTOP);
    WriteLog("Terminate\n");
    return 0;
  }

  // Close open library
  WriteLog("Unloading experimental DirectInput DLL\n");
  FreeLibrary(DIHinst);


  WriteLog("Filling out WNDCLASSEX structure\n");
  memset(&winclass, 0, sizeof(winclass));
  winclass.cbSize = sizeof(WNDCLASSEX);
  winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  winclass.hCursor = LoadCursor(NULL,IDC_ARROW);
  winclass.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_APPICON));
  winclass.hIconSm = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_APPICON));
  winclass.hInstance = hInstance;
  winclass.lpfnWndProc = WindowProc;
  GluStrLoad(IDS_WINDOWCLASS,wnd_class);
  winclass.lpszClassName = wnd_class.c_str();

  WriteLog("Registering window class\n");
	
  if(!RegisterClassEx(&winclass)) {
    WriteLog("Failed to register window class\n");
    return 0; 
  }

  GluStrLoad(IDS_WINDOWCAPTION,wnd_cap);

  // precalc window coordinates
  WriteLog("Calling CreateWindow\n");
  wx = GetSystemMetrics(SM_CXSCREEN)/2-GAME_MODEWIDTH/2;
  wy = GetSystemMetrics(SM_CYSCREEN)/2-GAME_MODEHEIGHT/2;
  hWnd =
    CreateWindow(wnd_class.c_str(), wnd_cap.c_str(),
                 WS_CAPTION | WS_POPUP | WS_VISIBLE,
		 wx, wy, GAME_MODEWIDTH, GAME_MODEHEIGHT,
		 0, 0, hInstance, 0);	

  if(!hWnd) {
    WriteLog("Failed to CreateWindow\n");
    return 0;
  }

  WriteLog("Calling GluInitialize\n");
  if(GluInitialize(hInstance, hWnd)) {
    WriteLog("GluInitialize Failed.  About to DestroyWindow.\n");
    DestroyWindow(hWnd);
    WriteLog("Terminate\n");
    return 0;
  }

  srand(TryAndReport((unsigned int)GetTickCount()));

  WriteLog("About to enter GluMain\n");
  GluMain();
  WriteLog("Forcing window closed\n");
  TryAndReport(PostMessage(hWnd, WM_CLOSE, 0, 0));

  while(!TryAndReport(GetMessage(&msg, 0, 0, 0))) {
    TryAndReport(DispatchMessage(&msg));
  }
	
  return 0;
}

