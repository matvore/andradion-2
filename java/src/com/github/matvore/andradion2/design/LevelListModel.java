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

import com.github.matvore.andradion2.data.ImmutableList;
import com.github.matvore.andradion2.data.LevelIndex;

import java.util.Arrays;
import java.util.List;
import javax.swing.AbstractListModel;

public class LevelListModel extends AbstractListModel {
  private static final long serialVersionUID = 1L;

  // A list of all the levels LevelMaker can edit.
  private static final List<LevelIndex> LEVELS
      = ImmutableList.copyOf(Arrays.asList(
          LevelIndex.SCHOOL_ENTRANCE,
          LevelIndex.SCHOOL_SOUTHWEST,
          LevelIndex.SCHOOL_SOUTHEAST,
          LevelIndex.SCHOOL_NORTHWEST,
          LevelIndex.SCHOOL_NORTHEAST,
          LevelIndex.SWITZERS_ROOM,
          LevelIndex.GIBBS_ROOM,
          LevelIndex.TACO_TRUCK,
          LevelIndex.UFO,
          LevelIndex.MEXICAN_BORDER,
          LevelIndex.HAWAII));
  private static final LevelListModel INSTANCE = new LevelListModel();

  private LevelListModel() {
    fireIntervalAdded(this, 0, LEVELS.size() - 1);
  }

  public static LevelListModel getInstance() {
    return INSTANCE;
  }

  @Override
  public LevelIndex getElementAt(int index) {
    return LEVELS.get(index);
  }

  @Override
  public int getSize() {
    return LEVELS.size();
  }
}
