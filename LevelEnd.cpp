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
#include "LevelEnd.h"

const FIXEDNUM LEVELEND_FATNESS = Fixed(20);

bool CLevelEnd::Collides(FIXEDNUM tx, FIXEDNUM ty) const {
   return abs(tx - x) < LEVELEND_FATNESS && abs(ty - y) < LEVELEND_FATNESS;
}
