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

import com.github.matvore.andradion2.data.Closeables;
import com.github.matvore.andradion2.data.Level;
import com.github.matvore.andradion2.data.LevelFormats;

import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.Task;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.File;
import java.io.InputStream;
import java.io.IOException;
import java.io.OutputStream;

public class LevelTextToBinary extends Task {
  private File input, output;

  public void setInput(File input) {
    this.input = input;
  }

  public void setOutput(File output) {
    this.output = output;
  }

  @Override
  public void execute() throws BuildException {
    InputStream inputStream = null;
    OutputStream outputStream = null;

    try {
      inputStream = new FileInputStream(input);
      outputStream = new FileOutputStream(output);

      Level level = LevelFormats.fromText(inputStream);
      LevelFormats.toBinary(level, outputStream);
    } catch (IOException e) {
      throw new BuildException(e);
    } finally {
      Closeables.closeQuietly(inputStream);
      Closeables.closeQuietly(outputStream);
    }
  }
}
