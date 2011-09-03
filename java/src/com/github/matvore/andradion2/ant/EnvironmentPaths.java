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

package com.github.matvore.andradion2.ant;

import java.io.File;
import java.util.Comparator;
import java.util.Map;

public class EnvironmentPaths {
  private static final EnvironmentPaths INSTANCE = new EnvironmentPaths();

  private final Comparator<? super String> stringComparator =
      String.CASE_INSENSITIVE_ORDER;

  private EnvironmentPaths() {
    // Nothing to do.
  }

  public static EnvironmentPaths getInstance() {
    return INSTANCE;
  }

  /**
   * Adds the given path to some environment variable.
   */
  public void add(
      String variableName, Map<String, String> environment, File newPath) {
    String existingVariableName = null, path = null;

    for (Map.Entry<String, String> entry : environment.entrySet()) {
      if (stringComparator.compare(entry.getKey(), variableName) == 0) {
        path = entry.getValue();
        existingVariableName = entry.getKey();
        break;
      }
    }

    if (existingVariableName == null) {
      environment.put(variableName, newPath.toString());
    } else {
      path = newPath.toString() + File.pathSeparator + path;
      environment.put(existingVariableName, path);
    }
  }
}
