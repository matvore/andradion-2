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

public enum LevelIndex {
  SCHOOL_ENTRANCE("1_"),
  SCHOOL_SOUTHWEST("2a"),
  SCHOOL_SOUTHEAST("2b"),
  SCHOOL_NORTHWEST("3a"),
  SCHOOL_NORTHEAST("3b"),
  SWITZERS_ROOM("4_"),
  GIBBS_ROOM("5_"),
  TACO_TRUCK("6a"),
  UFO("6b"),
  MEXICAN_BORDER("7a"),
  HAWAII("7b"),
  TIJUANA("8a"),
  HAWAII_ENDING("e1"),
  TIJUANA_ENDING("e2");

  private final String id;

  private LevelIndex(String id) {
    this.id = id;
  }

  public String getId() {
    return id;
  }
}
