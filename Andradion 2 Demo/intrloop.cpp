#include "Andradion 2.h"

// story and introduction constants
const char INTRO_WARPOUT_PATH[] =  "SOUNDS\\WARPOUT.WAV";
const char STORY_BMP_PATH[] =      "IMAGES\\STORY.BMP";
const char TURNER_BMP_PATH[] =     "IMAGES\\TURNER.BMP";
const char INTRO_MUSIC_PATH[] =    "MUSIC\\INTRO.MID";
const int STORY_HEIGHT = 562; // size of the story
const int STORY_WIDTH = 320;
const int TURNER_HEIGHT = 200; // size of turner bmp
const int TURNER_WIDTH = 320;
const int THROW_BACK_FPS = 30;
const int THROW_BACK_RATE = 3;
const int STORY_SCROLL_FPS = 12;
const int MAX_STAR_DIMNESS = 32;
const int NUM_STARS = 500;
const int STORY_SCALE = 4;
const float STORY_SLANT = 1.25f;
const int INTRO_MODE_WIDTH = 800; // width and height of the mode
const int INTRO_MODE_HEIGHT = 600;
const int INTRO_MODE_BPP = 16;
const int INTRO_MODE_REFRESH_RATE = 0;
const int STARSCAPE_WIDTH = 300;
const int STARSCAPE_HEIGHT = 300;
const int INTRO_TRANSITION_SQUARE_SIZE = 350; // squares of this size
const int INTRO_TRANSITION_SQUARES_PER_SEC = 300; // will appear this many times per second, and
const int INTRO_TRANSITION_DIM_MAGNITUDE = 1; // squares will dim by this
const int INTRO_TRANSITION_DIM_RATE = 3;        // every this many squares that are blitted
const int INTRO_MODE_BUFFERS = 1;
const float CINEMA_HEIGHT_TO_WIDTH = 9.0f/16.0f;
const int THROW_BACK_WIDTH_BEFORE_SCROLLING = 32;

