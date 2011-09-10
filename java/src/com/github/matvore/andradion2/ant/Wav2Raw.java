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

import com.github.matvore.andradion2.data.BinaryNumberOutputStream;
import com.github.matvore.andradion2.data.Closeables;
import com.github.matvore.andradion2.data.Lists;
import com.github.matvore.andradion2.data.NumberOutputStream;

import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.Task;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.List;

import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.UnsupportedAudioFileException;

/**
 * An Ant task to convert WAV audio files to raw sample data. This is used for
 * the Andradion 2 build process, so this task combines many WAV files into
 * a single raw data file. Before each stripped WAV file is a single 32-bit
 * little endian value indicating the number of bytes in the WAV sample data.
 */
public class Wav2Raw extends Task {
  private File output;
  private final List<InputFile> inputs = Lists.newArrayList();

  public static final class InputFile {
    private File path;

    public void setPath(File path) {
      this.path = path;
    }

    public File getPath() {
      return path;
    }
  }

  public void setOutput(File output) {
    this.output = output;
  }

  public void addInput(InputFile input) {
    this.inputs.add(input);
  }

  @Override
  public void execute() throws BuildException {
    NumberOutputStream destination = null;
    try {
      destination = BinaryNumberOutputStream.of(new FileOutputStream(output));
      byte[] buffer = new byte[1024];

      for (InputFile input : inputs) {
        AudioInputStream inputStream = null;
        try {
          inputStream = AudioSystem.getAudioInputStream(input.getPath());
          // Write size of raw sample data first, in bytes
          int rawSampleSize = inputStream.getFormat().getSampleSizeInBits() / 8
              * (int)inputStream.getFrameLength();
          destination.putDword(rawSampleSize);
          // Transfer sample data
          int bytesRead;
          while ((bytesRead = inputStream.read(buffer)) != -1) {
            destination.putBytes(buffer, 0, bytesRead);
          }
        } finally {
          Closeables.closeQuietly(inputStream);
        }
      }
    } catch (UnsupportedAudioFileException e) {
      throw new BuildException("JVM does not support this audio file.", e);
    } catch (IOException e) {
      throw new BuildException("IOException when stripping audio file.", e);
    } finally {
      Closeables.closeQuietly(destination);
    }
  }
}
