#include "stdafx.h"
#include "Fixed.h"
#include "SharedConstants.h"
#include "Certifiable.h"
#include "Graphics.h"
#include "Character.h"
#include "Fire.h"
#include "Glue.h"
#include "Deeds.h"
#include "resource.h"

const int MAX_STRINGLEN = 100;
const short WORST_TIME = 60*99+59;
const short WORST_SCORE = -1;

using std::pair;

static int level_availability[NUM_LEVELS];

// the best times
//  first half contains best time in seconds
//  second half contains the score the player
//  ended up getting in that time
static pair<short,short> best_times[NUM_LEVELS*NUM_DIFFICULTYLEVELS];

// the best scores
//  first half contains the best score
//  second half contains the time it took
//  that player to get that score in seconds
static pair<short,short> best_scores[NUM_LEVELS*NUM_DIFFICULTYLEVELS];


void DeeInitialize()
{
	// setup the default best score and times
	for(int i = 0; i < NUM_LEVELS; i++)
	{
		level_availability[i] = LEVELAVAIL_NONE;
		for(int j = 0; j < NUM_DIFFICULTYLEVELS; j++)
		{
			int magic_index = i * NUM_DIFFICULTYLEVELS + j;
			best_scores[magic_index].first = WORST_SCORE;
			best_scores[magic_index].second = WORST_TIME;

			best_times[magic_index].first = WORST_TIME;
			best_times[magic_index].second = WORST_SCORE;
		}
	}

	level_availability[0] = LEVELAVAIL_DANGERDANGER;
}

void DeeInitialize(HANDLE file)
{
	// read our accomplishments from the file
	int i;

	// first read level availability
	for(i = 0; i < NUM_LEVELS; i++)
	{
		DWORD read = 0;
		ReadFile(file,(void *)(level_availability + i),1,&read,NULL);

		if(read < 1)
		{
			// the file was too small and did not contain
			//  all the level data we needed
			// so the default status of the levels are all unavailable
			for(;i < NUM_LEVELS; i++)
			{
				level_availability[i] = LEVELAVAIL_NONE;
			}
		}
	}

	// now read our best times and best scores, in that order
	for(i = 0; i < NUM_LEVELS * NUM_DIFFICULTYLEVELS; i++)
	{
		DWORD read = 0;
		ReadFile(file,(void *)(best_times+i),sizeof(pair<short,short>),&read,NULL);

		if(sizeof(pair<short,short>) == read)
		{
			// we read what we needed to
			ReadFile(file,(void *)(best_scores+i),sizeof(pair<short,short>),&read,NULL);
		}
		else
		{
			best_times[i].first = WORST_TIME;
			best_times[i].second = WORST_SCORE;
		}

		if(sizeof(pair<short,short>) != read)
		{
			// we didn't read all we needed to in one of the
			//  two ReadFile calls, so we know we did not sufficiently
			//  read the best scores.
			best_scores[i].first = WORST_SCORE;
			best_times[i].second = WORST_TIME;
		}
	}
}

void DeeRelease(HANDLE file)
{
	// now we just have to write the data of all our accomplishments
	//  to the specified file.  The program is about to close
	int i;

	// first write the level availabilities
	for(i = 0; i < NUM_LEVELS; i++)
	{
		DWORD written;
		WriteFile(file,(void *)(level_availability+i),1,&written,NULL);
	}

	// now write best times and scores
	for(i = 0; i < NUM_LEVELS * NUM_DIFFICULTYLEVELS; i++)
	{
		DWORD written;
		WriteFile(file,(void *)(best_times+i),sizeof(pair<short,short>),&written,NULL);
		WriteFile(file,(void *)(best_scores+i),sizeof(pair<short,short>),&written,NULL);
	}
}

int DeeLevelAvailability(int level)
{
	// make sure the level parameter is within range
	assert(level >= 0);
	assert(level < NUM_LEVELS);

	return level_availability[level];
}

void DeeLevelComplete(FIXEDNUM since_start,int score,int next_level)
{
	// catch the timer before it has too much time to tick
	//  and it registers an unfair amount of time
	short time = (short)FixedCnvFrom<long>(since_start);

	if(time > WORST_TIME)
	{
		time = WORST_TIME;
	}

	// get the level we're working with
	int level = GLUlevel * NUM_DIFFICULTYLEVELS + GLUdifficulty;

	// see if this guy has the same time as before,
	//  but got a higher score
	if(best_times[level].first == time)
	{
		if(best_times[level].second < score)
		{
			// the score was beaten
			best_times[level].second = (short)score;
		}
	}
	else if(best_times[level].first > time)
	{
		// the time was beaten, and so both the time
		//  and score are overwritten
		best_times[level].first = time;
		best_times[level].second = (short)score;
	}

	// see if this guy has the same score as before,
	//  but got a better time
	if(best_scores[level].first == score)
	{
		if(best_scores[level].second > time)
		{
			// the time was beaten
			best_scores[level].second = time;
		}
	}
	else if(best_scores[level].first < score)
	{
		// the score was beaten, and so both the time
		//  and score are overwritten
		best_scores[level].first = (short)score;
		best_scores[level].second = time;
	}

	// see if we should update the level availability array
	if(next_level < NUM_LEVELS && level_availability[next_level] < GLUdifficulty + 1)
	{
		level_availability[next_level] = GLUdifficulty+1;
	}
}

void DeeGetLevelAccomplishments(string *lines)
{
	// this function will fill the vector of strings with one or more
	//  lines of text which should be presented to the user

	// the first line should be used to summarize all the data if only one
	//  line is available

	// all lines after the first can be used when there is room to output detail

	int level = GLUlevel * NUM_DIFFICULTYLEVELS + GLUdifficulty;

	if(WORST_TIME == best_times[level].first && WORST_SCORE == best_scores[level].first)
	{
		// no records have been set
		GluStrLoad(IDS_NORECORDS,lines[0]);

		lines[2] = lines[1] = lines[0];
	}
	else
	{
		// records have been set
		TCHAR buffer[MAX_STRINGLEN]; // general-purpose buffer

		GluStrLoad(IDS_RECORDSUMMARY,lines[0]); 

		// score and time, in that order:
		wsprintf
		(
			buffer,
			lines[0].c_str(),
			(int)best_scores[level].first,
			(int)(best_times[level].first/SECONDSPERMINUTE),
			(int)(best_times[level].first%SECONDSPERMINUTE)
		);
		lines[0] = buffer;

		// now do the best time line
		GluStrLoad(IDS_BESTTIMEFORMAT,lines[1]);
		wsprintf
		(
			buffer,
			lines[1].c_str(),
			(int)(best_times[level].first/SECONDSPERMINUTE),
			(int)(best_times[level].first%SECONDSPERMINUTE),
			(int)best_times[level].second
		);
		lines[1] = buffer;

		// now do the best score line
		GluStrLoad(IDS_BESTSCOREFORMAT,lines[2]);
		wsprintf
		(
			buffer,
			lines[2].c_str(),
			(int)best_scores[level].first,
			(int)(best_scores[level].second/SECONDSPERMINUTE),
			(int)(best_scores[level].second%SECONDSPERMINUTE)
		);
		lines[2] = buffer;
	}
}
