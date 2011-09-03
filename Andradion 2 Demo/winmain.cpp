#include "Andradion 2.h"

// global
HWND wnd;
HINSTANCE instance;

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	// first and foremost, check for DirectX 7.0 or better
    // Check for DirectX 7 by creating a DDraw7 object
    // First see if DDRAW.DLL even exists.
    LPDIRECTDRAW7 pDD7 = NULL;
	HINSTANCE DDHinst = NULL;
	typedef HRESULT(WINAPI * DIRECTDRAWCREATEEX)( GUID*, VOID**, REFIID, IUnknown* );
	DIRECTDRAWCREATEEX ddce = NULL;	
    if
	(
		!(DDHinst = LoadLibrary( "DDRAW.DLL" ))||
	    !(ddce = (DIRECTDRAWCREATEEX)GetProcAddress(DDHinst,"DirectDrawCreateEx")) ||
		!(SUCCEEDED(ddce(NULL,(void **)&pDD7,IID_IDirectDraw7,NULL)))
	)
    {
		// couldn't get ahold of dx7
		if(NULL != DDHinst)
		{
			FreeLibrary(DDHinst);
		}
		MessageBox
		(
			NULL,
			"DirectX 7.0 is required to run Andradion 2, but it is not installed.  "
			"To download DirectX 7.0, go to "
			"www.microsoft.com/directx/homeuser/downloads/default.asp or "
			"www.microsoft.com/directx\n"
			"E-mail me at matt@jusenkyo.com if you need any help.",
			WINDOW_CAPTION,
			MB_ICONSTOP
		);
	
		return 0;
    }

	pDD7->Release();

    // Close open library
    FreeLibrary( DDHinst );

	WNDCLASSEX winclass;
	HWND hWnd;
	MSG msg;
	bool will_run_again = true; // referring to the game loop function

	winclass.cbSize = sizeof(WNDCLASSEX);
	winclass.cbClsExtra = NULL;
	winclass.cbWndExtra = NULL;
	winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.hCursor = LoadCursor(NULL,IDC_ARROW);
	winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hInstance = hInstance;
	winclass.lpfnWndProc = DefWindowProc;
	winclass.lpszClassName = WINDOW_CLASS;
	winclass.lpszMenuName = NULL;
	winclass.style = 0;
	
	if(!RegisterClassEx(&winclass)) return 0; // register the window class

	hWnd =
		CreateWindowEx(
			NULL, WINDOW_CLASS, WINDOW_CAPTION, WS_POPUP | WS_VISIBLE,
			0, 0, GAME_MODE_WIDTH, GAME_MODE_HEIGHT,
			NULL, NULL, hInstance, NULL
		);

	if(!hWnd || KeybLib::Init(hInstance,hWnd))
	{ // failure
		MessageBox(hWnd,STARTUP_ERROR,ERROR_MSG_CAPTION,MB_ICONSTOP);
		return 0;
	}

	SoundLib::Init(hWnd);
	MusicLib::Init(hWnd);

	ShowCursor(FALSE);

	wnd = hWnd;
	instance = hInstance;

	GraphLib::EnableSurfaceRestorationGiveUp(true);

	// do the graphics
	gr = new GraphLib::CGraphics(
		wnd,
		GAME_MODE_WIDTH,
		GAME_MODE_HEIGHT,
		GAME_MODE_BPP,
		GAME_MODE_BUFFERS,
		NULL,
		GAME_MODE_REFRESH_RATE
	);

	gr->Certify();

	// randomize the random-number generator
	srand((unsigned int)GetTickCount());

	while(true) {
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			if(msg.message == WM_QUIT) {
				if(will_run_again) {
					mainloop(true);
					will_run_again = false;
				}
								
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if(will_run_again && mainloop(false)) {
			PostQuitMessage(0);
			will_run_again = false;
		}
	}

	CMedia::Media().LoadLevel(LEVEL_UNLOAD); // unload all of cmedia before graphics system is unloaded

	delete gr;
	GraphLib::EnableSurfaceRestorationGiveUp(false);

	MusicLib::Uninit();
	SoundLib::Uninit();
	KeybLib::Uninit();
	ShowCursor(TRUE);

	return msg.wParam;
}
