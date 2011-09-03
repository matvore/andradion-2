#ifdef _DEBUG

void StartProfileFrame();

void GetProfileData(std::vector<std::string>&);

// pass the size of the profile data array
//  (will grow automatically when necessary)
void InitializeProfiler(DWORD); 

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
