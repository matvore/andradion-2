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
#include "Fixed.h"
#include "Logger.h"
#include "Character.h"
#include "Glue.h"

using std::endl;
using std::string;

const char *WINDOW_CLASS = "Andradion 2FV WndClass";

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
  HINSTANCE DIHinst
      = LogResult("Try to load DInput", LoadLibrary("DINPUT.DLL"));
  WNDCLASSEX winclass;
  HWND hWnd;
  int wx, wy;
  MSG msg;

  if(!DIHinst) {
    // No DInput... must not be DX3
    string failure, title;
    GluStrLoad(IDS_OLDDX, failure);
    MessageBox(0, failure.c_str(), WINDOW_CAPTION, MB_ICONSTOP);
    logger << "Terminate" << endl;
    return 0;
  }

  // Close open library
  logger << "Unloading experimental DirectInput DLL" << endl;
  FreeLibrary(DIHinst);

  logger << "Filling out WNDCLASSEX structure" << endl;
  memset(&winclass, 0, sizeof(winclass));
  winclass.cbSize = sizeof(WNDCLASSEX);
  winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  winclass.hCursor = LoadCursor(NULL,IDC_ARROW);
  winclass.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_APPICON));
  winclass.hIconSm = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_APPICON));
  winclass.hInstance = hInstance;
  winclass.lpfnWndProc = WindowProc;
  winclass.lpszClassName = WINDOW_CLASS;

  logger << "Registering window class" << endl;
	
  if(!RegisterClassEx(&winclass)) {
    logger << "Failed to register window class" << endl;
    return 0; 
  }

  // precalc window coordinates
  logger << "Calling CreateWindow" << endl;
  wx = GetSystemMetrics(SM_CXSCREEN)/2-GAME_MODEWIDTH/2;
  wy = GetSystemMetrics(SM_CYSCREEN)/2-GAME_MODEHEIGHT/2;
  hWnd =
    CreateWindow(WINDOW_CLASS, WINDOW_CAPTION,
                 WS_CAPTION | WS_POPUP | WS_VISIBLE,
                 wx, wy, GAME_MODEWIDTH, GAME_MODEHEIGHT,
                 0, 0, hInstance, 0);	

  if(!hWnd) {
    logger << "Failed to CreateWindow" << endl;
    return 0;
  }

  logger << "Calling GluInitialize" << endl;
  if(GluInitialize(hInstance, hWnd)) {
    logger << "GluInitialize Failed.  About to DestroyWindow." << endl;
    DestroyWindow(hWnd);
    logger << "Terminate" << endl;
    return 0;
  }

  srand(LogResult("Random number seed", (unsigned int)GetTickCount()));

  logger << "About to enter GluMain" << endl;
  GluMain();
  logger << "Forcing window closed" << endl;
  LogResult("Post close message", PostMessage(hWnd, WM_CLOSE, 0, 0));

  while(!LogResult("Get window message", GetMessage(&msg, 0, 0, 0))) {
    LogResult("Dispatch window message", DispatchMessage(&msg));
  }

  return 0;
}
