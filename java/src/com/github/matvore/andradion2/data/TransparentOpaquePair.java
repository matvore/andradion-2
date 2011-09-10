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

import java.io.IOException;

public final class TransparentOpaquePair {
  private final int transparentPixels;
  private final byte[] opaquePixels;

  public TransparentOpaquePair(int transparentPixels, byte[] opaquePixels) {
    this.opaquePixels = opaquePixels;
    this.transparentPixels = transparentPixels;
  }

  public int getTransparentPixels() {
    return transparentPixels;
  }

  public int getOpaquePixelCount() {
    return opaquePixels.length;
  }

  public void writeOpaquePixelsTo(NumberOutputStream stream)
      throws IOException {
    stream.putBytes(opaquePixels);
  }

  public static TransparentOpaquePair readFrom(
      ByteMatrix data, int x, int y, byte transparentColor) {
    int dataIndex = data.index(x, y);
    int transparentPixels = 0;

    while (true) {
      if (x + transparentPixels == data.width) {
        return null;
      }
      if (data.bytes[dataIndex + transparentPixels] != transparentColor) {
        break;
      }
      transparentPixels++;
    }

    int opaquePixels = 0;
    do {
      opaquePixels++;
    } while (x + transparentPixels + opaquePixels < data.width &&
        data.bytes[dataIndex + transparentPixels + opaquePixels] !=
            transparentColor);

    byte[] opaqueData = new byte[opaquePixels];
    System.arraycopy(data.bytes, dataIndex + transparentPixels,
        opaqueData, 0, opaquePixels);

    return new TransparentOpaquePair(transparentPixels, opaqueData);
  }
}
