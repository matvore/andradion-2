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

import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.Task;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class MsvcCompile extends Task {
  private File input, output;
  private List<IncludePath> includePaths = new ArrayList<IncludePath>();

  public static class IncludePath {
    private File path;
    
    public void setPath(File path) {
      this.path = path;
    }

    public File getPath() {
      return path;
    }
  }

  /**
   * Sets the path and name of the output file, for example {@code out\Foo.obj}.
   */
  public void setOutput(File output) {
    this.output = output;
  }

  /**
   * Sets the path and name of the input file, for example {@code Foo.cpp}.
   */
  public void setInput(File input) {
    this.input = input;
  }

  /**
   * Adds a path to include in the search path for {@code #include} directives.
   */
  public void addIncludePath(IncludePath includePath) {
    includePaths.add(includePath);
  }

  public void execute() throws BuildException {
    
  }
}
