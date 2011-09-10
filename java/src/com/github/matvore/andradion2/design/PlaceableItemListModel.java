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

import java.util.Arrays;
import java.util.List;
import javax.swing.AbstractListModel;

public class PlaceableItemListModel extends AbstractListModel {
  private static final long serialVersionUID = 1L;

  private static final List<PlaceableItem> ITEMS =
      ImmutableList.copyOf(Arrays.asList(PlaceableItem.values()));
  private static final PlaceableItemListModel INSTANCE =
      new PlaceableItemListModel();

  private PlaceableItemListModel() {
    fireIntervalAdded(this, 0, ITEMS.size() - 1);
  }

  public static PlaceableItemListModel getInstance() {
    return INSTANCE;
  }

  @Override
  public PlaceableItem getElementAt(int index) {
    return ITEMS.get(index);
  }

  @Override
  public int getSize() {
    return ITEMS.size();
  }
}

