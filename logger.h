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

template <class T> inline T LogResult(const char *description, T opResult) {
  std::ofstream::fmtflags flags = logger.flags();
  logger << description << ": 0x" << std::hex << opResult << std::endl;
  logger.flags(flags);
  return opResult;
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

template <class T> inline T LogResult(const char *, T opResult) {
  // Do nothing
  return opResult;
}

#endif
