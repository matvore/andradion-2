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

#include <windows.h>

#include <ddraw.h>
#include <dinput.h>
#include <dsound.h>
#include <dmusicc.h>
#include <dmusici.h>
#include <dmusicf.h>
#include <dplay.h>

#include <cassert>
#include <cmath>
#include <fstream>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>

#include <bitset>
#include <list>
#include <queue>
#include <set>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>

typedef int FIXEDNUM;
#define Fixed(x) (FIXEDNUM((double)65536 * double(x)))

const int GAME_MODEWIDTH = 320;
const int GAME_MODEHEIGHT= 200;
const int TILE_WIDTH = 32;
const int TILE_HEIGHT = 30;
const FIXEDNUM FATNESS = Fixed(12);  

enum {WEAPON_PISTOL, WEAPON_MACHINEGUN, WEAPON_BAZOOKA, WEAPON_COUNT};
enum {CHAR_SALLY,
      CHAR_MILTON,
      CHAR_EVILTURNER,
      CHAR_TURNER,
      CHAR_SWITZ,
      CHAR_CHARMIN,
      CHAR_PEPSIONE,
      CHAR_COCACOLACLASSIC,
      CHAR_KOOLAIDGUY,
      CHAR_COUNT};

typedef std::vector<bool> bit_vector;

template <class T>
inline void InitDXStruct(T *obj) {
  memset((void *)obj, 0, sizeof(T));
  obj->dwSize = sizeof(T);
}

template <class T>
void Release(T **obj) {
  if (*obj) {
    (*obj)->Release();
    (*obj) = 0;
  }
}

template <class T> void Delete(T **obj) {delete *obj; *obj = 0;}

template <class T> void DeleteArr(T **obj) {delete[] *obj; *obj = 0;}

inline RECT Rect(int x0, int y0, int x1, int y1) {
  RECT result;

  result.left = x0;
  result.top = y0;
  result.right = x1;
  result.bottom = y1;

  return result;
}

template <class T>
class Array {
protected:
  int size;
  T *array;

public:
  typedef T *Iterator;

  Array() throw() : array(0), size(0) {}
  Array(int size) throw(std::bad_alloc) : array(new T[size]), size(size) {}
  ~Array() throw() {DeleteArr(&array);}

  void Clear() throw() {DeleteArr(&array); array = 0; size = 0;}
  void Resize(int s) throw(std::bad_alloc) {
    DeleteArr(&array);

    size = s;
    array = new T[s];
  }

  Iterator Begin() throw() {return array;}
  Iterator End() throw() {return array + size;}
  
  inline int Size() throw() {return size;}
  inline bool Empty() throw() {return !size;}

  inline T& operator[](int i) throw() {assert(i < size); return array[i];}
  inline T *operator+(int i) throw() {return array + i;}
};

template <class T>
class AutoComPtr {
protected:
  T *ptr;

public:
  AutoComPtr() throw() : ptr(0) {}
  AutoComPtr(const AutoComPtr& acp) throw() : ptr(acp.ptr) {
    if (ptr) {
      ptr->AddRef();
    }
  }

  AutoComPtr(REFCLSID rclsid, REFIID riid) throw() : ptr(0) {
    Create(rclsid, riid);
  }
  ~AutoComPtr() throw() {Reset();}

  void Reset(T *new_ptr = 0) throw() {
    if (ptr != new_ptr) {
      if (ptr) {
        ptr->Release();
      }

      ptr = new_ptr;

      if (ptr) {
        ptr->AddRef();
      }
    }
  }

  T *Release() throw() {T *result = ptr; ptr = 0; return result;}
  T *Get() throw() {return ptr;}
  void Create(REFCLSID rclsid, REFIID riid) throw() {
    HRESULT result;

    Reset();

    result = CoCreateInstance
      (rclsid, 0, CLSCTX_INPROC_SERVER, riid, (void **)&ptr);

    assert(SUCCEEDED(result));
  }

  T **GetPtrToPtr() throw() {return &ptr;}

  AutoComPtr& operator =(const AutoComPtr& rhs) throw() {
    Reset(rhs.ptr);

    return *this;
  }

  T *operator ->() const throw() {assert(ptr); return ptr;}

  operator bool() throw() {return 0 != ptr;}
  bool operator !() throw() {return !ptr;}
};

class Com {
protected:
  bool succeeded;

public:
  Com() throw() {succeeded = SUCCEEDED(CoInitialize(0));}
  ~Com() throw() {if (succeeded) {CoUninitialize();}}

  bool Succeeded() const throw() {return succeeded;}
};
