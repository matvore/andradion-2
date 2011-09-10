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

package com.github.matvore.andradion2.data;

import java.io.Closeable;
import java.io.IOException;

public interface NumberOutputStream extends Closeable {
  void flush() throws IOException;

  void putByte(int x) throws IOException;

  void putBytes(byte[] x) throws IOException;

  void putBytes(byte[] x, int offset, int length) throws IOException;

  void putWord(int x) throws IOException;

  void putDword(int x) throws IOException;

  void putWordThatIsUsuallyByte(int word) throws IOException;

  /**
   * Indicates that a data structure has been completed. This helps formatting
   * in some implementations.
   */
  void endStructure() throws IOException;

  /**
   * Indicates that a large set of data has been completed. This helps
   * formatting in some implementations.
   */
  void endSection() throws IOException;
}
