#include <string>
#include "Andradion 2.h"

// the media module !  For the CMedia class

using namespace GraphLib; // using graphics library
using namespace GenericClassLib;
using namespace SoundLib;
using namespace std; // using standard library

// CMedia is a singleton

//CMedia CMedia::media; // static member

// the constructor

CMedia::CMedia()
: current_level(-1)
{
	CMediaConstruction();
	this->LoadLevel(0);
}

void CMedia::CMediaConstruction() 
{
	Init_Error_Ready_System();
	int i; // looping
	string full_path; // full path of the current file to be loaded
	HBITMAP bload;

	lower_bitmap = NULL; // set these two pointers to NULL
	upper_bitmap = NULL; //  because no level is loaded

	// now load bitmaps and sounds

	// bitmaps first
	bitmaps = new CColorMap *[NUM_BITMAPS];

	if(NULL == bitmaps) // check for failure
	{
		Return_With_Error(1);
	}
		
	for(i = 0; i < NUM_BITMAPS; i++)
	{
		full_path = BITMAP_PATH;
		full_path += BITMAPS[i];

		bload = (HBITMAP)LoadImage(
			NULL,
			full_path.c_str(),
			IMAGE_BITMAP,
			0, 0, // default size
			LR_LOADFROMFILE
		);

		bitmaps[i] =new CColorMap(bload);

		DeleteObject((HGDIOBJ)bload);
	}

	// and now sounds
	sounds= new SOB[NUM_SOUNDS];

	// check for failure

	if(NULL == sounds)
	{
		Return_With_Error(1);
	}

	for(i = 0; i < NUM_SOUNDS; i++)
	{
		full_path = SOUND_PATH;
		full_path += SOUNDS[i];

		if(Create_SOB(&sounds[i],full_path.c_str()))
		{
			Return_With_Error(1);
		} // create this, while checking for error
	}

	tokens.resize(1);

	tokens[TURNER] = NULL;

	Begin_Error_Table_Return_Nothing
		Define_Error(1,RESOURCE_LOAD_ERROR)
	End_Error_Table_Exit_With_Error(NULL);
}


CMedia::~CMedia()
{
	if(LEVEL_UNLOAD != current_level)
	{
		this->CMediaDestruction();
	}
}

void CMedia::CMediaDestruction()
{
	int i; // looping

	delete upper_bitmap;
	delete lower_bitmap;

	// get rid of the bitmaps

	for(i = 0; i < NUM_BITMAPS; i++)
	{
		delete bitmaps[i];
		bitmaps[i] = NULL;
	}

	delete [] bitmaps;

	// get rid of the sounds

	for(i = 0; i < NUM_SOUNDS; i++)
	{
		Destroy_SOB(sounds[i]);
		sounds[i] = NULL;
	}

	delete [] sounds;

	this->current_level = LEVEL_UNLOAD;

	// get rid of all the tokens
	this->DestroyTokens(true);
}

// the level loader

