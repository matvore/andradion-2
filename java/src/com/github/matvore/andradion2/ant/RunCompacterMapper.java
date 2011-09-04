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

import com.github.matvore.andradion2.build.CompacterMapper;
import com.github.matvore.andradion2.data.Level;
import com.github.matvore.andradion2.data.LevelFormats;
import com.github.matvore.andradion2.data.Lists;

import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.Task;

import java.awt.Dimension;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.List;
import javax.imageio.ImageIO;

public class RunCompacterMapper extends Task {
  private File input, output;
  private File levelFile;
  private byte transparentColor;
  private int minimumColorBlockArea;
  private List<Size> patternSizes = Lists.newArrayList();
  private int minimumPatternArea;
  private int sectorWidth, sectorHeight;

  public final static class Size {
    private int width, height;

    public void setWidth(int width) {
      this.width = width;
    }

    public void setHeight(int height) {
      this.height = height;
    }

    public Dimension asDimension() {
      return new Dimension(width, height);
    }
  }

  /**
   * Sets the path of the level file from which to read the palette. The file
   * should be in plain text, not binary.
   */
  public void setLevelFile(File levelFile) {
    this.levelFile = levelFile;
  }

  /**
   * Sets the source image to compress.
   */
  public void setInput(File input) {
    this.input = input;
  }

  /**
   * Sets the path of the {@code .cmpset} file (CompacterMapper set) to
   * write to.
   */
  public void setOutput(File output) {
    this.output = output;
  }

  public void setTransparentColor(int transparentColor) {
    this.transparentColor = (byte)transparentColor;
  }

  public void addPatternSize(Size size) {
    patternSizes.add(size);
  }

  public void setMinimumColorBlockArea(int minimumColorBlockArea) {
    this.minimumColorBlockArea = minimumColorBlockArea;
  }
  
  public void setMinimumPatternArea(int minimumPatternArea) {
    this.minimumPatternArea = minimumPatternArea;
  }

  public void setSectorWidth(int sectorWidth) {
    this.sectorWidth = sectorWidth;
  }

  public void setSectorHeight(int sectorHeight) {
    this.sectorHeight = sectorHeight;
  }

  public void execute() throws BuildException {
    System.out.println("Compacting image at: " + input);

    List<Dimension> patternDimensions = Lists.newArrayList();
    for (Size patternSize : patternSizes) {
      patternDimensions.add(patternSize.asDimension());
    }
    CompacterMapper cmp = CompacterMapper.of(transparentColor,
        minimumColorBlockArea, patternDimensions, minimumPatternArea);
    BufferedImage masterImage = null;
    Level level;
    try {
      InputStream levelStream = new FileInputStream(levelFile);
      level = LevelFormats.fromText(levelStream);
      levelStream.close();
    } catch (IOException e) {
      throw new BuildException("Could not read level file: " + levelFile, e);
    }
    OutputStream cmpsetStream;

    try {
      cmpsetStream = new FileOutputStream(output);
    } catch (IOException e) {
      throw new BuildException("Could not write to cmpset file: " + output, e);
    }

    try {
      masterImage = ImageIO.read(input);
    } catch (IOException e) {
      throw new BuildException("Could not read image file: " + input, e);
    }
    try {
      for (int y = 0; y < masterImage.getHeight(); y += sectorHeight) {
        for (int x = 0; x < masterImage.getWidth(); x += sectorWidth) {
          int width = Math.min(sectorWidth, masterImage.getWidth() - x);
          int height = Math.min(sectorHeight, masterImage.getHeight() - y);
          BufferedImage imageSector =
              masterImage.getSubimage(x, y, width, height);
          cmp.compact(imageSector, level.getPalette(), cmpsetStream);
        }
      }

      cmpsetStream.close();
    } catch (IOException e) {
      throw new BuildException("Could not write cmp data to file.", e);
    }
  }
}
