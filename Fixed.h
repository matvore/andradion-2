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

typedef int FIXEDNUM;

template<class c> inline FIXEDNUM FixedCnvTo(const c& x) {
  return (FIXEDNUM)(x * c(65536));
}

template<class c> inline c FixedCnvFrom(const FIXEDNUM& x) {
  return c(x) / c(65536);
}

FIXEDNUM FixedMul(FIXEDNUM x, FIXEDNUM y);
