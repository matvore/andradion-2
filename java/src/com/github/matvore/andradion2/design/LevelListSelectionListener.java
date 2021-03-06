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

import java.io.IOException;
import javax.swing.JList;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;

public class LevelListSelectionListener implements ListSelectionListener {
  private final Levels levels;
  private final Images images;
  private final LevelEditPanel levelEditPanel;
  private final LevelSaver levelSaver;

  public LevelListSelectionListener(Levels levels, Images images,
      LevelEditPanel levelEditPanel, LevelSaver levelSaver) {
    this.levels = levels;
    this.images = images;
    this.levelEditPanel = levelEditPanel;
    this.levelSaver = levelSaver;
  }

  @Override
  public void valueChanged(ListSelectionEvent event) {
    if (event.getValueIsAdjusting()) {
      return;
    }

    JList levelList = (JList)event.getSource();
    LevelIndex newLevel = LevelListModel.getInstance().getElementAt(
        levelList.getSelectedIndex());
    try {
      levelEditPanel.setLevel(
          images.levelLowerImage(newLevel), levels.loadLevelItems(newLevel));
    } catch (IOException e) {
      throw new RuntimeException("Could not load level image.", e);
    }
    try {
      levelSaver.save();
    } catch (IOException e) {
      throw new RuntimeException("Could not save level.", e);
    }
  }
}
