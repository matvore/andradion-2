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

class Controller {
protected:
  class Button {
  public:
    /** -1 for keyboard; otherwise joy stick index. */
    char device;

    /** Either 0, 1, or 2 - Indicates the number of buttons that have valid
     * values. */
    char progress;

    union {
      /** for values >= 0, a button index; otherwise, invert the value for an
       * axis index. */
      char button_1;

      /** When device is -1, indicates the key corresponding with this button.
       */
      unsigned char key_index_1;
    };

    // for some actions, a pair of buttons/keys is needed, in which case this
    //  union is interpreted in the same manner as the union above
    union {
      char button_2;
      unsigned char key_index_2;
    };

    bool IsAxis() {return button_1 < 0;}

    int GetAxisValue();
    bool GetButtonValue(bool sub_action = false);
  };

  Button action_buttons[PERPLAYER_ACTIONS];

public:
  enum {STRAFE, FIRE, NEXTWEAPON, PICKUP, PISTOL, MACHINEGUN, BAZOOKA,
        SHOWHEALTH, MOVELEFTRIGHT, MOVEUPDOWN, PERPLAYER_ACTIONS};

  enum {RESTARTLEVEL, ENDGAME, PAUSE, SHOWBESTTIME, SHOWBESTSCORE,
        SLOWGAMESPEED, NORMALGAMESPEED, FASTGAMESPEED,
        GLOBAL_ACTIONS};

  Controller();
  ~Controller();

  FIXEDNUM MovementSpeed();
  int MovementDirection();

  bool ButtonDown(int action);
  bool WaitForButton(int action);
  std::string ButtonName(int action);
  static std::string ActionName(int action);

//   int OtherOptions();
//   std::string OtherOptionName(int index);
//   int OtherOptionsPossibilities(int index);
//   std::string OtherOptionPossibilityName(int index, int jindex);
//   void SetOtherOption(int index, int jindex);
//   int GetOtherOption(int index);

  bool WriteConfiguration(HANDLE file);
  bool ReadConfiguration(HANDLE file);
  static bool WriteGlobalConfiguration(HANDLE file);
  static bool ReadGlobalConfiguration(HANDLE file);

  static bool GlobalButtonDown(int action);
  static int GlobalWaitForButton(int action);
  static std::string GlobalButtonName(int action);
  static std::string GlobalActionName(int action);

  static void RefreshState();
};
