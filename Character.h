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

class CCharacter;
class GfxLock;

struct TCharacterPointer {
  TCharacterPointer(CCharacter *p) : ch(p) {}
  CCharacter *ch;
};

class CCharacter {
  friend bool operator >(const TCharacterPointer&,
			 const TCharacterPointer&);
  friend bool operator <(const TCharacterPointer&,
			 const TCharacterPointer&);

 public:
  // this calculate sector function figures out what sector the
  //  character should be in and sets its members accordingly, setting
  //  the row and col parameters to match
  void CalculateSector(int& row,int& col);
  bool IsOffScreen() const;

  /**
   * Performs a single frame of logic as if this
   * character were a computer-controlled entity.
   * @param target probably the hero: the character that the enemy
   *  wants to kill.
   * @return true iff the enemy is not off the side of the screen.
   */
  bool Logic(GfxLock& lock, const CCharacter& target);
	
  void Logic(GfxLock& lock);
  
  // used for locals (enemies or hero):
  void Setup(FIXEDNUM x, FIXEDNUM y, int model_, bool doing_mp);

  // used for remotes
  void Setup(unsigned int model_);
  
  bool DrawCharacter(); // returns true if character has moved
  void DrawMeters(int show_health);
  void SubtractHealth(int fire_type);
  void TryToFire();
  void CheckForEnemyCollision(const CCharacter&);

  inline void GetLocation(FIXEDNUM& x, FIXEDNUM& y) const {
    x = coor.first.x;
    y = coor.first.y - Fixed(TILE_HEIGHT/2);
  }
  
  inline FIXEDNUM X() const {return coor.first.x;}
  inline FIXEDNUM Y() const {return coor.first.y - Fixed(TILE_HEIGHT/2);}
  inline int Model() const {return model;}
  inline bool Dead() const {return CHARSTATE_DEAD == state
			      || CHARSTATE_DYING == state;}

  inline void GetSector(int& row,int& col) const {
    row = sector_row;
    col = sector_col;
  }

  inline bool HasFullHealth() const {return Fixed(1) == health;}
  
  inline bool HasFullAmmo(int weapon_type) const {
    return Fixed(1) == ammo[weapon_type];
  }
  
  inline FIXEDNUM Health() const {return health;}

  inline int Direction() const {return direction;}
  inline void SetDirection(int d) {direction = d;}

  inline void SetWeapon(int w) {current_weapon = w;}

  inline void SetPosition(FIXEDNUM x, FIXEDNUM y) {
    coor.first.x = x;
    coor.first.y = y + Fixed(TILE_HEIGHT/2);
    coor.second = coor.first;
  }

  void Walk(bool running);

  inline void Hurt() {
    state = CHARSTATE_HURT;
    frames_in_this_state = 0;
  }
  
  void TryToMove(GfxLock& lock);
  
 private:
  /** Resets ammo to original count (some pistol ammo, no
   * machine gun or bazooka).
   */
  void ResetAmmo();
  
  void PlaySound();
  void PowerUpCollisions();

  // these members are utilized for
  //  all kinds of characters
  int current_weapon, model, direction, state, sector_row, sector_col;
  std::pair<POINT, POINT> coor;
  bool reset_gamma; // used by draw function

  DWORD frames_since_last_fire;
  DWORD frames_in_this_state;
  DWORD frames_not_having_sworn;

  FIXEDNUM ammo[3];
  FIXEDNUM health;
};

// these less-than, greater-than operators help with sorting in the
//  drawing order
inline bool operator >(const TCharacterPointer& l,
		       const TCharacterPointer& r) {
  return l.ch->coor.first.y > r.ch->coor.first.y || r.ch->Dead();
}

inline bool operator <(const TCharacterPointer& l,
		       const TCharacterPointer& r) {
  return l.ch->coor.first.y < r.ch->coor.first.y || l.ch->Dead();
}

extern CCharacter hero;

/** Remote players or local aliens. */
extern std::vector<CCharacter> enemies;
