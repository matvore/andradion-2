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

import com.github.matvore.andradion2.data.Lists;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Point;
import java.awt.Rectangle;
import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.EnumMap;
import java.util.List;
import java.util.Map;

public class LevelFormats {
  private LevelFormats() {
    throw new UnsupportedOperationException("static-only class");
  }

  public static Level fromText(InputStream input)
      throws IOException, NumberFormatException {
    Level.Builder builder = Level.newBuilder();

    int paletteSize = readTextInteger(input);
    List<Color> palette = Lists.newArrayList();
    for (int i = 0; i < paletteSize; i++) {
      int r = readTextInteger(input);
      int g = readTextInteger(input);
      int b = readTextInteger(input);

      palette.add(new Color(r, g, b));
    }
    builder.withPalette(palette);

    int levelWidth = readTextInteger(input);
    int levelHeight = readTextInteger(input);
    builder.withLevelSize(new Dimension(levelWidth, levelHeight));

    int weatherPatternIndex = readTextInteger(input);
    builder.withWeatherPattern(WeatherPattern.values()[weatherPatternIndex]);
    int playerStartX = readTextInteger(input);
    int playerStartY = readTextInteger(input);
    builder.withPlayerStartLocation(new Point(playerStartX, playerStartY));

    int indoorRectangleCount = readTextInteger(input);
    List<Rectangle> indoorRectangles = Lists.newArrayList();
    for (int i = 0; i < indoorRectangleCount; i++) {
      int left = readTextInteger(input);
      int top = readTextInteger(input);
      int right = readTextInteger(input);
      int bottom = readTextInteger(input);

      indoorRectangles.add(
          new Rectangle(left, top, right - left, bottom - top));
    }
    builder.withIndoorRectangles(indoorRectangles);

    int levelEndCount = readTextInteger(input);
    List<LevelEnd> levelEnds = Lists.newArrayList();
    for (int i = 0; i < levelEndCount; i++) {
      int toLevel = readTextInteger(input);
      int levelEndX = readTextInteger(input);
      int levelEndY = readTextInteger(input);

      levelEnds.add(LevelEnd.of(
          LevelIndex.values()[toLevel], new Point(levelEndX, levelEndY)));
    }
    builder.withLevelEnds(levelEnds);

    Map<Entity, List<Point>> entities =
        new EnumMap<Entity, List<Point>>(Entity.class);
    for (Entity entityType : Entity.values()) {
      int count = readTextInteger(input);
      List<Point> locations = Lists.newArrayList();
      for (int i = 0; i < count; i++) {
        int x = readTextInteger(input);
        int y = readTextInteger(input);
        locations.add(new Point(x, y));
      }
      entities.put(entityType, locations);
    }
    builder.withEntities(entities);

    return builder.build();
  }

  public static void toBinary(Level level, OutputStream out)
      throws IOException {
    to(level, BinaryNumberOutputStream.of(out));
  }

  public static void toText(Level level, OutputStream out)
      throws IOException {
    to(level, TextNumberOutputStream.of(out));
  }

  private static void to(Level level, NumberOutputStream out)
      throws IOException {
    List<Color> palette = level.getPalette();
    out.putByte(palette.size());
    for (Color color : palette) {
      out.putByte(color.getRed());
      out.putByte(color.getGreen());
      out.putByte(color.getBlue());
    }

    Dimension levelSize = level.getLevelSize();
    out.putWord(levelSize.width);
    out.putWord(levelSize.height);

    out.putByte(level.getWeatherPattern().ordinal());

    Point playerStartLocation = level.getPlayerStartLocation();
    out.putWord(playerStartLocation.x);
    out.putWord(playerStartLocation.y);

    List<Rectangle> indoorRectangles = level.getIndoorRectangles();
    out.putByte(indoorRectangles.size());
    for (Rectangle indoorRectangle : indoorRectangles) {
      out.putWord(indoorRectangle.x);
      out.putWord(indoorRectangle.y);
      out.putWord(indoorRectangle.x + indoorRectangle.width);
      out.putWord(indoorRectangle.y + indoorRectangle.height);
    }

    List<LevelEnd> levelEnds = level.getLevelEnds();
    out.putByte(levelEnds.size());
    for (LevelEnd levelEnd : levelEnds) {
      out.putByte(levelEnd.getToLevel().ordinal());
      out.putWord(levelEnd.getLocation().x);
      out.putWord(levelEnd.getLocation().y);
    }

    Map<Entity, List<Point>> entities = level.getEntities();
    for (Entity entityType : Entity.values()) {
      List<Point> locations = entities.get(entityType);
      out.putByte(locations.size());
      for (Point location : locations) {
        out.putWord(location.x);
        out.putWord(location.y);
      }
    }
  }

  private static int readTextInteger(InputStream input)
      throws IOException, NumberFormatException {
    int totalValue, currentChar;

    boolean negative = false;

    // Skip whitespace
    while (true) {
      currentChar = input.read();
      if (currentChar == -1) {
        throw new EOFException();
      }
      if (currentChar <= ' ') {
        continue;
      }
      if (currentChar == '-') {
        negative = !negative;
      }
      if (currentChar >= '0' && currentChar <= '9') {
        totalValue = currentChar - '0';
        break;
      }
      throw new NumberFormatException(
          "Unexpected character: " + (char)currentChar);
    }

    while (true) {
      currentChar = input.read();
      if (currentChar <= ' ') {
        break;
      }
      if (currentChar >= '0' && currentChar <= '9') {
        totalValue *= 10;
        totalValue += currentChar - '0';
        continue;
      }
      throw new NumberFormatException(
          "Unexpected character: " + (char)currentChar);
    }

    if (negative) {
      totalValue = -totalValue;
    }

    return totalValue;
  }
}
