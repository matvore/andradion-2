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
#include "Fixed.h"

FIXEDNUM FixedDiv(FIXEDNUM x,FIXEDNUM y) {
  long long lowerPart, higherPart, full;

  lowerPart = (long long)(x << 16) & (long long)0xffff0000;
  higherPart = ((long long)(x) << 32) & (long long)0xffff000000000000;
  full = lowerPart | higherPart;

  full /= y;

  return (FIXEDNUM)(full >> 16);
}

FIXEDNUM FixedMul(FIXEDNUM x,FIXEDNUM y) {
  FIXEDNUM r;
  long long longLong = x;
  x *= y;
  x >>= 16;
  return (FIXEDNUM)x;
}
