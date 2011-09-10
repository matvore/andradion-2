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

import java.awt.Graphics;
import java.awt.Point;
import java.awt.image.BufferedImage;
import java.util.EnumMap;
import java.util.List;
import java.util.Map;

import javax.swing.JPanel;

import com.github.matvore.andradion2.data.Copier;
import com.github.matvore.andradion2.data.Lists;

public class LevelEditPanel extends JPanel {
  private static final long serialVersionUID = 1L;

  private final Images images;
  private BufferedImage levelImage;
  private Map<PlaceableItem, List<Point>> placeableItems;

  public LevelEditPanel(Images images) {
    this.images = images;
  }

  public void setLevel(BufferedImage levelImage,
      Map<PlaceableItem, List<Point>> placeableItems) {
    this.levelImage = levelImage;
    setPreferredSize(
        new java.awt.Dimension(levelImage.getWidth(), levelImage.getHeight()));

    this.placeableItems =
        new EnumMap<PlaceableItem, List<Point>>(PlaceableItem.class);
    for (PlaceableItem item : PlaceableItem.values()) {
      if (placeableItems.containsKey(item)) {
        this.placeableItems.put(item,
            Lists.deepCopyOf(placeableItems.get(item), Copier.FOR_POINT));
      } else {
        this.placeableItems.put(item, Lists.<Point>newArrayList());
      }
    }

    revalidate();
    repaint();
  }

  @Override
  protected void paintComponent(Graphics graphics) {
    super.paintComponent(graphics);
    graphics.drawImage(levelImage, 0, 0, null);

    for (PlaceableItem item : placeableItems.keySet()) {
      BufferedImage itemImage = images.imageFor(item);

      for (Point imageLocation : placeableItems.get(item)) {
        graphics.drawImage(itemImage,
            imageLocation.x - itemImage.getWidth() / 2,
            imageLocation.y - itemImage.getHeight() / 2, null);
      }
    }
  }
}
