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
#include "Resource.h"

using namespace std;

Resource::Resource(const char *resource_type, const char *resource_name)
  throw(Exception) {
  HRSRC res_handle = FindResource(0, resource_name, resource_type);

  if (!res_handle) {
    throw Exception(string("Could not find resource of type ")
                    + resource_type);
  }

  data_handle = LoadResource(0, res_handle);

  if (!data_handle) {
    throw Exception(string("Could not load resource of type ")
                    + resource_type);
  }

  ptr = (const BYTE *)LockResource(data_handle);

  if (!ptr) {
    throw Exception(string("Could not lock resource of type ")
                    + resource_type);
  }

  size = SizeofResource(0, res_handle);

  if (!size) {
    throw Exception(string("Could not find resource size of resource type ")
                           + resource_type);
  }
}

Resource::~Resource() throw() {FreeResource(data_handle);}
