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

import java.io.InputStream;

public class ProcessResult {
  private final String stdout;
  private final int exitCode;

  private ProcessResult(String stdout, int exitCode) {
    this.stdout = stdout;
    this.exitCode = exitCode;

    if (null == stdout) {
      throw new NullPointerException("stdout");
    }
  }

  public String getStdout() {
    return stdout;
  }

  public int getExitCode() {
    return exitCode;
  }

  public static ProcessResult of(Process process) {
    InputStream processOutput = process.getInputStream();
    StringBuilder stdoutBuilder = new StringBuilder();

    int outputChar;
    while ((outputChar = processOutput.read()) != -1) {
      stdoutBuilder.append((char)outputChar);
    }
    processOutput.close();

    int exitCode = process.waitFor();
    String fullOutput = stdoutBuilder.toString();
    return new ProcessResult(fullOutput, exitCode);
  }
}
