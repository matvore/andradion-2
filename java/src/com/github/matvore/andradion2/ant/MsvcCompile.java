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
import java.util.Map;

public class MsvcCompile extends Task {
  private File input, output;
  private List<IncludePath> includePaths = new ArrayList<IncludePath>();
  private PathConfiguration pathConfiguration = PathConfiguration.getInstance();

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
  public void addInclude(IncludePath includePath) {
    includePaths.add(includePath);
  }

  /**
   * Adds the given path to the {@code PATH} environment variable.
   */
  private void addPath(Map<String, String> environment, File newPath) {
    String pathKey = null, path = null;

    for (Map.Entry<String, String> entry : environment.entrySet()) {
      if (entry.getKey().toUpperCase().equals("PATH")) {
        path = entry.getValue();
        pathKey = entry.getKey();
        break;
      }
    }

    if (pathKey == null) {
      environment.put("PATH", newPath.toString());
    } else {
      path = newPath.toString() + File.pathSeparator + path;
      environment.put(pathKey, path);
    }
  }

  public void execute() throws BuildException {
    File msvcPath = pathConfiguration.getMsvcVcDirectory();
    msvcPath = new File(msvcPath, "bin" + File.separator + "CL.EXE");
    ProcessBuilder processBuilder = new ProcessBuilder();
    List<String> command = new ArrayList<String>();
    command.add(msvcPath.toString());
    for (IncludePath includePath : includePaths) {
      command.add("/I" + includePath.getPath());
    }
    command.add("/c"); // compile and don't link
    command.add("/Fo" + output.toString());

    File windowsHeadersDir = new File(
        pathConfiguration.getWindowsSdkDirectory(), "include");
    command.add("/I" + windowsHeadersDir);

    File msvcHeadersDir = new File(
        pathConfiguration.getMsvcVcDirectory(), "include");
    command.add("/I" + msvcHeadersDir);

    command.add(input.toString());

    // Add MSVC\Common7\IDE to PATH variable
    File common7ide = PathConfiguration.getInstance().getMsvcCommonDirectory();
    common7ide = new File(common7ide, "IDE");
    addPath(processBuilder.environment(), common7ide);

    ProcessResult result;
    try {
      Process msvc = processBuilder.command(command).start();
      result = ProcessResult.of(msvc);
    } catch (InterruptedException e) {
      throw new BuildException(
          "Thread interrupted when waiting for CL.exe.", e);
    } catch (IOException e) {
      throw new BuildException(
          "IOException when running CL.exe", e);
    }

    System.out.println(result.getStdout());
    if (result.getExitCode() != 0) {
      throw new BuildException(
          "CL.exe did not finish normally. Exit code: " + result.getExitCode());
    }
  }
}
