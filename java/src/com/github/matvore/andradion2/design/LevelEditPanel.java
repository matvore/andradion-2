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

import java.awt.Color;
import java.awt.Cursor;
import java.awt.Graphics;
import java.awt.Point;
import java.awt.Toolkit;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.image.BufferedImage;
import java.util.EnumMap;
import java.util.List;
import java.util.Map;

import javax.swing.JPanel;

import com.github.matvore.andradion2.data.Copier;
import com.github.matvore.andradion2.data.Lists;

public class LevelEditPanel extends JPanel {
  private static final long serialVersionUID = 1L;

  private static final int GUIDE_RECTANGLE_WIDTH = 320;
  private static final int GUIDE_RECTANGLE_HEIGHT = 240;

  private final Images images;
  private BufferedImage levelImage;
  private Map<PlaceableItem, List<Point>> placeableItems;
  private Point guideRectangleLocation;
  private PlaceableItem placeableItem;

  private final MouseMotionListener mouseMotionListener = new MouseMotionListener() {
    @Override
    public void mouseDragged(MouseEvent event) {
      // Do nothing.
    }

    @Override
    public void mouseMoved(MouseEvent event) {
      // TODO Auto-generated method stub
      Graphics graphics = LevelEditPanel.this.getGraphics();
      graphics.setXORMode(Color.GRAY);
      if (null != guideRectangleLocation) {
        drawGuideRectangle(graphics);
      }
      guideRectangleLocation = event.getPoint();
      drawGuideRectangle(graphics);
      graphics.dispose();
    }
  };

  private final MouseListener mouseListener = new MouseListener() {
    @Override
    public void mouseClicked(MouseEvent event) {
      Point whereClicked = event.getPoint();
      List<Point> alter = placeableItems.get(placeableItem);
      switch (event.getButton()) {
      case MouseEvent.BUTTON1:
        // Add
        alter.add((Point)whereClicked.clone());
        while (alter.size() > placeableItem.getMaxOccurrences()) {
          alter.remove(0);
        }
        break;
      case MouseEvent.BUTTON3:
        // Delete
        if (alter.size() <= placeableItem.getMinOccurrences()) {
          return;
        }
        long closestItemDistance = Long.MAX_VALUE;
        int indexOfClosestItem = -1;
        for (int index = 0; index < alter.size(); index++) {
          Point thisItem = alter.get(index);
          long xDiff = thisItem.x - whereClicked.x;
          long yDiff = thisItem.y - whereClicked.y;
          long distance = xDiff * xDiff + yDiff * yDiff;
          if (closestItemDistance > distance) {
            closestItemDistance = distance;
            indexOfClosestItem = index;
          }
        }
        if (closestItemDistance < 32) {
          alter.remove(indexOfClosestItem);
        }
        break;
      }
      guideRectangleLocation = null;
      repaint();
    }

    @Override
    public void mouseEntered(MouseEvent event) {
      // Do nothing.
    }

    @Override
    public void mouseExited(MouseEvent event) {
      // Do nothing.
    }

    @Override
    public void mousePressed(MouseEvent event) {
      // Do nothing.
    }

    @Override
    public void mouseReleased(MouseEvent event) {
      // Do nothing.
    }
  };

  public LevelEditPanel(Images images) {
    this.images = images;
    this.addMouseMotionListener(mouseMotionListener);
    this.addMouseListener(mouseListener);
  }

  private static Map<PlaceableItem, List<Point>> copyPlaceableItems(
      Map<PlaceableItem, List<Point>> source) {
    Map<PlaceableItem, List<Point>> result =
        new EnumMap<PlaceableItem, List<Point>>(PlaceableItem.class);
    for (PlaceableItem item : PlaceableItem.values()) {
      if (source.containsKey(item)) {
        result.put(item,
            Lists.deepCopyOf(source.get(item), Copier.FOR_POINT));
      } else {
        result.put(item, Lists.<Point>newArrayList());
      }
    }
    return result;
  }

  public void setLevel(BufferedImage levelImage,
      Map<PlaceableItem, List<Point>> placeableItems) {
    this.levelImage = levelImage;
    setPreferredSize(
        new java.awt.Dimension(levelImage.getWidth(), levelImage.getHeight()));

    this.placeableItems = copyPlaceableItems(placeableItems);

    guideRectangleLocation = null;
    revalidate();
    repaint();
  }

  public Map<PlaceableItem, List<Point>> getLevel() {
    return copyPlaceableItems(placeableItems);
  }

  public void setPlaceableItem(PlaceableItem placeableItem) {
    Toolkit cursorMaker = Toolkit.getDefaultToolkit();
    BufferedImage itemImage = images.imageFor(placeableItem);
    Cursor cursor = cursorMaker.createCustomCursor(itemImage,
        new Point(itemImage.getWidth() / 2, itemImage.getHeight() / 2),
        placeableItem.toString());
    setCursor(cursor);
    this.placeableItem = placeableItem;
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

  private void drawGuideRectangle(Graphics graphics) {
    int x = guideRectangleLocation.x - GUIDE_RECTANGLE_WIDTH / 2;
    int y = guideRectangleLocation.y - GUIDE_RECTANGLE_HEIGHT / 2;
    graphics.drawRect(x, y, GUIDE_RECTANGLE_WIDTH, GUIDE_RECTANGLE_HEIGHT);
  }
}
