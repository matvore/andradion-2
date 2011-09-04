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
import java.io.OutputStream;

public class BinaryOutputStream implements Closeable {
  private final OutputStream output;

  private BinaryOutputStream(OutputStream output) {
    this.output = output;
  }

  public static BinaryOutputStream of(OutputStream output) {
    return new BinaryOutputStream(output);
  }

  public void putByte(int x) throws IOException {
    output.write((byte)x);
  }

  public void putBytes(byte[] x) throws IOException {
    output.write(x);
  }

  public void putBytes(byte[] x, int offset, int length) throws IOException {
    output.write(x, offset, length);
  }

  public void putWord(int x) throws IOException {
    putByte(x);
    putByte(x >> 8);
  }

  public void putDword(int x) throws IOException {
    putWord(x);
    putWord(x >> 16);
  }

  public void putWordThatIsUsuallyByte(int word) throws IOException {
    if (word == 0 || (word & 0xff00) != 0) {
      putByte(0);
      putWord(word);
    } else {
      putByte(word);
    }
  }

  @Override
  public void close() {
    try {
      output.close();
    } catch (IOException e) {
      // Ignore.
    }
  }
}
