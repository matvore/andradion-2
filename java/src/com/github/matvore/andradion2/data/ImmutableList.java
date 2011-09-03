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

package com.github.matvore.andradion2.data;

import java.util.AbstractList;
import java.util.List;

public class ImmutableList {
  private static class MutablesListImpl<T> extends AbstractList<T> {
    private final Object[] elements;
    private final Copier<? super T> copier;

    public MutablesListImpl(Object[] elements, Copier<? super T> copier) {
      this.elements = elements;
      this.copier = copier;
    }

    @Override
    public int size() {
      return elements.length;
    }

    @Override
    @SuppressWarnings("unchecked")
    public T get(int index) {
      return copier.copy((T)elements[index]);
    }
  }

  private static class ListImpl<T> extends AbstractList<T> {
    private final Object[] elements;

    public ListImpl(Object[] elements) {
      this.elements = elements;
    }

    @Override
    public int size() {
      return elements.length;
    }

    @Override
    @SuppressWarnings("unchecked")
    public T get(int index) {
      return (T)elements[index];
    }
  }

  public static <T> List<T> copyOf(List<? extends T> original) {
    return new ListImpl<T>(original.toArray());
  }

  /**
   * Returns an immutable copy of the given {@code List}. This is designed
   * to work on lists of mutable objects. The generated list will indeed be
   * immutable as long as the given {@code copier} is correct. The copier
   * is used to make defensive copies of elements for returning rather than
   * returning the internal reference to an object.
   */
  public static <T> List<T> copyOf(
      List<? extends T> original, Copier<? super T> copier) {
    Object[] asArray = new Object[original.size()];
    int index = 0;
    for (T element : original) {
      if (element == null) {
        throw new NullPointerException("Null reference at index " + index);
      }
      asArray[index] = copier.copy(element);
      index++;
    }
    return new MutablesListImpl<T>(asArray, copier);
  }
}
