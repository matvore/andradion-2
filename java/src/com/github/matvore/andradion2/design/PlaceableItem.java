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

import com.github.matvore.andradion2.data.Entity;

public enum PlaceableItem {
  SALLY(Entity.SALLY, 0, Integer.MAX_VALUE),
  MILTON(Entity.MILTON, 0, Integer.MAX_VALUE),
  EVIL_TURNER(Entity.EVIL_TURNER, 0, Integer.MAX_VALUE),
  PISTOL(Entity.PISTOL, 0, Integer.MAX_VALUE),
  MACHINE_GUN(Entity.MACHINE_GUN, 0, Integer.MAX_VALUE),
  BAZOOKA(Entity.BAZOOKA, 0, Integer.MAX_VALUE),
  HEALTH(Entity.HEALTH, 0, Integer.MAX_VALUE),
  HERO(null, 1, 1);

  private final Entity entity;
  private final int minOccurrences, maxOccurrences;

  private PlaceableItem(
      Entity entity, int minOccurrences, int maxOccurrences) {
    this.entity = entity;
    this.minOccurrences = minOccurrences;
    this.maxOccurrences = maxOccurrences;
  }

  /**
   * Returns the entity that corresponds to this item. The Hero is not
   * considered an entity, so will return {@code null} for this method.
   */
  public Entity getEntity() {
    return entity;
  }

  public int getMinOccurrences() {
    return minOccurrences;
  }

  public int getMaxOccurrences() {
    return maxOccurrences;
  }
}
