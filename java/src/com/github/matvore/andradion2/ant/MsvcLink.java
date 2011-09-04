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
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class MsvcLink extends Task {
  private boolean detectDxSdkLibDir = false;
  private boolean detectMsvcLibDir = true;
  private boolean detectWindowsSdkLibDir = true;
  private List<Arg> args = new ArrayList<Arg>();
  private List<StandardLib> standardLibs = new ArrayList<StandardLib>();

  private final EnvironmentPaths environmentPaths =
      EnvironmentPaths.getInstance();
  private final PathConfiguration pathConfiguration =
      PathConfiguration.getInstance();

  public static class StandardLib {
    private String name;

    public void setName(String name) {
      this.name = name;
    }

    public String getName() {
      return name;
    }
  }

  /**
   * Set to {@code true} to automatically detect and use the Windows SDK
   * library directory. For example,
   * {@code C:\Program Files\Microsoft SDKs\Windows\v7.0\Lib}.
   * The default is {@code true}.
   */
  public void setDetectWindowsSdkLibDir(boolean detectWindowsSdkLibDir) {
    this.detectWindowsSdkLibDir = detectWindowsSdkLibDir;
  }

  /**
   * Set to {@code true} to automatically detect and use the MSVC library
   * directory. For example,
   * {@code C:\Program Files\Microsoft Visual Studio 9.0\VC\Lib}.
   * The default is {@code true}.
   */
  public void setDetectMsvcLibDir(boolean detectMsvcLibDir) {
    this.detectMsvcLibDir = detectMsvcLibDir;
  }

  /**
   * Set to {@code true} to automatically detect and use the MS DirectX
   * library directory. For example,
   * {@code C:\Program Files\Microsoft DirectX SDK (June 2010)\Lib\x86}.
   * The default is {@code false}.
   */
  public void setDetectDxSdkLibDir(boolean detectDxSdkLibDir) {
    this.detectDxSdkLibDir = detectDxSdkLibDir;
  }

  public void addStandardLib(StandardLib standardLib) {
    standardLibs.add(standardLib);
  }

  public void addArg(Arg arg) {
    args.add(arg);
  }

  @Override
  public void execute() throws BuildException {
    File linkerPath = pathConfiguration.getMsvcVcDirectory();
    linkerPath = new File(linkerPath, "bin" + File.separator + "LINK.exe");
    ProcessBuilder processBuilder = new ProcessBuilder();
    List<String> command = new ArrayList<String>();
    command.add(linkerPath.toString());
    for (Arg arg : args) {
      command.add(arg.toString());
    }

    for (StandardLib standardLib : standardLibs) {
      command.add(standardLib.getName());
    }

    if (detectDxSdkLibDir) {
      String envVariable = System.getenv("DXSDK_DIR");
      if (envVariable == null) {
        throw new BuildException("Could not find the DirectX SDK. Make sure " +
            "the DXSDK_DIR environment variable is set.");
      }
      File libPath = new File(envVariable, "Lib" + File.separator + "x86");
      command.add("/LIBPATH:" + libPath);
    }

    if (detectWindowsSdkLibDir) {
      File libPath = new File(
          pathConfiguration.getWindowsSdkDirectory(), "Lib");
      command.add("/LIBPATH:" + libPath);
    }

    if (detectMsvcLibDir) {
      File libPath = new File(
          pathConfiguration.getMsvcVcDirectory(), "Lib");
      command.add("/LIBPATH:" + libPath);
    }

    // Add MSVC\Common7\IDE to PATH variable
    File common7ide = pathConfiguration.getMsvcCommonDirectory();
    common7ide = new File(common7ide, "IDE");
    environmentPaths.add("PATH", processBuilder.environment(), common7ide);

    System.out.println("command line:");
    for (String argument : command) {
      System.out.print("  ");
      System.out.println(argument);
    }

    ProcessResult result;
    try {
      Process linker = processBuilder.command(command).start();
      result = ProcessResult.of(linker);
    } catch (InterruptedException e) {
      throw new BuildException(
          "Thread interrupted when waiting for LINK.exe.", e);
    } catch (IOException e) {
      throw new BuildException(
          "IOException when running LINK.exe", e);
    }

    System.out.println(result.getStdout());
    if (result.getExitCode() != 0) {
      throw new BuildException(
          "LINK.exe did not finish normally. Exit code: " +
          result.getExitCode());
    }
  }
}
