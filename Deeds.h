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

/** Indicates that <tt>DeeRelease()</tt> failed to write the Deeds
 * file.
 */
class DeedsWriteException : public std::exception {
public:
  virtual const char *what() const throw() {
    return "The Deeds file could not be written.";
  }
};

/** The number of levels in the game.
 */
const int NUM_LEVELS = 11;

/** The number of difficulty levels available.
 * Every level can be played at any difficulty level.
 */
const int NUM_DIFFICULTYLEVELS = 3;

/** Indicate the amount of availability given to a level, according
 * to what the player has already accomplished.
 */
enum {LEVELAVAIL_NONE,
      LEVELAVAIL_DANGNABIT,
      LEVELAVAIL_MYDEARCHILD,
      LEVELAVAIL_DANGERDANGER};

/** Puts the Deeds module into the initialized state, setting the all
 * the player's accomplishments to nil.
 */
void DeeInitialize() throw();

/** Puts the Deeds module into the initialized state, loading the
 * player's accomplishments from the given file.
 * If there is any error in reading the given file, then the
 * <tt>DeeInitialize()</tt> function is called to set the player's
 * accomplishments to nil.
 * @param file the file from which to load the player's accomplishments.
 */
void DeeInitialize(HANDLE file) throw(); 

/** Writes the data of all our accomplishments to the specified file
 * and puts the Deeds module into the uninitialized state.
 * @param file an output stream opened through the Windows API to which 
 *  the deeds information will be written.
 */
void DeeRelease(HANDLE file) throw(DeedsWriteException);

void DeeLevelComplete(int level, int difficulty,
                      FIXEDNUM since_start,
                      int score, int next_level) throw();

/** Determines if the player has ever beaten the given level at the
 * given difficulty. If the player has beaten the level at a harder difficulty
 * than specified, this function returns false.
 * @param level the level to see if the player ever won.
 * @param difficulty the difficulty at which to see if the player ever won.
 * @return true iff the player has beat the given level at exactly the given
 *  difficulty.
 */
bool DeeHasBeatenLevel(int level, int difficulty) throw();

/** Finds the best time and score achieved in the given level at the given
 * difficulty. It is required that <tt>DeeHasBeatenLevel(int, int)</tt> returns
 * true for the given level and difficulty.
 * @param level the level for which the best time and score will
 *  be determined.
 * @param difficulty the difficulty for which the best time and
 *  score will be determined.
 * @return a fixed-precision decimal and an integer indicating the best time
 *  and the best score, respectively. If the given level was beaten more than
 *  once at the given difficulty, then the best score and best time may have
 *  been achieved on different occasions.
 */
std::pair<FIXEDNUM, int> DeeGetBestTimeAndScore(int level,
                                                int difficulty) throw();

/** Finds the score the player earned when he set the best time record for
 * the given level and difficulty.
 * @param level the level to check.
 * @param difficulty the difficulty at which to check.
 * @return the score earned when the player set the best time record.
 */
int DeeGetScoreAtBestTime(int level, int difficulty) throw();

/** Finds the time taken to earn the best score the player ever earned.
 * If the best score had been attained more than once, than the time
 * returned is the best ever attained at that score.
 * @param level the level to check.
 * @param difficulty the difficulty to check.
 * @return the time taken to earn the best score the player ever
 *  earned.
 */
FIXEDNUM DeeGetTimeAtBestScore(int level, int difficulty) throw();

int DeeLevelAvailability(unsigned int level) throw();
