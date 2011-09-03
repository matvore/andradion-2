#include </gamelib/timer.h>
#include "Andradion 2.h"

// constants for end game
const char END_GAME_MUSIC_PATH[] = "MUSIC\\BETWIXT.MID";
const int END_NUM_FRAMES = 4;
const int MAX_FRAME_FILE_LEN = 20;
const char END_FRAME_LINEUP[END_NUM_FRAMES][MAX_FRAME_FILE_LEN] =
{
	"IMAGES\\ENDGAME1.BMP",
	"IMAGES\\ENDGAME2.BMP",
	"IMAGES\\ENDGAME3.BMP",
	"IMAGES\\ENDGAME4.BMP"
};
const int END_FRAME_WIDTH = 320;
const int END_FRAME_HEIGHT = 240;
const int SECS_PER_EFRAME = 14;

bool endgloop(bool stop) 
{
	static CTimer timer;

	static int current_frame = -1;

	CColorMap *frame;

	HBITMAP hbmp; // handle to a frame

	if(SECS_PER_EFRAME * (current_frame+1) < (int)timer.SecondsSinceLastRestart())
	{
		if(current_frame++ == -1)
		{
			// play our music
			MusicLib::Play(END_GAME_MUSIC_PATH);
			timer.Restart();
		}

		// load a new frame

		hbmp = (HBITMAP)LoadImage(
			NULL,
			END_FRAME_LINEUP[current_frame],
			IMAGE_BITMAP,
			0,
			0,
			LR_LOADFROMFILE
		);

		frame = new CColorMap(hbmp);

		DeleteObject(HGDIOBJ(hbmp));

		// blit straight to primary buffer
		gr->SetTargetBuffer(0);

		gr->TargetScreenArea().SetArea(0,0,GAME_MODE_WIDTH,GAME_MODE_HEIGHT);

		gr->TargetScreenArea().Certify();

		// put the new frame on the display
		gr->Put(*frame,false);

		// reset to back buffer
		gr->SetTargetBuffer(1);

		delete frame; // no longer needed
	}
	
	if(END_NUM_FRAMES == current_frame)
	{
		// stop the music
		MusicLib::Stop();
		return true; // leave for good
	}
	else
	{
		return false; // call back soon
	}
}
