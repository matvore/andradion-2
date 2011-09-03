// the header file for logging (only active in debug mode)

// the TryAndReport macro tries some kind of operation that returns a
// success/failure code, and saves the return value in the log, also
// returnning that value 
#if defined(_DEBUG)

#define LogArg(x) ,(x)
#define WriteLog std::printf

#define TryAndReport(op) TryAndReportB(op, #op)

template <class T> inline T TryAndReportB(T hr, const char *op) {
  std::printf("Result of %s: %x\n", op, (DWORD)hr);
  return hr;
}

#else

#define WriteLog(x)
#define LogArg(x)
#define TryAndReport(op) (op)

#endif
