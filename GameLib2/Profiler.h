#ifndef _D2AAB720_7C57_11d5_B6FE_0050040B0541_INCLUDED_
#define _D2AAB720_7C57_11d5_B6FE_0050040B0541_INCLUDED_

// header for an in-game profiler
#ifdef _DEBUG
	using NGameLib2::tstring;
	using std::vector;
	void StartProfileFrame();
	void GetProfileData(vector<tstring>&); // call every frame
	void InitializeProfiler(DWORD); // pass the size of the profile data array (will grow automatically when necessary)

	struct ProfileSample;

	class CProfile
	{
	public:
		CProfile(DWORD,const TCHAR *); // pass the index (we are using char not TCHAR because this is only used in debug version)
		~CProfile();
	private:
		ProfileSample *data; // contains a pointer to the sample data
	};

	#define BeginProfile(x) { CProfile cp##x(x,TEXT(#x))
	#define EndProfile() }
#else
	// not debugging; will not use profiler
	#define StartProfileFrame()
	#define GetProfileData(x)
	#define InitializeProfiler(x) 
	#define BeginProfile(x) {
	#define EndProfile() }
#endif

#endif