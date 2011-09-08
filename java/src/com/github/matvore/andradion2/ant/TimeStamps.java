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
import java.io.FileNotFoundException;
import java.io.IOException;

public class TimeStamps {
  public static long latestModified(Iterable<File> files) throws IOException {
    long lastModified = Long.MIN_VALUE;
    for (File file : files) {
      if (!file.exists()) {
        throw new FileNotFoundException(file.toString());
      }
      long thisFileLastModified = file.lastModified();
      if (thisFileLastModified == 0) {
        throw new IOException(
            "Could not read last modified timestamp of file: " + file);
      }
      lastModified = Math.max(lastModified, thisFileLastModified);
    }

    if (lastModified == Long.MIN_VALUE) {
      throw new IllegalArgumentException("files argument is empty.");
    }

    return lastModified;
  }
}
