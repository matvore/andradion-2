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
import java.util.Collections;
import java.util.EnumMap;
import java.util.List;
import java.util.Map;

public class Level {
  private List<Color> palette;
  private Dimension levelSize;
  private WeatherPattern weatherPattern;
  private Point playerStartLocation;
  private List<Rectangle> indoorRectangles;
  private List<LevelEnd> levelEnds;
  private Map<Entity, List<Point>> entities;

  private static Map<Entity, List<Point>> copyEntities(
      Map<Entity, List<Point>> entities) {
    Map<Entity, List<Point>> result =
        new EnumMap<Entity, List<Point>>(Entity.class);
    for (Map.Entry<Entity, List<Point>> group : entities.entrySet()) {
      List<Point> locationList = ImmutableList.copyOf(
          group.getValue(), Copier.FOR_POINT);
      result.put(group.getKey(), locationList);
    }
    return Collections.unmodifiableMap(result);
  }

  private Level(
      List<Color> palette,
      Dimension levelSize,
      WeatherPattern weatherPattern,
      Point playerStartLocation,
      List<Rectangle> indoorRectangles,
      List<LevelEnd> levelEnds,
      Map<Entity, List<Point>> entities) {
    this.palette = palette;
    this.levelSize = levelSize;
    this.weatherPattern = weatherPattern;
    this.playerStartLocation = playerStartLocation;
    this.indoorRectangles = indoorRectangles;
    this.levelEnds = levelEnds;
    this.entities = entities;
  }

  public List<Color> getPalette() {
    return palette;
  }

  public Dimension getLevelSize() {
    return levelSize;
  }

  public WeatherPattern getWeatherPattern() {
    return weatherPattern;
  }

  public Point getPlayerStartLocation() {
    return (Point)playerStartLocation.clone();
  }

  public List<Rectangle> getIndoorRectangles() {
    return indoorRectangles;
  }

  public List<LevelEnd> getLevelEnds() {
    return levelEnds;
  }

  public Map<Entity, List<Point>> getEntities() {
    return entities;
  }

  public static Builder newBuilder() {
    return new Builder();
  }

  public static class Builder {
    private List<Color> palette;
    private Dimension levelSize;
    private WeatherPattern weatherPattern;
    private Point playerStartLocation;
    private List<Rectangle> indoorRectangles;
    private List<LevelEnd> levelEnds;
    private Map<Entity, List<Point>> entities;

    private Builder() {
      // Do nothing
    }

    public Builder withPalette(List<Color> palette) {
      this.palette = ImmutableList.copyOf(palette);
      return this;
    }

    public Builder withLevelSize(Dimension levelSize) {
      this.levelSize = (Dimension)levelSize.clone();
      return this;
    }

    public Builder withWeatherPattern(WeatherPattern weatherPattern) {
      this.weatherPattern = weatherPattern;
      return this;
    }

    public Builder withPlayerStartLocation(Point playerStartLocation) {
      this.playerStartLocation = (Point)playerStartLocation.clone();
      return this;
    }

    public Builder withIndoorRectangles(List<Rectangle> indoorRectangles) {
      this.indoorRectangles = ImmutableList.copyOf(
          indoorRectangles, Copier.FOR_RECTANGLE);
      return this;
    }

    public Builder withLevelEnds(List<LevelEnd> levelEnds) {
      this.levelEnds = ImmutableList.copyOf(levelEnds);
      return this;
    }

    public Builder withEntities(Map<Entity, List<Point>> entities) {
      this.entities = copyEntities(entities);
      return this;
    }

    public Level build() {
      return new Level(
          palette,
          levelSize,
          weatherPattern,
          playerStartLocation,
          indoorRectangles,
          levelEnds,
          entities);
    }
  }
}
