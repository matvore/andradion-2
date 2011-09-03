#ifdef _DEBUG

using std::vector;
void StartProfileFrame();
void GetProfileData(vector<string>&); // call every frame
void InitializeProfiler(DWORD); // pass the size of the profile data array (will grow automatically when necessary)

struct ProfileSample;

class CProfile {
 public:
  CProfile(DWORD, const char *); // pass the index and name
  ~CProfile();
 private:
  ProfileSample *data; // contains a pointer to the sample data
};

#define BeginProfile(x) { CProfile cp##x(x,#x)
#define EndProfile() }

#else

#define StartProfileFrame()
#define GetProfileData(x)
#define InitializeProfiler(x) 
#define BeginProfile(x) {
#define EndProfile() }

#endif

