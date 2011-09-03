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

package com.github.matvore.andradion2.build;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.image.BufferedImage;
import java.io.Closeable;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

public class CompacterMapper {
  private CompacterMapper() {
    throw new UnsupportedOperationException("static-only class");
  }

  private static List<Rectangle> uniformDimensionRectangleList(
      List<Point> locations, Dimension dimension) {
    List<Rectangle> result = new ArrayList<Rectangle>();
    for (Point location : locations) {
      result.add(new Rectangle(location, dimension));
    }
    return result;
  }

  public static void compact(
      BufferedImage source, List<Color> palette, OutputStream output,
      int minimumColorBlockArea, List<Dimension> patternAreas,
      int minimumPatternArea) throws IOException {
  }
}
