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

class Resource {
protected:
  HGLOBAL data_handle;
  const BYTE *ptr;
  int size;

public:
  class Exception : public std::exception {
  public:
    const std::string message;

    Exception(const std::string& message) throw() : message(message) {}
    virtual ~Exception() throw() {}

    virtual const char *what() const throw() {return message.c_str();}
  };

  Resource(const char *resource_type, const char *resource_name)
    throw(Exception);
  ~Resource() throw();

  const BYTE *GetPtr(int offset = 0) throw() {return ptr + offset;}
  int GetSize() throw() {return size;}
};
