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
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Capable of reading the configured value of installation paths in the Windows
 * registry.
 */
public class PathConfiguration {
  private static final PathConfiguration INSTANCE = new PathConfiguration();

  private File msvcCompiler;

  private PathConfiguration() {
    // Nothing to do.
  }
  
  public static PathConfiguration getInstance() {
    return INSTANCE;
  }

  /**
   * Returns the path of the Microsoft Visual C++ compiler and linker, or
   * {@code cl.exe}.
   */
  public synchronized File getMsvcCompiler() {
    if (null != msvcCompiler) {
      return msvcCompiler;
    }

    Process regQueryProcess = Runtime.getRuntime().exec(
        "REG QUERY HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\" +
        "9.0\\Setup\\VC /v ProductDir");
    ProcessResult regQuery = ProcessResult.of(regQueryProcess);

    if (regQuery.getExitCode() != 0) { 
      throw new RegQueryProcessFailedException("Non-zero exit code " +
          regQuery.getExitCode() + ": " + regQuery.getStdout());
    }

    Pattern pathCapture = Pattern.compile(
        "\\s+ProductDir\\s+REG_SZ\\s+([^\\r\\n]+)[\\r\\n]");
    Matcher matcher = pathCapture.matcher(regQuery.getStdout());
    if (!matcher.find()) {
      throw new CouldNotParseRegQueryOutputException(
          "REG QUERY output: " + regQuery.getStdout());
    }

    msvcCompiler = new File(matcher.group(1));
    return msvcCompiler;
  }
}
