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

#include "StdAfx.h"
#include "BitMatrix.h"

using std::auto_ptr;

class BitOffset {
public:
  BitOffset(int byte_index, int bit_index)
      : byte_index(byte_index), bit_index(bit_index) {}

  const int byte_index, bit_index;

  static BitOffset of(int width, int x, int y) {
    int bit_index = x + y * width;
    int byte_index = bit_index / 8;
    bit_index %= 8;
    return BitOffset(byte_index, bit_index);
  }
};

auto_ptr<CBitMatrix> CBitMatrix::forDimensions(int width, int height) {
  int bytes_required = (width * height + 7) / 8;
  char *data = new char[bytes_required];
  memset(data, 0, bytes_required);
  return auto_ptr<CBitMatrix>(new CBitMatrix(width, data));
}

CBitMatrix::~CBitMatrix() {
  delete [] data;
}

void CBitMatrix::set(int x, int y) {
  BitOffset offset = BitOffset::of(width, x, y);
  data[offset.byte_index] |= 1 << offset.bit_index;
}

bool CBitMatrix::get(int x, int y) const {
  BitOffset offset = BitOffset::of(width, x, y);
  return 0 != (data[offset.byte_index] & (1 << offset.bit_index));
}
