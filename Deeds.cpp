/** \file Deeds.cpp
 * Module that maintains a database of the player's accomplishments,
 * including which levels have been beaten at what difficulties,
 * and what the best time and score are for the beaten levels.
 */

#include "StdAfx.h"
#include "Fixed.h"
#include "Deeds.h"

using std::pair;
using std::string;

const int MAX_STRINGLEN = 100;
const short WORST_TIME = 60*99+59;
const short WORST_SCORE = -1;
const unsigned int ACCOMPLISHMENTS_SIZE = 2 * sizeof(pair<short, short>)
  * NUM_LEVELS * NUM_DIFFICULTYLEVELS + NUM_LEVELS;

static int level_availability[NUM_LEVELS];

/** the best times.
 * <ul>
 * <li>first half contains best time in seconds</li>
 * <li>second half contains the score the player
 *     ended up getting in that time.</li>
 * </ul>
 */
static pair<short, short> best_times[NUM_LEVELS*NUM_DIFFICULTYLEVELS];

/** the best scores.
 * <ul>
 * <li>first half contains the best score</li>
 * <li>second half contains the time it took
 *     that player to get that score in seconds</li>
 * </ul>
 */
static pair<short, short> best_scores[NUM_LEVELS*NUM_DIFFICULTYLEVELS];

static inline int GetDeedId(int level, int difficulty) throw() {
  assert(level >= 0 && level < NUM_LEVELS);
  assert(difficulty >= 0 && difficulty < NUM_DIFFICULTYLEVELS);

  return (level * NUM_DIFFICULTYLEVELS) + difficulty;
}

void DeeInitialize() throw() {
  // setup the default best score and times
  for(int i = 0; i < NUM_LEVELS; i++) {
    level_availability[i] = LEVELAVAIL_NONE;
    for(int j = 0; j < NUM_DIFFICULTYLEVELS; j++) {
      int magic_index = i * NUM_DIFFICULTYLEVELS + j;
      best_scores[magic_index].first = WORST_SCORE;
      best_scores[magic_index].second = WORST_TIME;

      best_times[magic_index].first = WORST_TIME;
      best_times[magic_index].second = WORST_SCORE;
    }
  }

  level_availability[0] = LEVELAVAIL_DANGERDANGER;
}

void DeeInitialize(HANDLE file) throw() {
  // read our accomplishments from the file
  BOOL success = TRUE;
  DWORD total_read = 0;

  // first read level availability
  for(int i = 0; i < NUM_LEVELS; i++) {
    DWORD read;
    unsigned char avail;
      
    success = success && ReadFile(file, (void *)&avail, 1, &read, 0);
    
    level_availability[i] = avail;

    total_read += read;

    if (avail > LEVELAVAIL_DANGERDANGER) {
      success = FALSE;
    }
  }

  // now read our best times and best scores, in that order
  for(int i = 0; i < NUM_LEVELS * NUM_DIFFICULTYLEVELS; i++) {
    DWORD read;

    success = success && ReadFile(file, (void *)(best_times+i),
                                  sizeof(pair<short, short>), &read, 0);
    total_read += read;
    success = success && ReadFile(file, (void *)(best_scores+i),
                                  sizeof(pair<short, short>), &read, 0);
    total_read += read;
  }

  // if the expected number of bytes was not read, or if one of the
  //  ReadFile calls failed, set the accomplishments of the player to
  //  nil.
  if (total_read != ACCOMPLISHMENTS_SIZE || !success) {
    DeeInitialize();
  }
}

void DeeRelease(HANDLE file) throw(DeedsWriteException) {
  DWORD total_written = 0;
  BOOL success = TRUE;

  // first write the level availabilities
  for(int i = 0; i < NUM_LEVELS; i++) {
    DWORD written;
    success = success && WriteFile(file, (void *)(level_availability+i),
                                   1, &written, 0);

    total_written += written;
  }

  // now write best times and scores
  for(int i = 0; i < NUM_LEVELS * NUM_DIFFICULTYLEVELS; i++) {
    DWORD written;
    success = success && WriteFile(file, (void *)(best_times+i),
                                   sizeof(pair<short,short>), &written, 0);

    total_written += written;
    
    success = success && WriteFile(file, (void *)(best_scores+i),
                                   sizeof(pair<short,short>), &written, 0);

    total_written += written;
  }

  if (ACCOMPLISHMENTS_SIZE != total_written || !success) {
    throw DeedsWriteException();
  }
}

int DeeLevelAvailability(unsigned int level) throw() {
  // make sure the level parameter is within range
  assert(level < NUM_LEVELS);

  return level_availability[level];
}

void DeeLevelComplete(int level, int difficulty,
                      FIXEDNUM since_start,
                      int score, int next_level) throw() {
  // catch the timer before it has too much time to tick
  //  and it registers an unfair amount of time
  short time = (short)FixedCnvFrom<long>(since_start);
  int deed_id = GetDeedId(level, difficulty);

  if(time > WORST_TIME) {
    time = WORST_TIME;
  }

  // see if this guy has the same time as before,
  //  but got a higher score
  if(best_times[deed_id].first == time) {
    if(best_times[deed_id].second < score) {
      // the score was beaten
      best_times[deed_id].second = (short)score;
    }
  } else if(best_times[deed_id].first > time) {
    // the time was beaten, and so both the time
    //  and score are overwritten
    best_times[deed_id].first = time;
    best_times[deed_id].second = (short)score;
  }

  // see if this guy has the same score as before,
  //  but got a better time
  if(best_scores[deed_id].first == score) {
    if(best_scores[deed_id].second > time) {
      // the time was beaten
      best_scores[deed_id].second = time;
    }
  } else if(best_scores[deed_id].first < score) {
    // the score was beaten, and so both the time
    //  and score are overwritten
    best_scores[deed_id].first = (short)score;
    best_scores[deed_id].second = time;
  }

  // see if we should update the level availability array
  if(next_level < NUM_LEVELS
     && level_availability[next_level] < difficulty + 1) {
    level_availability[next_level] = difficulty+1;
  }
}

bool DeeHasBeatenLevel(int level, int difficulty) throw() {
  int deed_id = GetDeedId(level, difficulty);

  return WORST_TIME != best_times[deed_id].first
    && WORST_SCORE != best_scores[deed_id].first;
}

pair<FIXEDNUM, int> DeeGetBestTimeAndScore(int level,
                                           int difficulty) throw() {
  assert(DeeHasBeatenLevel(level, difficulty));

  int deed_id = GetDeedId(level, difficulty);

  return pair<FIXEDNUM, int>(best_times[deed_id].first,
                             best_scores[deed_id].first);
}

int DeeGetScoreAtBestTime(int level, int difficulty) throw() {
  assert(DeeHasBeatenLevel(level, difficulty));

  return best_times[GetDeedId(level, difficulty)].second;
}

FIXEDNUM DeeGetTimeAtBestScore(int level, int difficulty) throw() {
  assert(DeeHasBeatenLevel(level, difficulty));

  return best_scores[GetDeedId(level, difficulty)].second;
}

