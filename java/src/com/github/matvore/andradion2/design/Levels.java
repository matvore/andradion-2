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

package com.github.matvore.andradion2.design;

import com.github.matvore.andradion2.data.Closeables;
import com.github.matvore.andradion2.data.Entity;
import com.github.matvore.andradion2.data.Level;
import com.github.matvore.andradion2.data.LevelFormats;
import com.github.matvore.andradion2.data.LevelIndex;

import java.awt.Point;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Arrays;
import java.util.EnumMap;
import java.util.List;
import java.util.Map;

public class Levels {
  private final File levelDirectory;

  private static final Point getDefaultStartPlayerLocation() {
    return new Point(50, 50);
  }

  public Levels(File levelDirectory) {
    this.levelDirectory = levelDirectory;
  }

  public static Levels withLevelDirectory(File levelDirectory) {
    return new Levels(levelDirectory);
  }

  private File levelFile(LevelIndex levelIndex) {
    return new File(levelDirectory, levelIndex.getId() + ".dat");
  }

  private Level getRawLevelData(LevelIndex levelIndex) throws IOException {
    InputStream levelStream = new FileInputStream(levelFile(levelIndex));

    return LevelFormats.fromText(levelStream);
  }

  public Map<PlaceableItem, List<Point>> loadLevelItems(LevelIndex levelIndex)
      throws IOException {
    Level rawLevelData = getRawLevelData(levelIndex);
    Map<PlaceableItem, List<Point>> result =
        new EnumMap<PlaceableItem, List<Point>>(PlaceableItem.class);
    result.put(PlaceableItem.HERO,
        Arrays.asList(rawLevelData.getPlayerStartLocation()));
    for (PlaceableItem item : PlaceableItem.values()) {
      if (item.getEntity() == null) {
        continue;
      }

      result.put(item, rawLevelData.getEntities().get(item.getEntity()));
    }

    return result;
  }

  public void saveLevelItems(
      LevelIndex levelIndex, Map<PlaceableItem, List<Point>> items)
      throws IOException {
    Level rawLevelData = getRawLevelData(levelIndex);
    Level.Builder levelBuilder = Level.newBuilder();
    levelBuilder
        .withIndoorRectangles(rawLevelData.getIndoorRectangles())
        .withLevelEnds(rawLevelData.getLevelEnds())
        .withLevelSize(rawLevelData.getLevelSize())
        .withPalette(rawLevelData.getPalette())
        .withWeatherPattern(rawLevelData.getWeatherPattern());

    List<Point> heroStartList = items.get(PlaceableItem.HERO);
    Point heroStartLocation;
    if (heroStartList == null || heroStartList.isEmpty()) {
      heroStartLocation = getDefaultStartPlayerLocation();
    } else {
      heroStartLocation = heroStartList.get(0);
    }
    levelBuilder.withPlayerStartLocation(heroStartLocation);

    Map<Entity, List<Point>> entities =
        new EnumMap<Entity, List<Point>>(Entity.class);
    for (PlaceableItem item : PlaceableItem.values()) {
      if (item.getEntity() == null) {
        continue;
      }
      entities.put(item.getEntity(), items.get(item));
    }
    levelBuilder.withEntities(entities);
    rawLevelData = levelBuilder.build();

    OutputStream levelStream = null;
    try {
      levelStream = new FileOutputStream(levelFile(levelIndex));
      LevelFormats.toText(levelBuilder.build(), levelStream);
    } finally {
      Closeables.closeQuietly(levelStream);
    }
  }
}
