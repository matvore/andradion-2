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

import com.github.matvore.andradion2.data.Copier;
import com.github.matvore.andradion2.data.ImmutableList;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

public class CompacterMapper {
  private static class ByteMatrix {
    public byte[] bytes;
    public int width;

    public int index(int x, int y) {
      return y * width + x;
    }

    public static ByteMatrix ofSize(int width, int height) {
      ByteMatrix result = new ByteMatrix();
      result.bytes = new byte[width * height];
      result.width = width;
      return result;
    }
  }

  private final List<Color> palette;
  private final byte transparentColor;
  private final int minimumColorBlockArea;
  private final List<Dimension> patternAreas;
  private final int minimumPatternArea;

  private CompacterMapper(List<Color> palette, byte transparentColor,
      int minimumColorBlockArea, List<Dimension> patternAreas,
      int minimumPatternArea) {
    this.palette = palette;
    this.transparentColor = transparentColor;
    this.minimumColorBlockArea = minimumColorBlockArea;
    this.patternAreas = patternAreas;
    this.minimumPatternArea = minimumPatternArea;
  }

  public static CompacterMapper of(List<Color> palette, byte transparentColor,
      int minimumColorBlockArea, List<Dimension> patternAreas,
      int minimumPatternArea) {
    return new CompacterMapper(ImmutableList.copyOf(palette),
        transparentColor, minimumColorBlockArea,
        ImmutableList.copyOf(patternAreas, Copier.FOR_DIMENSION),
        minimumPatternArea);
  }

  private static List<Rectangle> uniformDimensionRectangleList(
      List<Point> locations, Dimension dimension) {
    List<Rectangle> result = new ArrayList<Rectangle>();
    for (Point location : locations) {
      result.add(new Rectangle(location, dimension));
    }
    return result;
  }

  private static void drawRectangle(
      ByteMatrix data, byte color, Rectangle area) {
    int index = data.index(area.x, area.y);
    for (int y = 0; y < area.height; y++) {
      for (int x = 0; x < area.width; x++) {
        data.bytes[index + x] = color;
      }
      index += data.width;
    }
  }

  private static ByteMatrix makeCopy(ByteMatrix source, Rectangle area) {
    ByteMatrix result = ByteMatrix.ofSize(area.width, area.height);
    int sourceIndex = source.index(area.x, area.y);
    int resultIndex = 0;
    for (int y = 0; y < area.height; y++) {
      System.arraycopy(
          source.bytes, sourceIndex, result.bytes, resultIndex, area.width);
      resultIndex += result.width;
      sourceIndex += source.width;
    }
    return result;
  }

  private static boolean allOneColor(
      ByteMatrix data, Rectangle area, byte color) {
    int index = data.index(area.x, area.y);
    for (int y = 0; y < area.height; y++) {
      for (int x = 0; x < area.width; x++) {
        if (data.bytes[index + x] != color) {
          return false;
        }
      }
      index += data.width;
    }
    return true;
  }

  /**
   * Finds rectangles of the color at startCoor and puts them in a
   * list of {@code Rectangles}. Will only return rectangles whose upper-left
   * coordinates are after or at startCoor, and replacing them with
   * {@code transparentColor} at the same time.  Ignores rectangles whose areas
   * are less than {@code minimumColorBlockArea}.
   */
  private static List<Rectangle> findAndClearColorBlocks(
      ByteMatrix data, Point startCoor) {
    return null;
  }

  public static void compact(BufferedImage source, OutputStream output)
      throws IOException {
  }
}