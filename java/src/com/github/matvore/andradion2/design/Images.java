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

import com.github.matvore.andradion2.data.LevelIndex;

import java.awt.Image;
import java.io.File;
import java.io.IOException;
import java.util.Collections;
import java.util.EnumMap;
import java.util.Map;
import javax.imageio.ImageIO;

/**
 * Loads images required for editing levels.
 */
public class Images {
  private final File levelImageDirectory;
  private final Map<PlaceableItem, Image> itemImages;

  private static final Map<PlaceableItem, File> ITEM_IMAGE_FILES;

  static {
    Map<PlaceableItem, File> itemImageFiles =
        new EnumMap<PlaceableItem, File>(PlaceableItem.class);
    itemImageFiles.put(PlaceableItem.SALLY, new File("1sallys.bmp"));
    itemImageFiles.put(PlaceableItem.MILTON, new File("1miltons.bmp"));
    itemImageFiles.put(PlaceableItem.EVIL_TURNER, new File("1evilturners.bmp"));
    itemImageFiles.put(PlaceableItem.PISTOL, new File("pistole.bmp"));
    itemImageFiles.put(PlaceableItem.MACHINE_GUN, new File("mge.bmp"));
    itemImageFiles.put(PlaceableItem.BAZOOKA, new File("bazookae.bmp"));
    itemImageFiles.put(PlaceableItem.HEALTH, new File("health.bmp"));
    ITEM_IMAGE_FILES = Collections.unmodifiableMap(itemImageFiles);
  }

  private Images(
      File levelImageDirectory, Map<PlaceableItem, Image> itemImages) {
    this.levelImageDirectory = levelImageDirectory;
    this.itemImages = itemImages;
  }

  public static Images withDirectories(
      File itemImageDirectory, File levelImageDirectory) throws IOException {
    Map<PlaceableItem, Image> itemImages =
        new EnumMap<PlaceableItem, Image>(PlaceableItem.class);
    for (PlaceableItem item : ITEM_IMAGE_FILES.keySet()) {
      File imagePath = new File(
          itemImageDirectory, ITEM_IMAGE_FILES.get(item).toString());
      itemImages.put(item, ImageIO.read(imagePath));
    }

    return new Images(levelImageDirectory, itemImages);
  }

  public Image levelLowerImage(LevelIndex level) throws IOException {
    String imageFile = level.getId() + "_.png";
    File imagePath = new File(levelImageDirectory, imageFile);

    return ImageIO.read(imagePath);
  }
}