bool intrloop(bool stop) {
	using namespace GraphLib;
	using namespace MusicLib;
	using namespace KeybLib;
	
	static bool has_loaded = false;
	static bool has_drawn_back = false;
	static int story_y; // position in the story
	static RECT turner_blt;
	
	static CColorMap *stars; // the stars in the background
	static CColorMap *back_buffer;
	static CColorMap *story;
	static CColorMap *turner;

	CColor leaving; // color of the squares when the player is leaving
	
	HBITMAP story_bmp;
	HBITMAP turner_bmp;
	HBITMAP loading_bmp;
	CColorMap *loading;
	SOB warpout;
	DWORD warp_status; // playing or not of the above sound
	
	DDSURFACEDESC2 starsd;
	unsigned short *starsb;
	DDBLTFX fx;
	unsigned char keyb[KEYBOARD_BUFFER_SIZE];
	int i; // looping
	RECT source,dest;

	Enable_Sync();

	if(!has_loaded) {

		// change video mode
		delete gr;
		gr = new CGraphics(
			wnd,
			INTRO_MODE_WIDTH,
			INTRO_MODE_HEIGHT,
			INTRO_MODE_BPP,
			INTRO_MODE_BUFFERS,
			NULL,
			INTRO_MODE_REFRESH_RATE
		);
		gr->Certify();
		
		// put the loading . . . image on the screen
		loading_bmp =
			(HBITMAP)LoadImage(
				NULL,
				LOADING_BMP_PATH,
				IMAGE_BITMAP,
				0,
				0,
				LR_LOADFROMFILE
			);
		
		loading = new CColorMap(loading_bmp);
		
		DeleteObject((HGDIOBJ)loading_bmp);

		gr->TargetScreenArea().SetArea(0,0,INTRO_MODE_WIDTH,INTRO_MODE_HEIGHT);
		gr->TargetScreenArea().Certify();
		gr->Put(*loading,false);

		delete loading;
		
		// done with that

		stars = new CColorMap(STARSCAPE_WIDTH,STARSCAPE_HEIGHT);
		back_buffer = new CColorMap(INTRO_MODE_WIDTH,INTRO_MODE_HEIGHT);

		story_bmp = (HBITMAP)LoadImage(NULL,STORY_BMP_PATH,IMAGE_BITMAP,STORY_WIDTH,STORY_HEIGHT,LR_LOADFROMFILE);
		story = new CColorMap(story_bmp);
		DeleteObject(story_bmp);

		turner_bmp = (HBITMAP)LoadImage(NULL,TURNER_BMP_PATH,IMAGE_BITMAP,TURNER_WIDTH,TURNER_HEIGHT,LR_LOADFROMFILE);
		turner = new CColorMap(turner_bmp);
		DeleteObject(turner_bmp);
	
		// now to create the starscape

		// clear the starscape
		memset((void *)&fx,0,sizeof(fx));
		fx.dwFillColor = 0;
		fx.dwSize = sizeof(fx);

		// fill with black
		while(FAILED(stars->Data()->Blt(
			NULL,NULL,NULL,DDBLT_WAIT | DDBLT_COLORFILL,&fx
		)))
		{
			SurfaceLost(stars->Data());
		}
			
		// init the starsd struct
		memset((void *)&starsd,0,sizeof(starsd));
		starsd.dwSize = sizeof(starsd);

		// lock the stars
		while(FAILED(stars->Data()->Lock(
			NULL,
			&starsd,
			DDLOCK_WAIT | DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR,
			NULL
		)))
		{
			SurfaceLost(stars->Data());
		}

		srand(clock()); // seed rng
		starsb = (unsigned short *)starsd.lpSurface;

		for(i = 0; i < NUM_STARS; i++) { // plot a bunch of stars
			CColor sc; // star color
		
			sc.SetColor( // a randomly bright white
				(rand()%MAX_STAR_DIMNESS)+256-MAX_STAR_DIMNESS,
				(rand()%MAX_STAR_DIMNESS)+256-MAX_STAR_DIMNESS,
				(rand()%MAX_STAR_DIMNESS)+256-MAX_STAR_DIMNESS
			);
			sc.Certify(); // certify the color

			// put it on the scape
			starsb[(rand()%STARSCAPE_HEIGHT)*(starsd.lPitch>>1) + rand()%STARSCAPE_WIDTH]=
				sc.Color16();
		}

		// unlock the stars
		while(FAILED(stars->Data()->Unlock(NULL)))
		{
			SurfaceLost(stars->Data());
		}		

		story_y = -1;

		Play(INTRO_MUSIC_PATH); // play the music

		// clear the screen from the loading . . . message
		gr->TargetScreenArea().SetArea(0,0,INTRO_MODE_WIDTH,INTRO_MODE_HEIGHT);
		gr->TargetScreenArea().Certify();

		// create the black color and put down the rectangle
		CColor black(0,0,0,0);
		black.Certify();

		SoundLib::Create_SOB(&warpout,INTRO_WARPOUT_PATH); // load the warpout sound
		
		warpout->Play(0,0,0); // play our warpout sound

		while( // pause until the sound has been played
			(SUCCEEDED(warpout->GetStatus(&warp_status))) &&
			(warp_status & DSBSTATUS_PLAYING)
		);

		SoundLib::Destroy_SOB(warpout); // destroy our new sound

		gr->Rectangle(black);

		// use 16:9 aspect ration for cinema effect
		gr->TargetScreenArea().SetWidth(INTRO_MODE_WIDTH);
		gr->TargetScreenArea().SetHeight(int((float)INTRO_MODE_WIDTH * CINEMA_HEIGHT_TO_WIDTH));
		gr->TargetScreenArea().SetLeft(0);
		gr->TargetScreenArea().SetTop(
			(INTRO_MODE_HEIGHT>>1)-(gr->TargetScreenArea().GetHeight()>>1)
		);
		gr->TargetScreenArea().Certify();

		// the width of the turner photo = (4 * target_screen_area_height)/3
		turner_blt.left = (INTRO_MODE_WIDTH>>1)-(((4*gr->TargetScreenArea().GetHeight())/3)>>1);
		turner_blt.right = turner_blt.left+((4*gr->TargetScreenArea().GetHeight())/3);
		turner_blt.top = 0;
		turner_blt.bottom = INTRO_MODE_HEIGHT;

		has_loaded = true;

		// done loading!

		return false; // keep going
	}

	// put the stars on the back buffer
	while(FAILED(back_buffer->Data()->Blt(
		NULL,
		stars->Data(),
		NULL,
		DDBLT_WAIT,
		NULL
	)))
	{
		SurfaceLost(back_buffer->Data());
	}

	Start_Frame_For_Sync();

	// draw back the turner photo
	if(!has_drawn_back) {
		// put the turner photo on the back buffer
		while(FAILED(back_buffer->Data()->Blt(
			&turner_blt,
			turner->Data(),
			NULL,
			DDBLT_WAIT,
			NULL)))
		{
			SurfaceLost(back_buffer->Data());
		}

		turner_blt.left += THROW_BACK_RATE;
		turner_blt.right -= THROW_BACK_RATE;
		turner_blt.top += THROW_BACK_RATE;
		turner_blt.bottom -= THROW_BACK_RATE;
		if(turner_blt.left >= turner_blt.right || turner_blt.top >= turner_blt.bottom) {
			has_drawn_back = true;
		}
	}

	if(turner_blt.right - turner_blt.left + 1 < THROW_BACK_WIDTH_BEFORE_SCROLLING) {
		story_y++; // increment our position in the story

		source.left = 0; source.right = STORY_WIDTH;
		source.top = story_y;
		source.bottom = story_y+1;
		dest.left = 0;
		dest.right = INTRO_MODE_WIDTH;
		dest.top = INTRO_MODE_HEIGHT-STORY_SCALE;
		dest.bottom = INTRO_MODE_HEIGHT;

		for(double error=0.00;dest.left < dest.right;dest.top-=STORY_SCALE,dest.bottom-=STORY_SCALE,source.top--,source.bottom--) {
			error += STORY_SLANT;
			while(error >= 1.00) {
				dest.left+=1;
				dest.right-=1;
				error -= 1.00;
			}
			if(source.top >= 0 && source.top < STORY_HEIGHT)
				back_buffer->Data()->Blt(&dest,story->Data(),&source,DDBLT_KEYSRC,NULL);
		}
	}

	// blit the back buffer
	gr->Put(*back_buffer,false);

	if(has_drawn_back) {
		End_Frame_For_Sync(STORY_SCROLL_FPS);
	} else {
		End_Frame_For_Sync(THROW_BACK_FPS);
	}

	// check for keyboard input now
	KeybLib::Get_State(keyb); // get keyboard state

	// when they press arrow keys or spacebar, change story_y
	if(has_drawn_back) {
		if(keyb[DIK_UP] & HIGHEST_BIT8) story_y -= 4;
		else if(keyb[DIK_DOWN] & HIGHEST_BIT8) story_y += 2;
		else if(keyb[DIK_SPACE] & HIGHEST_BIT8) story_y -= 1;
	}
	
	if(story_y < 0) story_y = 0;
	if(story_y - INTRO_MODE_HEIGHT / STORY_SCALE > STORY_HEIGHT)
		story_y=STORY_HEIGHT+INTRO_MODE_HEIGHT/STORY_SCALE;

	if(stop) keyb[0] = HIGHEST_BIT8; // simulate key press if the caller wants us to stop
			
	for(i = 0; i < KEYBOARD_BUFFER_SIZE; i++)
		if(keyb[i]&HIGHEST_BIT8 && i != DIK_UP && i != DIK_DOWN && i != DIK_SPACE) { // key pressed
			// release everything
			delete stars;
			delete story;
			delete back_buffer;
			delete turner; // just in case

			has_loaded = false;
			has_drawn_back = false;

			Stop(); // stop the music

			// screen transition 
			gr->TargetScreenArea().SetWidth(INTRO_TRANSITION_SQUARE_SIZE);
			gr->TargetScreenArea().SetHeight(INTRO_TRANSITION_SQUARE_SIZE);
			leaving.SetColor(255,255,255);
			leaving.Certify();
			i = 0;
			do {
				unsigned char color;
				
				Start_Frame_For_Sync();
				gr->TargetScreenArea().SetLeft(
					rand()%(INTRO_MODE_WIDTH-INTRO_TRANSITION_SQUARE_SIZE)
				);
				gr->TargetScreenArea().SetTop(
					rand()%(INTRO_MODE_HEIGHT-INTRO_TRANSITION_SQUARE_SIZE)
				);
				gr->TargetScreenArea().Certify();
				gr->Rectangle(leaving);
				i++; // another square
				if(i >= INTRO_TRANSITION_DIM_RATE) {
					i -= INTRO_TRANSITION_DIM_RATE;
					leaving.GetColor(color,color,color);
					if(color > INTRO_TRANSITION_DIM_MAGNITUDE) color -= INTRO_TRANSITION_DIM_MAGNITUDE;
					else color = 0;
					leaving.SetColor(color,color,color);
					leaving.Certify();
				}
				End_Frame_For_Sync(INTRO_TRANSITION_SQUARES_PER_SEC);
			} while(leaving.Color16());

			// reset graphics mode
			delete gr;
			gr = new CGraphics(
				wnd,
				GAME_MODE_WIDTH,
				GAME_MODE_HEIGHT,
				GAME_MODE_BPP,
				GAME_MODE_BUFFERS,
				NULL,
				GAME_MODE_REFRESH_RATE
			);
			gr->Certify();
			return true; // we're done with the intro
		}
	return false; // keep going
}
