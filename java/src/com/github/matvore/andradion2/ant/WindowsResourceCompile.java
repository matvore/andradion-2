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

import com.github.matvore.andradion2.data.Lists;

import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.Task;

import java.io.File;
import java.io.IOException;
import java.util.List;

/**
 * A task to execute the Windows resource compiler of the Windows Platform SDK.
 */
public class WindowsResourceCompile extends Task {
  private List<Arg> args = Lists.newArrayList();
  private boolean detectWindowsSdkHeaderDir = true;
  private boolean detectMsvcHeaderDir = true;
  private File rcFile;

  private final PathConfiguration pathConfiguration =
      PathConfiguration.getInstance();

  /**
   * Adds an additional argument that will be passed to {@code RC.exe}.
   */
  public void addArg(Arg arg) {
    this.args.add(arg);
  }

  /**
   * Sets the RC file to compile.
   */
  public void setRcFile(File rcFile) {
    this.rcFile = rcFile;
  }

  /**
   * Set to {@code true} to automatically detect and use the Windows SDK
   * header directory. For example,
   * {@code C:\Program Files\Microsoft SDKs\Windows\v7.0\Include}.
   * The default is {@code true}.
   */
  public void setDetectWindowsSdkHeaderDir(
      boolean detectWindowsSdkHeaderDir) {
    this.detectWindowsSdkHeaderDir = detectWindowsSdkHeaderDir;
  }

  /**
   * Set to {@code true} to automatically detect and use the MSVC header
   * directory. For example,
   * {@code C:\Program Files\Microsoft Visual Studio 9.0\VC\Include}.
   * The default is {@code true}.
   */
  public void setDetectMsvcHeaderDir(boolean detectMsvcHeaderDir) {
    this.detectMsvcHeaderDir = detectMsvcHeaderDir;
  }

  @Override
  public void execute() throws BuildException {
    File rcPath = pathConfiguration.getWindowsSdkDirectory();
    rcPath = new File(rcPath, "Bin" + File.separator + "RC.exe");
    ProcessBuilder processBuilder = new ProcessBuilder();
    List<String> command = Lists.newArrayList();
    command.add(rcPath.toString());
    for (Arg arg : args) {
      command.add(arg.toString());
    }

    if (detectWindowsSdkHeaderDir) {
      File windowsHeadersDir = new File(
         pathConfiguration.getWindowsSdkDirectory(), "include");
      command.add("/i" + windowsHeadersDir.toString());
    }

    if (detectMsvcHeaderDir) {
      File msvcHeadersDir = new File(
          pathConfiguration.getMsvcVcDirectory(), "include");
      command.add("/i" + msvcHeadersDir);
    }

    command.add(rcFile.toString());

    System.out.println("command line:");
    for (String argument : command) {
      System.out.print("  ");
      System.out.println(argument);
    }

    ProcessResult result;
    try {
      Process msvc = processBuilder.command(command).start();
      result = ProcessResult.of(msvc);
    } catch (InterruptedException e) {
      throw new BuildException(
          "Thread interrupted when waiting for RC.exe.", e);
    } catch (IOException e) {
      throw new BuildException(
          "IOException when running RC.exe", e);
    }

    System.out.println(result.getStdout());
    if (result.getExitCode() != 0) {
      throw new BuildException(
          "RC.exe did not finish normally. Exit code: " + result.getExitCode());
    }
  }
}
