// the header file for logging (only active in debug mode)
#if !defined(_2AF88E00_3281_11d5_B6FE_0050040B0541_INCLUDED_)
#define      _2AF88E00_3281_11d5_B6FE_0050040B0541_INCLUDED_

// the TryAndReport macro tries some kind of operation that returns a success/failure code, and
// saves the return value in the log, also returnning that value
#if defined(_DEBUG)

#define LogArg(x) ,(x)
void BeginLog(const char *filename_);
void WriteLog(const char *lpszText,...);

bool LogEnabled();
void EnableLog(bool on_or_off);
	
#define TryAndReport(op) TryAndReportB(op,#op)
template <class T> inline T TryAndReportB(T hr,const char *op)
{
  WriteLog("Result of %s: %x",op,(DWORD)hr);
  return hr;
}

#else
#define BeginLog(x)
#define WriteLog(x)
#define LogArg(x)

#define LogEnabled() ((bool)false)
#define EnableLog(x)

// this function tries some kind of operation that returns a success/failure code
#define TryAndReport(op) (op)
#endif 

#endif
