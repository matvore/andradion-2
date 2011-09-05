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

#if defined(_DEBUG)

extern std::ofstream logger;

// the TryAndReport macro tries some kind of operation that returns a
// success/failure code, and saves the return value in the log, also
// returning that value
#define TryAndReport(op) TryAndReportB(op, #op)

template <class T> inline T TryAndReportB(T hr, const char *op) {
  std::ofstream::fmtflags flags = logger.flags();
  logger << "Result of " << op << ": " << std::hex << (DWORD)hr << std::endl;
  logger.flags(flags);
  return hr;
}

#else

class IgnoredLog {
public:
  template <class T>
  inline IgnoredLog& operator <<(const T& value) {
    // Do nothing
    return *this;
  }

  inline IgnoredLog& operator <<(std::ostream& (*func)(std::ostream&)) {
    // Do nothing
    return *this;
  }
};

extern IgnoredLog logger;

#define TryAndReport(op) (op)

#endif
