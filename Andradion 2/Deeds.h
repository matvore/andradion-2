// Accomplishments.h: interface for the Accomplishments module
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ACCOMPLISHMENTS_H__9D4F25A1_C2EB_11D4_B6FE_0050040B0541__INCLUDED_)
#define AFX_ACCOMPLISHMENTS_H__9D4F25A1_C2EB_11D4_B6FE_0050040B0541__INCLUDED_

using NGameLib2::CTimer;
using NGameLib2::tstring;

// this class will keep track of the current level
//  by calling CGlue accessors
void DeeInitialize(); // call when no ini file was found
void DeeInitialize(HANDLE file); // the initialize function receives the handle to a file which it should read the data from
void DeeRelease(HANDLE file); // the release function receives the handle to a file which it should write the data to
int  DeeLevelAvailability(int level);
void DeeLevelComplete(FIXEDNUM since_start,int score,int next_level); // call LevelComplete whenever a level is finished
void DeeGetLevelAccomplishments(tstring *lines); // call when a level is started and user needs to know high scores
                                                //  the lines argument will be filled with lines of text
                                                //  the user needs to see

#endif // !defined(AFX_ACCOMPLISHMENTS_H__9D4F25A1_C2EB_11D4_B6FE_0050040B0541__INCLUDED_)
