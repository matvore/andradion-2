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

public enum WeatherPattern {
  PLAIN,
  FULL_CYCLE,
  DAY_TO_STORM_TO_NIGHT,
  LIGHT_RAIN_DAY,

  HEAVY_RAIN,
  LIGHTNING_STORM,
  LIGHT_RAIN_NIGHT,
  LIGHT_RAIN_NIGHT_AND_CRICKETS,

  NIGHT_CRICKETS,
  DAY_TO_STORM,
  NIGHT_TO_DAY,
  RANDOMLY_CHANGES;
}
