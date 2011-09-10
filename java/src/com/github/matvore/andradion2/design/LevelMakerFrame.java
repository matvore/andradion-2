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

import java.awt.BorderLayout;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.IOException;

import javax.swing.JFrame;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;

public class LevelMakerFrame {
  private static class OnExitSaveListener extends WindowAdapter {
    private final LevelSaver levelSaver;

    public OnExitSaveListener(LevelSaver levelSaver) {
      this.levelSaver = levelSaver;
    }

    @Override
    public void windowClosing(WindowEvent event) {
      try {
        levelSaver.save();
      } catch (IOException e) {
        throw new RuntimeException("Could not save level changes.", e);
      }
    }
  }

  public static JFrame create(Levels levels, Images images) {
    JFrame result = new JFrame();
    JList levelList = new JList(LevelListModel.getInstance());
    JList placeableList = new JList(PlaceableItemListModel.getInstance());
    JPanel rightHandPanel = new JPanel();
    rightHandPanel.setLayout(new BorderLayout());
    rightHandPanel.add(levelList, BorderLayout.NORTH);
    rightHandPanel.add(placeableList, BorderLayout.SOUTH);
    LevelEditPanel levelEditPanel = new LevelEditPanel(images);
    LevelSaver levelSaver = new LevelSaver(levels, levelList, levelEditPanel);
    levelList.addListSelectionListener(new LevelListSelectionListener(
        levels, images, levelEditPanel, levelSaver));
    levelList.setSelectedIndex(0);
    placeableList.addListSelectionListener(
        new PlaceableItemListSelectionListener(levelEditPanel));
    placeableList.setSelectedIndex(0);
    JScrollPane levelEditScrollPane = new JScrollPane(levelEditPanel);
    result.add(rightHandPanel, BorderLayout.EAST);
    result.add(levelEditScrollPane, BorderLayout.CENTER);
    result.addWindowListener(new OnExitSaveListener(levelSaver));
    return result;
  }
}
