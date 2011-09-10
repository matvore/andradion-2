package com.github.matvore.andradion2.data;

import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;

public class TextNumberOutputStream implements NumberOutputStream {
  private final Writer writer;

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

  @Override
  public void putByte(int x) throws IOException {
    writer.write((x & 0xff) + " ");
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
    writer.write((x & 0xffff) + " ");
  }

  @Override
  public void putDword(int x) throws IOException {
    if (x < 0) {
      throw new IllegalArgumentException(
          "Unsupported value (negative or too large): " + x);
    }
    writer.write(x + " ");
  }

  @Override
  public void putWordThatIsUsuallyByte(int word) throws IOException {
    putWord(word);
  }
}
