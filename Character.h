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

class CPowerUp;

enum {CHARSTATE_ALIVE,
      CHARSTATE_WALKING,
      CHARSTATE_HURT,
      CHARSTATE_DYING,
      CHARSTATE_DEAD,
      CHARSTATE_UNKNOWING,
      CHARSTATE_FIRING,
      CHARSTATE_UNINIT = -1};

class Character {
public:
  class Ptr {
  protected:
    int index;

  public:
    Ptr() : index(0) {}
    Ptr(int index) : index(index) {}

    int Index() const {return index;}

    inline Character *operator ->() const {return &all[index];}
    inline Character &operator *() const {return all[index];}

    // these less-than, greater-than operators help with sorting in the
    //  drawing order
    inline bool operator >(const Ptr& rhs) const {
      return all[index].coor.first.y > rhs->coor.first.y || rhs->Dead();
    }

    inline bool operator <(const Ptr& rhs) const {
      return all[index].coor.first.y < rhs->coor.first.y || all[index].Dead();
    }

    inline bool operator ==(const Ptr& rhs) const {
      return all[index].coor.first.y == rhs->coor.first.y;
    }
  };

protected:
  void PlaySound();

  // these members are utilized for
  //  all kinds of characters
  int current_weapon, model, direction, state;
  std::pair<POINT, POINT> coor;
  bool reset_gamma; // used by draw function

  DWORD frames_since_last_fire;
  DWORD frames_in_this_state;
  DWORD frames_not_having_sworn;

  FIXEDNUM health;

  /** Indicates whether this Character is controlled by a human, either at this
   * computer or at some other computer over the network. */
  bool controlled_by_human;

  /** Draws the health and ammo meters on the back buffer as if we were using a
   * GfxBasic object for graphics.
   */
  void DrawMetersBasic
  (bool show_health, FIXEDNUM virtual_health, FIXEDNUM virtual_ammo);

  /** Draws the health and ammo meters on the front buffer as if it were using a
   * GfxPretty object for graphics.
  */
  void DrawMetersPretty(FIXEDNUM virtual_health, FIXEDNUM virtual_ammo);

  static BYTE border_color;
  static BYTE ammo_color;
  static BYTE ammo_color_dim;
  static BYTE health_color;
  static BYTE health_color_dim;

  static std::vector<Character> all;

public:
  static void AnalyzePalette();

  inline static Ptr Get(int i) {return Ptr(i);}
  inline static int Count() {return all.size();}
  static Ptr Add() {int i = all.size(); all.resize(i+1); return Ptr(i);}
  static void Clear(bool keep_first = false) {
    all.resize((keep_first && Count()) ? 1 : 0);
  }

  void PowerUpCollisions(std::vector<CPowerUp> *pups);

  bool IsOffScreen() const;

  /**
   * Performs a single frame of logic as if this
   * character were a computer-controlled entity.
   * @return true iff the enemy is not off the side of the screen.
   */
  bool EnemyLogic();

  void Logic();

  // used for locals (enemies or hero):
  void Setup(FIXEDNUM x, FIXEDNUM y, int model_, bool doing_mp,
             bool controlled_by_human_);

  // used for remotes
  void Setup(unsigned int model_);

  /** Draws the character to the screen.
   * @param cxt the viewing context to draw to.
   * @return true if the character has moved.
   */
  bool DrawCharacter();
  void DrawMeters(bool show_health);
  void SubtractHealth(int fire_type);
  void TryToFire();
  void CheckForEnemyCollision(const Character&);

  inline void GetLocation(FIXEDNUM& x, FIXEDNUM& y) const {
    x = coor.first.x;
    y = coor.first.y - Fixed(TILE_HEIGHT/2);
  }

  inline FIXEDNUM X() const {return coor.first.x;}
  inline FIXEDNUM Y() const {return coor.first.y - Fixed(TILE_HEIGHT/2);}
  inline int Model() const {return model;}
  inline bool Dead() const {
    return CHARSTATE_DEAD == state || CHARSTATE_DYING == state;
  }

  inline bool HasFullHealth() const {return Fixed(1) == health;}

  inline FIXEDNUM Health() const {return health;}

  inline int Direction() const {return direction;}
  inline void SetDirection(int d) {direction = d;}

  inline void SetWeapon(int w) {current_weapon = w;}

  inline bool ControlledByHuman() const {return controlled_by_human;}

  inline void SetPosition(FIXEDNUM x, FIXEDNUM y) {
    coor.first.x = x;
    coor.first.y = y + Fixed(TILE_HEIGHT/2);
    coor.second = coor.first;
  }

  void Walk(bool running);

  inline void Hurt() {state = CHARSTATE_HURT; frames_in_this_state = 0;}

  void TryToMove();
};
