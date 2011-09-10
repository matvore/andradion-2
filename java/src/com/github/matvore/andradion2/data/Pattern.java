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

import java.awt.Point;
import java.io.IOException;
import java.util.List;

public final class Pattern {
  private final ByteMatrix pixelData;
  private final List<Point> occurrences;

  public Pattern(ByteMatrix pixelData, List<Point> occurrences) {
    this.pixelData = pixelData.clone();
    this.occurrences = ImmutableList.copyOf(occurrences, Copier.FOR_POINT);
  }

  public void writePixelDataTo(NumberOutputStream output) throws IOException {
    output.putBytes(pixelData.bytes);
  }

  public int getWidth() {
    return pixelData.width;
  }

  public int getHeight() {
    return pixelData.height;
  }

  public List<Point> getOccurrences() {
    return occurrences;
  }
}
