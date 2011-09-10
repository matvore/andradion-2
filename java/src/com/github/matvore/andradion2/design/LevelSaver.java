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

import java.io.IOException;

import javax.swing.JList;

import com.github.matvore.andradion2.data.LevelIndex;

public class LevelSaver {
  private final Levels levels;
  private final JList levelList;
  private final LevelEditPanel levelEditPanel;

  public LevelSaver(
      Levels levels, JList levelList, LevelEditPanel levelEditPanel) {
    this.levels = levels;
    this.levelList = levelList;
    this.levelEditPanel = levelEditPanel;
  }

  public void save() throws IOException {
    int currentLevelInt = levelList.getSelectedIndex();
    LevelIndex currentLevel =
        LevelListModel.getInstance().getElementAt(currentLevelInt);
    levels.saveLevelItems(currentLevel, levelEditPanel.getLevel());
  }
}
