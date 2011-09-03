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
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class CompacterMapper {
  private static final class ByteMatrix implements Cloneable {
    public final byte[] bytes;
    public final int width, height;

    public ByteMatrix(int width, int height) {
      bytes = new byte[width * height];
      this.width = width;
      this.height = height;
    }

    public int index(int x, int y) {
      return y * width + x;
    }

    @Override
    public ByteMatrix clone() {
      ByteMatrix copy = new ByteMatrix(width, height);
      System.arraycopy(bytes, 0, copy.bytes, 0, bytes.length);
      return copy;
    }
  }

  private enum ColorThat {IS, IS_NOT}

  private final byte transparentColor;
  private final int minimumColorBlockArea;
  private final List<Dimension> patternAreas;
  private final int minimumPatternArea;

  private CompacterMapper(byte transparentColor,
      int minimumColorBlockArea, List<Dimension> patternAreas,
      int minimumPatternArea) {
    this.transparentColor = transparentColor;
    this.minimumColorBlockArea = minimumColorBlockArea;
    this.patternAreas = patternAreas;
    this.minimumPatternArea = minimumPatternArea;
  }

  public static CompacterMapper of(byte transparentColor,
      int minimumColorBlockArea, List<Dimension> patternAreas,
      int minimumPatternArea) {
    return new CompacterMapper(transparentColor, minimumColorBlockArea,
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
    ByteMatrix result = new ByteMatrix(area.width, area.height);
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

  private static boolean areaIs(ColorThat target, byte color,
      ByteMatrix data, Rectangle area) {
    boolean xor = (target == ColorThat.IS_NOT);
    int index = data.index(area.x, area.y);
    for (int y = 0; y < area.height; y++) {
      for (int x = 0; x < area.width; x++) {
        if ((data.bytes[index + x] != color) ^ xor) {
          return false;
        }
      }
      index += data.width;
    }
    return true;
  }

  /**
   * Finds rectangles of the color at {@code startX, startY} and puts them in a
   * list of {@code Rectangles}. Will only return rectangles whose upper-left
   * coordinates are after or at the starting coordinates, and replacing them
   * with {@code transparentColor} at the same time.  Ignores rectangles whose
   * areas are less than {@code minimumColorBlockArea}.
   */
  private List<Rectangle> findAndClearColorBlocks(
      ByteMatrix data, int startX, int startY) {
    List<Rectangle> result = new ArrayList<Rectangle>();
    int currX = startX, currY = startY;
    int dataPosition = data.index(currX, currY);
    byte baseColor = data.bytes[dataPosition];
    Rectangle scanningArea = new Rectangle();

    for (; dataPosition < data.bytes.length; dataPosition++) {
      if (baseColor == data.bytes[dataPosition]) {
        scanningArea.x = currX;
        scanningArea.width = 1;
        scanningArea.y = currY;
        scanningArea.height = 1;

        // Expand to the right.
        do {
          scanningArea.x++;
        } while (scanningArea.x < data.width &&
            areaIs(ColorThat.IS, baseColor, data, scanningArea));

        scanningArea.width = scanningArea.x - currX;
        scanningArea.x = currX;

        // Expand downward.
        do {
          scanningArea.y++;
        } while (scanningArea.y < data.height &&
            areaIs(ColorThat.IS, baseColor, data, scanningArea));

        scanningArea.height = scanningArea.y - currY;
        scanningArea.y = currY;

        if (scanningArea.width * scanningArea.height >= minimumColorBlockArea) {
          result.add(scanningArea);
          drawRectangle(data, transparentColor, scanningArea);
          scanningArea = new Rectangle();
        }
      }

      // Increment x and wrap around if appropriate.
      if (++currX >= data.width) {
        currX = 0;
        currY++;
      }
    }
    return result;
  }

  /**
   * Returns {@code true} iff two areas have the same pixel data.
   */
  private static boolean sameData(
      ByteMatrix data, Dimension dimension, Point coors1, Point coors2) {
    int index1 = data.index(coors1.x, coors1.y);
    int index2 = data.index(coors2.x, coors2.y);
    int changingRows = data.width - dimension.width;

    for (int y = 0; y < dimension.height; y++) {
      for (int x = 0; x < dimension.width; x++) {
        if (data.bytes[index1++] != data.bytes[index2++]) {
          return false;
        }
      }
      index1 += changingRows;
      index2 += changingRows;
    }

    return true;
  }

  private static boolean sameAndSeparate(
      ByteMatrix data, Dimension dimension, List<Point> coors) {
    // compare similar data
    Point first = coors.get(0);
    for (int i = coors.size() - 1; i >= 1; i--) {
      Point second = coors.get(i);
      if (!sameData(data, dimension, first, second)) {
        return false;
      }
    }

    // look for overlapping (the first coor does not overlap at all)
    for (int c1 = 0; c1 < coors.size(); c1++) {
      Point c1Coor = coors.get(c1);
      int c1Left = c1Coor.x;
      int c1Right = c1Coor.x + dimension.width;
      int c1Top = c1Coor.y;
      int c1Bottom = c1Coor.y + dimension.height;

      for (int c2 = c1 + 1; c2 < coors.size(); c2++) {
        Point c2Coor = coors.get(c2);

        if (!((c1Left >= c2Coor.x + dimension.width) ||
            (c1Right <= c2Coor.x) ||
            (c1Top >= c2Coor.y + dimension.height) ||
            (c1Bottom <= c2Coor.y))) {
          return false;
        }
      }
    }

    return true;
  }

  private static Point findPixel(ColorThat target, byte color,
      ByteMatrix data, int startX, int startY) {
    int dataIndex = data.index(startX, startY);
    int coorX = startX, coorY = startY;
    boolean xor = target == ColorThat.IS_NOT;

    while (true) {
      if ((data.bytes[dataIndex] == color) ^ xor) {
        return new Point(coorX, coorY);
      }

      // Increment current position.
      if (++coorX >= data.width) {
        coorX = 0;
        if (++coorY >= data.height) {
          return null;
        }
      }

      dataIndex++;
    }
  }

  public void compact(BufferedImage source, List<Color> palette,
      OutputStream output) throws IOException {
    int width = source.getWidth(), height = source.getHeight();
    if (width > 256 || height > 256) {
      throw new IllegalArgumentException(
          "Bitmap is too large; dimensions must be <= 256: " +
          width + ", " + height);
    }
    if (palette.isEmpty() || palette.size() > 256) {
      throw new IllegalArgumentException(
          "palette is too large; must be in the range of [1, 256]: " +
          palette.size());
    }

    // Translate bitmap data into 8-bit palette entries.
    int[] rgbRow = new int[width];
    ByteMatrix eightBitImage = new ByteMatrix(width, height);
    int pixelIndex = 0;
    for (int y = 0; y < height; y++) {
      source.getRGB(0, y, width, 1, rgbRow, 0, width);
      for (int x = 0; x < width; x++, pixelIndex++) {
        int r = (rgbRow[x] >> 16) & 0xff;
        int g = (rgbRow[x] >> 8) & 0xff;
        int b = rgbRow[x] & 0xff;
        int bestMatch = -1, closeness = Integer.MAX_VALUE;

        int i = 0;
        for (Color thisColor : palette) {
          int thisCloseness =
              Math.abs(r - thisColor.getRed()) +
              Math.abs(g - thisColor.getGreen()) +
              Math.abs(b - thisColor.getBlue());

          if (thisCloseness < closeness) {
            bestMatch = i;
            closeness = thisCloseness;
            if (closeness == 0) {
              break;
            }
          }
          i++;
        }
        eightBitImage.bytes[pixelIndex++] = (byte)bestMatch;
      }
    }
    compact(eightBitImage, output);
  }

  private void compact(ByteMatrix data, OutputStream output)
      throws IOException {
    Map<Byte, List<Rectangle>> colorBlocks =
        new HashMap<Byte, List<Rectangle>>();
    List<ByteMatrix> patterns = new ArrayList<ByteMatrix>();
    List<List<Rectangle>> patternAreas = new ArrayList<List<Rectangle>>();
    data = data.clone();

    // Search for color blocks.

    int locX = 0, locY = 0;

    // Contains all checked pixel colors.
    Point nonTransparentCoor;

    while ((nonTransparentCoor = findPixel(
        ColorThat.IS_NOT, transparentColor, data, locX, locY)) != null) {
      Byte colorHere = data.bytes[
          data.index(nonTransparentCoor.x, nonTransparentCoor.y)];

      // Only process if we have not processed this color before.
      if (!colorBlocks.containsKey(colorHere)) {
        List<Rectangle> colorBlockAreas = findAndClearColorBlocks(
           data, nonTransparentCoor.x, nonTransparentCoor.y);

        colorBlocks.put(colorHere, colorBlockAreas);
      }

      if (++locX >= data.width) {
        locX = 0;
        if (++locY >= data.height) {
          break;
        }
      }
    }

    // Find patterns.
  }
}
