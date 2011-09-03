void DeeInitialize();
void DeeInitialize(HANDLE file); 
void DeeRelease(HANDLE file); 
void DeeLevelComplete(FIXEDNUM since_start,int score,int next_level); 
void DeeGetLevelAccomplishments(string *lines);
int DeeLevelAvailability(int level);
