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

const int MAX_FIRES = 10; // maximum number of bullets at a time

class Fire {
public:
  static Fire *UnusedSlot();

  bool OkayToDelete() const;

  // pass zero for the last parameter when the gun was fired by a
  // remote player
  void Setup(FIXEDNUM sx_, FIXEDNUM sy_,
             int direction_, int type_,
             bool remotely_generated_);

  // use for bazookas triggered by remote systems
  void Setup(FIXEDNUM sx_, FIXEDNUM sy_,
             FIXEDNUM tx_, FIXEDNUM ty_);

  void Logic();
  void Draw();
  static void AnalyzePalette();

  Fire();

private:
  void PlaySound();
  static BYTE bullet_trail_color;
  void Collides(std::vector<int> *dest);
  FIXEDNUM x, y;
  FIXEDNUM sx, sy; // start location
  int direction, type, state, horizontal_collision_flags;

  DWORD frames_since_explosion_started;

  bool remotely_generated;

  // when a bazooka is remotely generated, the start and stop
  //  coordinates have already been determined by a remote computer
};

extern Fire fires[MAX_FIRES];
