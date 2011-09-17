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

class Buffer {
  static inline void *allocate(unsigned int size) throw(std::bad_alloc) {
    void *result = std::malloc(size);

    if (!result && size) {
      throw std::bad_alloc();
    }

    return result;
  }

  unsigned int size;
  void *buf;

public:
  Buffer() throw() : size(0), buf(0) {}

  Buffer(const void *src, unsigned int size) throw(std::bad_alloc)
    : size(size), buf(allocate(size)) {
    std::memcpy(buf, src, size);
  }

  Buffer(unsigned int size) throw(std::bad_alloc)
    : size(size), buf(allocate(size)) { }

  Buffer(const Buffer& b) throw(std::bad_alloc)
    : buf(allocate(b.size)), size(b.size) {
    std::memcpy(buf, b.buf, size);
  }

  const void *Get() const throw() {return buf;}
  void *Get() throw() {return buf;}
  unsigned int Size() const throw() {return size;}

  Buffer& operator=(const Buffer& rhs) throw(std::bad_alloc) {
    free(buf);
    buf = allocate(rhs.size);

    size = rhs.size;

    std::memcpy(buf, rhs.buf, size);

    return *this;
  }

  void Reallocate(unsigned int s) throw(std::bad_alloc) {
    void *old_buf = buf;
    buf = allocate(s);

    std::memcpy(buf, old_buf, s < size ? s : size);

    size = s;

    free(old_buf);
  }

  ~Buffer() throw() {free(buf);}
};
