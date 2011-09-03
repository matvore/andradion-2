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

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Point;
import java.awt.Rectangle;
import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
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
    List<Color> palette = new ArrayList<Color>();
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
    List<Rectangle> indoorRectangles = new ArrayList<Rectangle>();
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
    List<LevelEnd> levelEnds = new ArrayList<LevelEnd>();
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
      List<Point> locations = new ArrayList<Point>();
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

  public static void toBinary(Level level, OutputStream output)
      throws IOException {
    List<Color> palette = level.getPalette();
    writeByte(output, palette.size());
    for (Color color : palette) {
      writeByte(output, color.getRed());
      writeByte(output, color.getGreen());
      writeByte(output, color.getBlue());
    }

    Dimension levelSize = level.getLevelSize();
    writeWord(output, levelSize.width);
    writeWord(output, levelSize.height);

    writeByte(output, level.getWeatherPattern().ordinal());

    Point playerStartLocation = level.getPlayerStartLocation();
    writeWord(output, playerStartLocation.x);
    writeWord(output, playerStartLocation.y);

    List<Rectangle> indoorRectangles = level.getIndoorRectangles();
    writeByte(output, indoorRectangles.size());
    for (Rectangle indoorRectangle : indoorRectangles) {
      writeWord(output, indoorRectangle.x);
      writeWord(output, indoorRectangle.y);
      writeWord(output, indoorRectangle.x + indoorRectangle.width);
      writeWord(output, indoorRectangle.y + indoorRectangle.height);
    }

    List<LevelEnd> levelEnds = level.getLevelEnds();
    writeByte(output, levelEnds.size());
    for (LevelEnd levelEnd : levelEnds) {
      writeByte(output, levelEnd.getToLevel().ordinal());
      writeWord(output, levelEnd.getLocation().x);
      writeWord(output, levelEnd.getLocation().y);
    }

    Map<Entity, List<Point>> entities = level.getEntities();
    for (Entity entityType : Entity.values()) {
      List<Point> locations = entities.get(entityType);
      writeByte(output, locations.size());
      for (Point location : locations) {
        writeWord(output, location.x);
        writeWord(output, location.y);
      }
    }
  }

  private static void writeByte(OutputStream output, int value)
      throws IOException {
    output.write((byte)value);
  }

  private static void writeWord(OutputStream output, int value)
      throws IOException {
    writeByte(output, value & 0xff);
    writeByte(output, value >> 8);
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
