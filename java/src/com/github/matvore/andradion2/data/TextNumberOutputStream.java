package com.github.matvore.andradion2.data;

import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;

public class TextNumberOutputStream implements NumberOutputStream {
  private final Writer writer;
  private boolean lineIsEmpty = true;

  private static final String LINE_SEPARATOR =
      System.getProperty("line.separator");

  public static NumberOutputStream of(OutputStream output) {
    return new TextNumberOutputStream(new OutputStreamWriter(output));
  }

  private TextNumberOutputStream(Writer writer) {
    this.writer = writer;
  }

  @Override
  public void close() throws IOException {
    writer.close();
  }

  private void putFormattedNumber(String number) throws IOException {
    if (!lineIsEmpty) {
      writer.write(' ');
    }
    writer.write(number);
    lineIsEmpty = false;
  }

  @Override
  public void putByte(int x) throws IOException {
    putFormattedNumber(Integer.toString(x & 0xff));
  }

  @Override
  public void putBytes(byte[] x) throws IOException {
    for (byte b : x) {
      putByte(b);
    }
  }

  @Override
  public void putBytes(byte[] x, int offset, int length) throws IOException {
    for (int i = offset; i < offset + length; i++) {
      putByte(x[i]);
    }
  }

  @Override
  public void putWord(int x) throws IOException {
    putFormattedNumber(Integer.toString(x & 0xffff));
  }

  @Override
  public void putDword(int x) throws IOException {
    long longX = (long)x & 0xffffffffL;
    putFormattedNumber(Long.toString(longX));
  }

  @Override
  public void putWordThatIsUsuallyByte(int word) throws IOException {
    putWord(word);
  }

  @Override
  public void flush() throws IOException {
    writer.flush();
  }

  @Override
  public void endStructure() throws IOException {
    writer.write(LINE_SEPARATOR);
    lineIsEmpty = true;
  }

  @Override
  public void endSection() throws IOException {
    writer.write(LINE_SEPARATOR);
    lineIsEmpty = true;
  }
}