void CMedia::LoadLevel(int index) {
	Init_Error_Ready_System();

	if(LEVEL_UNLOAD == current_level)
	{
		this->CMediaConstruction();
	}

	if(LEVEL_UNLOAD == index)
	{
		this->CMediaDestruction();
		return;
	}

	string r; // resource to be loaded next
	string full_path; // full path

	int i; // a general-purpose integer variable
	int j; // a general-purpose integer variable
	int k;
	int l;
	int m;
	int n;
	int o;
	int p;

	int current_token_index; // needed when enumerating enemies, health packs, ammo

	ifstream
		lvl;

	CTurner *
		turner;

	CWall *
		wall;

	CLevelEnd *
		lend;

	CEnemy *
		enemy;

	CAmmo *
		ammo;

	CHealthPack *
		hp;

	current_level = index; // keep track of the current level

	// clean up from any last level we may have done
	delete upper_bitmap;
	delete lower_bitmap;

	// first, put down the loading level bitmap

	// set target buffer to primary so we can blit directly
	gr->SetTargetBuffer(0);

	// fill the whole screen with the screen-loading
	gr->TargetScreenArea().SetArea(0,0,GAME_MODE_WIDTH,GAME_MODE_HEIGHT);
	gr->TargetScreenArea().Certify();

	// load the bitmap
	HBITMAP
		loading_bmp = (HBITMAP)
			LoadImage(
				NULL,
				LOADING_BMP_PATH,
				IMAGE_BITMAP,
				0,
				0,
				LR_LOADFROMFILE
			);

	// create the color map of the loading bitmap
	CColorMap *
		loading = // (CColorMap *)
			new CColorMap(loading_bmp);

	// the bitmap object is no longer needed, so delete it
	DeleteObject((HGDIOBJ)loading_bmp);

	// put it on the screen; no transparency because it's full-screen
	gr->Put(*loading,false);

	// reset the target buffer
	gr->SetTargetBuffer(1);

	// the loading color map is no longer needed because it's already on the screen,
	//  so delete it
	delete loading;

	// play the tango music
	MusicLib::Play(LOADING_MUSIC_PATH);

	// now the loading bitmap is on the screen

	// make sure the level index is valid
	assert(index >= 0);
	assert(index < NUM_LEVELS);
	
	string fn;
	
	fn += LEVEL_PATH;
	fn += LEVELS[index];

	// load the file

	lvl.open(fn.c_str()); // use the new string we put together

	if(lvl.fail())
	{
		Return_With_Error(1);
	}

	// the file has been loaded

	// load the color map of the lower bitmap level

	lvl >> r;
	
	full_path = BITMAP_PATH + r;

	loading_bmp = (HBITMAP)LoadImage(NULL,full_path.c_str(),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);

	lower_bitmap = new CColorMap(loading_bmp);

	DeleteObject((HGDIOBJ)loading_bmp);

	// load the color map of the upper bitmap level

	lvl >> r;

	full_path = BITMAP_PATH + r;

	loading_bmp = (HBITMAP)LoadImage(NULL,full_path.c_str(),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);

	upper_bitmap = new CColorMap(loading_bmp);
	
	DeleteObject((HGDIOBJ)loading_bmp);

	// done with both the bitmaps, now we gotta load the music

	lvl >> outdoor_music;
	lvl >> indoor_music;

	// get turner data

	// make room for turner if we gotta
	if(NULL == tokens[TURNER]) { // if the length is nada
		// allocate the turner class
		turner = new CTurner; 

		if(NULL == turner)
		{
			Return_With_Error(1);
		}

		// store it into token array
		tokens[TURNER] = (IToken *)turner;
		
		// all initialization of turner is done in the constructor for it
	}
	else
	{
		turner = (CTurner *)tokens[TURNER];
	}

	// empty out the tokens array except for the first element (turner)
	this->DestroyTokens(false);

	// set up the walking_data matrix
	walking_data.resize(
		lower_bitmap->GetHeight() / TILE_HEIGHT,
		lower_bitmap->GetWidth() / TILE_WIDTH
	);

	// get turner's coordinates
	lvl >> i;
	lvl >> j;

	// set position first so the previous coordinates
	//  are invalid so music is played always
	turner->SetPosition(-1,-1);

	turner->Move(i+1,j+1);

	assert(i >= 0);
	assert(j >= 0);

	// fill it up with walkable values
	for(i = 0; i < walking_data.numrows(); i++)
	{
		assert(i >= 0);
		for(j = 0; j < walking_data.numcols(); j++)
		{
			walking_data[i][j] = WALKABLE_OUTSIDE;
		}
	}

	// find out how many unwalkable regions there are
	lvl >> j;

	// loop through each rectangle which defines unwalkable regions
	for(i = 0; i < j; i++)
	{
		lvl >> m;
		lvl >> n;
		lvl >> o;
		lvl >> p;
		
		// x range: m to o
		// y range: n to p

		// loop through y/rows
		for(k = n; k < p; k+=TILE_HEIGHT)
		{
			assert(k/TILE_HEIGHT >= 0);
			assert(k >= 0);
			// loop x/cols
			for(l = m; l < o; l+=TILE_WIDTH)
			{
				walking_data[k/TILE_HEIGHT][l/TILE_WIDTH] =
					WALKABLE_WALL;
			}
		}
	}

	// see what we did just now with the walls?
	//  now we do it with the indoor regions

	lvl >> j;

	for(i = 0; i < j; i++)
	{
		lvl >> m;
		lvl >> n;
		lvl >> o;
		lvl >> p;

		for(k = n; k < p; k+= TILE_HEIGHT)
		{
			assert(k/TILE_HEIGHT >= 0);
			assert(k >= 0);
			for(l = m; l < o; l+=TILE_WIDTH)
			{
				walking_data[k/TILE_HEIGHT][l/TILE_WIDTH] =
					WALKABLE_INSIDE;
			}
		}
	}

	// add to the length for room for the wall
	tokens.resize(2);

	wall = new CWall; // create a new wall
	
	if(NULL == wall) // we could fail
	{
		Return_With_Error(1); // so leave
	}

	tokens[WALL] = (IToken *)wall;

	current_token_index = WALL+1; // better get this stored, by the way

	// that's all we need for the wall, so now let's do the level ends

	lvl >> j;

	for(i = 0; i < j; i++)
	{
		lvl >> k;
		lvl >> l;
		lvl >> m;

		lend = new CLevelEnd;

		// check for malloc failure
		if(NULL == lend)
		{
			Return_With_Error(1);
		}

		lend->SetState(k);
		lend->SetPosition(l,m);

		tokens.resize(current_token_index+1);
		tokens[current_token_index++] = (IToken *)lend;
	}

	// do the enemies
	for(p = 0; p < 3; p++) // p is the current enemy type
	{
		lvl >> j; // need to know how many there are

		for(i = 0; i < j; i++)
		{
			lvl >> k; // get x position
			lvl >> l; // get y position

			enemy = new CEnemy(p);

			// check for failure
			if(NULL == enemy)
			{
				Return_With_Error(1);
			}

			enemy->SetPosition(k,l); // need the coordinates

			// other enemy properties are done by the constructor

			tokens.resize(current_token_index+1);
			tokens[current_token_index++] = (IToken *)enemy;
		}
	}

	// do the ammo types like we did the enemies
	for(p = 0; p < NUM_WEAPONS; p++)
	{
		lvl >> j;

		for(i = 0; i < j; i++)
		{
			lvl >> k;
			lvl >> l;

			ammo = new CAmmo(p);

			if(NULL == ammo)
			{
				Return_With_Error(1);
			}

			ammo->SetPosition(k,l);

			// other ammo properties are done by the constructor

			tokens.resize(current_token_index+1);
			tokens[current_token_index++] = (IToken *)ammo;
		}
	}

	// do the health packs in the same manner now
	lvl >> j; // get the health pack count

	for(i = 0; i < j; i++)
	{
		lvl >> k;
		lvl >> l;

		hp = new CHealthPack;

		if(NULL == hp)
		{
			Return_With_Error(1);
		}

		hp->SetPosition(k,l);

		tokens.resize(current_token_index+1);
		tokens[current_token_index++] = (IToken *)hp;

	}

	// and now we've finished

	lvl.close();

	// stop the loading music
	MusicLib::Stop();

	Begin_Error_Table_Return_Nothing
		Define_Error(1,LEVEL_LOAD_ERROR)
	End_Error_Table_Exit_With_Error(NULL);
}


void CMedia::DestroyTokens(bool destroy_turner)
{
	int start_i = ((true == destroy_turner) ? 0 : 1);
	int i; // loop variable

	for(i = start_i; i < tokens.size(); i++)
	{
		if(NULL == this->tokens[i])
		{
			continue;
		}
		// find the type and delete it respectively
		switch(tokens[i]->GetType())
		{
		case TURNER:
			delete (CTurner *)(tokens[i]);
			break;
		case WALL:
			delete (CWall *)(tokens[i]);
			break;
		case LEVEL_END:
			delete (CLevelEnd *)(tokens[i]);
			break;
		case ENEMY:
			delete (CEnemy *)(tokens[i]);
			break;
		case AMMO:
			delete (CAmmo *)(tokens[i]);
			break;
		case HEALTH_PACK:
			delete (CHealthPack *)(tokens[i]);
			break;
		default:
			assert(false); // we went wrong because there was an invalid token
		}
		tokens[i] = NULL;
	} // endif

	this->tokens.resize(1);
}
